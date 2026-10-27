// Composes the proving slice without a window: lays the book out, draws
// spreads into memory, times both, checks the hit map against itself and
// writes a spread as a PNG. See README.md beside this file.
//
//   page_compose [--form reading|study] [--face garamond|portsmouth] [--step small|medium|large]
//                [--width W] [--height H] [--page N] [--png FILE] [--measure TURNS]
//
// --width and --height are device pixels. --page is the first page of the
// spread to draw (the left page; study form pairs page 2k with 2k + 1).
// --measure draws TURNS spreads, walking through the book, and reports the
// time of each draw; the first draw fills the glyph cache and is reported
// apart. One JSON object is printed on one line.

#include "page/embedded_assets.hpp"
#include "page/hit_map.hpp"
#include "page/png_writer.hpp"
#include "page/proving_book.hpp"

#include "core/file_io.hpp"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
using bible::page::BookForm;
using bible::page::BookLayout;
using bible::page::BookSettings;
using bible::page::HitMap;
using bible::page::HitWord;
using bible::page::LatinFace;
using bible::page::PageLayout;
using bible::page::PixelTarget;
using bible::page::ProvingBook;
using bible::page::SpreadGeometry;
using bible::page::TypeStep;

struct Options final {
    BookSettings settings{};
    std::string form_name{"reading"};
    std::string face_name{"garamond"};
    std::string step_name{"medium"};
    std::uint32_t width{2120U};
    std::uint32_t height{1360U};
    std::uint32_t page{0U};
    std::uint32_t measure_turns{0U};
    std::string png{};
};

double milliseconds_since(Clock::time_point start) {
    const std::chrono::duration<double, std::milli> elapsed = Clock::now() - start;
    const double milliseconds = elapsed.count();
    return milliseconds;
}

bool parse_number(std::string_view text, std::uint32_t minimum, std::uint32_t maximum, std::uint32_t& destination) {
    std::uint32_t value = 0U;
    const char* first = text.data();
    const char* last = text.data() + text.size();
    const std::from_chars_result parsed = std::from_chars(first, last, value);
    if (parsed.ec != std::errc{} || parsed.ptr != last || value < minimum || value > maximum) {
        return false;
    }
    destination = value;
    return true;
}

bool parse_arguments(int argc, char** argv, Options& options) {
    if (argc % 2 != 1) {
        return false;
    }
    for (int index = 1; index + 1 < argc; index += 2) {
        const std::string_view key(argv[index]);
        const std::string_view value(argv[index + 1]);
        if (key == "--form" && value == "reading") {
            options.settings.form = BookForm::reading;
        } else if (key == "--form" && value == "study") {
            options.settings.form = BookForm::study;
        } else if (key == "--face" && value == "garamond") {
            options.settings.face = LatinFace::eb_garamond;
        } else if (key == "--face" && value == "portsmouth") {
            options.settings.face = LatinFace::portsmouth;
        } else if (key == "--step" && value == "small") {
            options.settings.step = TypeStep::small;
        } else if (key == "--step" && value == "medium") {
            options.settings.step = TypeStep::medium;
        } else if (key == "--step" && value == "large") {
            options.settings.step = TypeStep::large;
        } else if (key == "--width" && parse_number(value, 16U, 8192U, options.width)) {
        } else if (key == "--height" && parse_number(value, 16U, 8192U, options.height)) {
        } else if (key == "--page" && parse_number(value, 0U, 10000U, options.page)) {
        } else if (key == "--measure" && parse_number(value, 1U, 100000U, options.measure_turns)) {
        } else if (key == "--png") {
            options.png = value;
        } else {
            return false;
        }
        if (key == "--form") {
            options.form_name = value;
        } else if (key == "--face") {
            options.face_name = value;
        } else if (key == "--step") {
            options.step_name = value;
        }
    }
    return true;
}

// What a laid-out book holds in memory, counted from its vectors: the
// figure behind "memory per cached page".
std::uint64_t layout_bytes(const PageLayout& page) {
    std::uint64_t bytes = sizeof(PageLayout) + page.head.capacity();
    for (const bible::page::PlacedRun& run : page.runs) {
        bytes += sizeof(bible::page::PlacedRun) + run.glyphs.capacity() * sizeof(bible::page::ShapedGlyph);
    }
    bytes += page.rules.capacity() * sizeof(bible::page::PlacedRule);
    bytes += page.words.capacity() * sizeof(bible::page::PlacedWord);
    for (const bible::page::PageVerse& verse : page.verses) {
        bytes += sizeof(bible::page::PageVerse) + verse.text.capacity();
    }
    return bytes;
}

// Every word's own centre must find that word.
std::uint32_t hit_failures(const HitMap& map) {
    std::uint32_t failures = 0U;
    for (const HitWord& word : map.words()) {
        const std::int32_t x = word.x0 + (word.x1 - word.x0) / 2;
        const std::int32_t y = word.y0 + (word.y1 - word.y0) / 2;
        const HitWord* found = map.find(x, y);
        if (found == nullptr || !((*found).word == word.word)) {
            ++failures;
        }
    }
    return failures;
}

double percentile(const std::vector<double>& sorted, double fraction) {
    const double rank = std::ceil(fraction * static_cast<double>(sorted.size()));
    const std::size_t index = rank < 1.0 ? 0U : static_cast<std::size_t>(rank) - 1U;
    const double value = sorted[std::min(index, sorted.size() - 1U)];
    return value;
}

