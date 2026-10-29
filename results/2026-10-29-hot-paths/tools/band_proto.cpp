// SCRATCH (B1 prototype, never committed): banded replay of one Gallery frame.
//
// For each case (page, scale, damage) it measures, on one host:
// - direct: Window::paint into a SkiaRaster, as the paint workload does;
// - record: Window::paint into a RecordingPainter (the serial traversal and
//   chunk replay a banded frame still pays before rasterizing);
// - replay1: the recorded frame replayed into the SkiaRaster on the caller;
// - banded T x B: the recorded frame replayed in B horizontal bands of device
//   rows on T pool workers (gui_forms_atomic_pool begin/wait). Each worker
//   owns its own SkiaRaster (own fonts, text engine, glyph caches) and its
//   own copy of the commands with fresh retained caches, so no gui_forms
//   state is shared; only stx_skia's process-wide state (SkResourceCache,
//   once-initialised tables) is.
// Pixels: the direct frame's hash, and the hash of the direct frame with
// every band's damaged rows replaced by the rows its worker drew.
//
// Usage: band_proto <font-dir> cases
//        band_proto <font-dir> hold <threads> <bands> <seconds>   (pages at scale 2, for sampling)

#include "gallery.hpp"
#include "gallery_page_specs.hpp"
#include "gui_forms/gui_forms.hpp"
#include "skia_raster.hpp"
#include "../src/core/damage/device_damage/device_damage.hpp"
#include "../src/core/display/recording_painter/recording_painter.hpp"
#include "../src/core/display/replay/replay_display_chunk.hpp"
#include "../src/core/threading/atomic_pool/atomic_pool.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <unistd.h>

