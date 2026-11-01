// Bake study: a copy of cpp/lamp_keys.cpp with switches for cheaper bakes, each
// measured against the reference (bake_study/compare). With no switches it
// renders exactly as lamp_keys does.
//   --bbox        rays outside the lens start at its bounding box and stop
//                 past it (those that miss it stop at once)
//   --grey        one channel through the medium (the grades here are grey;
//                 the channel is copied to the other two)
//   --eps E       the marches' surface tolerance (reference 2e-4)
//   --chord S     the largest step along a chord inside the lens (reference 0.008)
//   --snap        a hit is moved onto the lens surface (two Newton steps along
//                 the normal) before it is shaded: the march stops anywhere
//                 within its tolerance, and an inside ray started from a hit
//                 at the far edge of it could end at once (an empty chord)
//   --convex      a ray leaving the lens of a control-only key cannot meet it
//                 again (it is convex and alone), so it takes the room's light
//                 in its direction without being marched
//   --split       bake apart: at full resolution and cheaply, the surface (the
//                 room reflected in the outer face, the legend's ink and its
//                 light, the coverage); at --t-scale N (1/N per side) with
//                 --t-grid G samples per side, the light transmitted from
//                 inside, without the legend. A runtime joins them:
//                   shown = reflection + ink × ink light + (1 − ink) × transmitted.
//                 Writes <stem>_surface.f32 and <stem>_inside.f32, and their join
//                 as the standard cache (for compare).
//   --poly        the smooth lens lies inside the polyhedron of its planes (a
//                 smooth maximum is never below the maximum): a ray's crossing
//                 of the polyhedron is found exactly, and the march starts
//                 0.02 before its exit (inside) or at its entry (outside),
//                 falling back to the full march when that start is wrong
//   --flat        where one plane leads all others by 2.5 × the rounding, the
//                 smooth surface is exactly that plane (no smooth maximum adds
//                 anything there): a ray's crossing of the polyhedron there is
//                 the exact hit or exit, taken without marching or snapping
//   --gauss N     in a medium of uniform extinction (hidden coil, no divider)
//                 the transmittance along a chord is exactly e^(−σt); with the
//                 transmittance as the variable, ∫e^(−σt)S dt = (1/σ)∫S du is
//                 integrated by N-point Gauss–Legendre on the smooth source alone
//   --fast-relief the relief's 14 waves (integer wave vectors) from two sine and
//                 cosine pairs by angle-addition recurrences, not 14 cosines
//   --analytic    a box lens's normal is its distance field's gradient, carried
//                 exactly through the smooth maxima (one evaluation, not six)
//   --adaptive T  one sample per pixel; a pixel takes the full 2 × 2 grid only
//                 where its neighbours' hit differs, the legend is within two
//                 pixels, or a neighbour's light differs by more than T (relative)
//
// Dialog keys lit from within (a copy of lamps.cpp for the OK and CANCEL keys,
// with what the engine renders taught: the colour is the lamp's; a clear key
// has a black void behind it; its coil is large and sparse in a faint haze; the
// top has a moulded relief on its normal).
//
// Panel lamps lit from within: an elevated keycap, a faceted jewel indicator and
// a split-legend annunciator, rendered with the participating-child law of the
// bfft photonic transport engine (photonic_transport_field/native/
// child_boundary_response.hpp: ConvexOpticalChild::evaluate_law).
//
// Each lens is a convex dielectric child. A ray refracts in; along every chord
// inside, light is retained by T = exp(−∫σ) and the medium contributes its own
// source radiance, L_out = B²·T·L_in + B·(1−T)·S; internal reflections are
// followed leg by leg until the weight falls below a cutoff. The source function
// varies in space: each lamp is a white-hot coiled filament on two dark lead
// wires, suspended in the lens, and the lens carries the lamp's light diffused
// to a degree set by its grade. Reflected copies of the filament in the lens's
// faces arise from the internal legs; nothing is sampled at random inside a lens.
//
// Lighting is linear in each lamp's power, so each pixel is written to one
// image for the room's light and one per lamp at unit white power. A frame is
// tone(room + Σ power_i × colour_i × lamp_i), per channel.
//
// Usage: lamps <output directory> <size in pixels> <keycap|annunciator|indicator>
// Writes <family>_<grade>_<up|down>.f32: int32 width, height and lamp count,
// then the room image and each lamp's image, width × height × 3 float32 each;
// and <family>_<grade>_<state>_<off|on>.bmp previews.

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

// bfft's CONV (standalone_conv_resize_demo/native/conv_native.c), as the
// photonic engine's render_adaptive uses it for certified scanline intervals.
extern "C" int conv_prepare_profile_f32(const float*, int, int, float*);
extern "C" int conv_evaluate_prepared_profile_f32(const float*, const float*, int, int, const float*, float*, int);
extern "C" int conv_resize_prepared_lines_f32(const float*, const float*, int, int, float*, int);