int run(const Options& options) {
    ProvingBook book;
    const Clock::time_point load_start = Clock::now();
    const bool loaded = book.load(bible::page::embedded_proving_assets());
    const double load_ms = milliseconds_since(load_start);
    if (!loaded) {
        std::cerr << "page_compose: " << book.failure() << std::endl;
        return 1;
    }

    const Clock::time_point layout_start = Clock::now();
    const BookLayout& layout = book.layout(options.settings);
    const double layout_ms = milliseconds_since(layout_start);
    // The same layout again with every word already shaped: what is left
    // when shaping costs nothing, which is the most that shaping at build
    // time (candidate C of the type-setting decision) could save.
    const std::uint64_t misses_before = book.typesetter().counters().shape_cache_misses;
    const Clock::time_point relayout_start = Clock::now();
    BookLayout again;
    bible::page::lay_out_book(book.passages(), options.settings, book.typesetter(), again);
    const double relayout_ms = milliseconds_since(relayout_start);
    const std::uint64_t relayout_misses = book.typesetter().counters().shape_cache_misses - misses_before;
    std::uint64_t layout_total_bytes = 0U;
    for (const PageLayout& page : layout.pages) {
        layout_total_bytes += layout_bytes(page);
    }

    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(options.width) * options.height * 4U);
    PixelTarget target;
    target.pixels = pixels.data();
    target.row_bytes = static_cast<std::size_t>(options.width) * 4U;
    target.width = options.width;
    target.height = options.height;
    HitMap left_words;
    HitMap right_words;

    const Clock::time_point first_start = Clock::now();
    const SpreadGeometry geometry = book.draw(options.settings, options.page, target, left_words, right_words);
    const double first_draw_ms = milliseconds_since(first_start);
    const std::uint32_t failures = hit_failures(left_words) + hit_failures(right_words);
    const std::size_t words_on_spread = left_words.words().size() + right_words.words().size();

    if (!options.png.empty()) {
        std::vector<std::uint8_t> png;
        bible::page::encode_png(target, png);
        const bible::core::FileWriteStatus written =
            bible::core::write_file_atomic(std::filesystem::path(options.png), png);
        if (written != bible::core::FileWriteStatus::ok) {
            std::cerr << "page_compose: cannot write " << options.png << std::endl;
            return 1;
        }
    }

    std::vector<double> draws;
    if (options.measure_turns > 0U && !layout.pages.empty()) {
        const std::size_t step = geometry.folded ? 1U : 2U;
        std::size_t page = 0U;
        HitMap scratch_left;
        HitMap scratch_right;
        for (std::uint32_t turn = 0U; turn < options.measure_turns; ++turn) {
            page += step;
            if (page >= layout.pages.size()) {
                page = 0U;
            }
            const Clock::time_point start = Clock::now();
            static_cast<void>(book.draw(options.settings, page, target, scratch_left, scratch_right));
            draws.push_back(milliseconds_since(start));
        }
        std::sort(draws.begin(), draws.end());
    }

    const bible::page::TypesetterCounters& type = book.typesetter().counters();
    const bible::page::RasterCounters& raster = book.rasterizer().counters();
    std::cout << "{\"page_compose\":{\"form\":\"" << options.form_name << "\",\"face\":\"" << options.face_name
              << "\",\"step\":\"" << options.step_name << "\",\"width\":" << options.width
              << ",\"height\":" << options.height << ",\"folded\":" << (geometry.folded ? "true" : "false")
              << ",\"page_width\":" << geometry.left.width << ",\"page_height\":" << geometry.left.height
              << ",\"pages\":" << layout.pages.size() << ",\"load_ms\":" << load_ms << ",\"layout_ms\":" << layout_ms << ",\"relayout_ms\":" << relayout_ms
              << ",\"relayout_shape_misses\":" << relayout_misses
              << ",\"first_draw_ms\":" << first_draw_ms;
    if (!draws.empty()) {
        std::cout << ",\"draw_ms\":{\"count\":" << draws.size() << ",\"min\":" << draws.front()
                  << ",\"p50\":" << percentile(draws, 0.50) << ",\"p95\":" << percentile(draws, 0.95)
                  << ",\"max\":" << draws.back() << "}";
    }
    std::cout << ",\"layout_bytes\":" << layout_total_bytes << ",\"english_lines\":" << layout.counters.english_lines
              << ",\"original_lines\":" << layout.counters.original_lines
              << ",\"footnotes\":" << layout.counters.footnotes
              << ",\"overflowing_pages\":" << layout.counters.overflowing_pages
              << ",\"shape_requests\":" << type.shape_requests << ",\"shape_cache_entries\":" << type.shape_cache_entries
              << ",\"shape_cache_glyphs\":" << type.shape_cache_glyphs
              << ",\"shape_cache_key_bytes\":" << type.shape_cache_key_bytes
              << ",\"missing_glyphs\":" << type.missing_glyphs << ",\"synthesized_glyphs\":" << type.synthesized_glyphs
              << ",\"overfull_lines\":" << type.overfull_lines << ",\"loose_lines\":" << type.loose_lines
              << ",\"glyphs_drawn\":" << raster.glyphs_drawn << ",\"glyph_cache_entries\":" << raster.glyph_cache_entries
              << ",\"glyph_cache_bytes\":" << raster.glyph_cache_bytes << ",\"words_on_spread\":" << words_on_spread
              << ",\"hit_failures\":" << failures << "}}" << std::endl;
    return failures == 0U ? 0 : 1;
}

} // namespace

int main(int argc, char** argv) {
    try {
        Options options;
        if (!parse_arguments(argc, argv, options)) {
            std::cerr << "usage: page_compose [--form reading|study] [--face garamond|portsmouth] "
                         "[--step small|medium|large] [--width W] [--height H] [--page N] [--png FILE] "
                         "[--measure TURNS]"
                      << std::endl;
            return 2;
        }
        const int exit_code = run(options);
        return exit_code;
    } catch (const std::exception& error) {
        std::cerr << "page_compose: " << error.what() << std::endl;
        return 1;
    }
}
