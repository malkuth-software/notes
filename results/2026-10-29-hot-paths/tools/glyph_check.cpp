// SCRATCH (hot-paths track, F1): hash GlyphRunRasterizer output over the
// bundled faces, sizes, phases and syntheses, so a build before and after a
// change can be compared line by line. Also times rasterization per glyph.
// Usage: hp_glyph_check <font-directory> [time]
#include "../src/render/text/glyph_run/glyph_run_raster.hpp"
#include "../src/render/text/harfbuzz/harfbuzz_font_engine.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <array>
#include <exception>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace text = gui_forms::render::text;

namespace {

struct Face final {
    const char* file;
    std::uint32_t glyphs;
    gui_forms::FontFaceId id;
};

std::uint64_t fold(std::uint64_t hash, const std::uint64_t value) {
    hash ^= value;
    hash *= 1099511628211ULL;
    return hash;
}

std::uint64_t failures = 0;
std::uint32_t current_id = 0;
const char* current_face = "";

std::uint64_t coverage_hash(std::uint64_t hash, const text::GlyphRunCoverage& coverage);

template <typename Call>
std::uint64_t guarded(std::uint64_t hash, Call call) {
    try {
        return coverage_hash(hash, call());
    } catch (const std::exception& error) {
        ++failures;
        std::fprintf(stderr, "FAIL %s glyph %u: %s\n", current_face, current_id, error.what());
        return fold(hash, std::hash<std::string>{}(error.what()));
    }
}

std::uint64_t coverage_hash(std::uint64_t hash, const text::GlyphRunCoverage& coverage) {
    hash = fold(hash, coverage.width);
    hash = fold(hash, coverage.height);
    hash = fold(hash, static_cast<std::uint32_t>(coverage.left));
    hash = fold(hash, static_cast<std::uint32_t>(coverage.top));
    for (const std::uint8_t pixel : coverage.pixels) hash = fold(hash, pixel);
    return hash;
}

// Glyph IDs sampled from a face: the first 700, then every 37th.
std::vector<std::uint32_t> sampled(const std::uint32_t count) {
    std::vector<std::uint32_t> ids;
    for (std::uint32_t id = 0; id < count; id = id < 700U ? id + 1U : id + 37U) ids.push_back(id);
    return ids;
}

// One run: glyph at four separated positions with four different phases.
text::ShapedText one_glyph_run(const gui_forms::FontFaceId face, const std::uint32_t glyph, const double size) {
    text::ShapedText shaped;
    text::ShapedFontRun run;
    run.face = face;
    const std::array<double, 4> offsets{0.0, 0.25, 0.5, 0.828125};
    for (std::size_t index = 0; index < offsets.size(); ++index) {
        text::ShapedGlyph placed{};
        placed.glyph.value = glyph;
        placed.x = static_cast<float>(static_cast<double>(index) * size * 3.0 + offsets[index]);
        placed.y = static_cast<float>(index % 2U);
        run.glyphs.push_back(placed);
    }
    shaped.runs.push_back(run);
    return shaped;
}

} // namespace