namespace lamps {

struct Lean final {
    bool bbox{};
    bool grey{};
    double eps{2e-4};
    double chord_step{0.008};
    double adaptive{};  // 0: off
    bool snap{};        // move each hit onto the surface before shading it
    bool convex{};      // a ray leaving the only (convex) lens sees the room at once
    double turn{0.999}; // adaptive: refine where neighbours' form normals differ more (cosine)
    bool split{};       // bake the surface and the transmitted light apart (see render_split)
    int t_scale{1};     // the transmitted light's resolution divisor
    int t_grid{2};      // its samples per pixel side
    bool t_smooth{};    // the transmitted light seen through a top without its relief
    bool t_linear{};    // and the relief's effect on it to first order (see join_split)
    int grid{2};        // samples per pixel side (the reference's 2)
    bool analytic{};    // a box lens's normal from its distance field's exact gradient
    bool poly{};        // marches start near the lens's bounding polyhedron's crossing
    bool flat{};        // exact hits and exits on flat faces, without marching
    int gauss{};        // Gauss–Legendre points per chord in a uniform medium (0: stepping)
    bool fast_relief{}; // the relief's waves by angle-addition recurrences
    bool intervals{};   // the transmitted light by the engine's certified scanline intervals
    int interval_tolerance{0};  // allowed 8-bit difference at a witness (the engine's terminal_error − 1)
};

// ---- the optical path signature (the engine's topology label) ---------------------------------
// A hash of the faces a sample's light crosses: the entry face, then each leg's
// exit face and what happened there. Two samples with equal signatures took
// the same optical path through the lens.
thread_local std::uint64_t path_signature = 0;

void sign_path(std::uint64_t event) {
    std::uint64_t x = event + 0x9e3779b97f4a7c15ULL + (path_signature << 6) + (path_signature >> 2);
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    path_signature ^= x;
}
Lean lean{};

constexpr double pi = 3.14159265358979323846;

struct Vec3 final {
    double x{};
    double y{};
    double z{};
};

Vec3 add(Vec3 a, Vec3 b) { return Vec3{a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 sub(Vec3 a, Vec3 b) { return Vec3{a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 scale(Vec3 a, double s) { return Vec3{a.x * s, a.y * s, a.z * s}; }
Vec3 mul(Vec3 a, Vec3 b) { return Vec3{a.x * b.x, a.y * b.y, a.z * b.z}; }
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 cross(Vec3 a, Vec3 b) { return Vec3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
double length(Vec3 a) { return std::sqrt(dot(a, a)); }
Vec3 normalized(Vec3 a) { return scale(a, 1.0 / length(a)); }
double max_component(Vec3 a) { return std::max(a.x, std::max(a.y, a.z)); }
Vec3 reflected(Vec3 d, Vec3 n) { return normalized(sub(d, scale(n, 2.0 * dot(d, n)))); }

Vec3 exp_neg(Vec3 sigma, double t) {
    return Vec3{std::exp(-sigma.x * t), std::exp(-sigma.y * t), std::exp(-sigma.z * t)};
}

double smoothstep(double a, double b, double x) {
    const double t = std::clamp((x - a) / (b - a), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

// Polynomial smooth maximum: max(a, b) raised by at most k/4 where they meet,
// which rounds the edge between two planes with a radius of about k.
double smooth_max(double a, double b, double k) {
    const double h = std::max(k - std::abs(a - b), 0.0) / k;
    return std::max(a, b) + h * h * k * 0.25;
}

// ---- scene -------------------------------------------------------------------------------
//
// Units: a lens is about 0.68 across; z up out of the panel; y up the screen.

enum class Family : std::uint8_t {
    keycap,
    jewel,
    annunciator,
};

// How the lens's face is lettered.
enum class Legend : std::uint8_t {
    none,
    black_ink,   // opaque black lettering on the lit field
    white_ink,   // opaque white lettering on the lit field
    dead_front,  // the face painted black; the letters are white diffuser ink and glow
    grey_ink,    // opaque mid-grey lettering
    lit_ink,     // white diffuser ink on the open face: grey by the room, lit through by the lamps
};

// A lens grade as a participating medium.
struct Grade final {
    const char* name{};
    Vec3 density{};           // extinction σ per channel
    double albedo{};          // share of σ that re-emits (the rest is absorbed)
    double glow_reach{};      // distance over which a lamp's diffused light falls to half
    double room{};            // the room light the lens itself carries, as source radiance
    double rear_reflector{};  // reflectance of a silvered back face (0: plain polished face)
    double glow_gain{1.0};    // brightness of the diffused lamp light against the filament's
    double rear_diffuser{};   // albedo of a frosted back face that scatters the lamps' light
    Vec3 pigment{};           // added absorption per channel: the material's own colour
    double near_reach{};      // a second, short reach: a brighter, soft area round the lamp
    double near_gain{};       // its brightness against the long glow (0: none)
};

// One lamp: a coiled filament about the axis a–b, on two lead wires that run
// down from its ends to the base of the lens.
struct Lamp final {
    Vec3 a{};
    Vec3 b{};
    double coil_radius{0.014};
    double pitch{0.011};
    double wire_radius{0.0032};
    double lead_radius{0.0045};
};

constexpr int max_lamps = 2;

struct Plane final {
    Vec3 normal{};
    double offset{};  // the plane is dot(normal, p) = offset
};

struct Segment final {
    double ax{};
    double ay{};
    double bx{};
    double by{};
};

struct Scene final {
    Family family{Family::keycap};
    // keycap and annunciator body
    double half_base{0.34};
    double half_top{0.275};
    double bottom{-0.07};
    double top{0.30};
    double chamfer{0.05};
    double corner{0.07};
    double softness{0.03};
    double dish{0.012};
    double aspect{1.0};  // the lens's height on screen over its width
    // jewel body: the convex hull's planes, and a little edge softening
    std::vector<Plane> facets;
    double facet_softness{0.003};
    // the panel
    bool round_well{false};
    double well_half{0.385};
    double well_radius{0.06};
    double well_floor{-0.22};
    double press{0.0};
    // lamps
    std::array<Lamp, max_lamps> lamps{};
    int lamp_count{1};
    double filament_radiance{24.0};
    // annunciator: an opaque divider between its halves, the legend, and
    // whether the sidewalls are painted to block light
    bool divider{false};
    Legend legend_style{Legend::none};
    bool sidewall_blocked{false};
    double field_of_view{21.0};  // the camera's, in degrees: small lamps are framed closer
    // A control draws only itself: no panel, no well, nothing beyond its own
    // edge. The camera looks straight down and the image spans exactly
    // ±frame_half_x by ±frame_half_y at z = 0; rays that miss the lens are
    // transparent.
    bool control_only{false};
    double frame_half_x{};
    double frame_half_y{};
    // The control camera's eye: straight above by default; moved toward the
    // viewer (negative y) it looks at the key a little from the front, so the
    // front sidewall shows. The frame is the window on z = 0 that the key's
    // projection fills, centred at (0, frame_centre_y).
    Vec3 control_eye{0.0, 0.0, 2.6};
    double frame_centre_y{};
    std::vector<Segment> legend;
    double legend_half_width{0.0062};
    Grade grade{};
    // the moulded relief of a box lens's top: slopes of its orange-peel waves
    // and of its finer grain (0: polished)
    double relief{};
    double grain{};
    // the study's constant tilt of the top's normal (first-order grain), added to the relief's slope
    double tilt_x{};
    double tilt_y{};
    // a matte black back face: a clear key over a black void, where nothing
    // reflects back up from behind its cold coil
    bool black_back{};
    // the coil gives its light but is not seen: no wire in the medium, only
    // its diffused glow
    bool hidden_coil{};
};

Vec3 lowered(const Scene& scene, Vec3 p) { return Vec3{p.x, p.y, p.z + scene.press}; }

double box_lens_distance(const Scene& scene, Vec3 world) {
    const Vec3 p = lowered(scene, world);
    // y is measured in units of the aspect, so the lens is aspect × as tall
    const double ax = std::abs(p.x);
    const double ay = std::abs(p.y) / scene.aspect;
    const double height = scene.top - scene.bottom;
    const double slope = (scene.half_base - scene.half_top) / height;
    const double side_norm = 1.0 / std::sqrt(1.0 + slope * slope);
    const double half_here = scene.half_base - slope * (p.z - scene.bottom);
    const double across = std::min(1.0, ax / scene.half_top);
    const double top_here = scene.top - scene.dish * (1.0 - across * across);
    const double k = scene.softness;
    double d = p.z - top_here;
    d = smooth_max(d, scene.bottom - p.z, k);
    d = smooth_max(d, (ax - half_here) * side_norm, k);
    d = smooth_max(d, (ay - half_here) * side_norm * scene.aspect, k);
    const double c = scene.chamfer;
    d = smooth_max(d, (ax + p.z - (scene.half_top + scene.top - c)) * 0.7071067811865476, k);
    d = smooth_max(d, (std::abs(p.y) + p.z - (scene.half_top * scene.aspect + scene.top - c)) * 0.7071067811865476, k);
    d = smooth_max(d, (ax + std::abs(p.y) - (half_here * (1.0 + scene.aspect) - scene.corner)) * 0.7071067811865476, k);
    return d;
}

// The same distance with its gradient. For smooth_max(a, b, k) the gradient is
// w·∇a + (1 − w)·∇b with w = [a > b] − (h / 2)·sign(a − b), h = max(k − |a − b|, 0) / k.
struct DistanceGradient final {
    double d{};
    Vec3 g{};
};

DistanceGradient smooth_max_gradient(DistanceGradient a, DistanceGradient b, double k) {
    const double h = std::max(k - std::abs(a.d - b.d), 0.0) / k;
    const double sign = a.d > b.d ? 1.0 : (a.d < b.d ? -1.0 : 0.0);
    const double w = (a.d > b.d ? 1.0 : 0.0) - 0.5 * h * sign;
    DistanceGradient out{};
    out.d = std::max(a.d, b.d) + h * h * k * 0.25;
    out.g = add(scale(a.g, w), scale(b.g, 1.0 - w));
    return out;
}

DistanceGradient box_lens_gradient(const Scene& scene, Vec3 world) {
    const Vec3 p = lowered(scene, world);
    const double sx = p.x < 0.0 ? -1.0 : 1.0;
    const double sy = p.y < 0.0 ? -1.0 : 1.0;
    const double ax = std::abs(p.x);
    const double ay = std::abs(p.y) / scene.aspect;
    const double height = scene.top - scene.bottom;
    const double slope = (scene.half_base - scene.half_top) / height;
    const double side_norm = 1.0 / std::sqrt(1.0 + slope * slope);
    const double half_here = scene.half_base - slope * (p.z - scene.bottom);
    const double across = std::min(1.0, ax / scene.half_top);
    const double top_here = scene.top - scene.dish * (1.0 - across * across);
    const double k = scene.softness;
    const double r = 0.7071067811865476;
    // the dish's slope across x, inside the top's half width
    const double dish_x = ax < scene.half_top ? -2.0 * scene.dish * across / scene.half_top * sx : 0.0;
    DistanceGradient d{p.z - top_here, Vec3{-dish_x, 0.0, 1.0}};
    d = smooth_max_gradient(d, DistanceGradient{scene.bottom - p.z, Vec3{0.0, 0.0, -1.0}}, k);
    d = smooth_max_gradient(d, DistanceGradient{(ax - half_here) * side_norm, Vec3{sx * side_norm, 0.0, slope * side_norm}}, k);
    d = smooth_max_gradient(d, DistanceGradient{(ay - half_here) * side_norm * scene.aspect, Vec3{0.0, sy * side_norm, slope * side_norm * scene.aspect}}, k);
    const double c = scene.chamfer;
    d = smooth_max_gradient(d, DistanceGradient{(ax + p.z - (scene.half_top + scene.top - c)) * r, Vec3{sx * r, 0.0, r}}, k);
    d = smooth_max_gradient(d, DistanceGradient{(std::abs(p.y) + p.z - (scene.half_top * scene.aspect + scene.top - c)) * r, Vec3{0.0, sy * r, r}}, k);
    d = smooth_max_gradient(d, DistanceGradient{(ax + std::abs(p.y) - (half_here * (1.0 + scene.aspect) - scene.corner)) * r, Vec3{sx * r, sy * r, slope * (1.0 + scene.aspect) * r}}, k);
    return d;
}

double jewel_distance(const Scene& scene, Vec3 world) {
    const Vec3 p = lowered(scene, world);
    double d = -1e9;
    for (const Plane& plane : scene.facets) {
        d = smooth_max(d, dot(plane.normal, p) - plane.offset, scene.facet_softness);
    }
    return d;
}

double lens_distance(const Scene& scene, Vec3 p) {
    switch (scene.family) {
    case Family::jewel: return jewel_distance(scene, p);
    case Family::keycap:
    case Family::annunciator:
    default: return box_lens_distance(scene, p);
    }
}

double rounded_rect(double x, double y, double half_x, double half_y, double radius) {
    const double qx = std::abs(x) - half_x + radius;
    const double qy = std::abs(y) - half_y + radius;
    const double ox = std::max(qx, 0.0);
    const double oy = std::max(qy, 0.0);
    return std::sqrt(ox * ox + oy * oy) + std::min(std::max(qx, qy), 0.0) - radius;
}

// The panel with its well: solid below z = 0 outside the well, and below the
// well's floor.
double plate_distance(const Scene& scene, Vec3 p) {
    const double well = scene.round_well ? std::sqrt(p.x * p.x + p.y * p.y) - scene.well_half
                                         : rounded_rect(p.x, p.y, scene.well_half, scene.well_half * scene.aspect, scene.well_radius);
    return std::min(std::max(p.z, -well), p.z - scene.well_floor);
}

// A jewel cut as a round brilliant's crown: a flat table, three rings of
// facets falling away from it, a short vertical girdle and a flat back.
void cut_jewel(Scene& scene) {
    scene.facets.clear();
    const double table_z = 0.22;
    scene.facets.push_back(Plane{Vec3{0.0, 0.0, 1.0}, table_z});
    scene.facets.push_back(Plane{Vec3{0.0, 0.0, -1.0}, 0.08});  // the back, at z = −0.08
    struct Ring final {
        int count;
        double tilt_degrees;
        double inner_radius;
        double outer_radius;
        double phase;  // as a fraction of one facet
    };
    const std::array<Ring, 3> rings = {
        Ring{8, 22.0, 0.10, 0.19, 0.0},
        Ring{16, 45.0, 0.19, 0.26, 0.5},
        Ring{16, 68.0, 0.26, 0.30, 0.0},
    };
    double z = table_z;
    for (const Ring& ring : rings) {
        const double tilt = ring.tilt_degrees * pi / 180.0;
        for (int i = 0; i < ring.count; ++i) {
            const double azimuth = 2.0 * pi * (i + ring.phase) / ring.count;
            const Vec3 n{std::sin(tilt) * std::cos(azimuth), std::sin(tilt) * std::sin(azimuth), std::cos(tilt)};
            const Vec3 through{ring.inner_radius * std::cos(azimuth), ring.inner_radius * std::sin(azimuth), z};
            scene.facets.push_back(Plane{n, dot(n, through)});
        }
        z -= (ring.outer_radius - ring.inner_radius) * std::tan(tilt);
    }
    for (int i = 0; i < 16; ++i) {
        const double azimuth = 2.0 * pi * (i + 0.5) / 16.0;
        scene.facets.push_back(Plane{Vec3{std::cos(azimuth), std::sin(azimuth), 0.0}, 0.30});
    }
}

// ---- legend: an angular stroke font in the military manner ------------------------------
//
// Each glyph is a set of polylines on a 4 × 6 grid, separated by '|'.

struct GlyphDefinition final {
    char letter;
    const char* strokes;
};

constexpr std::array<GlyphDefinition, 23> glyphs = {
    GlyphDefinition{'W', "0,6 1,0 2,3.5 3,0 4,6"},
    GlyphDefinition{'V', "0,6 2,0 4,6"},
    GlyphDefinition{'B', "0,0 0,6 3,6 4,5 4,4 3,3 0,3|3,3 4,2 4,1 3,0 0,0"},
    GlyphDefinition{'-', "1,3 3,3"},
    GlyphDefinition{'A', "0,0 0,4.5 1.5,6 2.5,6 4,4.5 4,0|0,3 4,3"},
    GlyphDefinition{'C', "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1"},
    GlyphDefinition{'D', "0,0 0,6 3,6 4,5 4,1 3,0 0,0"},
    GlyphDefinition{'E', "4,6 0,6 0,0 4,0|0,3 3,3"},
    GlyphDefinition{'F', "4,6 0,6 0,0|0,3 3,3"},
    GlyphDefinition{'G', "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1 4,3 2.5,3"},
    GlyphDefinition{'H', "0,0 0,6|4,0 4,6|0,3 4,3"},
    GlyphDefinition{'I', "2,0 2,6|1,6 3,6|1,0 3,0"},
    GlyphDefinition{'L', "0,6 0,0 4,0"},
    GlyphDefinition{'M', "0,0 0,6 2,3 4,6 4,0"},
    GlyphDefinition{'N', "0,0 0,6 4,0 4,6"},
    GlyphDefinition{'O', "1,0 0,1 0,5 1,6 3,6 4,5 4,1 3,0 1,0"},
    GlyphDefinition{'P', "0,0 0,6 3,6 4,5 4,4 3,3 0,3"},
    GlyphDefinition{'R', "0,0 0,6 3,6 4,5 4,4 3,3 0,3|2,3 4,0"},
    GlyphDefinition{'S', "4,5 3,6 1,6 0,5 0,4 1,3 3,3 4,2 4,1 3,0 1,0 0,1"},
    GlyphDefinition{'T', "0,6 4,6|2,6 2,0"},
    GlyphDefinition{'U', "0,6 0,1 1,0 3,0 4,1 4,6"},
    GlyphDefinition{'Y', "0,6 2,3 4,6|2,3 2,0"},
    GlyphDefinition{'K', "0,0 0,6|4,6 0,2|1.3,3.3 4,0"},
};

constexpr double glyph_advance = 5.5;

// Append one line of text, centred on x = 0 with its baseline at `baseline`,
// letters `height` tall.
void set_line(Scene& scene, const std::string& text, double baseline, double height) {
    const double unit = height / 6.0;
    const double width = (static_cast<double>(text.size()) * glyph_advance - 1.5) * unit;
    double pen = -0.5 * width;
    for (const char letter : text) {
        const GlyphDefinition* found = nullptr;
        for (const GlyphDefinition& glyph : glyphs) {
            if (glyph.letter == letter) {
                found = &glyph;
            }
        }
        if (found != nullptr) {
            const char* cursor = (*found).strokes;
            bool have_previous = false;
            double px = 0.0;
            double py = 0.0;
            while (*cursor != '\0') {
                if (*cursor == '|') {
                    have_previous = false;
                    ++cursor;
                    continue;
                }
                char* end = nullptr;
                const double gx = std::strtod(cursor, &end);
                cursor = end + 1;  // the comma
                const double gy = std::strtod(cursor, &end);
                cursor = end;
                while (*cursor == ' ') {
                    ++cursor;
                }
                const double x = pen + gx * unit;
                const double y = baseline + gy * unit;
                if (have_previous) {
                    scene.legend.push_back(Segment{px, py, x, y});
                }
                px = x;
                py = y;
                have_previous = true;
            }
        }
        pen += glyph_advance * unit;
    }
}

double segment_distance(const Segment& s, double x, double y) {
    const double vx = s.bx - s.ax;
    const double vy = s.by - s.ay;
    const double wx = x - s.ax;
    const double wy = y - s.ay;
    const double t = std::clamp((wx * vx + wy * vy) / std::max(1e-12, vx * vx + vy * vy), 0.0, 1.0);
    const double dx = wx - vx * t;
    const double dy = wy - vy * t;
    return std::sqrt(dx * dx + dy * dy);
}

// Share of a point's footprint covered by legend ink.
double legend_coverage(const Scene& scene, double x, double y) {
    double d = 1e9;
    for (const Segment& s : scene.legend) {
        d = std::min(d, segment_distance(s, x, y));
    }
    const double edge = 0.0018;
    return 1.0 - smoothstep(scene.legend_half_width - edge, scene.legend_half_width + edge, d);
}

// ---- marching -----------------------------------------------------------------------------

enum class Surface : std::uint8_t {
    none,
    lens,
    plate,
};

struct Hit final {
    Surface surface{Surface::none};
    double t{};
};

// The lens's bounding box (lowered by the press), widened by a margin over
// the smooth maximum's bulge; the interval of t inside it, or false.
bool lens_box_interval(const Scene& scene, Vec3 origin, Vec3 direction, double& t0, double& t1) {
    const double margin = 0.01;
    const double hx = std::max(scene.half_base, scene.half_top) + margin;
    const double hy = hx * scene.aspect + margin;
    const double lo_z = scene.bottom - scene.press - margin;
    const double hi_z = scene.top - scene.press + margin;
    const double lo[3] = {-hx, -hy, lo_z};
    const double hi[3] = {hx, hy, hi_z};
    const double o[3] = {origin.x, origin.y, origin.z};
    const double d[3] = {direction.x, direction.y, direction.z};
    t0 = 0.0;
    t1 = 1e30;
    for (int axis = 0; axis < 3; ++axis) {
        if (std::abs(d[axis]) < 1e-12) {
            if (o[axis] < lo[axis] || o[axis] > hi[axis]) {
                return false;
            }
            continue;
        }
        double a = (lo[axis] - o[axis]) / d[axis];
        double b = (hi[axis] - o[axis]) / d[axis];
        if (a > b) {
            std::swap(a, b);
        }
        t0 = std::max(t0, a);
        t1 = std::min(t1, b);
    }
    return t0 <= t1;
}

// The box lens's planes, in lowered coordinates (z + press): each is
// n · p − c, and the lens without its rounding is where all are ≤ 0.
struct LensPlane final {
    Vec3 n{};
    double c{};
};

int box_lens_planes(const Scene& scene, std::array<LensPlane, 16>& planes) {
    const double height = scene.top - scene.bottom;
    const double slope = (scene.half_base - scene.half_top) / height;
    const double sn = 1.0 / std::sqrt(1.0 + slope * slope);
    const double a = scene.aspect;
    const double r = 0.7071067811865476;
    const double base = scene.half_base + slope * scene.bottom;
    int count = 0;
    planes[static_cast<std::size_t>(count++)] = LensPlane{Vec3{0.0, 0.0, 1.0}, scene.top};
    planes[static_cast<std::size_t>(count++)] = LensPlane{Vec3{0.0, 0.0, -1.0}, -scene.bottom};
    for (int side = -1; side <= 1; side += 2) {
        const double sd = side;
        planes[static_cast<std::size_t>(count++)] = LensPlane{Vec3{sd * sn, 0.0, slope * sn}, base * sn};
        planes[static_cast<std::size_t>(count++)] = LensPlane{Vec3{0.0, sd * sn, slope * sn * a}, base * sn * a};
        planes[static_cast<std::size_t>(count++)] = LensPlane{Vec3{sd * r, 0.0, r}, (scene.half_top + scene.top - scene.chamfer) * r};
        planes[static_cast<std::size_t>(count++)] = LensPlane{Vec3{0.0, sd * r, r}, (scene.half_top * a + scene.top - scene.chamfer) * r};
        for (int other = -1; other <= 1; other += 2) {
            planes[static_cast<std::size_t>(count++)] = LensPlane{Vec3{sd * r, other * r, slope * (1.0 + a) * r}, (base * (1.0 + a) - scene.corner) * r};
        }
    }
    return count;
}

// The ray's interval inside the polyhedron, [t_in, t_out]; false if it misses.
bool polyhedron_interval(const Scene& scene, Vec3 origin, Vec3 direction, double& t_in, double& t_out) {
    std::array<LensPlane, 16> planes{};
    const int count = box_lens_planes(scene, planes);
    const Vec3 o{origin.x, origin.y, origin.z + scene.press};
    t_in = -1e30;
    t_out = 1e30;
    for (int i = 0; i < count; ++i) {
        const LensPlane& plane = planes[static_cast<std::size_t>(i)];
        const double nd = dot(plane.n, direction);
        const double gap = plane.c - dot(plane.n, o);  // > 0: the origin is inside this plane
        if (std::abs(nd) < 1e-12) {
            if (gap < 0.0) {
                return false;
            }
            continue;
        }
        const double t = gap / nd;
        if (nd > 0.0) {
            t_out = std::min(t_out, t);
        } else {
            t_in = std::max(t_in, t);
        }
    }
    return t_in <= t_out;
}

bool polyhedron_usable(const Scene& scene) {
    return lean.poly && scene.family != Family::jewel && scene.dish == 0.0;
}

// Is the smooth surface exactly a plane here? True when the largest plane term
// leads every other by 2.5 × the rounding: each smooth maximum along the chain
// then adds nothing (bulges of at most k/4 apiece cannot close that gap).
bool flat_point(const Scene& scene, Vec3 world) {
    if (!lean.flat || !polyhedron_usable(scene)) {
        return false;
    }
    std::array<LensPlane, 16> planes{};
    const int count = box_lens_planes(scene, planes);
    const Vec3 p{world.x, world.y, world.z + scene.press};
    double best = -1e30;
    double next = -1e30;
    for (int i = 0; i < count; ++i) {
        const double v = dot(planes[static_cast<std::size_t>(i)].n, p) - planes[static_cast<std::size_t>(i)].c;
        if (v > best) {
            next = best;
            best = v;
        } else if (v > next) {
            next = v;
        }
    }
    return best - next > 2.5 * scene.softness;
}

// Which face of the box lens a surface point is on: its largest plane term, or
// the pair of the two largest where they blend within the rounding.
std::uint64_t face_label(const Scene& scene, Vec3 world) {
    std::array<LensPlane, 16> planes{};
    const int count = box_lens_planes(scene, planes);
    const Vec3 p{world.x, world.y, world.z + scene.press};
    int first = -1;
    int second = -1;
    double best = -1e30;
    double next = -1e30;
    for (int i = 0; i < count; ++i) {
        const double v = dot(planes[static_cast<std::size_t>(i)].n, p) - planes[static_cast<std::size_t>(i)].c;
        if (v > best) {
            next = best;
            second = first;
            best = v;
            first = i;
        } else if (v > next) {
            next = v;
            second = i;
        }
    }
    if (best - next < scene.softness && second >= 0) {
        return 64U + static_cast<std::uint64_t>(std::min(first, second)) * 16U + static_cast<std::uint64_t>(std::max(first, second));
    }
    return static_cast<std::uint64_t>(first);
}

Hit march_outside_clipped(const Scene& scene, Vec3 origin, Vec3 direction, bool clip);

Hit march_outside(const Scene& scene, Vec3 origin, Vec3 direction) {
    const Hit hit = march_outside_clipped(scene, origin, direction, lean.bbox);
    static const bool check = std::getenv("BBOX_CHECK") != nullptr;  // read once
    if (lean.bbox && check) {
        const Hit plain = march_outside_clipped(scene, origin, direction, false);
        if (plain.surface != hit.surface || std::abs(plain.t - hit.t) > 2e-5) {
            static int reported = 0;
            if (reported++ < 6) {
                std::fprintf(stderr, "mismatch: origin %.4f %.4f %.4f dir %.4f %.4f %.4f  plain %d t %.5f  clipped %d t %.5f\n", origin.x, origin.y,
                             origin.z, direction.x, direction.y, direction.z, static_cast<int>(plain.surface), plain.t, static_cast<int>(hit.surface), hit.t);
            }
        }
    }
    return hit;
}

Hit march_outside_clipped(const Scene& scene, Vec3 origin, Vec3 direction, bool clip) {
    Hit hit{};
    double t = 0.0;
    double t_end = 1e30;
    if (clip && scene.control_only && scene.family != Family::jewel) {
        double t0 = 0.0;
        double t1 = 0.0;
        if (!lens_box_interval(scene, origin, direction, t0, t1)) {
            return hit;
        }
        t = t0;
        t_end = t1;
        if (polyhedron_usable(scene)) {
            double p_in = 0.0;
            double p_out = 0.0;
            if (!polyhedron_interval(scene, origin, direction, p_in, p_out) || p_out < 0.0) {
                return hit;  // the smooth lens is inside the polyhedron: it is missed too
            }
            t = std::max(t, p_in);
            if (p_in >= 0.0 && flat_point(scene, add(origin, scale(direction, p_in)))) {
                hit.surface = Surface::lens;
                hit.t = p_in;  // exactly on the face
                return hit;
            }
        }
    }
    for (int step = 0; step < 200; ++step) {
        if (t > t_end) {
            return hit;
        }
        const Vec3 p = add(origin, scale(direction, t));
        if ((p.z > 0.8 && direction.z > 0.0) || t > 8.0) {
            return hit;
        }
        const double dl = lens_distance(scene, p);
        const double dp = scene.control_only ? 1e9 : plate_distance(scene, p);
        const double d = std::min(dl, dp);
        if (d < lean.eps) {
            hit.surface = dl < dp ? Surface::lens : Surface::plate;
            hit.t = t;
            return hit;
        }
        t += d * 0.9;
    }
    return hit;
}

double march_inside(const Scene& scene, Vec3 origin, Vec3 direction) {
    double t = 0.0;
    if (polyhedron_usable(scene)) {
        double p_in = 0.0;
        double p_out = 0.0;
        if (polyhedron_interval(scene, origin, direction, p_in, p_out)) {
            if (p_out > 0.0 && flat_point(scene, add(origin, scale(direction, p_out)))) {
                return p_out;  // exactly on the face
            }
            const double start = p_out - 0.02;
            if (start > 0.0 && lens_distance(scene, add(origin, scale(direction, start))) < -lean.eps) {
                t = start;  // still inside: the exit is ahead
            }
        }
    }
    for (int step = 0; step < 240; ++step) {
        const double d = -lens_distance(scene, add(origin, scale(direction, t)));
        if (d < lean.eps) {
            return t;
        }
        t += d * 0.9;
        if (t > 4.0) {
            return t;
        }
    }
    return t;
}

// ---- the moulded relief ---------------------------------------------------------------------
//
// A height field over the lens's top, as a slope (dh/dx, dh/dy). The waves are
// Stillwater's caustic surface (playsuite-games shared/ambient/src/caustics.cpp,
// 0efc1f0): fourteen integer wave vectors on a tile, amplitude 1/|k|^1.75,
// scaled so that typical peaks reach a slope of 1. The grain is smooth value
// noise on cells about two pixels across.

struct ReliefWave final {
    double kx{};
    double ky{};
    double amplitude{};
    double phase{};
};

struct ReliefWaves final {
    std::array<ReliefWave, 14> waves{};
};

ReliefWaves make_relief_waves() {
    ReliefWaves result{};
    std::uint32_t state = 1U * 2654435761U + 12345U;
    std::size_t count = 0;
    double bound = 0.0;
    while (count < result.waves.size()) {
        state = state * 1664525U + 1013904223U;
        std::uint32_t x = state;
        x ^= x >> 16U;
        x *= 0x7FEB352DU;
        x ^= x >> 15U;
        const int kx = static_cast<int>(x % 15U) - 7;
        state = state * 1664525U + 1013904223U;
        x = state;
        x ^= x >> 16U;
        x *= 0x7FEB352DU;
        x ^= x >> 15U;
        const int ky = static_cast<int>(x % 15U) - 7;
        const double radius = std::sqrt(static_cast<double>(kx * kx + ky * ky));
        if (radius < 1.4 || radius > 7.2) {
            continue;
        }
        state = state * 1664525U + 1013904223U;
        x = state;
        x ^= x >> 16U;
        x *= 0x7FEB352DU;
        x ^= x >> 15U;
        ReliefWave wave{};
        wave.kx = kx;
        wave.ky = ky;
        wave.amplitude = 1.0 / std::pow(radius, 1.75);
        wave.phase = static_cast<double>(x & 0xFFFFFFU) / 16777216.0;
        bound += wave.amplitude * radius;
        result.waves[count] = wave;
        ++count;
    }
    for (ReliefWave& wave : result.waves) {
        wave.amplitude /= bound * 0.45;
    }
    return result;
}

const ReliefWaves relief_waves = make_relief_waves();
constexpr double relief_tile = 0.36;   // lens units per wave tile
constexpr double grain_cell = 0.0045;  // lens units per grain cell

double grain_hash(int x, int y) {
    std::uint32_t h = static_cast<std::uint32_t>(x) * 0x8DA6B343U ^ static_cast<std::uint32_t>(y) * 0xD8163841U;
    h ^= h >> 13U;
    h *= 0x85EBCA6BU;
    h ^= h >> 16U;
    return static_cast<double>(h & 0xFFFFFFU) / 16777216.0 * 2.0 - 1.0;
}

void relief_slope(const Scene& scene, double x, double y, double& gx, double& gy) {
    gx = 0.0;
    gy = 0.0;
    if (scene.relief > 0.0 && lean.fast_relief) {
        // cos(m·a) and sin(m·a) for m = 0..7 by the angle-addition recurrence
        const double k = 2.0 * pi / relief_tile;
        std::array<double, 8> ca{};
        std::array<double, 8> sa{};
        std::array<double, 8> cb{};
        std::array<double, 8> sb{};
        ca[0] = 1.0;
        cb[0] = 1.0;
        ca[1] = std::cos(k * x);
        sa[1] = std::sin(k * x);
        cb[1] = std::cos(k * y);
        sb[1] = std::sin(k * y);
        for (std::size_t m = 2; m < 8U; ++m) {
            ca[m] = ca[m - 1U] * ca[1] - sa[m - 1U] * sa[1];
            sa[m] = sa[m - 1U] * ca[1] + ca[m - 1U] * sa[1];
            cb[m] = cb[m - 1U] * cb[1] - sb[m - 1U] * sb[1];
            sb[m] = sb[m - 1U] * cb[1] + cb[m - 1U] * sb[1];
        }
        for (const ReliefWave& wave : relief_waves.waves) {
            const std::size_t mx = static_cast<std::size_t>(std::abs(static_cast<int>(wave.kx)));
            const std::size_t my = static_cast<std::size_t>(std::abs(static_cast<int>(wave.ky)));
            const double cx = ca[mx];
            const double sx = wave.kx < 0.0 ? -sa[mx] : sa[mx];
            const double cy = cb[my];
            const double sy = wave.ky < 0.0 ? -sb[my] : sb[my];
            const double c_ab = cx * cy - sx * sy;
            const double s_ab = sx * cy + cx * sy;
            const double phase = 2.0 * pi * wave.phase;
            const double c = (c_ab * std::cos(phase) - s_ab * std::sin(phase)) * wave.amplitude;
            gx += c * wave.kx;
            gy += c * wave.ky;
        }
        gx *= scene.relief;
        gy *= scene.relief;
    } else if (scene.relief > 0.0) {
        const double k = 2.0 * pi / relief_tile;
        for (const ReliefWave& wave : relief_waves.waves) {
            const double c = std::cos(k * (wave.kx * x + wave.ky * y) + 2.0 * pi * wave.phase) * wave.amplitude;
            gx += c * wave.kx;
            gy += c * wave.ky;
        }
        gx *= scene.relief;
        gy *= scene.relief;
    }
    if (scene.grain > 0.0) {
        const double u = x / grain_cell;
        const double v = y / grain_cell;
        const int i = static_cast<int>(std::floor(u));
        const int j = static_cast<int>(std::floor(v));
        const double fu = u - i;
        const double fv = v - j;
        const double su = fu * fu * fu * (fu * (fu * 6.0 - 15.0) + 10.0);
        const double sv = fv * fv * fv * (fv * (fv * 6.0 - 15.0) + 10.0);
        const double du = 30.0 * fu * fu * (fu * (fu - 2.0) + 1.0);
        const double dv = 30.0 * fv * fv * (fv * (fv - 2.0) + 1.0);
        const double a = grain_hash(i, j);
        const double b = grain_hash(i + 1, j);
        const double c = grain_hash(i, j + 1);
        const double d = grain_hash(i + 1, j + 1);
        // h = a + (b − a)·su + (c − a)·sv + (a − b − c + d)·su·sv, per cell
        gx += scene.grain * 0.5 * ((b - a) + (a - b - c + d) * sv) * du;
        gy += scene.grain * 0.5 * ((c - a) + (a - b - c + d) * su) * dv;
    }
}

// The lens's normal, with the moulded relief on its top face only (on the
// chamfer, at grazing angles, it sparkled as a ragged rim). It is used both
// entering and leaving, so reflection and refraction see the same surface.
Vec3 lens_normal(const Scene& scene, Vec3 p) {
    Vec3 n{};
    if (lean.analytic && scene.family != Family::jewel) {
        n = normalized(box_lens_gradient(scene, p).g);
    } else {
        const double e = 2e-4;
        const double dx = lens_distance(scene, Vec3{p.x + e, p.y, p.z}) - lens_distance(scene, Vec3{p.x - e, p.y, p.z});
        const double dy = lens_distance(scene, Vec3{p.x, p.y + e, p.z}) - lens_distance(scene, Vec3{p.x, p.y - e, p.z});
        const double dz = lens_distance(scene, Vec3{p.x, p.y, p.z + e}) - lens_distance(scene, Vec3{p.x, p.y, p.z - e});
        n = normalized(Vec3{dx, dy, dz});
    }
    if (scene.family == Family::jewel || (scene.relief <= 0.0 && scene.grain <= 0.0 && scene.tilt_x == 0.0 && scene.tilt_y == 0.0) || n.z < 0.85) {
        return n;
    }
    double gx = 0.0;
    double gy = 0.0;
    relief_slope(scene, p.x, p.y, gx, gy);
    gx += scene.tilt_x;
    gy += scene.tilt_y;
    const double weight = smoothstep(0.85, 0.97, n.z);
    return normalized(Vec3{n.x - gx * weight, n.y - gy * weight, n.z});
}

Vec3 plate_normal(const Scene& scene, Vec3 p) {
    const double e = 2e-4;
    const double dx = plate_distance(scene, Vec3{p.x + e, p.y, p.z}) - plate_distance(scene, Vec3{p.x - e, p.y, p.z});
    const double dy = plate_distance(scene, Vec3{p.x, p.y + e, p.z}) - plate_distance(scene, Vec3{p.x, p.y - e, p.z});
    const double dz = plate_distance(scene, Vec3{p.x, p.y, p.z + e}) - plate_distance(scene, Vec3{p.x, p.y, p.z - e});
    return normalized(Vec3{dx, dy, dz});
}

// ---- light ----------------------------------------------------------------------------------

// A dark room with one large softbox above and in front of the panel.
double environment(Vec3 d) {
    if (d.z <= 0.0) {
        return 0.004;
    }
    const Vec3 key_light = normalized(Vec3{-0.15, -0.55, 1.0});
    const Vec3 u = normalized(cross(key_light, Vec3{0.0, 0.0, 1.0}));
    const Vec3 v = cross(u, key_light);
    const double c = dot(d, key_light);
    double box = 0.0;
    if (c > 0.0) {
        const double a = std::abs(dot(d, u)) / 0.42;
        const double b = std::abs(dot(d, v)) / 0.22;
        const double m = std::max(a, b);
        box = m < 1.0 ? 1.0 : std::max(0.0, 1.0 - (m - 1.0) * 6.0);
    }
    return 0.012 + 0.03 * d.z + 3.0 * box;
}

double fresnel_dielectric(double cos_i, double eta_i, double eta_t) {
    const double sin_t = eta_i / eta_t * std::sqrt(std::max(0.0, 1.0 - cos_i * cos_i));
    if (sin_t >= 1.0) {
        return 1.0;
    }
    const double cos_t = std::sqrt(std::max(0.0, 1.0 - sin_t * sin_t));
    const double rs = (eta_i * cos_i - eta_t * cos_t) / (eta_i * cos_i + eta_t * cos_t);
    const double rp = (eta_t * cos_i - eta_i * cos_t) / (eta_t * cos_i + eta_i * cos_t);
    return 0.5 * (rs * rs + rp * rp);
}

void basis(Vec3 n, Vec3& u, Vec3& v) {
    const Vec3 helper = std::abs(n.z) < 0.9 ? Vec3{0.0, 0.0, 1.0} : Vec3{1.0, 0.0, 0.0};
    u = normalized(cross(helper, n));
    v = cross(n, u);
}

// A fixed, well-spread set of cosine-weighted directions about n, rotated by
// `rotation`: a spiral on the disc lifted to the hemisphere.
Vec3 spiral_direction(int i, int count, double rotation, Vec3 n) {
    const double golden = 2.399963229728653;
    const double r1 = (i + 0.5) / count;
    const double r = std::sqrt(r1);
    const double phi = i * golden + rotation;
    Vec3 u{};
    Vec3 v{};
    basis(n, u, v);
    return normalized(add(add(scale(u, r * std::cos(phi)), scale(v, r * std::sin(phi))), scale(n, std::sqrt(1.0 - r1))));
}

double hashed_rotation(Vec3 p) {
    const std::uint32_t qx = static_cast<std::uint32_t>(static_cast<std::int32_t>(std::floor(p.x * 4096.0)));
    const std::uint32_t qy = static_cast<std::uint32_t>(static_cast<std::int32_t>(std::floor(p.y * 4096.0)));
    const std::uint32_t qz = static_cast<std::uint32_t>(static_cast<std::int32_t>(std::floor(p.z * 4096.0)));
    std::uint32_t h = qx * 0x27d4eb2dU ^ qy * 0x165667b1U ^ qz * 0x9e3779b1U;
    h = (h ^ (h >> 15U)) * 0x85ebca6bU;
    h = (h ^ (h >> 13U)) * 0xc2b2ae35U;
    h ^= h >> 16U;
    return 2.0 * pi * (h * (1.0 / 4294967296.0));
}

// The room's light on an upward-facing matte surface, per unit albedo
// (cosine-weighted mean of the environment), computed once.
double room_on_upward_surface() {
    double sum = 0.0;
    const int count = 4096;
    for (int i = 0; i < count; ++i) {
        sum += environment(spiral_direction(i, count, 0.0, Vec3{0.0, 0.0, 1.0}));
    }
    return sum / count;
}

// ---- the participating medium -----------------------------------------------------------

double axis_distance(const Scene& scene, const Lamp& lamp, Vec3 p) {
    const Vec3 a = sub(lamp.a, Vec3{0.0, 0.0, scene.press});
    const Vec3 b = sub(lamp.b, Vec3{0.0, 0.0, scene.press});
    const Vec3 ba = sub(b, a);
    const double h = std::clamp(dot(sub(p, a), ba) / dot(ba, ba), 0.0, 1.0);
    return length(sub(p, add(a, scale(ba, h))));
}

// Distance to the filament's wire. The coil runs along x; the nearest point is
// taken among the three turns at the point's own angle, which is close to
// exact while the pitch is small against the coil radius.
double filament_distance(const Scene& scene, const Lamp& lamp, Vec3 p) {
    const Vec3 c = sub(scale(add(lamp.a, lamp.b), 0.5), Vec3{0.0, 0.0, scene.press});
    const double half = 0.5 * (lamp.b.x - lamp.a.x);
    const Vec3 q = sub(p, c);
    const double theta = std::atan2(q.z, q.y);
    const double k0 = std::round(q.x / lamp.pitch - theta / (2.0 * pi));
    double best = 1e9;
    for (int k = -1; k <= 1; ++k) {
        const double x = std::clamp(lamp.pitch * (theta / (2.0 * pi) + k0 + k), -half, half);
        const double angle = 2.0 * pi * x / lamp.pitch;
        const Vec3 h{x, lamp.coil_radius * std::cos(angle), lamp.coil_radius * std::sin(angle)};
        best = std::min(best, length(sub(q, h)));
    }
    return best - lamp.wire_radius;
}

double lead_distance(const Scene& scene, const Lamp& lamp, Vec3 p) {
    const double foot = scene.family == Family::jewel ? -0.075 - scene.press : scene.bottom - scene.press + 0.01;
    double best = 1e9;
    for (int side = 0; side < 2; ++side) {
        const Vec3 top = sub(side == 0 ? lamp.a : lamp.b, Vec3{0.0, 0.0, scene.press});
        const double z = std::clamp(p.z, foot, top.z);
        best = std::min(best, length(sub(p, Vec3{top.x, top.y, z})));
    }
    return best - lamp.lead_radius;
}

constexpr double wire_density = 400.0;

struct MediumPoint final {
    Vec3 extinction{};
    std::array<Vec3, max_lamps> glow{};  // σ·S per lamp, at unit lamp power
    Vec3 room{};
};

MediumPoint medium_at(const Scene& scene, Vec3 p) {
    const Grade& g = scene.grade;
    const double soft = 0.0008;
    MediumPoint m{};
    double solid = 0.0;
    std::array<double, max_lamps> hot{};
    for (int i = 0; i < scene.lamp_count; ++i) {
        const Lamp& lamp = scene.lamps[static_cast<std::size_t>(i)];
        const double wire = scene.hidden_coil ? 0.0 : 1.0 - smoothstep(-soft, soft, filament_distance(scene, lamp, p));
        const double lead = scene.hidden_coil ? 0.0 : 1.0 - smoothstep(-soft, soft, lead_distance(scene, lamp, p));
        hot[static_cast<std::size_t>(i)] = wire;
        solid = std::max(solid, std::max(wire, lead));
    }
    if (scene.divider) {
        // an opaque sheet between the two halves, below the lens face
        solid = std::max(solid, 1.0 - smoothstep(0.003, 0.0045, std::abs(p.y)));
    }
    const double dense = wire_density * solid;
    m.extinction = add(add(g.density, g.pigment), Vec3{dense, dense, dense});
    for (int i = 0; i < scene.lamp_count; ++i) {
        const Lamp& lamp = scene.lamps[static_cast<std::size_t>(i)];
        const double beyond = std::max(0.0, axis_distance(scene, lamp, p) - lamp.coil_radius);
        const double reach = g.glow_reach;
        double diffused = reach * reach / (reach * reach + beyond * beyond);
        if (g.near_gain > 0.0) {
            const double near = g.near_reach;
            diffused += g.near_gain * near * near / (near * near + beyond * beyond);
        }
        if (scene.divider && (p.y > 0.0) != (lamp.a.y > 0.0)) {
            diffused = 0.0;  // the divider keeps each lamp's light to its own half
        }
        const double filament = wire_density * hot[static_cast<std::size_t>(i)] * scene.filament_radiance;
        m.glow[static_cast<std::size_t>(i)] = add(Vec3{filament, filament, filament},
                                                  scale(g.density, g.albedo * g.glow_gain * diffused * (1.0 - solid)));
    }
    m.room = scale(g.density, g.albedo * g.room * (1.0 - solid));
    return m;
}

double wire_clearance(const Scene& scene, Vec3 p) {
    if (scene.hidden_coil) {
        return 1.0;  // no wire to step round
    }
    double d = 1e9;
    for (int i = 0; i < scene.lamp_count; ++i) {
        const Lamp& lamp = scene.lamps[static_cast<std::size_t>(i)];
        d = std::min(d, std::min(filament_distance(scene, lamp, p), lead_distance(scene, lamp, p)));
    }
    return std::max(0.0, d);
}

struct Radiance final {
    Vec3 base{};
    std::array<Vec3, max_lamps> glow{};
};

Radiance radiance_add(const Radiance& a, const Radiance& b) {
    Radiance r{};
    r.base = add(a.base, b.base);
    for (int i = 0; i < max_lamps; ++i) {
        r.glow[static_cast<std::size_t>(i)] = add(a.glow[static_cast<std::size_t>(i)], b.glow[static_cast<std::size_t>(i)]);
    }
    return r;
}

Radiance radiance_scale(const Radiance& a, Vec3 w) {
    Radiance r{};
    r.base = mul(a.base, w);
    for (int i = 0; i < max_lamps; ++i) {
        r.glow[static_cast<std::size_t>(i)] = mul(a.glow[static_cast<std::size_t>(i)], w);
    }
    return r;
}

double channel(Vec3 v, int c) { return c == 0 ? v.x : (c == 1 ? v.y : v.z); }

void add_channel(Vec3& v, int c, double amount) {
    if (c == 0) {
        v.x += amount;
    } else if (c == 1) {
        v.y += amount;
    } else {
        v.z += amount;
    }
}

constexpr double lens_index = 1.49;

// One chord: midpoint quadrature of the source densities under the running
// transmittance, each step exact for constant σ and S over it, and shortened
// near the wires so that a wire a pixel wide is never stepped over.
// Gauss–Legendre nodes and weights on [0, 1] (weights sum to 1).
struct GaussRule final {
    std::array<double, 8> x{};
    std::array<double, 8> w{};
    int n{};
};

GaussRule gauss_rule(int n) {
    GaussRule r{};
    r.n = std::clamp(n, 1, 8);
    // roots of P_n by Newton from Chebyshev guesses
    for (int i = 0; i < r.n; ++i) {
        double z = std::cos(pi * (i + 0.75) / (r.n + 0.5));
        double dp = 1.0;
        for (int it = 0; it < 100; ++it) {
            double p0 = 1.0;
            double p1 = z;
            for (int k = 2; k <= r.n; ++k) {
                const double p2 = ((2.0 * k - 1.0) * z * p1 - (k - 1.0) * p0) / k;
                p0 = p1;
                p1 = p2;
            }
            const double pn = r.n == 1 ? z : p1;
            const double pm = r.n == 1 ? 1.0 : p0;
            dp = r.n * (z * pn - pm) / (z * z - 1.0);
            const double step = pn / dp;
            z -= step;
            if (std::abs(step) < 1e-15) {
                break;
            }
        }
        r.x[static_cast<std::size_t>(i)] = 0.5 * (1.0 - z);
        r.w[static_cast<std::size_t>(i)] = 1.0 / ((1.0 - z * z) * dp * dp);  // half of 2/((1−z²)P'²)
    }
    return r;
}

Vec3 integrate_chord(const Scene& scene, Vec3 origin, Vec3 direction, double chord, Radiance& out) {
    if (lean.gauss > 0 && scene.hidden_coil && !scene.divider) {
        static const GaussRule rule = gauss_rule(lean.gauss);
        const Grade& g = scene.grade;
        const double sigma = g.density.x + g.pigment.x;  // uniform and grey here
        const double end = std::exp(-sigma * chord);
        const double span = sigma > 1e-9 ? 1.0 - end : chord;  // in u = 1 − e^(−σt), or in t
        for (int j = 0; j < rule.n; ++j) {
            const double u = span * rule.x[static_cast<std::size_t>(j)];
            const double t = sigma > 1e-9 ? -std::log(1.0 - u) / sigma : u;
            const double weight = (sigma > 1e-9 ? span / sigma : span) * rule.w[static_cast<std::size_t>(j)];
            const MediumPoint m = medium_at(scene, add(origin, scale(direction, t)));
            // the source densities are σ·S; the weight carries the 1/σ of the substitution
            const double room = m.room.x * weight;
            out.base = add(out.base, Vec3{room, room, room});
            for (int i = 0; i < scene.lamp_count; ++i) {
                const double gl = m.glow[static_cast<std::size_t>(i)].x * weight;
                out.glow[static_cast<std::size_t>(i)] = add(out.glow[static_cast<std::size_t>(i)], Vec3{gl, gl, gl});
            }
        }
        return Vec3{end, end, end};
    }
    Vec3 transmittance{1.0, 1.0, 1.0};
    double t = 0.0;
    for (int step = 0; step < 800 && t < chord; ++step) {
        const double clearance = wire_clearance(scene, add(origin, scale(direction, t)));
        const double dt = std::min(chord - t, std::clamp(clearance * 0.5, 0.0006, lean.chord_step));
        const MediumPoint m = medium_at(scene, add(origin, scale(direction, t + 0.5 * dt)));
        t += dt;
        const Vec3 step_t = exp_neg(m.extinction, dt);
        for (int c = 0; c < (lean.grey ? 1 : 3); ++c) {
            const double sigma = channel(m.extinction, c);
            const double factor = sigma > 1e-9 ? (1.0 - channel(step_t, c)) / sigma : dt;
            const double w = channel(transmittance, c) * factor;
            add_channel(out.base, c, w * channel(m.room, c));
            for (int i = 0; i < scene.lamp_count; ++i) {
                add_channel(out.glow[static_cast<std::size_t>(i)], c, w * channel(m.glow[static_cast<std::size_t>(i)], c));
            }
        }
        if (lean.grey) {
            // the grade is grey: the other channels are the first's
            out.base.y = out.base.x;
            out.base.z = out.base.x;
            for (int i = 0; i < scene.lamp_count; ++i) {
                Vec3& g = out.glow[static_cast<std::size_t>(i)];
                g.y = g.x;
                g.z = g.x;
            }
        }
        transmittance = mul(transmittance, step_t);
        if (max_component(transmittance) < 1e-4) {
            break;
        }
    }
    return transmittance;
}

Radiance outside_radiance(const Scene& scene, Vec3 origin, Vec3 direction, int depth, bool gather);

// What covers the lens's outer face at a point with outward normal n.
struct Coat final {
    bool painted{};       // black paint: no light passes
    double ink{};         // share of the footprint under legend ink
    double ink_albedo{};  // the ink's reflectance (opaque inks)
    bool ink_glows{};     // dead front: the ink is a diffuser the lamps shine through
};

Coat coat_at(const Scene& scene, Vec3 p, Vec3 n) {
    Coat coat{};
    const bool face = n.z > 0.9;
    if (scene.black_back && n.z < -0.7) {
        coat.painted = true;
        return coat;
    }
    switch (scene.legend_style) {
    case Legend::dead_front:
        coat.painted = true;
        coat.ink = face ? legend_coverage(scene, p.x, p.y) : 0.0;
        coat.ink_albedo = 0.72;
        coat.ink_glows = true;
        return coat;
    case Legend::black_ink:
    case Legend::white_ink:
    case Legend::grey_ink:
        coat.painted = scene.sidewall_blocked && !face && n.z > -0.7;
        coat.ink = face ? legend_coverage(scene, p.x, p.y) : 0.0;
        coat.ink_albedo = scene.legend_style == Legend::white_ink ? 0.80 : (scene.legend_style == Legend::grey_ink ? 0.22 : 0.03);
        return coat;
    case Legend::lit_ink:
        coat.painted = scene.sidewall_blocked && !face && n.z > -0.7;
        coat.ink = face ? legend_coverage(scene, p.x, p.y) : 0.0;
        coat.ink_albedo = 0.72;
        coat.ink_glows = true;
        return coat;
    case Legend::none:
    default:
        coat.painted = scene.sidewall_blocked && !face && n.z > -0.7;
        return coat;
    }
}

// Irradiance from a lamp's coil at a point of a surface whose outward normal
// is `outward`, per unit lamp power: the coil seen as a cylinder of radiance
// filament_radiance, summed over eight lengths, each its projected area times
// the cosine at the surface over the squared distance. Lamps behind the
// divider do not reach the other half.
double lamp_irradiance(const Scene& scene, const Lamp& lamp, Vec3 p, Vec3 outward) {
    if (scene.divider && (p.y > 0.0) != (lamp.a.y > 0.0)) {
        return 0.0;
    }
    const Vec3 a = sub(lamp.a, Vec3{0.0, 0.0, scene.press});
    const Vec3 b = sub(lamp.b, Vec3{0.0, 0.0, scene.press});
    const int pieces = 8;
    const double piece = length(sub(b, a)) / pieces;
    double e = 0.0;
    for (int j = 0; j < pieces; ++j) {
        const Vec3 q = add(a, scale(sub(b, a), (j + 0.5) / pieces));
        const Vec3 to = sub(q, p);
        const double d2 = dot(to, to);
        const double cos_surface = -dot(to, outward) / std::sqrt(d2);  // the lamp is inside, opposite the outward normal
        if (cos_surface > 0.0) {
            e += 2.0 * lamp.coil_radius * piece * cos_surface / d2;
        }
    }
    return scene.filament_radiance * e;
}

double room_light = 0.0;  // set once in main: room_on_upward_surface()

// The legs inside a lens, from a point inside it: each chord contributes its
// source and retains T; each exit returns a port to the outside and the
// reflected remainder continues (the participating child law).
Radiance internal_response(const Scene& scene, Vec3 p, Vec3 d, Vec3 weight, int depth, bool gather) {
    Radiance out{};
    for (int leg = 0; leg < 10 && max_component(weight) > 2e-3; ++leg) {
        const double t_exit = march_inside(scene, p, d);
        Radiance chord{};
        const Vec3 retained = integrate_chord(scene, p, d, t_exit, chord);
        out = radiance_add(out, radiance_scale(chord, weight));
        weight = mul(weight, retained);
        p = add(p, scale(d, t_exit));
        const Vec3 outward = lens_normal(scene, p);
        if (lean.intervals) {
            sign_path(1000U + face_label(scene, p));
        }
        const double c = std::max(0.0, dot(d, outward));
        const Coat coat = coat_at(scene, p, outward);
        if (scene.grade.rear_diffuser > 0.0 && outward.z < -0.7) {
            // a frosted back: it re-emits the lamps' light diffusely and
            // takes the leg no further
            for (int i = 0; i < scene.lamp_count; ++i) {
                const double l = scene.grade.rear_diffuser * lamp_irradiance(scene, scene.lamps[static_cast<std::size_t>(i)], p, outward) / pi;
                out.glow[static_cast<std::size_t>(i)] = add(out.glow[static_cast<std::size_t>(i)], scale(weight, l));
            }
            break;
        }
        if (scene.grade.rear_reflector > 0.0 && outward.z < -0.7) {
            weight = scale(weight, scene.grade.rear_reflector);
        } else if (coat.painted) {
            // black paint takes almost everything; dead-front ink returns a
            // share diffusely, here kept along the mirror direction
            weight = scale(weight, 0.03 * (1.0 - coat.ink) + (coat.ink_glows ? 0.35 : coat.ink_albedo) * coat.ink);
        } else {
            // opaque ink blocks its share and reflects by its albedo
            const double f_out = fresnel_dielectric(c, lens_index, 1.0);
            if (lean.intervals) {
                sign_path(f_out >= 1.0 ? 2001U : 2000U);  // total internal reflection, or a way out
            }
            const double open = 1.0 - coat.ink;
            if (f_out < 1.0 && depth > 0 && open > 0.0) {
                const double k = 1.0 - lens_index * lens_index * (1.0 - c * c);
                const Vec3 t = normalized(add(scale(d, lens_index), scale(outward, std::sqrt(std::max(0.0, k)) - lens_index * c)));
                const Radiance beyond = outside_radiance(scene, add(p, scale(outward, 4e-4)), t, depth - 1, gather);
                out = radiance_add(out, radiance_scale(beyond, scale(weight, (1.0 - f_out) * open)));
            }
            weight = scale(weight, f_out * open + coat.ink_albedo * coat.ink);
        }
        d = reflected(d, outward);
        p = sub(p, scale(outward, 4e-4));
    }
    return out;
}

constexpr int diffuser_gather_count = 12;

// The legend's white ink is a thin diffuser: it passes on the light arriving
// from inside the lens, averaged over the hemisphere below it.
Radiance through_ink(const Scene& scene, Vec3 p, Vec3 n, int depth) {
    Radiance sum{};
    const Vec3 inward = scale(n, -1.0);
    const double rotation = hashed_rotation(p);
    const Vec3 start = sub(p, scale(n, 4e-4));
    for (int i = 0; i < diffuser_gather_count; ++i) {
        const Vec3 d = spiral_direction(i, diffuser_gather_count, rotation, inward);
        sum = radiance_add(sum, internal_response(scene, start, d, Vec3{1.0, 1.0, 1.0}, depth, false));
    }
    const double k = 1.0 / diffuser_gather_count;
    return radiance_scale(sum, Vec3{k, k, k});
}


Vec3 lens_normal(const Scene& scene, Vec3 p);

Vec3 snapped(const Scene& scene, Vec3 p) {
    for (int i = 0; i < 2; ++i) {
        const double d = lens_distance(scene, p);
        p = sub(p, scale(lens_normal(scene, p), d));
    }
    return p;
}

Radiance lens_response(const Scene& scene, Vec3 hit_position, Vec3 direction, int depth, bool gather) {
    const Vec3 position = lean.snap && !flat_point(scene, hit_position) ? snapped(scene, hit_position) : hit_position;
    Radiance out{};
    const Vec3 n = lens_normal(scene, position);
    const double cos_i = std::max(0.0, -dot(direction, n));
    const double f_in = fresnel_dielectric(cos_i, 1.0, lens_index);
    if (depth > 0) {
        const Radiance mirror = outside_radiance(scene, add(position, scale(n, 4e-4)), reflected(direction, n), depth - 1, gather);
        out = radiance_add(out, radiance_scale(mirror, Vec3{f_in, f_in, f_in}));
    } else {
        const double e = f_in * environment(reflected(direction, n));
        out.base = add(out.base, Vec3{e, e, e});
    }
    const Coat coat = coat_at(scene, position, n);
    if (coat.painted) {
        // Gloss black paint; on a dead front, legend in white diffuser ink
        // which the room lights from outside and the lamps from inside.
        const double ink = coat.ink_glows ? coat.ink : 0.0;
        const double under = 1.0 - f_in;
        const double paint = 0.025 * room_light * (1.0 - ink) * under;
        out.base = add(out.base, Vec3{paint, paint, paint});
        if (ink > 0.0) {
            const double lit = 0.72 * room_light * ink * under;
            out.base = add(out.base, Vec3{lit, lit, lit});
            if (depth > 0) {
                const Radiance inner = through_ink(scene, position, n, depth - 1);
                out = radiance_add(out, radiance_scale(inner, Vec3{0.6 * ink * under, 0.6 * ink * under, 0.6 * ink * under}));
            }
        }
        return out;
    }
    // diffuser ink on an open face: lit by the room from outside and by the
    // lamps from inside, through it
    if (coat.ink_glows && coat.ink > 0.0) {
        const double under = 1.0 - f_in;
        const double lit = 0.72 * room_light * coat.ink * under;
        out.base = add(out.base, Vec3{lit, lit, lit});
        if (depth > 0) {
            const Radiance inner = through_ink(scene, position, n, depth - 1);
            const double k = 0.6 * coat.ink * under;
            out = radiance_add(out, radiance_scale(inner, Vec3{k, k, k}));
        }
        // and the lamps' own light falling on the ink from below, passed on
        // diffusely (as a frosted back passes it): the letters glow
        for (int i = 0; i < scene.lamp_count; ++i) {
            const double l = 0.6 * coat.ink * under * lamp_irradiance(scene, scene.lamps[static_cast<std::size_t>(i)], position, n) / pi;
            out.glow[static_cast<std::size_t>(i)] = add(out.glow[static_cast<std::size_t>(i)], Vec3{l, l, l});
        }
    } else if (coat.ink > 0.0) {
        // opaque ink on an unpainted face: lit by the room only, and it blocks
        // its share of the view into the lens
        const double lit = coat.ink * coat.ink_albedo * room_light * (1.0 - f_in);
        out.base = add(out.base, Vec3{lit, lit, lit});
    }
    const double open = (1.0 - f_in) * (1.0 - coat.ink);
    if (open <= 0.0) {
        return out;
    }
    const double eta = 1.0 / lens_index;
    const double cos_t2 = 1.0 - eta * eta * (1.0 - cos_i * cos_i);
    const Vec3 d = normalized(add(scale(direction, eta), scale(n, eta * cos_i - std::sqrt(std::max(0.0, cos_t2)))));
    return radiance_add(out, internal_response(scene, sub(position, scale(n, 4e-4)), d, Vec3{open, open, open}, depth, gather));
}

// The panel face gathers 16 directions; the well's interior, which a clear
// lens magnifies, gathers 48.
constexpr int plate_gather_count = 16;
constexpr int well_gather_count = 48;

Radiance plate_response(const Scene& scene, Vec3 position, Vec3 direction, int depth, bool gather) {
    Radiance out{};
    const Vec3 n = plate_normal(scene, position);
    const bool top_face = position.z > -1e-3 && n.z > 0.9;
    const double albedo = top_face ? 0.035 : 0.10;
    if (top_face) {
        // satin coat: the room reflected, softly
        const double coat = fresnel_dielectric(std::max(0.0, -dot(direction, n)), 1.0, 1.5);
        const Vec3 r = reflected(direction, n);
        const double e = coat * 0.5 * (environment(r) + environment(normalized(add(r, Vec3{0.08, 0.05, 0.0}))));
        out.base = add(out.base, Vec3{e, e, e});
    }
    if (!gather || depth <= 0) {
        const double e = albedo * room_light;
        out.base = add(out.base, Vec3{e, e, e});
        return out;
    }
    const double rotation = hashed_rotation(position);
    const Vec3 lifted = add(position, scale(n, 4e-4));
    const int count = top_face ? plate_gather_count : well_gather_count;
    Radiance sum{};
    for (int i = 0; i < count; ++i) {
        sum = radiance_add(sum, outside_radiance(scene, lifted, spiral_direction(i, count, rotation, n), depth - 1, false));
    }
    const double k = albedo / count;
    return radiance_add(out, radiance_scale(sum, Vec3{k, k, k}));
}

Radiance outside_radiance(const Scene& scene, Vec3 origin, Vec3 direction, int depth, bool gather) {
    if (lean.convex && scene.control_only && scene.family != Family::jewel && lens_distance(scene, origin) >= 0.0) {
        // leaving (or reflected off) the lens: nothing else is in the scene
        const double e = environment(direction);
        Radiance r{};
        r.base = Vec3{e, e, e};
        return r;
    }
    const Hit hit = march_outside(scene, origin, direction);
    if (hit.surface == Surface::none) {
        const double e = environment(direction);
        Radiance r{};
        r.base = Vec3{e, e, e};
        return r;
    }
    const Vec3 p = add(origin, scale(direction, hit.t));
    if (hit.surface == Surface::lens) {
        return lens_response(scene, p, direction, depth, gather);
    }
    return plate_response(scene, p, direction, depth, gather);
}

// ---- camera and images ----------------------------------------------------------------------

struct Image final {
    int width{};
    int height{};
    int lamps{};
    std::vector<float> base;                         // width × height × 3, linear
    std::array<std::vector<float>, max_lamps> glow;  // per lamp, at unit white power
    std::vector<float> alpha;                        // width × height: the control's coverage
};

// Head-on, as a control is seen on screen, from a little in front.
struct Camera final {
    Vec3 eye{0.0, -0.32, 2.6};
    Vec3 target{0.0, 0.0, 0.08};
    double field_of_view{21.0};  // degrees, vertical
};

struct RowJob final {
    const Scene* scene{};
    int grid{};
    int first_row{};
    int row_step{};
    Image* image{};
};

// One control-only sample: its light (zero when it misses), whether it hit
// the lens, and whether the legend is near its hit point.
struct Sample final {
    Radiance light{};
    bool hit{};
    bool near_legend{};
    Vec3 shape_normal{};  // the lens's normal without the relief (where its form turns)
};

Sample control_sample(const Scene& scene, double sx, double sy, double near_distance) {
    Sample sample{};
    const Vec3 above = scene.control_eye;
    const Vec3 direction = normalized(sub(Vec3{sx * scene.frame_half_x, scene.frame_centre_y + sy * scene.frame_half_y, 0.0}, above));
    const Hit hit = march_outside(scene, above, direction);
    if (hit.surface == Surface::lens) {
        const Vec3 p = add(above, scale(direction, hit.t));
        sample.light = lens_response(scene, p, direction, 3, false);
        sample.hit = true;
        if (lean.adaptive > 0.0) {
            // the distance field's gradient: the form's normal, without the relief
            const double e = 2e-4;
            const double gx = lens_distance(scene, Vec3{p.x + e, p.y, p.z}) - lens_distance(scene, Vec3{p.x - e, p.y, p.z});
            const double gy = lens_distance(scene, Vec3{p.x, p.y + e, p.z}) - lens_distance(scene, Vec3{p.x, p.y - e, p.z});
            const double gz = lens_distance(scene, Vec3{p.x, p.y, p.z + e}) - lens_distance(scene, Vec3{p.x, p.y, p.z - e});
            sample.shape_normal = normalized(Vec3{gx, gy, gz});
        }
        if (!scene.legend.empty()) {
            double d = 1e9;
            for (const Segment& segment : scene.legend) {
                d = std::min(d, segment_distance(segment, p.x, p.y));
            }
            sample.near_legend = d < scene.legend_half_width + near_distance;
        }
    }
    return sample;
}

void render_rows(RowJob job) {
    const Scene& scene = *job.scene;
    Image& image = *job.image;
    Camera camera{};
    camera.field_of_view = scene.field_of_view;
    const Vec3 forward = normalized(sub(camera.target, camera.eye));
    const Vec3 right = normalized(cross(forward, Vec3{0.0, 1.0, 0.0}));
    const Vec3 up = cross(right, forward);
    const double half = std::tan(camera.field_of_view * pi / 360.0);
    const int w = image.width;
    const int h = image.height;
    const int g = job.grid;
    for (int y = job.first_row; y < h; y += job.row_step) {
        for (int x = 0; x < w; ++x) {
            Radiance sum{};
            int covered = 0;
            for (int s = 0; s < g * g; ++s) {
                const double sx = (x + ((s % g) + 0.5) / g) / w * 2.0 - 1.0;
                const double sy = 1.0 - (y + ((s / g) + 0.5) / g) / h * 2.0;
                if (scene.control_only) {
                    const Sample sample = control_sample(scene, sx, sy, 0.0);
                    if (sample.hit) {
                        sum = radiance_add(sum, sample.light);
                        ++covered;
                    }
                    continue;
                }
                const Vec3 direction = normalized(add(forward, add(scale(right, sx * half), scale(up, sy * half))));
                sum = radiance_add(sum, outside_radiance(scene, camera.eye, direction, 3, true));
                ++covered;
            }
            const double inv = 1.0 / (g * g);
            image.alpha[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] = static_cast<float>(covered * inv);
            const std::size_t at = (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)) * 3U;
            image.base[at] = static_cast<float>(sum.base.x * inv);
            image.base[at + 1U] = static_cast<float>(sum.base.y * inv);
            image.base[at + 2U] = static_cast<float>(sum.base.z * inv);
            for (int i = 0; i < image.lamps; ++i) {
                const Vec3 v = sum.glow[static_cast<std::size_t>(i)];
                std::vector<float>& out = image.glow[static_cast<std::size_t>(i)];
                out[at] = static_cast<float>(v.x * inv);
                out[at + 1U] = static_cast<float>(v.y * inv);
                out[at + 2U] = static_cast<float>(v.z * inv);
            }
        }
    }
}

// Adaptive: the first pass takes each pixel's centre sample; the second gives
// the full grid only to pixels that need it, and the rest keep their centre.
struct AdaptiveJob final {
    const Scene* scene{};
    int grid{};
    int first_row{};
    int row_step{};
    int pass{};
    std::vector<Sample>* centres{};
    std::vector<unsigned char>* refined{};
    Image* image{};
};

double light_level(const Radiance& r, int lamps) {
    double v = r.base.x;
    for (int i = 0; i < lamps; ++i) {
        v += r.glow[static_cast<std::size_t>(i)].x;
    }
    return v;
}

void write_pixel(Image& image, int x, int y, const Radiance& sum, double inv, double coverage) {
    const int w = image.width;
    image.alpha[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] = static_cast<float>(coverage);
    const std::size_t at = (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)) * 3U;
    image.base[at] = static_cast<float>(sum.base.x * inv);
    image.base[at + 1U] = static_cast<float>(sum.base.y * inv);
    image.base[at + 2U] = static_cast<float>(sum.base.z * inv);
    for (int i = 0; i < image.lamps; ++i) {
        const Vec3 v = sum.glow[static_cast<std::size_t>(i)];
        std::vector<float>& out = image.glow[static_cast<std::size_t>(i)];
        out[at] = static_cast<float>(v.x * inv);
        out[at + 1U] = static_cast<float>(v.y * inv);
        out[at + 2U] = static_cast<float>(v.z * inv);
    }
}

void render_adaptive_rows(AdaptiveJob job) {
    const Scene& scene = *job.scene;
    Image& image = *job.image;
    std::vector<Sample>& centres = *job.centres;
    const int w = image.width;
    const int h = image.height;
    const int g = job.grid;
    const double pixel_units = 2.0 * scene.frame_half_x / w;
    for (int y = job.first_row; y < h; y += job.row_step) {
        for (int x = 0; x < w; ++x) {
            const std::size_t k = static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x);
            if (job.pass == 0) {
                const double sx = (x + 0.5) / w * 2.0 - 1.0;
                const double sy = 1.0 - (y + 0.5) / h * 2.0;
                centres[k] = control_sample(scene, sx, sy, 2.0 * pixel_units);
                continue;
            }
            const Sample& centre = centres[k];
            bool refine = centre.near_legend;
            const double here = light_level(centre.light, image.lamps);
            for (int dy = -1; dy <= 1 && !refine; ++dy) {
                for (int dx = -1; dx <= 1 && !refine; ++dx) {
                    const int nx = std::clamp(x + dx, 0, w - 1);
                    const int ny = std::clamp(y + dy, 0, h - 1);
                    const Sample& other = centres[static_cast<std::size_t>(ny) * static_cast<std::size_t>(w) + static_cast<std::size_t>(nx)];
                    if (other.hit != centre.hit) {
                        refine = true;
                    } else if (centre.hit && dot(other.shape_normal, centre.shape_normal) < lean.turn) {
                        refine = true;  // the form turns here: a rounded edge may hold a highlight thinner than a pixel
                    } else if (centre.hit) {
                        const double there = light_level(other.light, image.lamps);
                        refine = std::abs(there - here) > lean.adaptive * std::max(0.05, std::max(here, there));
                    }
                }
            }
            (*job.refined)[k] = refine ? 1U : 0U;
            if (!refine) {
                write_pixel(image, x, y, centre.light, 1.0, centre.hit ? 1.0 : 0.0);
                continue;
            }
            Radiance sum{};
            int covered = 0;
            for (int s = 0; s < g * g; ++s) {
                const double sx = (x + ((s % g) + 0.5) / g) / w * 2.0 - 1.0;
                const double sy = 1.0 - (y + ((s / g) + 0.5) / g) / h * 2.0;
                const Sample sample = control_sample(scene, sx, sy, 0.0);
                if (sample.hit) {
                    sum = radiance_add(sum, sample.light);
                    ++covered;
                }
            }
            const double inv = 1.0 / (g * g);
            write_pixel(image, x, y, sum, inv, covered * inv);
        }
    }
}

std::size_t refined_pixels = 0;  // the last adaptive render's refined pixel count

Image render(const Scene& scene, int width, int height, int grid) {
    Image image{};
    image.width = width;
    image.height = height;
    image.lamps = scene.lamp_count;
    const std::size_t pixels = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    const std::size_t count = pixels * 3U;
    image.base.assign(count, 0.0F);
    image.alpha.assign(pixels, 0.0F);
    for (int i = 0; i < image.lamps; ++i) {
        image.glow[static_cast<std::size_t>(i)].assign(count, 0.0F);
    }
    const unsigned threads = std::max(1U, std::thread::hardware_concurrency());
    if (lean.adaptive > 0.0 && scene.control_only) {
        std::vector<Sample> centres(pixels);
        std::vector<unsigned char> refined(pixels, 0U);
        for (int pass = 0; pass < 2; ++pass) {
            std::vector<std::thread> workers;
            workers.reserve(threads);
            for (unsigned i = 0; i < threads; ++i) {
                workers.emplace_back(render_adaptive_rows, AdaptiveJob{&scene, grid, static_cast<int>(i), static_cast<int>(threads), pass, &centres, &refined, &image});
            }
            for (std::thread& worker : workers) {
                worker.join();
            }
        }
        refined_pixels = 0;
        for (const unsigned char r : refined) {
            refined_pixels += r;
        }
        return image;
    }
    std::vector<std::thread> workers;
    workers.reserve(threads);
    for (unsigned i = 0; i < threads; ++i) {
        workers.emplace_back(render_rows, RowJob{&scene, grid, static_cast<int>(i), static_cast<int>(threads), &image});
    }
    for (std::thread& worker : workers) {
        worker.join();
    }
    return image;
}

// ---- split bake: surface at full resolution, transmitted light reduced -----------------------

double ink_albedo_of(Legend legend) {
    switch (legend) {
    case Legend::white_ink: return 0.80;
    case Legend::grey_ink: return 0.22;
    case Legend::black_ink: return 0.03;
    case Legend::none:
    case Legend::dead_front:
    case Legend::lit_ink:
    default: return 0.0;
    }
}

// The surface at full resolution: per pixel, premultiplied by coverage, the
// room reflected in the outer face, the ink's share, the ink's own light
// (its albedo × the room × (1 − F)) times its share, and the coverage.
struct SurfaceImage final {
    int width{};
    int height{};
    std::vector<float> reflection;
    std::vector<float> ink;
    std::vector<float> ink_light;
    std::vector<float> coverage;
    std::vector<float> slope_x;  // the relief's slope at the hit, times coverage (first-order join)
    std::vector<float> slope_y;
};

struct SplitJob final {
    const Scene* scene{};
    const Scene* bare{};  // the scene without its legend: the transmitted light's
    int grid{};
    int first_row{};
    int row_step{};
    SurfaceImage* surface{};
    Image* inside{};      // reduced resolution
};

constexpr double tilt_step = 0.02;  // the first-order join's finite difference

// One camera ray's hit: its snapped point and the outer face's response.
struct SurfaceSample final {
    bool hit{};
    Vec3 position{};
    Vec3 direction{};
    double reflection{};
    double ink{};
    double ink_light{};
    double slope_x{};
    double slope_y{};
};

SurfaceSample surface_sample(const Scene& scene, double sx, double sy) {
    SurfaceSample out{};
    const Vec3 above = scene.control_eye;
    const Vec3 direction = normalized(sub(Vec3{sx * scene.frame_half_x, scene.frame_centre_y + sy * scene.frame_half_y, 0.0}, above));
    const Hit hit = march_outside(scene, above, direction);
    if (hit.surface != Surface::lens) {
        return out;
    }
    const Vec3 raw = add(above, scale(direction, hit.t));
    const Vec3 p = lean.snap && !flat_point(scene, raw) ? snapped(scene, raw) : raw;
    const Vec3 n = lens_normal(scene, p);
    const double cos_i = std::max(0.0, -dot(direction, n));
    const double f_in = fresnel_dielectric(cos_i, 1.0, lens_index);
    out.hit = true;
    out.position = p;
    out.direction = direction;
    out.reflection = f_in * environment(reflected(direction, n));
    const Coat coat = coat_at(scene, p, n);
    out.ink = coat.ink;
    out.ink_light = ink_albedo_of(scene.legend_style) * room_light * (1.0 - f_in);
    // the relief's slope where it applies (the top), as lens_normal weighs it
    const Vec3 rough = lens_normal(scene, p);
    (void)rough;
    double gx = 0.0;
    double gy = 0.0;
    relief_slope(scene, p.x, p.y, gx, gy);
    {
        const double e = 2e-4;
        const double nx = lens_distance(scene, Vec3{p.x + e, p.y, p.z}) - lens_distance(scene, Vec3{p.x - e, p.y, p.z});
        const double ny = lens_distance(scene, Vec3{p.x, p.y + e, p.z}) - lens_distance(scene, Vec3{p.x, p.y - e, p.z});
        const double nz = lens_distance(scene, Vec3{p.x, p.y, p.z + e}) - lens_distance(scene, Vec3{p.x, p.y, p.z - e});
        const Vec3 form = normalized(Vec3{nx, ny, nz});
        const double weight = form.z < 0.85 ? 0.0 : smoothstep(0.85, 0.97, form.z);
        out.slope_x = gx * weight;
        out.slope_y = gy * weight;
    }
    return out;
}

void render_surface_rows(SplitJob job) {
    const Scene& scene = *job.scene;
    SurfaceImage& image = *job.surface;
    const int w = image.width;
    const int h = image.height;
    const int g = job.grid;
    for (int y = job.first_row; y < h; y += job.row_step) {
        for (int x = 0; x < w; ++x) {
            double reflection = 0.0;
            double ink = 0.0;
            double ink_light = 0.0;
            double slope_x = 0.0;
            double slope_y = 0.0;
            int covered = 0;
            for (int s = 0; s < g * g; ++s) {
                const double sx = (x + ((s % g) + 0.5) / g) / w * 2.0 - 1.0;
                const double sy = 1.0 - (y + ((s / g) + 0.5) / g) / h * 2.0;
                const SurfaceSample sample = surface_sample(scene, sx, sy);
                if (sample.hit) {
                    reflection += sample.reflection;
                    ink += sample.ink;
                    ink_light += sample.ink * sample.ink_light;
                    slope_x += sample.slope_x;
                    slope_y += sample.slope_y;
                    ++covered;
                }
            }
            const double inv = 1.0 / (g * g);
            const std::size_t k = static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x);
            image.reflection[k] = static_cast<float>(reflection * inv);
            image.ink[k] = static_cast<float>(ink * inv);
            image.ink_light[k] = static_cast<float>(ink_light * inv);
            image.coverage[k] = static_cast<float>(covered * inv);
            image.slope_x[k] = static_cast<float>(slope_x * inv);
            image.slope_y[k] = static_cast<float>(slope_y * inv);
        }
    }
}

// The transmitted light: the bare lens's whole response less its outer
// reflection, at the reduced resolution.
void render_inside_rows(SplitJob job) {
    const Scene& bare = *job.bare;
    Image& image = *job.inside;
    const int w = image.width;
    const int h = image.height;
    const int g = job.grid;
    for (int y = job.first_row; y < h; y += job.row_step) {
        for (int x = 0; x < w; ++x) {
            Radiance sum{};
            int covered = 0;
            for (int s = 0; s < g * g; ++s) {
                const double sx = (x + ((s % g) + 0.5) / g) / w * 2.0 - 1.0;
                const double sy = 1.0 - (y + ((s / g) + 0.5) / g) / h * 2.0;
                const SurfaceSample surface = surface_sample(bare, sx, sy);
                if (!surface.hit) {
                    continue;
                }
                Radiance light = lens_response(bare, surface.position, surface.direction, 3, false);
                light.base = sub(light.base, Vec3{surface.reflection, surface.reflection, surface.reflection});
                sum = radiance_add(sum, light);
                ++covered;
            }
            const double inv = 1.0 / (g * g);
            write_pixel(image, x, y, sum, inv, covered * inv);
        }
    }
}

// ---- the engine's certified scanline intervals (render_adaptive, ported) ---------------------
//
// Per row, each run of pixels whose centre rays enter the same face is one
// interval. Five exact controls (at 0, 1/4, 1/2, 3/4, 1) give a CONV profile;
// twelve exact witnesses (at the sixteenths between them) must carry the same
// optical path signature and land within the tolerance in 8-bit display
// values (the light alone and the light with every lamp at unit power), or
// the interval is split in two. Intervals shorter than five are exact.
// One sample per pixel (its centre), as the engine's camera takes.
struct IntervalCounts final {
    std::uint64_t exact{};
    std::uint64_t interpolated{};
    std::uint64_t splits{};
};
std::atomic<std::uint64_t> interval_exact{0};
std::atomic<std::uint64_t> interval_interpolated{0};
std::atomic<std::uint64_t> interval_splits{0};

std::uint8_t encode(double v);

std::uint8_t display_byte(double v) {
    const double toned = 1.0 - std::exp(-1.3 * std::max(0.0, v));
    return encode(toned);
}

void render_inside_intervals(const Scene& bare, Image& image, std::atomic<int>& next_row) {
    const int w = image.width;
    const int h = image.height;
    const int lanes = 1 + image.lamps;
    struct Pixel final {
        bool hit{};
        bool known{};
        std::uint64_t entry{};
        std::uint64_t label{};
        std::array<float, 1 + max_lamps> value{};
        std::array<std::uint8_t, 2> bytes{};  // off, on
        SurfaceSample surface{};
    };
    std::vector<Pixel> row(static_cast<std::size_t>(w));
    std::vector<float> synthesized;
    IntervalCounts counts{};
    for (;;) {
        const int y = next_row.fetch_add(1);
        if (y >= h) {
            break;
        }
        for (int x = 0; x < w; ++x) {
            Pixel& px = row[static_cast<std::size_t>(x)];
            px = Pixel{};
            const double sx = (x + 0.5) / w * 2.0 - 1.0;
            const double sy = 1.0 - (y + 0.5) / h * 2.0;
            px.surface = surface_sample(bare, sx, sy);
            px.hit = px.surface.hit;
            px.entry = px.hit ? face_label(bare, px.surface.position) : 0U;
        }
        const auto evaluate = [&](int x) -> Pixel& {
            Pixel& px = row[static_cast<std::size_t>(x)];
            if (px.known) {
                return px;
            }
            px.known = true;
            ++counts.exact;
            path_signature = 0;
            sign_path(px.entry);
            Radiance light = lens_response(bare, px.surface.position, px.surface.direction, 3, false);
            px.label = path_signature;
            double on = light.base.x - px.surface.reflection;
            px.value[0] = static_cast<float>(light.base.x - px.surface.reflection);
            for (int i = 0; i < image.lamps; ++i) {
                px.value[static_cast<std::size_t>(i) + 1U] = static_cast<float>(light.glow[static_cast<std::size_t>(i)].x);
                on += light.glow[static_cast<std::size_t>(i)].x;
            }
            px.bytes[0] = display_byte(px.value[0]);
            px.bytes[1] = display_byte(on);
            return px;
        };
        const auto store = [&](int x, const float* value) {
            Radiance r{};
            r.base = Vec3{value[0], value[0], value[0]};
            for (int i = 0; i < image.lamps; ++i) {
                const double g = value[i + 1];
                r.glow[static_cast<std::size_t>(i)] = Vec3{g, g, g};
            }
            write_pixel(image, x, y, r, 1.0, 1.0);
        };
        const auto exact_segment = [&](int a, int b) {
            for (int x = a; x <= b; ++x) {
                store(x, evaluate(x).value.data());
            }
        };
        // recursive bisection, as the engine's `approximate`
        struct Approximate final {
            static void run(int a, int b, const decltype(evaluate)& evaluate_ref, const decltype(exact_segment)& exact_ref,
                            const decltype(store)& store_ref, int lanes_count, std::vector<float>& synth, IntervalCounts& c) {
                const int length = b - a + 1;
                if (length < 5) {
                    exact_ref(a, b);
                    return;
                }
                constexpr std::array<double, 5> controls{{0.0, 0.25, 0.5, 0.75, 1.0}};
                constexpr std::array<double, 12> probes{{1.0 / 16, 2.0 / 16, 3.0 / 16, 5.0 / 16, 6.0 / 16, 7.0 / 16, 9.0 / 16, 10.0 / 16, 11.0 / 16, 13.0 / 16, 14.0 / 16, 15.0 / 16}};
                std::array<int, 5> sites{};
                std::array<float, 5 * (1 + max_lamps)> source{};
                for (std::size_t i = 0; i < 5U; ++i) {
                    sites[i] = std::clamp(static_cast<int>(std::lround(a + controls[i] * (b - a))), a, b);
                    const auto& px = evaluate_ref(sites[i]);
                    for (int l = 0; l < lanes_count; ++l) {
                        source[i * static_cast<std::size_t>(lanes_count) + static_cast<std::size_t>(l)] = px.value[static_cast<std::size_t>(l)];
                    }
                }
                bool reject = false;
                const std::uint64_t label = evaluate_ref(sites[0]).label;
                for (std::size_t i = 1; i < 5U; ++i) {
                    reject = reject || evaluate_ref(sites[i]).label != label;
                }
                std::array<int, 12> probe{};
                std::array<float, 12> positions{};
                for (std::size_t i = 0; !reject && i < 12U; ++i) {
                    probe[i] = std::clamp(static_cast<int>(std::lround(a + probes[i] * (b - a))), a, b);
                    if (evaluate_ref(probe[i]).label != label) {
                        reject = true;
                    }
                    positions[i] = static_cast<float>(4.0 * (probe[i] - a) / (b - a));
                }
                std::array<float, 20 * (1 + max_lamps)> prepared{};
                std::array<float, 12 * (1 + max_lamps)> predicted{};
                if (!reject) {
                    if (conv_prepare_profile_f32(source.data(), 5, lanes_count, prepared.data()) != 0 ||
                        conv_evaluate_prepared_profile_f32(source.data(), prepared.data(), 5, lanes_count, positions.data(), predicted.data(), 12) != 0) {
                        reject = true;
                    }
                }
                for (std::size_t i = 0; !reject && i < 12U; ++i) {
                    const auto& px = evaluate_ref(probe[i]);
                    double on = predicted[i * static_cast<std::size_t>(lanes_count)];
                    for (int l = 1; l < lanes_count; ++l) {
                        on += predicted[i * static_cast<std::size_t>(lanes_count) + static_cast<std::size_t>(l)];
                    }
                    const int off_byte = display_byte(predicted[i * static_cast<std::size_t>(lanes_count)]);
                    const int on_byte = display_byte(on);
                    if (std::abs(off_byte - px.bytes[0]) > lean.interval_tolerance || std::abs(on_byte - px.bytes[1]) > lean.interval_tolerance) {
                        reject = true;
                    }
                }
                if (reject) {
                    ++c.splits;
                    const int middle = (a + b) / 2;
                    if (middle <= a || middle >= b) {
                        exact_ref(a, b);
                        return;
                    }
                    run(a, middle, evaluate_ref, exact_ref, store_ref, lanes_count, synth, c);
                    run(middle + 1, b, evaluate_ref, exact_ref, store_ref, lanes_count, synth, c);
                    return;
                }
                synth.resize(static_cast<std::size_t>(length) * static_cast<std::size_t>(lanes_count));
                if (conv_resize_prepared_lines_f32(source.data(), prepared.data(), 5, lanes_count, synth.data(), length) != 0) {
                    exact_ref(a, b);
                    return;
                }
                for (int x = a; x <= b; ++x) {
                    bool control = false;
                    for (const int site : sites) {
                        control = control || x == site;
                    }
                    if (control) {
                        store_ref(x, evaluate_ref(x).value.data());
                    } else {
                        store_ref(x, synth.data() + static_cast<std::size_t>(x - a) * static_cast<std::size_t>(lanes_count));
                        ++c.interpolated;
                    }
                }
            }
        };
        int begin = 0;
        while (begin < w) {
            const Pixel& first = row[static_cast<std::size_t>(begin)];
            int end = begin;
            while (end + 1 < w && row[static_cast<std::size_t>(end) + 1U].hit == first.hit && row[static_cast<std::size_t>(end) + 1U].entry == first.entry) {
                ++end;
            }
            if (first.hit) {
                Approximate::run(begin, end, evaluate, exact_segment, store, lanes, synthesized, counts);
            }
            begin = end + 1;
        }
    }
    interval_exact += counts.exact;
    interval_interpolated += counts.interpolated;
    interval_splits += counts.splits;
}

struct SplitBake final {
    SurfaceImage surface{};
    Image inside{};
    Image inside_dx{};  // first order: (inside at tilt (step, 0) − inside) / step
    Image inside_dy{};
    double surface_seconds{};
    double inside_seconds{};
};

SplitBake render_split(const Scene& scene, int width, int height) {
    SplitBake bake{};
    Scene bare = scene;
    bare.legend.clear();
    bare.legend_style = Legend::none;
    if (lean.t_smooth || lean.t_linear) {
        bare.relief = 0.0;
        bare.grain = 0.0;
    }
    const std::size_t pixels = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    bake.surface.width = width;
    bake.surface.height = height;
    bake.surface.reflection.assign(pixels, 0.0F);
    bake.surface.ink.assign(pixels, 0.0F);
    bake.surface.ink_light.assign(pixels, 0.0F);
    bake.surface.coverage.assign(pixels, 0.0F);
    bake.surface.slope_x.assign(pixels, 0.0F);
    bake.surface.slope_y.assign(pixels, 0.0F);
    const int iw = (width + lean.t_scale - 1) / lean.t_scale;
    const int ih = (height + lean.t_scale - 1) / lean.t_scale;
    bake.inside.width = iw;
    bake.inside.height = ih;
    bake.inside.lamps = scene.lamp_count;
    const std::size_t inside_pixels = static_cast<std::size_t>(iw) * static_cast<std::size_t>(ih);
    bake.inside.base.assign(inside_pixels * 3U, 0.0F);
    bake.inside.alpha.assign(inside_pixels, 0.0F);
    for (int i = 0; i < bake.inside.lamps; ++i) {
        bake.inside.glow[static_cast<std::size_t>(i)].assign(inside_pixels * 3U, 0.0F);
    }
    // The reduced image spans the same frame: its pixels are t_scale times
    // larger, so the frame is widened to the rounded-up size.
    Scene bare_frame = bare;
    bare_frame.frame_half_x = scene.frame_half_x * (static_cast<double>(iw) * lean.t_scale) / width;
    bare_frame.frame_half_y = scene.frame_half_y * (static_cast<double>(ih) * lean.t_scale) / height;
    bare_frame.frame_centre_y = scene.frame_centre_y - (bare_frame.frame_half_y - scene.frame_half_y);
    const unsigned threads = std::max(1U, std::thread::hardware_concurrency());
    std::atomic<int> interval_rows{0};
    for (int pass = 0; pass < 2; ++pass) {
        const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::vector<std::thread> workers;
        workers.reserve(threads);
        for (unsigned i = 0; i < threads; ++i) {
            const SplitJob job{&scene, &bare_frame, pass == 0 ? 2 : lean.t_grid, static_cast<int>(i), static_cast<int>(threads), &bake.surface, &bake.inside};
            if (pass == 0) {
                workers.emplace_back(render_surface_rows, job);
            } else if (lean.intervals) {
                workers.emplace_back(render_inside_intervals, std::cref(bare_frame), std::ref(bake.inside), std::ref(interval_rows));
            } else {
                workers.emplace_back(render_inside_rows, job);
            }
        }
        for (std::thread& worker : workers) {
            worker.join();
        }
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        (pass == 0 ? bake.surface_seconds : bake.inside_seconds) = seconds;
    }
    if (lean.t_linear) {
        const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        for (int axis = 0; axis < 2; ++axis) {
            Scene tilted = bare_frame;
            (axis == 0 ? tilted.tilt_x : tilted.tilt_y) = tilt_step;
            Image& out = axis == 0 ? bake.inside_dx : bake.inside_dy;
            out = bake.inside;
            std::vector<std::thread> workers;
            workers.reserve(threads);
            for (unsigned i = 0; i < threads; ++i) {
                workers.emplace_back(render_inside_rows, SplitJob{&scene, &tilted, lean.t_grid, static_cast<int>(i), static_cast<int>(threads), &bake.surface, &out});
            }
            for (std::thread& worker : workers) {
                worker.join();
            }
            // to a difference quotient
            for (std::size_t k = 0; k < out.base.size(); ++k) {
                out.base[k] = (out.base[k] - bake.inside.base[k]) / static_cast<float>(tilt_step);
                for (int i = 0; i < out.lamps; ++i) {
                    out.glow[static_cast<std::size_t>(i)][k] = (out.glow[static_cast<std::size_t>(i)][k] - bake.inside.glow[static_cast<std::size_t>(i)][k]) / static_cast<float>(tilt_step);
                }
            }
        }
        bake.inside_seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    }
    return bake;
}

// The runtime's join, written as a standard cache: the transmitted light
// brought to full resolution (bilinear, weighted by its coverage so the edge
// does not darken), then shown = reflection + ink × ink light + (1 − ink) × transmitted.
Image join_split(const SplitBake& bake) {
    const SurfaceImage& s = bake.surface;
    const Image& t = bake.inside;
    const int w = s.width;
    const int h = s.height;
    const double scale_x = static_cast<double>(t.width) * lean.t_scale / w;  // the reduced frame's widening
    const double scale_y = static_cast<double>(t.height) * lean.t_scale / h;
    Image out{};
    out.width = w;
    out.height = h;
    out.lamps = t.lamps;
    const std::size_t pixels = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
    out.base.assign(pixels * 3U, 0.0F);
    out.alpha = s.coverage;
    for (int i = 0; i < out.lamps; ++i) {
        out.glow[static_cast<std::size_t>(i)].assign(pixels * 3U, 0.0F);
    }
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const std::size_t k = static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x);
            // this pixel's centre in the reduced image's pixel coordinates: its
            // frame is widened about the centre across, and downward from the top
            const double across = ((x + 0.5) / w * 2.0 - 1.0) / scale_x;
            const double u = std::clamp((across + 1.0) / 2.0 * t.width - 0.5, 0.0, static_cast<double>(t.width - 1));
            const double v = std::clamp((y + 0.5) * t.height / (h * scale_y) - 0.5, 0.0, static_cast<double>(t.height - 1));
            const int x0 = static_cast<int>(u);
            const int y0 = static_cast<int>(v);
            const int x1 = std::min(t.width - 1, x0 + 1);
            const int y1 = std::min(t.height - 1, y0 + 1);
            const double ax = u - x0;
            const double ay = v - y0;
            const std::array<int, 4> xs = {x0, x1, x0, x1};
            const std::array<int, 4> ys = {y0, y0, y1, y1};
            const std::array<double, 4> ws = {(1 - ax) * (1 - ay), ax * (1 - ay), (1 - ax) * ay, ax * ay};
            // premultiplied light and coverage, interpolated, then divided: the
            // unpremultiplied light, weighted by coverage
            double weight = 0.0;
            double base = 0.0;
            std::array<double, max_lamps> glow{};
            // the relief's slope here, per unit coverage (the first-order join)
            const double cover = s.coverage[k];
            const double gx = lean.t_linear && cover > 0.0 ? s.slope_x[k] / cover : 0.0;
            const double gy = lean.t_linear && cover > 0.0 ? s.slope_y[k] / cover : 0.0;
            for (int j = 0; j < 4; ++j) {
                const std::size_t q = static_cast<std::size_t>(ys[static_cast<std::size_t>(j)]) * static_cast<std::size_t>(t.width) + static_cast<std::size_t>(xs[static_cast<std::size_t>(j)]);
                const double wj = ws[static_cast<std::size_t>(j)];
                weight += wj * t.alpha[q];
                double b = t.base[q * 3U];
                if (lean.t_linear) {
                    b += bake.inside_dx.base[q * 3U] * gx + bake.inside_dy.base[q * 3U] * gy;
                }
                base += wj * b;
                for (int i = 0; i < t.lamps; ++i) {
                    double gl = t.glow[static_cast<std::size_t>(i)][q * 3U];
                    if (lean.t_linear) {
                        gl += bake.inside_dx.glow[static_cast<std::size_t>(i)][q * 3U] * gx + bake.inside_dy.glow[static_cast<std::size_t>(i)][q * 3U] * gy;
                    }
                    glow[static_cast<std::size_t>(i)] += wj * gl;
                }
            }
            if (weight <= 1e-9 && s.coverage[k] > 0.0F) {
                // an edge pixel whose own centre missed: its covered neighbours' light
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        const int nx = std::clamp(x0 + dx, 0, t.width - 1);
                        const int ny = std::clamp(y0 + dy, 0, t.height - 1);
                        const std::size_t q = static_cast<std::size_t>(ny) * static_cast<std::size_t>(t.width) + static_cast<std::size_t>(nx);
                        weight += t.alpha[q];
                        base += t.base[q * 3U];
                        for (int i = 0; i < t.lamps; ++i) {
                            glow[static_cast<std::size_t>(i)] += t.glow[static_cast<std::size_t>(i)][q * 3U];
                        }
                    }
                }
            }
            const double inside_base = weight > 1e-9 ? base / weight : 0.0;
            const double open = std::max(0.0, static_cast<double>(s.coverage[k]) - s.ink[k]);  // covered, not inked
            const double shown = s.reflection[k] + s.ink_light[k] + open * inside_base;
            for (std::size_t c = 0; c < 3U; ++c) {
                out.base[k * 3U + c] = static_cast<float>(shown);
                for (int i = 0; i < t.lamps; ++i) {
                    const double g = weight > 1e-9 ? glow[static_cast<std::size_t>(i)] / weight : 0.0;
                    out.glow[static_cast<std::size_t>(i)][k * 3U + c] = static_cast<float>(open * g);
                }
            }
        }
    }
    return out;
}