namespace {

using gui_forms::DamageRegion;
using gui_forms::FontRole;
using gui_forms::Rect;
using gui_forms::Size;
using gui_forms::Window;
using gui_forms::render::SkiaRaster;
using gui_forms::detail::DisplayChunk;
using gui_forms::detail::DisplayCommand;
using Clock = std::chrono::steady_clock;

struct BundledFace final {
    const char* file;
    FontRole role;
    std::uint16_t weight;
    bool italic;
    bool fallback;
};

constexpr std::array<BundledFace, 17> bundled_faces{{
    {"PortsmouthRapids.ttf", FontRole::control, 400, false, false},
    {"PortsmouthRapids-Bold.ttf", FontRole::control, 700, false, false},
    {"Carlito-Regular.ttf", FontRole::content, 400, false, false},
    {"Carlito-Bold.ttf", FontRole::content, 700, false, false},
    {"Carlito-Italic.ttf", FontRole::content, 400, true, false},
    {"Carlito-BoldItalic.ttf", FontRole::content, 700, true, false},
    {"Cousine-Regular.ttf", FontRole::monospace, 400, false, false},
    {"Cousine-Bold.ttf", FontRole::monospace, 700, false, false},
    {"Cousine-Italic.ttf", FontRole::monospace, 400, true, false},
    {"Cousine-BoldItalic.ttf", FontRole::monospace, 700, true, false},
    {"NotoSansCJKjp-Regular.otf", FontRole::content, 400, false, true},
    {"NotoSansArabic-Regular.ttf", FontRole::content, 400, false, true},
    {"NotoSansHebrew-Regular.ttf", FontRole::content, 400, false, true},
    {"NotoSansDevanagari-Regular.ttf", FontRole::content, 400, false, true},
    {"NotoSansBengali-Regular.ttf", FontRole::content, 400, false, true},
    {"NotoSansGurmukhi-Regular.ttf", FontRole::content, 400, false, true},
    {"NotoEmoji-Regular.ttf", FontRole::content, 400, false, true},
}};

std::filesystem::path font_directory;

void register_faces(SkiaRaster& raster) {
    for (const BundledFace& face : bundled_faces) {
        const std::string path = (font_directory / face.file).string();
        const bool accepted = face.fallback
            ? raster.register_fallback_typeface_file(face.weight, face.italic, path.c_str())
            : raster.register_typeface_file(face.role, face.weight, face.italic, path.c_str());
        if (!accepted) throw std::runtime_error("cannot load " + path);
    }
}

std::uint64_t fnv(const std::byte* bytes, std::size_t size) {
    std::uint64_t hash = 14695981039346656037ULL;
    for (std::size_t index = 0; index + 8 <= size; index += 8) {
        std::uint64_t word = 0;
        std::memcpy(&word, bytes + index, 8);
        hash ^= word;
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::uint64_t nanoseconds(Clock::duration elapsed) {
    return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count());
}

std::uint64_t median(std::vector<std::uint64_t> samples) {
    std::sort(samples.begin(), samples.end());
    return samples.empty() ? 0 : samples[samples.size() / 2U];
}

// One worker's private replay state.
struct Lane final {
    std::unique_ptr<SkiaRaster> raster;
    std::shared_ptr<const DisplayChunk> chunk;
    std::uint64_t busy_ns = 0;
};

struct Band final {
    int first_row = 0;
    int end_row = 0;
    DamageRegion region;
    int lane = -1;
};

struct Frame final {
    std::vector<Lane>* lanes = nullptr;
    std::vector<Band>* bands = nullptr;
    std::atomic<int> next_lane{0};
    int generation = 0;
};

struct BandTask final {
    Frame* frame = nullptr;
    std::size_t band = 0;
};

thread_local int lane_generation = -1;
thread_local int lane_index = -1;

void paint_band(void* argument) {
    BandTask& task = *static_cast<BandTask*>(argument);
    Frame& frame = *task.frame;
    if (lane_generation != frame.generation) {
        lane_generation = frame.generation;
        lane_index = frame.next_lane.fetch_add(1);
    }
    Lane& lane = (*frame.lanes)[static_cast<std::size_t>(lane_index)];
    Band& band = (*frame.bands)[task.band];
    band.lane = lane_index;
    const Clock::time_point started = Clock::now();
    (*lane.raster).begin_frame(band.region);
    static_cast<void>(gui_forms::detail::replay_display_chunk(*lane.chunk, *lane.raster));
    (*lane.raster).end_frame();
    lane.busy_ns += nanoseconds(Clock::now() - started);
}

// The recorded commands with fresh retained caches, so a lane's memos are its own.
std::shared_ptr<const DisplayChunk> private_copy(const DisplayChunk& chunk) {
    std::vector<DisplayCommand> commands = chunk.commands();
    for (DisplayCommand& command : commands) {
        if (command.retained_cache) command.retained_cache = std::make_shared<gui_forms::RetainedDrawCache>();
    }
    return std::make_shared<const DisplayChunk>(chunk.generation(), chunk.plane(), chunk.logical_bounds(),
                                                std::move(commands));
}

void paint_direct(SkiaRaster& raster, Window& window, Rect aligned) {
    DamageRegion region;
    region.add(aligned);
    raster.begin_frame(region);
    static_cast<void>(window.paint(raster, aligned));
    raster.end_frame();
}

std::shared_ptr<const DisplayChunk> record(SkiaRaster& metrics, Window& window, Rect aligned) {
    gui_forms::detail::RecordingPainter recorder(&metrics);
    static_cast<void>(window.paint(recorder, aligned));
    return recorder.finish(1U, gui_forms::PaintPlane::control, aligned);
}

void prepare(SkiaRaster& raster, Window& window, Size size, double scale) {
    // resize answers false when the size is unchanged.
    static_cast<void>(raster.resize(size, scale));
    if (raster.pixel_width() != static_cast<std::uint32_t>(std::ceil(size.width * scale)) ||
        !raster.synchronize_images(window.image_resources()))
        throw std::runtime_error("raster resize failed");
}

struct Config final {
    std::uint32_t threads;
    std::uint32_t bands;
};

constexpr std::array<Config, 7> configs{{{1, 1}, {2, 2}, {4, 4}, {4, 8}, {6, 12}, {8, 8}, {8, 16}}};
constexpr std::size_t warmup = 4;
constexpr std::size_t measured = 24;

struct Pools final {
    std::vector<gui_forms_atomic_pool_t*> pools;
    std::vector<std::vector<Lane>> lanes;
    ~Pools() {
        for (gui_forms_atomic_pool_t* pool : pools) gui_forms_atomic_pool_destroy(pool);
    }
};

int generation_counter = 0;

// Bands of device rows over the aligned damage, each with its damage rectangle.
std::vector<Band> make_bands(Rect aligned, double scale, std::uint32_t count) {
    const int first = static_cast<int>(std::floor(aligned.y * scale));
    const int end = static_cast<int>(std::ceil((aligned.y + aligned.height) * scale));
    std::vector<Band> bands(count);
    for (std::uint32_t index = 0; index < count; ++index) {
        Band& band = bands[index];
        band.first_row = first + static_cast<int>((static_cast<long long>(end - first) * index) / count);
        band.end_row = first + static_cast<int>((static_cast<long long>(end - first) * (index + 1U)) / count);
        const Rect rows{aligned.x, band.first_row / scale, aligned.width,
                        (band.end_row - band.first_row) / scale};
        band.region.add(rows);
    }
    return bands;
}

// Runs one banded frame; returns its wall time.
std::uint64_t banded_frame(gui_forms_atomic_pool_t* pool, std::vector<Lane>& lanes, std::vector<Band>& bands) {
    Frame frame;
    frame.lanes = &lanes;
    frame.bands = &bands;
    frame.generation = ++generation_counter;
    std::vector<BandTask> tasks(bands.size());
    std::vector<gui_forms_atomic_pool_task_t> entries(bands.size());
    for (std::size_t index = 0; index < bands.size(); ++index) {
        tasks[index] = {&frame, index};
        entries[index] = {paint_band, &tasks[index]};
    }
    const Clock::time_point started = Clock::now();
    if (gui_forms_atomic_pool_begin_checked(pool, entries.data(), static_cast<std::uint32_t>(entries.size())) != THREADPOOL_OK ||
        gui_forms_atomic_pool_wait_checked(pool) != THREADPOOL_OK)
        throw std::runtime_error("pool failed");
    return nanoseconds(Clock::now() - started);
}

std::uint64_t assembled_hash(const SkiaRaster& reference, const std::vector<Lane>& lanes, const std::vector<Band>& bands) {
    const std::size_t row_bytes = reference.row_bytes();
    std::vector<std::byte> pixels(reference.byte_size());
    std::memcpy(pixels.data(), reference.pixels(), pixels.size());
    for (const Band& band : bands) {
        const SkiaRaster& source = *lanes[static_cast<std::size_t>(band.lane)].raster;
        const std::byte* from = static_cast<const std::byte*>(source.pixels());
        for (int row = band.first_row; row < band.end_row; ++row) {
            if (row < 0 || static_cast<std::uint32_t>(row) >= reference.pixel_height()) continue;
            std::memcpy(pixels.data() + static_cast<std::size_t>(row) * row_bytes,
                        from + static_cast<std::size_t>(row) * row_bytes, row_bytes);
        }
    }
    return fnv(pixels.data(), pixels.size());
}


// Row ranges where the assembled banded frame differs from the reference, with
// column extents and the largest channel difference, next to the band edges.
void report_difference(const SkiaRaster& reference, const std::vector<Lane>& lanes, const std::vector<Band>& bands) {
    const std::size_t row_bytes = reference.row_bytes();
    const std::uint32_t width = reference.pixel_width();
    const std::uint8_t* base = static_cast<const std::uint8_t*>(reference.pixels());
    std::fprintf(stderr, "bands:");
    for (const Band& band : bands) std::fprintf(stderr, " [%d,%d)@%d", band.first_row, band.end_row, band.lane);
    std::fprintf(stderr, "\n");
    int open_first = -1;
    int open_last = -1;
    std::uint32_t left = width;
    std::uint32_t right = 0;
    int largest = 0;
    int printed = 0;
    for (const Band& band : bands) {
        const std::uint8_t* drawn = static_cast<const std::uint8_t*>(lanes[static_cast<std::size_t>(band.lane)].raster->pixels());
        for (int row = band.first_row; row < band.end_row; ++row) {
            const std::uint8_t* a = base + static_cast<std::size_t>(row) * row_bytes;
            const std::uint8_t* b = drawn + static_cast<std::size_t>(row) * row_bytes;
            bool differs = false;
            for (std::uint32_t x = 0; x < width; ++x) {
                for (int channel = 0; channel < 4; ++channel) {
                    const int delta = std::abs(int(a[x * 4 + channel]) - int(b[x * 4 + channel]));
                    if (delta != 0) {
                        differs = true;
                        left = std::min(left, x);
                        right = std::max(right, x);
                        largest = std::max(largest, delta);
                    }
                }
            }
            if (differs) {
                if (open_first >= 0 && row == open_last + 1) open_last = row;
                else {
                    if (open_first >= 0 && printed < 40) {
                        std::fprintf(stderr, "  rows %d-%d cols %u-%u max %d\n", open_first, open_last, left, right, largest);
                        ++printed;
                    }
                    open_first = open_last = row;
                    left = width; right = 0; largest = 0;
                    // re-scan this row's extent
                    for (std::uint32_t x = 0; x < width; ++x)
                        for (int channel = 0; channel < 4; ++channel) {
                            const int delta = std::abs(int(a[x * 4 + channel]) - int(b[x * 4 + channel]));
                            if (delta != 0) { left = std::min(left, x); right = std::max(right, x); largest = std::max(largest, delta); }
                        }
                }
            }
        }
    }
    if (open_first >= 0) std::fprintf(stderr, "  rows %d-%d cols %u-%u max %d\n", open_first, open_last, left, right, largest);
}

void run_case(Pools& pools, SkiaRaster& raster, Window& window, const std::string& page, double scale,
              Rect damage, double percent) {
    const Rect aligned = gui_forms::detail::align_damage_outward(damage, scale);
    std::vector<std::uint64_t> samples;
    std::size_t frame = 0;
    for (frame = 0; frame < warmup; ++frame) paint_direct(raster, window, aligned);
    for (frame = 0; frame < measured; ++frame) {
        const Clock::time_point started = Clock::now();
        paint_direct(raster, window, aligned);
        samples.push_back(nanoseconds(Clock::now() - started));
    }
    const std::uint64_t direct = median(samples);
    const std::uint64_t direct_hash = fnv(static_cast<const std::byte*>(raster.pixels()), raster.byte_size());
    samples.clear();
    std::shared_ptr<const DisplayChunk> chunk;
    for (frame = 0; frame < warmup + measured; ++frame) {
        const Clock::time_point started = Clock::now();
        chunk = record(raster, window, aligned);
        if (frame >= warmup) samples.push_back(nanoseconds(Clock::now() - started));
    }
    const std::uint64_t recording = median(samples);
    samples.clear();
    DamageRegion region;
    region.add(aligned);
    for (frame = 0; frame < warmup + measured; ++frame) {
        const Clock::time_point started = Clock::now();
        raster.begin_frame(region);
        static_cast<void>(gui_forms::detail::replay_display_chunk(*chunk, raster));
        raster.end_frame();
        if (frame >= warmup) samples.push_back(nanoseconds(Clock::now() - started));
    }
    const std::uint64_t replay = median(samples);
    const std::uint64_t replay_hash = fnv(static_cast<const std::byte*>(raster.pixels()), raster.byte_size());
    std::printf("{\"page\":\"%s\",\"scale\":%g,\"damage\":%g,\"commands\":%zu,\"direct_ns\":%llu,\"record_ns\":%llu,"
                "\"replay1_ns\":%llu,\"replay_identical\":%s,\"banded\":[",
                page.c_str(), scale, percent, (*chunk).commands().size(), static_cast<unsigned long long>(direct),
                static_cast<unsigned long long>(recording), static_cast<unsigned long long>(replay),
                replay_hash == direct_hash ? "true" : "false");
    for (std::size_t config = 0; config < configs.size(); ++config) {
        std::vector<Lane>& lanes = pools.lanes[config];
        for (Lane& lane : lanes) {
            prepare(*lane.raster, window, window.client_size(), scale);
            lane.chunk = private_copy(*chunk);
            lane.busy_ns = 0;
        }
        std::vector<Band> bands = make_bands(aligned, scale, configs[config].bands);
        samples.clear();
        std::uint64_t busy_total = 0;
        for (frame = 0; frame < warmup + measured; ++frame) {
            if (frame == warmup) for (Lane& lane : lanes) lane.busy_ns = 0;
            const std::uint64_t wall = banded_frame(pools.pools[config], lanes, bands);
            if (frame >= warmup) samples.push_back(wall);
        }
        for (const Lane& lane : lanes) busy_total += lane.busy_ns;
        const bool identical = assembled_hash(raster, lanes, bands) == direct_hash;
        if (!identical && std::getenv("BAND_DIFF") != nullptr) {
            std::fprintf(stderr, "DIFF %s scale %g T%u B%u\n", page.c_str(), scale, configs[config].threads, configs[config].bands);
            report_difference(raster, lanes, bands);
        }
        std::printf("%s{\"threads\":%u,\"bands\":%u,\"wall_ns\":%llu,\"busy_per_frame_ns\":%llu,\"identical\":%s}",
                    config == 0 ? "" : ",", configs[config].threads, configs[config].bands,
                    static_cast<unsigned long long>(median(samples)),
                    static_cast<unsigned long long>(busy_total / measured), identical ? "true" : "false");
    }
    std::printf("]}\n");
    std::fflush(stdout);
}

void make_pools(Pools& pools) {
    for (const Config& config : configs) {
        gui_forms_atomic_pool_t* pool = nullptr;
        if (gui_forms_atomic_pool_create_checked(config.threads, &pool) != THREADPOOL_OK)
            throw std::runtime_error("pool create failed");
        pools.pools.push_back(pool);
        std::vector<Lane> lanes(config.threads);
        for (Lane& lane : lanes) {
            lane.raster = std::make_unique<SkiaRaster>();
            register_faces(*lane.raster);
        }
        pools.lanes.push_back(std::move(lanes));
    }
}

void run_cases(Window& window, SkiaRaster& raster) {
    Pools pools;
    make_pools(pools);
    const Size damage_size{900.0, 620.0};
    const std::string first_page(gui_forms::gallery::selected_gallery_page(window));
    for (const double scale : {1.0, 2.0, 3.0}) {
        window.resize(damage_size);
        window.set_scale(scale);
        window.perform_layout();
        prepare(raster, window, damage_size, scale);
        static_cast<void>(window.take_damage());
        paint_direct(raster, window, {0.0, 0.0, damage_size.width, damage_size.height});
        for (const double percent : {10.0, 100.0}) {
            run_case(pools, raster, window, first_page, scale,
                     {0.0, 0.0, damage_size.width, damage_size.height * percent / 100.0}, percent);
        }
    }
    const Size size = gui_forms::gallery::gallery_default_size;
    for (const double scale : {1.0, 2.0}) {
        window.resize(size);
        window.set_scale(scale);
        window.perform_layout();
        prepare(raster, window, size, scale);
        for (const gui_forms::gallery::GalleryPageSpec& spec : gui_forms::gallery::gallery_pages) {
            if (!gui_forms::gallery::select_gallery_page(window, spec.id)) throw std::runtime_error("page");
            window.perform_layout();
            static_cast<void>(window.take_damage());
            run_case(pools, raster, window, std::string(spec.id), scale, {0.0, 0.0, size.width, size.height}, 100.0);
        }
    }
}


std::size_t differing_pixels(const SkiaRaster& reference, const std::vector<Lane>& lanes, const std::vector<Band>& bands) {
    const std::size_t row_bytes = reference.row_bytes();
    const std::uint32_t width = reference.pixel_width();
    const std::uint32_t* base = static_cast<const std::uint32_t*>(reference.pixels());
    std::size_t count = 0;
    for (std::size_t index = 0; index < bands.size(); ++index) {
        const std::uint32_t* drawn = static_cast<const std::uint32_t*>(lanes[index].raster->pixels());
        for (int row = bands[index].first_row; row < bands[index].end_row; ++row)
            for (std::uint32_t x = 0; x < width; ++x) {
                const std::size_t at = static_cast<std::size_t>(row) * (row_bytes / 4U) + x;
                if (base[at] != drawn[at]) ++count;
            }
    }
    return count;
}

std::shared_ptr<const DisplayChunk> prefix(const DisplayChunk& chunk, std::size_t count) {
    std::vector<DisplayCommand> commands(chunk.commands().begin(), chunk.commands().begin() + static_cast<std::ptrdiff_t>(count));
    return std::make_shared<const DisplayChunk>(chunk.generation(), chunk.plane(), chunk.logical_bounds(), std::move(commands));
}

// Serial, no threads: replays every prefix of the recorded frame into the
// reference and into one raster per band, and prints each command after which
// the number of differing pixels changes.
void bisect(Window& window, SkiaRaster& raster, const std::string& page, double scale, std::uint32_t band_count) {
    const Size size = gui_forms::gallery::gallery_default_size;
    window.resize(size);
    window.set_scale(scale);
    window.perform_layout();
    prepare(raster, window, size, scale);
    if (!gui_forms::gallery::select_gallery_page(window, page)) throw std::runtime_error("page");
    window.perform_layout();
    static_cast<void>(window.take_damage());
    const Rect aligned = gui_forms::detail::align_damage_outward({0.0, 0.0, size.width, size.height}, scale);
    paint_direct(raster, window, aligned);
    const std::shared_ptr<const DisplayChunk> chunk = record(raster, window, aligned);
    std::vector<Band> bands = make_bands(aligned, scale, band_count);
    std::vector<Lane> lanes(band_count);
    for (std::size_t index = 0; index < lanes.size(); ++index) {
        lanes[index].raster = std::make_unique<SkiaRaster>();
        register_faces(*lanes[index].raster);
        prepare(*lanes[index].raster, window, size, scale);
        lanes[index].chunk = private_copy(*chunk);
        bands[index].lane = static_cast<int>(index);
    }
    const std::shared_ptr<const DisplayChunk> reference_chunk = private_copy(*chunk);
    DamageRegion whole;
    whole.add(aligned);
    std::size_t previous = 0;
    const std::vector<DisplayCommand>& commands = (*chunk).commands();
    std::fprintf(stderr, "bisect %s scale %g bands %u commands %zu\n", page.c_str(), scale, band_count, commands.size());
    for (std::size_t count = 1; count <= commands.size(); ++count) {
        raster.begin_frame(whole);
        static_cast<void>(gui_forms::detail::replay_display_chunk(*prefix(*reference_chunk, count), raster));
        raster.end_frame();
        for (std::size_t index = 0; index < lanes.size(); ++index) {
            (*lanes[index].raster).begin_frame(bands[index].region);
            static_cast<void>(gui_forms::detail::replay_display_chunk(*prefix(*lanes[index].chunk, count), *lanes[index].raster));
            (*lanes[index].raster).end_frame();
        }
        const std::size_t now = differing_pixels(raster, lanes, bands);
        if (now != previous) {
            const DisplayCommand& command = commands[count - 1U];
            std::fprintf(stderr, "  after #%zu op %d rect %.3f,%.3f %.3fx%.3f scalars %.3f %.3f %.3f text '%.24s' : %zu differing\n",
                         count - 1U, static_cast<int>(command.operation), command.rect.x, command.rect.y,
                         command.rect.width, command.rect.height, command.scalar, command.secondary_scalar,
                         command.tertiary_scalar, command.text.c_str(), now);
            previous = now;
        }
    }
}


// Today's partial repaint: a full paint, then the same frame repainted with
// damage rows [0, row) only. Counts the pixels the partial repaint changes.
void seam(Window& window, SkiaRaster& raster, const std::string& page, double scale, double row) {
    const Size size = gui_forms::gallery::gallery_default_size;
    window.resize(size);
    window.set_scale(scale);
    window.perform_layout();
    prepare(raster, window, size, scale);
    if (!gui_forms::gallery::select_gallery_page(window, page)) throw std::runtime_error("page");
    window.perform_layout();
    static_cast<void>(window.take_damage());
    const Rect whole = gui_forms::detail::align_damage_outward({0.0, 0.0, size.width, size.height}, scale);
    paint_direct(raster, window, whole);
    paint_direct(raster, window, whole);
    std::vector<std::uint32_t> full(raster.byte_size() / 4U);
    std::memcpy(full.data(), raster.pixels(), raster.byte_size());
    const Rect part = gui_forms::detail::align_damage_outward({0.0, 0.0, size.width, row / scale}, scale);
    paint_direct(raster, window, part);
    const std::uint32_t* now = static_cast<const std::uint32_t*>(raster.pixels());
    std::size_t changed = 0;
    std::size_t first = full.size();
    std::size_t last = 0;
    for (std::size_t index = 0; index < full.size(); ++index) {
        if (full[index] != now[index]) {
            ++changed;
            first = std::min(first, index);
            last = std::max(last, index);
        }
    }
    const std::size_t stride = raster.row_bytes() / 4U;
    std::fprintf(stderr, "seam %s scale %g damage rows [0,%g): %zu pixels changed", page.c_str(), scale, row, changed);
    if (changed != 0) std::fprintf(stderr, ", rows %zu-%zu", first / stride, last / stride);
    std::fprintf(stderr, "\n");
}

// Loops every page at scale 2, banded, for `seconds`, so a sampler can attribute waits.
void hold(Window& window, SkiaRaster& raster, std::uint32_t threads, std::uint32_t bands, double seconds) {
    gui_forms_atomic_pool_t* pool = nullptr;
    if (gui_forms_atomic_pool_create_checked(threads, &pool) != THREADPOOL_OK) throw std::runtime_error("pool");
    std::vector<Lane> lanes(threads);
    for (Lane& lane : lanes) {
        lane.raster = std::make_unique<SkiaRaster>();
        register_faces(*lane.raster);
    }
    const Size size = gui_forms::gallery::gallery_default_size;
    const double scale = 2.0;
    window.resize(size);
    window.set_scale(scale);
    window.perform_layout();
    prepare(raster, window, size, scale);
    const Rect aligned = gui_forms::detail::align_damage_outward({0.0, 0.0, size.width, size.height}, scale);
    std::vector<std::shared_ptr<const DisplayChunk>> chunks;
    for (const gui_forms::gallery::GalleryPageSpec& spec : gui_forms::gallery::gallery_pages) {
        static_cast<void>(gui_forms::gallery::select_gallery_page(window, spec.id));
        window.perform_layout();
        static_cast<void>(window.take_damage());
        chunks.push_back(record(raster, window, aligned));
    }
    for (Lane& lane : lanes) prepare(*lane.raster, window, size, scale);
    std::vector<Band> band_list = make_bands(aligned, scale, bands);
    std::vector<std::vector<std::shared_ptr<const DisplayChunk>>> copies(threads);
    for (std::uint32_t lane = 0; lane < threads; ++lane)
        for (const std::shared_ptr<const DisplayChunk>& chunk : chunks) copies[lane].push_back(private_copy(*chunk));
    std::fprintf(stderr, "hold ready pid %d\n", static_cast<int>(getpid()));
    const Clock::time_point end = Clock::now() + std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(seconds));
    std::uint64_t frames = 0;
    std::uint64_t wall = 0;
    while (Clock::now() < end) {
        for (std::size_t page = 0; page < chunks.size(); ++page) {
            for (std::uint32_t lane = 0; lane < threads; ++lane) lanes[lane].chunk = copies[lane][page];
            wall += banded_frame(pool, lanes, band_list);
            ++frames;
        }
    }
    std::printf("{\"hold\":true,\"threads\":%u,\"bands\":%u,\"frames\":%llu,\"mean_frame_ns\":%llu}\n", threads, bands,
                static_cast<unsigned long long>(frames), static_cast<unsigned long long>(wall / std::max<std::uint64_t>(frames, 1)));
    gui_forms_atomic_pool_destroy(pool);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) return 2;
    try {
        font_directory = argv[1];
        const std::unique_ptr<Window> gallery = gui_forms::gallery::make_gallery();
        SkiaRaster raster;
        register_faces(raster);
        const std::string mode(argv[2]);
        if (mode == "cases") run_cases(*gallery, raster);
        else if (mode == "hold" && argc == 6)
            hold(*gallery, raster, static_cast<std::uint32_t>(std::stoul(argv[3])),
                 static_cast<std::uint32_t>(std::stoul(argv[4])), std::stod(argv[5]));
        else if (mode == "bisect" && argc == 6)
            bisect(*gallery, raster, argv[3], std::stod(argv[4]), static_cast<std::uint32_t>(std::stoul(argv[5])));
        else if (mode == "seam" && argc == 6)
            seam(*gallery, raster, argv[3], std::stod(argv[4]), std::stod(argv[5]));
        else return 2;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "band_proto failed: " << error.what() << '\n';
        return 1;
    }
}