int main(int argc, char** argv) {
    const std::filesystem::path fonts(argv[1]);
    const bool timing = argc > 2;
    text::HarfBuzzFontEngine engine;
    std::vector<Face> faces{{"PortsmouthRapids.ttf", 0, {}}, {"PortsmouthRapids-Bold.ttf", 0, {}},
                            {"Carlito-Regular.ttf", 0, {}}, {"Carlito-Bold.ttf", 0, {}},
                            {"Carlito-Italic.ttf", 0, {}}, {"Cousine-Regular.ttf", 0, {}},
                            {"NotoSansCJKjp-Regular.otf", 0, {}}, {"NotoSansArabic-Regular.ttf", 0, {}},
                            {"NotoSansDevanagari-Regular.ttf", 0, {}}, {"NotoSansBengali-Regular.ttf", 0, {}},
                            {"NotoEmoji-Regular.ttf", 0, {}}};
    std::vector<std::vector<char>> keep;
    FT_Library library = nullptr;
    FT_Init_FreeType(&library);
    for (Face& face : faces) {
        std::ifstream stream(fonts / face.file, std::ios::binary);
        keep.emplace_back((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        const std::vector<char>& bytes = keep.back();
        const std::span<const std::byte> encoded(reinterpret_cast<const std::byte*>(bytes.data()), bytes.size());
        face.id = *engine.register_fallback_typeface(400U, false, encoded);
        FT_Face probe = nullptr;
        FT_New_Memory_Face(library, reinterpret_cast<const FT_Byte*>(bytes.data()), static_cast<FT_Long>(bytes.size()),
                           0, &probe);
        face.glyphs = static_cast<std::uint32_t>((*probe).num_glyphs);
        FT_Done_Face(probe);
    }
    FT_Done_FreeType(library);
    text::GlyphRunRasterizer rasterizer(engine);
    if (timing) {
        // Label-like runs: Carlito 13 px, glyphs 3..90 repeated, 100 passes.
        const Face& face = faces[2];
        text::ShapedText shaped;
        text::ShapedFontRun run;
        run.face = face.id;
        for (std::uint32_t index = 0; index < 400U; ++index) {
            text::ShapedGlyph placed{};
            placed.glyph.value = 3U + index * 7U % 88U;
            placed.x = static_cast<float>(index * 7.3);
            run.glyphs.push_back(placed);
        }
        shaped.runs.push_back(run);
        gui_forms::FontSpec font{};
        font.size = 13.0;
        std::uint64_t sink = 0;
        for (int warm = 0; warm < 5; ++warm) sink = coverage_hash(sink, rasterizer.rasterize(shaped, font, 1.0, 17));
        const int passes = 400;
        const std::chrono::steady_clock::time_point started = std::chrono::steady_clock::now();
        for (int pass = 0; pass < passes; ++pass) {
            sink = coverage_hash(sink, rasterizer.rasterize(shaped, font, 1.0, pass % 64));
        }
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
        std::printf("repeats ns_per_glyph %.1f (sink %llx)\n", seconds * 1e9 / (passes * 400.0),
                    static_cast<unsigned long long>(sink));
        // Cold: 88 distinct glyphs once each, at a new 26.6 size every pass,
        // so every glyph is loaded and hinted.
        text::ShapedText distinct;
        text::ShapedFontRun distinct_run;
        distinct_run.face = face.id;
        for (std::uint32_t index = 0; index < 88U; ++index) {
            text::ShapedGlyph placed{};
            placed.glyph.value = 3U + index;
            placed.x = static_cast<float>(index * 7.3);
            distinct_run.glyphs.push_back(placed);
        }
        distinct.runs.push_back(distinct_run);
        const int cold_passes = 600;
        const std::chrono::steady_clock::time_point cold_started = std::chrono::steady_clock::now();
        for (int pass = 0; pass < cold_passes; ++pass) {
            gui_forms::FontSpec sized{};
            sized.size = 12.0 + pass / 64.0;
            sink = coverage_hash(sink, rasterizer.rasterize(distinct, sized, 1.0, pass % 64));
        }
        const double cold_seconds =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - cold_started).count();
        std::printf("distinct ns_per_glyph %.1f (sink %llx)\n", cold_seconds * 1e9 / (cold_passes * 88.0),
                    static_cast<unsigned long long>(sink));
        return 0;
    }
    const std::array<double, 4> sizes{9.0, 13.0, 17.0, 32.0};
    std::uint64_t total = 14695981039346656037ULL;
    for (const Face& face : faces) {
        const std::vector<std::uint32_t> ids = sampled(face.glyphs);
        for (const double size : sizes) {
            for (int synthesis = 0; synthesis < 4; ++synthesis) {
                for (const double scale : {1.0, 2.0}) {
                    if (scale == 2.0 && size != 13.0) continue;
                    std::uint64_t hash = 14695981039346656037ULL;
                    gui_forms::FontSpec font{};
                    font.size = size;
                    text::GlyphRunSynthesis options{};
                    options.embolden = (synthesis & 1) != 0;
                    options.oblique = (synthesis & 2) != 0;
                    for (const std::uint32_t id : ids) {
                        const text::ShapedText shaped = one_glyph_run(face.id, id, size);
                        struct Call final {
                            text::GlyphRunRasterizer& rasterizer; const text::ShapedText& shaped;
                            gui_forms::FontSpec font; double scale; std::int32_t phase; text::GlyphRunSynthesis options;
                            text::GlyphRunCoverage operator()() const {
                                return rasterizer.rasterize(shaped, font, scale, phase, options);
                            }
                        };
                        hash = guarded(hash, Call{rasterizer, shaped, font, scale,
                                                  static_cast<std::int32_t>(id % 64U), options});
                    }
                    std::printf("%s size %.0f scale %.0f synthesis %d glyphs %zu hash %016llx\n", face.file, size,
                                scale, synthesis, ids.size(), static_cast<unsigned long long>(hash));
                    total = fold(total, hash);
                }
            }
        }
        // Rotated and skewed (unhinted) placement.
        const std::array<text::GlyphRunTransform, 2> transforms{
            text::GlyphRunTransform{0.866, -0.5, 0.5, 0.866}, text::GlyphRunTransform{1.5, 0.3, 0.0, 1.5}};
        for (std::size_t index = 0; index < transforms.size(); ++index) {
            for (int synthesis = 0; synthesis < 4; ++synthesis) {
                std::uint64_t hash = 14695981039346656037ULL;
                gui_forms::FontSpec font{};
                font.size = 13.0;
                text::GlyphRunSynthesis options{};
                options.embolden = (synthesis & 1) != 0;
                options.oblique = (synthesis & 2) != 0;
                for (const std::uint32_t id : ids) {
                    const text::ShapedText shaped = one_glyph_run(face.id, id, 13.0);
                    current_id = id; current_face = face.file;
                    text::GlyphRunOrigin origin{};
                    struct Call final {
                        text::GlyphRunRasterizer& rasterizer; const text::ShapedText& shaped; gui_forms::FontSpec font;
                        text::GlyphRunTransform transform; double x; text::GlyphRunSynthesis options;
                        text::GlyphRunOrigin& origin;
                        text::GlyphRunCoverage operator()() const {
                            return rasterizer.rasterize_transformed(shaped, font, transform, x, 20.3, options, origin);
                        }
                    };
                    hash = guarded(hash, Call{rasterizer, shaped, font, transforms[index], 10.0 + (id % 64U) / 64.0,
                                              options, origin});
                    hash = fold(hash, static_cast<std::uint64_t>(origin.pixel_x));
                    hash = fold(hash, static_cast<std::uint64_t>(origin.pixel_y));
                }
                std::printf("%s transform %zu synthesis %d hash %016llx\n", face.file, index, synthesis,
                            static_cast<unsigned long long>(hash));
                total = fold(total, hash);
            }
        }
    }
    std::printf("failures %llu\ntotal %016llx\n", static_cast<unsigned long long>(failures),
                static_cast<unsigned long long>(total));
    return 0;
}