bool write_floats(std::FILE* file, const std::vector<float>& values) {
    return std::fwrite(values.data(), sizeof(float), values.size(), file) == values.size();
}

// <stem>_surface.f32: int32 width, height; reflection, ink, ink light, coverage (float32 each).
// <stem>_inside.f32: a standard cache at the reduced resolution.
bool write_split(const std::string& stem, const SplitBake& bake);

bool write_buffers(const std::string& path, const Image& image) {
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    // width, height, lamp count, then base, each lamp, and the coverage
    // plane; colour planes are premultiplied by coverage
    const std::int32_t header[3] = {image.width, image.height, image.lamps};
    bool ok = std::fwrite(header, sizeof(header), 1U, file) == 1U;
    ok = ok && std::fwrite(image.base.data(), sizeof(float), image.base.size(), file) == image.base.size();
    for (int i = 0; i < image.lamps; ++i) {
        const std::vector<float>& glow = image.glow[static_cast<std::size_t>(i)];
        ok = ok && std::fwrite(glow.data(), sizeof(float), glow.size(), file) == glow.size();
    }
    ok = ok && std::fwrite(image.alpha.data(), sizeof(float), image.alpha.size(), file) == image.alpha.size();
    return std::fclose(file) == 0 && ok;
}

bool write_split(const std::string& stem, const SplitBake& bake) {
    std::FILE* file = std::fopen((stem + "_surface.f32").c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    const std::int32_t header[2] = {bake.surface.width, bake.surface.height};
    bool ok = std::fwrite(header, sizeof(header), 1U, file) == 1U;
    ok = ok && write_floats(file, bake.surface.reflection) && write_floats(file, bake.surface.ink) && write_floats(file, bake.surface.ink_light) &&
         write_floats(file, bake.surface.coverage);
    ok = std::fclose(file) == 0 && ok;
    return ok && write_buffers(stem + "_inside.f32", bake.inside);
}

std::uint8_t encode(double v) {
    const double c = std::clamp(v, 0.0, 1.0);
    const double s = c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
    return static_cast<std::uint8_t>(std::lround(s * 255.0));
}

double tone(double v) { return 1.0 - std::exp(-v * 1.3); }

// A 24-bit bottom-up BMP of tone(base + Σ power × colour_i × glow_i).
bool write_preview(const std::string& path, const Image& image, const std::array<Vec3, max_lamps>& colours, double power) {
    const int w = image.width;
    const int h = image.height;
    const int row = (w * 3 + 3) & ~3;
    const std::uint32_t data_size = static_cast<std::uint32_t>(row * h);
    std::array<std::uint8_t, 54> head{};
    head[0] = 'B';
    head[1] = 'M';
    const std::uint32_t file_size = 54U + data_size;
    for (int i = 0; i < 4; ++i) {
        head[static_cast<std::size_t>(2 + i)] = static_cast<std::uint8_t>(file_size >> (8 * i));
        head[static_cast<std::size_t>(18 + i)] = static_cast<std::uint8_t>(static_cast<std::uint32_t>(w) >> (8 * i));
        head[static_cast<std::size_t>(22 + i)] = static_cast<std::uint8_t>(static_cast<std::uint32_t>(h) >> (8 * i));
        head[static_cast<std::size_t>(34 + i)] = static_cast<std::uint8_t>(data_size >> (8 * i));
    }
    head[10] = 54;
    head[14] = 40;
    head[26] = 1;
    head[28] = 24;
    std::vector<std::uint8_t> pixels(data_size, 0U);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const std::size_t at = (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)) * 3U;
            std::array<double, 3> v = {image.base[at], image.base[at + 1U], image.base[at + 2U]};
            for (int i = 0; i < image.lamps; ++i) {
                const std::vector<float>& glow = image.glow[static_cast<std::size_t>(i)];
                const Vec3 c = colours[static_cast<std::size_t>(i)];
                v[0] += power * c.x * glow[at];
                v[1] += power * c.y * glow[at + 1U];
                v[2] += power * c.z * glow[at + 2U];
            }
            const std::size_t out = static_cast<std::size_t>(h - 1 - y) * static_cast<std::size_t>(row) + static_cast<std::size_t>(x) * 3U;
            const double a = image.alpha[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)];
            // unpremultiply, tone, and lay over a dark panel grey
            const double behind = 0.018;
            const double inv_a = a > 1e-6 ? 1.0 / a : 0.0;
            pixels[out] = encode(tone(v[2] * inv_a) * a + behind * (1.0 - a));
            pixels[out + 1U] = encode(tone(v[1] * inv_a) * a + behind * (1.0 - a));
            pixels[out + 2U] = encode(tone(v[0] * inv_a) * a + behind * (1.0 - a));
        }
    }
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    bool ok = std::fwrite(head.data(), 1U, head.size(), file) == head.size();
    ok = ok && std::fwrite(pixels.data(), 1U, pixels.size(), file) == pixels.size();
    return std::fclose(file) == 0 && ok;
}

