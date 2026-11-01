#pragma once

// Stateless surface finishes and blasted-metal glints, painted straight into a
// framebuffer.
//
// Every device pixel is a pure function of its position relative to the
// surface's own origin, the display scale and the parameters. Nothing is
// baked: repainting any damage rectangle writes exactly the bytes a full
// repaint would. The finish kernel's only working storage is two rows of
// samples the width of the damage, supplied by the caller; the glint kernel
// needs none.
//
// These are ports of finish_kernel.js and glint_kernel.js in this directory;
// surface_check.cpp compares their bytes.

#include <cstddef>
#include <cstdint>
#include <vector>

namespace industrial_surfaces {

struct Rgb8 final {
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};
};

// A rectangle of device pixels.
struct PixelRect final {
    int x{};
    int y{};
    int width{};
    int height{};
};

// Premultiplied sRGB BGRA8 pixels, rows `row_bytes` apart. Borrowed for one
// call; the kernels write opaque pixels only.
struct FramebufferView final {
    std::byte* pixels{};
    std::size_t row_bytes{};
    int width{};
    int height{};
};

// ---- finishes ----------------------------------------------------------------------

enum class FinishKind : std::uint8_t {
    dusty_matte,
    blasted_under_paint,  // superseded by the glint kernel; kept for comparison
    orange_peel,
    hammertone,
    wrinkle,
    bead_blast,
    sand_cast,
    flat,
};

struct FinishParameters final {
    FinishKind kind{FinishKind::dusty_matte};
    std::int32_t seed{};
    double scale{1.0};                           // device pixels per logical unit
    Rgb8 paint{0x2f, 0x6e, 0x6c};
    Rgb8 dust{0xbd, 0xb6, 0xa8};
    double light_azimuth_degrees{125.0};         // 0 = from the right, counter-clockwise
    double light_elevation_degrees{38.0};
    double ambient{0.32};
    double relief_scale{1.0};
    double gloss_scale{1.0};
};

// One row entry: the finish's height (logical units), albedo multiplier, mix
// towards the dust colour, and specular multiplier. Stored as float, as the
// reference stores them.
struct FinishSample final {
    float height{};
    float albedo{};
    float dust{};
    float specular{};
};

// The two rows the finish kernel works in: the row it shades and the one
// below it, each the damage width plus one. Reserve once, reuse.
struct FinishScratch final {
    std::vector<FinishSample> upper;
    std::vector<FinishSample> lower;
};

void reserve_finish_scratch(FinishScratch& scratch, int damage_width);

// Paint the part of `damage` that lies inside both `surface` and the target.
// The finish is anchored to `surface`'s origin. Returns the number of pixels
// written; writes nothing for an empty intersection. `scratch` must have been
// reserved for at least the clipped damage width.
std::size_t paint_finish(FramebufferView target, PixelRect surface, PixelRect damage,
                         const FinishParameters& parameters, FinishScratch& scratch);

// ---- glints -------------------------------------------------------------------------

struct GlintParameters final {
    std::int32_t seed{5};
    double scale{1.0};          // device pixels per logical unit
    double grain{0.05};         // facet size, logical units
    double speck{0.75};         // speck size, logical units (≤ one pixel: none)
    double alpha{0.42};         // Beckmann tilt spread
    double tau{0.034};          // facet tolerance, in slope
    double diffuse{0.46};
    double specular{1.23};
    double mottle{0.06};
    double iridescence{0.0};
    double film{0.0};           // paint film over the metal, 0 = bare
    double film_alpha{0.3};
    double ambient{0.35};
    Rgb8 metal{0xa3, 0xa3, 0xa3};
    Rgb8 paint{0x80, 0x80, 0x80};
    // light and eye positions in logical units relative to the surface origin
    double light_x{0.0};
    double light_y{0.0};
    double light_z{900.0};
    double eye_x{0.0};
    double eye_y{0.0};
    double eye_z{700.0};
};

enum class GlintPreset : std::uint8_t {
    bead_blast,
    sand_blast,
    iridescent,
    coarse_grit,
    painted_sand,
};

// The fitted presets of glint_kernel.js (light and eye left at defaults).
GlintParameters glint_preset(GlintPreset preset);

std::size_t paint_glints(FramebufferView target, PixelRect surface, PixelRect damage, const GlintParameters& parameters);

// Working storage for paint_glints_retained, owned by the caller and reused
// across calls: one tile's retained speck results. Its size depends only on
// the tile and the parameters, so a caller painting many damage rectangles
// reserves it once (reserve_glint_scratch) per thread.
struct GlintScratch final {
    std::vector<std::int32_t> counts;  // facet count per (speck point, half-vector bin); −1 before first use
    std::vector<double> hues;          // the hue hash of the same
    std::vector<double> jitter;        // each speck cell's two jittered points, x then y
    std::vector<double> lambdas;       // per half-vector bin: its expected facet count, and e^(−λ)
    std::vector<double> starts;
};

// Reserves `scratch` for paint_glints_retained with these parameters.
void reserve_glint_scratch(GlintScratch& scratch, const GlintParameters& parameters);

// The same bytes as paint_glints, painted in 32 × 32 tiles. A speck point's
// facet count in a half-vector bin, its hue and its jitter do not depend on
// the pixel asking; paint_glints recomputes them for every pixel in reach.
// Here each is computed once per tile, on first use, and kept in `scratch`.
// Pixels combine them in paint_glints' order, so the output is identical.
// Damage rectangles are independent: separate threads may paint disjoint ones,
// each with its own scratch. Returns the pixels written (0 when `scratch` was
// reserved too small).
std::size_t paint_glints_retained(FramebufferView target, PixelRect surface, PixelRect damage, const GlintParameters& parameters,
                                  GlintScratch& scratch);

}  // namespace industrial_surfaces
