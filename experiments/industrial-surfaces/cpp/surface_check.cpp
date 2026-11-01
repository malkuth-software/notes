// Checks the C++ surface kernels against the JavaScript reference, checks that
// repainting in damage pieces gives the full repaint's bytes, and measures
// throughput.
//
// Usage: surface_check <directory of JS reference .rgba files>
// (tools/js_reference.mjs writes them; the cases here must match it.)

#include "surface_kernels.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace check {

namespace is = industrial_surfaces;

constexpr int width = 320;
constexpr int height = 200;
constexpr double scale = 2.0;

struct Buffer final {
    int width{};
    int height{};
    std::vector<std::byte> pixels;
};

Buffer make_buffer(int w, int h) {
    Buffer b{};
    b.width = w;
    b.height = h;
    b.pixels.assign(static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 4U, std::byte{0});
    return b;
}

is::FramebufferView view_of(Buffer& b) {
    return is::FramebufferView{b.pixels.data(), static_cast<std::size_t>(b.width) * 4U, b.width, b.height};
}

// Compare BGRA output with an RGBA reference: count of differing channel
// values and the largest difference.
struct Comparison final {
    bool loaded{};
    std::size_t differing{};
    int largest{};
};

Comparison compare_with(const std::string& path, const Buffer& b) {
    Comparison c{};
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr) {
        return c;
    }
    std::vector<std::uint8_t> ref(b.pixels.size());
    const std::size_t got = std::fread(ref.data(), 1U, ref.size(), file);
    std::fclose(file);
    if (got != ref.size()) {
        return c;
    }
    c.loaded = true;
    for (std::size_t i = 0; i < ref.size(); i += 4U) {
        const std::array<int, 3> ours = {std::to_integer<int>(b.pixels[i + 2U]), std::to_integer<int>(b.pixels[i + 1U]),
                                         std::to_integer<int>(b.pixels[i])};
        for (std::size_t ch = 0; ch < 3U; ++ch) {
            const int d = std::abs(ours[ch] - static_cast<int>(ref[i + ch]));
            if (d != 0) {
                ++c.differing;
                c.largest = std::max(c.largest, d);
            }
        }
    }
    return c;
}

// A fixed sequence of damage rectangles that together cover the surface.
std::vector<is::PixelRect> damage_pieces(int w, int h) {
    std::vector<is::PixelRect> pieces;
    std::uint32_t state = 12345U;
    for (int i = 0; i < 12; ++i) {
        state = state * 1664525U + 1013904223U;
        const int pw = 7 + static_cast<int>(state % static_cast<std::uint32_t>(w / 3));
        state = state * 1664525U + 1013904223U;
        const int ph = 5 + static_cast<int>(state % static_cast<std::uint32_t>(h / 3));
        state = state * 1664525U + 1013904223U;
        const int px = static_cast<int>(state % static_cast<std::uint32_t>(w - pw));
        state = state * 1664525U + 1013904223U;
        const int py = static_cast<int>(state % static_cast<std::uint32_t>(h - ph));
        pieces.push_back(is::PixelRect{px, py, pw, ph});
    }
    for (int y = 0; y < h; y += 37) {
        pieces.push_back(is::PixelRect{0, y, w, 37});
    }
    return pieces;
}

double megapixels_per_second(std::size_t pixels, double seconds) { return static_cast<double>(pixels) / seconds / 1e6; }

struct FinishCase final {
    const char* reference;
    is::FinishKind kind;
};

struct GlintCase final {
    const char* reference;
    is::GlintPreset preset;
};

is::FinishParameters finish_parameters(is::FinishKind kind) {
    is::FinishParameters p{};
    p.kind = kind;
    p.seed = 7;
    p.scale = scale;
    p.paint = is::Rgb8{0x2f, 0x6e, 0x6c};
    return p;
}

is::GlintParameters glint_parameters(is::GlintPreset preset, int w, int h) {
    is::GlintParameters p = is::glint_preset(preset);
    p.seed = 5;
    p.scale = scale;
    p.light_x = 0.62 * w / scale;
    p.light_y = 0.35 * h / scale;
    p.light_z = 900.0;
    p.eye_x = 0.5 * w / scale;
    p.eye_y = 0.5 * h / scale;
    p.eye_z = 700.0;
    return p;
}

}  // namespace check