// ---- the three families ------------------------------------------------------------------------

Scene make_keycap() {
    Scene scene{};
    scene.family = Family::keycap;
    scene.lamps[0] = Lamp{Vec3{-0.10, 0.0, 0.12}, Vec3{0.10, 0.0, 0.12}};
    scene.lamp_count = 1;
    return scene;
}

Scene make_jewel() {
    Scene scene{};
    scene.family = Family::jewel;
    cut_jewel(scene);
    scene.round_well = true;
    scene.well_half = 0.32;
    scene.lamps[0] = Lamp{Vec3{-0.06, 0.0, 0.03}, Vec3{0.06, 0.0, 0.03}, 0.012, 0.010, 0.0030, 0.0040};
    scene.lamp_count = 1;
    return scene;
}

// A split caption, as on an annunciator panel: a rectangular lens, two lamps
// behind a divider (one per half), and a two-line legend.
//
// `sidewall` is the lens's height above the panel: 0.12 is a flush caption,
// 0.4 or so a keycap. Taller lenses taper a little more.
Scene make_annunciator(const std::string& upper, const std::string& lower, double sidewall) {
    Scene scene{};
    scene.family = Family::annunciator;
    scene.aspect = 0.62;
    scene.half_base = 0.34;
    scene.half_top = 0.315 - 0.12 * std::max(0.0, sidewall - 0.12);
    scene.top = sidewall;
    scene.bottom = -0.07;
    scene.chamfer = 0.022;
    scene.corner = 0.04;
    scene.softness = 0.014;
    scene.dish = 0.0;
    scene.well_half = 0.365;
    scene.well_radius = 0.03;
    scene.lamps[0] = Lamp{Vec3{-0.09, 0.075, 0.02}, Vec3{0.09, 0.075, 0.02}};
    scene.lamps[1] = Lamp{Vec3{-0.09, -0.075, 0.02}, Vec3{0.09, -0.075, 0.02}};
    scene.lamp_count = 2;
    // one line of legend: centred, both lamps lighting one field, no divider
    const bool single = lower.empty();
    scene.divider = !single;
    scene.control_only = true;
    // the camera is 2.6 above the panel: a taller top projects larger
    const double reach = std::max(scene.half_base, scene.half_top * 2.6 / (2.6 - scene.top));
    scene.frame_half_x = reach + 0.004;
    scene.frame_half_y = reach * scene.aspect + 0.004;
    if (single) {
        set_line(scene, upper, -0.04, 0.08);
    } else {
        set_line(scene, upper, 0.028, 0.06);
        set_line(scene, lower, -0.088, 0.06);
    }
    return scene;
}

