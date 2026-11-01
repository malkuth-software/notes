// Body study: the demo's painted body with the glint kernel, three ways.
//
//   reference  paint_glints, one thread (as ported from glint_kernel.js)
//   retained   paint_glints_retained, one thread
//   threaded   paint_glints_retained, one band of rows per thread
//
// The body is demo.html's: 1000 × 620 device pixels at scale 2; ГОСТ 397
// (#4d676d) below the 78-pixel title strip with film 0.55 and specular 1.6,
// the strip in #656f74 with film 0.85; light at (0.2, 0.05) of the body and
// 900 high, eye at its centre and 700 high. Each way is timed (median of five,
// wall clock) and its bytes compared with the reference's.
//
// Usage: body_study [threads] [<out.rgba>: the threaded body as RGBA rows]
#include "../surface_kernels.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

namespace {

namespace is = industrial_surfaces;

constexpr int body_width = 1000;
constexpr int body_height = 620;
constexpr int strip_height = 78;

is::GlintParameters body_parameters(bool strip) {
    is::GlintParameters p = is::glint_preset(is::GlintPreset::painted_sand);
    p.scale = 2.0;
    p.seed = 11;
    p.ambient = 0.35;
    p.film = strip ? 0.85 : 0.55;
    p.specular = 1.6;
    p.light_x = 0.2 * body_width / p.scale;
    p.light_y = 0.05 * body_height / p.scale;
    p.light_z = 900.0;
    p.eye_x = 0.5 * body_width / p.scale;
    p.eye_y = 0.5 * body_height / p.scale;
    p.eye_z = 700.0;
    p.paint = strip ? is::Rgb8{0x65, 0x6f, 0x74} : is::Rgb8{0x4d, 0x67, 0x6d};
    return p;
}

enum class Way : std::uint8_t {
    reference,
    retained,
};

struct Band final {
    std::vector<std::byte>* pixels{};
    int first_row{};
    int end_row{};
    Way way{Way::reference};
};

// Paints the body's rows [first_row, end_row): the strip's and the panel's
// parts of that band.
void paint_band(Band band) {
    const is::FramebufferView view{(*band.pixels).data(), static_cast<std::size_t>(body_width) * 4U, body_width, body_height};
    const is::PixelRect surface{0, 0, body_width, body_height};
    const is::GlintParameters panel = body_parameters(false);
    const is::GlintParameters strip = body_parameters(true);
    is::GlintScratch scratch{};
    is::reserve_glint_scratch(scratch, panel);
    const std::array<is::PixelRect, 2> parts = {is::PixelRect{0, strip_height, body_width, body_height - strip_height},
                                                is::PixelRect{0, 0, body_width, strip_height}};
    for (std::size_t i = 0; i < parts.size(); ++i) {
        const is::PixelRect part = parts[i];
        const int y0 = std::max(part.y, band.first_row);
        const int y1 = std::min(part.y + part.height, band.end_row);
        if (y1 <= y0) {
            continue;
        }
        const is::PixelRect damage{0, y0, body_width, y1 - y0};
        const is::GlintParameters& p = i == 0 ? panel : strip;
        if (band.way == Way::reference) {
            is::paint_glints(view, surface, damage, p);
        } else {
            is::paint_glints_retained(view, surface, damage, p, scratch);
        }
    }
}

double paint_body(std::vector<std::byte>& pixels, Way way, int threads) {
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    if (threads <= 1) {
        paint_band(Band{&pixels, 0, body_height, way});
    } else {
        std::vector<std::thread> workers;
        workers.reserve(static_cast<std::size_t>(threads));
        for (int t = 0; t < threads; ++t) {
            const int first = body_height * t / threads;
            const int end = body_height * (t + 1) / threads;
            workers.emplace_back(paint_band, Band{&pixels, first, end, way});
        }
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
}

double median_ms(std::vector<std::byte>& pixels, Way way, int threads) {
    std::array<double, 5> times{};
    for (double& t : times) {
        t = paint_body(pixels, way, threads);
    }
    std::sort(times.begin(), times.end());
    return times[2];
}

std::size_t differing(const std::vector<std::byte>& a, const std::vector<std::byte>& b) {
    std::size_t count = 0;
    for (std::size_t i = 0; i < a.size(); i += 4U) {
        const bool same = a[i] == b[i] && a[i + 1U] == b[i + 1U] && a[i + 2U] == b[i + 2U];
        count += same ? 0U : 1U;
    }
    return count;
}

} // namespace

int main(int argc, char** argv) {
    const int threads = argc > 1 ? std::max(1, std::atoi(argv[1])) : static_cast<int>(std::max(1U, std::thread::hardware_concurrency()));
    const std::size_t bytes = static_cast<std::size_t>(body_width) * static_cast<std::size_t>(body_height) * 4U;
    std::vector<std::byte> reference(bytes);
    std::vector<std::byte> retained(bytes);
    std::vector<std::byte> threaded(bytes);
    const double reference_ms = median_ms(reference, Way::reference, 1);
    const double retained_ms = median_ms(retained, Way::retained, 1);
    const double threaded_ms = median_ms(threaded, Way::retained, threads);
    std::printf("reference %.1f ms · retained %.1f ms (%.2fx) · retained on %d threads %.1f ms (%.2fx)\n", reference_ms, retained_ms,
                reference_ms / retained_ms, threads, threaded_ms, reference_ms / threaded_ms);
    std::printf("pixels differing from the reference: retained %zu, threaded %zu (of %d)\n", differing(reference, retained),
                differing(reference, threaded), body_width * body_height);
    if (argc > 2) {
        // RGBA rows for a page (the framebuffer holds BGRA)
        std::vector<unsigned char> rgba(bytes);
        for (std::size_t i = 0; i < bytes; i += 4U) {
            rgba[i] = static_cast<unsigned char>(threaded[i + 2U]);
            rgba[i + 1U] = static_cast<unsigned char>(threaded[i + 1U]);
            rgba[i + 2U] = static_cast<unsigned char>(threaded[i]);
            rgba[i + 3U] = 255U;
        }
        std::FILE* file = std::fopen(argv[2], "wb");
        if (file == nullptr || std::fwrite(rgba.data(), 1U, rgba.size(), file) != rgba.size() || std::fclose(file) != 0) {
            std::fprintf(stderr, "cannot write %s\n", argv[2]);
            return 1;
        }
    }
    return 0;
}