int main(int argc, char** argv) {
    namespace is = industrial_surfaces;
    const std::string references = argc > 1 ? argv[1] : "build/js_ref";
    const std::array<check::FinishCase, 8> finishes = {
        check::FinishCase{"dusty_matte", is::FinishKind::dusty_matte},
        check::FinishCase{"sand_matte", is::FinishKind::blasted_under_paint},
        check::FinishCase{"orange_peel", is::FinishKind::orange_peel},
        check::FinishCase{"hammertone", is::FinishKind::hammertone},
        check::FinishCase{"wrinkle", is::FinishKind::wrinkle},
        check::FinishCase{"bead_blast", is::FinishKind::bead_blast},
        check::FinishCase{"sand_cast", is::FinishKind::sand_cast},
        check::FinishCase{"flat_baseline", is::FinishKind::flat},
    };
    const std::array<check::GlintCase, 5> glints = {
        check::GlintCase{"bead_blast", is::GlintPreset::bead_blast},
        check::GlintCase{"sand_blast", is::GlintPreset::sand_blast},
        check::GlintCase{"iridescent", is::GlintPreset::iridescent},
        check::GlintCase{"coarse_grit", is::GlintPreset::coarse_grit},
        check::GlintCase{"painted_sand", is::GlintPreset::painted_sand},
    };
    bool all_identical_damage = true;
    std::printf("%-22s %-14s %-24s %s\n", "case", "vs JavaScript", "damage pieces", "throughput (1024x512, best of 3)");
    is::FinishScratch scratch{};
    is::reserve_finish_scratch(scratch, 1024);
    for (const check::FinishCase& c : finishes) {
        check::Buffer full = check::make_buffer(check::width, check::height);
        const is::PixelRect surface{0, 0, check::width, check::height};
        const is::FinishParameters p = check::finish_parameters(c.kind);
        is::paint_finish(check::view_of(full), surface, surface, p, scratch);
        const check::Comparison js = check::compare_with(references + "/finish_" + c.reference + ".rgba", full);
        check::Buffer pieces = check::make_buffer(check::width, check::height);
        for (const is::PixelRect& r : check::damage_pieces(check::width, check::height)) {
            is::paint_finish(check::view_of(pieces), surface, r, p, scratch);
        }
        const bool same = pieces.pixels == full.pixels;
        all_identical_damage = all_identical_damage && same;
        check::Buffer big = check::make_buffer(1024, 512);
        double best = 1e30;
        for (int run = 0; run < 3; ++run) {
            const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
            is::paint_finish(check::view_of(big), is::PixelRect{0, 0, 1024, 512}, is::PixelRect{0, 0, 1024, 512}, p, scratch);
            best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
        }
        char verdict[64];
        if (!js.loaded) {
            std::snprintf(verdict, sizeof(verdict), "no reference");
        } else {
            std::snprintf(verdict, sizeof(verdict), "%zu diff, max %d", js.differing, js.largest);
        }
        std::printf("finish %-15s %-14s %-24s %.1f Mpx/s\n", c.reference, verdict, same ? "identical" : "DIFFERENT",
                    check::megapixels_per_second(1024U * 512U, best));
    }
    for (const check::GlintCase& c : glints) {
        check::Buffer full = check::make_buffer(check::width, check::height);
        const is::PixelRect surface{0, 0, check::width, check::height};
        const is::GlintParameters p = check::glint_parameters(c.preset, check::width, check::height);
        is::paint_glints(check::view_of(full), surface, surface, p);
        const check::Comparison js = check::compare_with(references + "/glint_" + c.reference + ".rgba", full);
        check::Buffer pieces = check::make_buffer(check::width, check::height);
        for (const is::PixelRect& r : check::damage_pieces(check::width, check::height)) {
            is::paint_glints(check::view_of(pieces), surface, r, p);
        }
        const bool same = pieces.pixels == full.pixels;
        all_identical_damage = all_identical_damage && same;
        check::Buffer big = check::make_buffer(1024, 512);
        const is::GlintParameters pb = check::glint_parameters(c.preset, 1024, 512);
        double best = 1e30;
        for (int run = 0; run < 3; ++run) {
            const std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
            is::paint_glints(check::view_of(big), is::PixelRect{0, 0, 1024, 512}, is::PixelRect{0, 0, 1024, 512}, pb);
            best = std::min(best, std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count());
        }
        char verdict[64];
        if (!js.loaded) {
            std::snprintf(verdict, sizeof(verdict), "no reference");
        } else {
            std::snprintf(verdict, sizeof(verdict), "%zu diff, max %d", js.differing, js.largest);
        }
        std::printf("glint  %-15s %-14s %-24s %.1f Mpx/s\n", c.reference, verdict, same ? "identical" : "DIFFERENT",
                    check::megapixels_per_second(1024U * 512U, best));
    }
    std::printf("single thread, -O2; %s\n", all_identical_damage ? "all damage repaints identical" : "SOME DAMAGE REPAINTS DIFFER");
    return all_identical_damage ? 0 : 1;
}