// A small jewel indicator, as on old panels: the faceted jewel sits under the
// panel and only its crown shows through a round hole smaller than the jewel,
// framed close so that it renders small. The lamp is behind the crown.
Scene make_indicator() {
    Scene scene = make_jewel();
    scene.well_half = 0.19;
    scene.field_of_view = 19.0;
    return scene;
}

struct Style final {
    const char* name{};
    Grade grade{};
    Legend legend{Legend::none};
    bool blocked{};
    const char* upper{""};
    const char* lower{""};
    bool coil_at_back{};  // long horizontal coils just above a frosted back
    double sidewall{0.12};  // the lens's height above the panel
    bool one_coil{};        // one large, sparse coil across the middle (the dialog's OK)
    bool deep_lamps{};      // lamps at the bottom of an opaque shell, seen only as their diffused light
    double relief{};        // the top's orange-peel slope
    double grain{};         // the top's grain slope
    double height_share{};  // a dialog key's height over its short side (shape_key); 0: use sidewall
};

// Long, open coils lying just above the caption's back face, one per half.
void set_back_coils(Scene& scene) {
    const double z = scene.bottom + 0.022;
    scene.lamps[0] = Lamp{Vec3{-0.11, 0.075, z}, Vec3{0.11, 0.075, z}, 0.012, 0.019, 0.0028, 0.0035};
    scene.lamps[1] = Lamp{Vec3{-0.11, -0.075, z}, Vec3{0.11, -0.075, z}, 0.012, 0.019, 0.0028, 0.0035};
}

// The dialog keys' proportions. A key's height above the panel is given as a
// share of its short side (the owner's judgements on this 0.68 × 0.42 key:
// 0.28, a flush caption, too flat; 0.95, a keycap, too tall; 0.57
// right). Its sides lean in more as it gets taller, from nearly
// upright on a flush caption (3°) to about a sculpted keycap's lean (20° at
// a full short side; a DSA keycap's sides lean about 20°: 18.2 mm base,
// 12.7 mm top, 7.6 mm tall):
//   lean = 3° + 17° × clamp((height share − 0.25) / 0.75, 0, 1).
void shape_key(Scene& scene, double height_share) {
    const double short_side = 2.0 * scene.half_base * scene.aspect;
    scene.top = height_share * short_side;
    const double t = std::clamp((height_share - 0.25) / 0.75, 0.0, 1.0);
    const double lean = (3.0 + 17.0 * t) * pi / 180.0;
    scene.half_top = scene.half_base - std::tan(lean) * (scene.top - scene.bottom);
}

// Tilts the control camera toward the viewer by `degrees` and fits the frame
// to the key's projection: the base and top corners carried along the rays
// from the eye to z = 0, with a margin of 0.004.
void tilt_control_camera(Scene& scene, double degrees) {
    const double height = 2.6;
    scene.control_eye = Vec3{0.0, -height * std::tan(degrees * pi / 180.0), height};
    const Vec3 eye = scene.control_eye;
    double low_x = 1e9;
    double high_x = -1e9;
    double low_y = 1e9;
    double high_y = -1e9;
    const std::array<double, 2> levels = {scene.bottom, scene.top};
    const std::array<double, 2> halves = {scene.half_base, std::max(scene.half_base, scene.half_top)};
    for (std::size_t level = 0; level < levels.size(); ++level) {
        for (int corner = 0; corner < 4; ++corner) {
            const double x = (corner & 1) != 0 ? halves[level] : -halves[level];
            const double y = (corner & 2) != 0 ? halves[level] * scene.aspect : -halves[level] * scene.aspect;
            const double k = eye.z / (eye.z - levels[level]);  // along the ray from the eye, down to z = 0
            const double px = eye.x + (x - eye.x) * k;
            const double py = eye.y + (y - eye.y) * k;
            low_x = std::min(low_x, px);
            high_x = std::max(high_x, px);
            low_y = std::min(low_y, py);
            high_y = std::max(high_y, py);
        }
    }
    scene.frame_half_x = 0.5 * (high_x - low_x) + 0.004;
    scene.frame_half_y = 0.5 * (high_y - low_y) + 0.004;
    scene.frame_centre_y = 0.5 * (high_y + low_y);
}

// One large, sparse coil across a single-line key: few wide turns low in the
// lens, over the black void behind it.
void set_one_coil(Scene& scene) {
    scene.hidden_coil = true;
    const double z = scene.bottom + 0.022;
    scene.lamps[0] = Lamp{Vec3{-0.17, 0.0, z}, Vec3{0.17, 0.0, z}, 0.034, 0.062, 0.0034, 0.0040};
    scene.lamp_count = 1;
}

}  // namespace lamps

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: lamps <output directory> <size> <keycap|annunciator|indicator>\n");
        return 2;
    }
    const std::string directory = argv[1];
    const int size = std::clamp(std::atoi(argv[2]), 16, 2048);
    const std::string family = argv[3];
    for (int i = 4; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--bbox") {
            lamps::lean.bbox = true;
        } else if (arg == "--grey") {
            lamps::lean.grey = true;
        } else if (arg == "--eps" && i + 1 < argc) {
            lamps::lean.eps = std::atof(argv[++i]);
        } else if (arg == "--chord" && i + 1 < argc) {
            lamps::lean.chord_step = std::atof(argv[++i]);
        } else if (arg == "--turn" && i + 1 < argc) {
            lamps::lean.turn = std::atof(argv[++i]);
        } else if (arg == "--flat") {
            lamps::lean.flat = true;
        } else if (arg == "--gauss" && i + 1 < argc) {
            lamps::lean.gauss = std::atoi(argv[++i]);
        } else if (arg == "--fast-relief") {
            lamps::lean.fast_relief = true;
        } else if (arg == "--intervals") {
            lamps::lean.intervals = true;
        } else if (arg == "--interval-tolerance" && i + 1 < argc) {
            lamps::lean.interval_tolerance = std::atoi(argv[++i]);
        } else if (arg == "--poly") {
            lamps::lean.poly = true;
        } else if (arg == "--analytic") {
            lamps::lean.analytic = true;
        } else if (arg == "--grid" && i + 1 < argc) {
            lamps::lean.grid = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--t-linear") {
            lamps::lean.t_linear = true;
        } else if (arg == "--t-smooth") {
            lamps::lean.t_smooth = true;
        } else if (arg == "--split") {
            lamps::lean.split = true;
        } else if (arg == "--t-scale" && i + 1 < argc) {
            lamps::lean.t_scale = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--t-grid" && i + 1 < argc) {
            lamps::lean.t_grid = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--convex") {
            lamps::lean.convex = true;
        } else if (arg == "--snap") {
            lamps::lean.snap = true;
        } else if (arg == "--adaptive" && i + 1 < argc) {
            lamps::lean.adaptive = std::atof(argv[++i]);
        } else {
            std::fprintf(stderr, "unknown switch %s\n", arg.c_str());
            return 2;
        }
    }
    const std::chrono::steady_clock::time_point all_start = std::chrono::steady_clock::now();
    double render_seconds = 0.0;
    lamps::room_light = lamps::room_on_upward_surface();
    // name, σ per channel, albedo, glow reach, room source, silvered back, glow gain
    const lamps::Grade clear{"clear", lamps::Vec3{0.05, 0.05, 0.05}, 0.9, 0.03, 0.02, 0.0, 1.0};
    const lamps::Grade clear_frosted_back{"clear", lamps::Vec3{0.05, 0.05, 0.05}, 0.9, 0.03, 0.02, 0.0, 1.0, 0.85};
    const lamps::Grade backed{"backed", lamps::Vec3{0.05, 0.05, 0.05}, 0.9, 0.04, 0.02, 0.85, 2.0};
    const lamps::Grade opaque{"opaque", lamps::Vec3{22.0, 22.0, 22.0}, 0.97, 0.18, 0.10, 0.0, 6.0};
    std::vector<lamps::Style> styles;
    if (family == "annunciator") {
        styles = {
            lamps::Style{"opaque_black_blocked", opaque, lamps::Legend::black_ink, true, "HYD", "PRESS"},
            lamps::Style{"opaque_white_open", opaque, lamps::Legend::white_ink, false, "AC", "FAIL"},
            lamps::Style{"clear_black_blocked", clear_frosted_back, lamps::Legend::black_ink, true, "MASTER", "WARNING", true},
            lamps::Style{"clear_white_open", clear_frosted_back, lamps::Legend::white_ink, false, "ANTI-ICE", "ON", true},
            lamps::Style{"opaque_deadfront", opaque, lamps::Legend::dead_front, true, "MASTER", "CAUTION"},
            lamps::Style{"opaque_black_blocked_mid", opaque, lamps::Legend::black_ink, true, "HYD", "PRESS", false, 0.24},
            lamps::Style{"opaque_black_blocked_keycap", opaque, lamps::Legend::black_ink, true, "HYD", "PRESS", false, 0.40},
            lamps::Style{"opaque_black_open_keycap", opaque, lamps::Legend::black_ink, false, "AC", "FAIL", false, 0.40},
            lamps::Style{"clear_black_blocked_keycap", clear_frosted_back, lamps::Legend::black_ink, true, "MASTER", "WARNING", true, 0.40},
        };
    } else if (family == "dialog") {
        // The colour is the lamp's (applied at composite): CANCEL is white
        // opaque plastic, OK clear acrylic with a faint haze (extinction 0.6,
        // its glow reaching three times as far as the panel captions') over a
        // black void, with no frosted back. Both tops carry the moulded relief.
        // Height 0.24, the panel's mid captions'.
        // OK is lit as the ANTI-ICE caption (approved): clear, its lamp just
        // above a frosted back that re-emits only the lamp's light, so it is
        // dark when off and brightest round the lamp when on. CANCEL's white
        // plastic adds a short-reach brightening round its lamps.
        lamps::Grade white_near = opaque;
        white_near.near_reach = 0.06;
        white_near.near_gain = 4.0;
        styles = {
            lamps::Style{"cancel", white_near, lamps::Legend::grey_ink, false, "CANCEL", "", false, 0.24, false, true, 0.05, 0.05, 0.57},
            lamps::Style{"ok", clear_frosted_back, lamps::Legend::white_ink, false, "OK", "", false, 0.24, true, false, 0.05, 0.05, 0.57},
        };
    } else if (family == "indicator") {
        styles = {
            lamps::Style{"backed", backed, lamps::Legend::none, false, "", ""},
            lamps::Style{"clear", clear, lamps::Legend::none, false, "", ""},
        };
    } else if (family == "keycap") {
        styles = {
            lamps::Style{"clear", clear, lamps::Legend::none, false, "", ""},
            lamps::Style{"opaque", opaque, lamps::Legend::none, false, "", ""},
        };
    } else {
        std::fprintf(stderr, "unknown family %s\n", family.c_str());
        return 2;
    }
    const std::array<lamps::Vec3, lamps::max_lamps> colours = {lamps::Vec3{1.0, 0.55, 0.10}, lamps::Vec3{1.0, 0.16, 0.05}};
    for (const lamps::Style& style : styles) {
        lamps::Scene shape = (family == "annunciator" || family == "dialog") ? lamps::make_annunciator(style.upper, style.lower, style.sidewall)
                             : family == "indicator" ? lamps::make_indicator()
                                                     : lamps::make_keycap();
        shape.grade = style.grade;
        shape.legend_style = style.legend;
        shape.sidewall_blocked = style.blocked;
        if (style.coil_at_back) {
            lamps::set_back_coils(shape);
        }
        if (style.one_coil) {
            lamps::set_one_coil(shape);
        }
        if (style.deep_lamps) {
            shape.hidden_coil = true;
            for (lamps::Lamp& lamp : shape.lamps) {
                lamp.a.z = shape.bottom + 0.012;
                lamp.b.z = shape.bottom + 0.012;
            }
        }
        shape.relief = style.relief;
        shape.grain = style.grain;
        if (family == "dialog") {
            shape.aspect = 0.36;  // as wide as a program dialog's buttons (about 2.8 : 1)
        }
        if (style.height_share > 0.0) {
            lamps::shape_key(shape, style.height_share);
        }
        if (family == "dialog") {
            lamps::tilt_control_camera(shape, 16.0);
        }
        for (int pressed = 0; pressed < 2; ++pressed) {
            lamps::Scene scene = shape;
            scene.press = pressed != 0 ? (family == "indicator" ? 0.0 : 0.04) : 0.0;
            const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            int width = size;
            int height = size;
            if (scene.control_only) {
                height = std::max(1, static_cast<int>(std::lround(size * scene.frame_half_y / scene.frame_half_x)));
            }
            double surface_seconds = 0.0;
            double inside_seconds = 0.0;
            lamps::Image image{};
            lamps::SplitBake split{};
            if (lamps::lean.split) {
                split = lamps::render_split(scene, width, height);
                surface_seconds = split.surface_seconds;
                inside_seconds = split.inside_seconds;
                image = lamps::join_split(split);
            } else {
                image = lamps::render(scene, width, height, lamps::lean.grid);
            }
            const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
            const std::string stem = directory + "/" + family + "_" + style.name + (pressed != 0 ? "_down" : "_up");
            render_seconds += lamps::lean.split ? surface_seconds + inside_seconds : seconds;
            bool written = lamps::write_buffers(stem + ".f32", image);
            if (lamps::lean.split) {
                written = written && lamps::write_split(stem, split);
                std::printf("  split: surface %.3f s, inside %.3f s (%dx%d)\n", surface_seconds, inside_seconds, split.inside.width, split.inside.height);
            }
            const double refined_share = lamps::lean.adaptive > 0.0 ? 100.0 * static_cast<double>(lamps::refined_pixels) / (width * height) : 100.0;
            std::printf("%-12s %-22s %-4s %dx%d  %.3f s  refined %.1f%%%s\n", family.c_str(), style.name, pressed != 0 ? "down" : "up", width, height,
                        seconds, refined_share, written ? "" : "  (write failed)");
            (void)colours;
            if (!written) {
                return 1;
            }
        }
    }
    const double all_seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - all_start).count();
    std::printf("total render %.3f s (wall %.3f s)\n", render_seconds, all_seconds);
    if (lamps::lean.intervals) {
        const double e = static_cast<double>(lamps::interval_exact.load());
        const double n = static_cast<double>(lamps::interval_interpolated.load());
        std::printf("intervals: %.0f exact, %.0f interpolated (%.1f%% exact), %llu splits\n", e, n, 100.0 * e / std::max(1.0, e + n),
                    static_cast<unsigned long long>(lamps::interval_splits.load()));
    }
    return 0;
}
