// An acrylic keycap lit from within by a glowing rod, rendered with the
// participating-child law of the bfft photonic transport engine
// (photonic_transport_field/native/child_boundary_response.hpp and
// regime_scene_native.cpp: ConvexOpticalChild::evaluate_law, VolumeChord).
//
// The key is a convex dielectric child. A camera ray refracts in; along every
// chord inside, light is retained by T = exp(−∫σ) and the medium contributes
// its own source radiance, L_out = B²·T·L_in + B·(1−T)·S; internal reflections
// are followed leg by leg (total internal reflection keeps everything) until the
// weight falls below a cutoff. The engine uses one homogeneous source radiance
// per medium; here the source function varies in space: the rod is a dense,
// glowing body, and the acrylic around it carries the rod's diffused light,
// falling off with distance. There is no random sampling inside the key.
//
// (The Monte Carlo version this replaces is acrylic_key.cpp, kept as the
// comparison.)
//
// The scene: a black anodised plate with a square well, and an elevated
// acrylic key standing in it (tapered sides, a shallow dish on top, chamfers
// softened by a smooth maximum of the bounding planes), seen head-on.
//
// Every light in the scene other than the room is the rod, and transport is
// linear in its power, so each pixel is written to two images: `base`, the
// room's light alone, and `glow`, the rod's light at unit white power. A frame
// is then tone(base + power × colour × glow) per channel.
//
// Usage: lit_key <output directory> <size in pixels>
// Writes <grade>_<up|down>.f32 (int32 width, int32 height, then base and glow
// as width × height × 3 float32 each) and <grade>_<state>_<off|on>.bmp.

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

namespace keycap {

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

// The acrylic's grade, as a participating medium. Lengths are in key units
// (the key's base is 0.68 wide).
struct Grade final {
    const char* name{};
    Vec3 density{};        // extinction σ per channel in the acrylic
    double albedo{};       // share of σ that re-emits (the rest is absorbed)
    double glow_reach{};   // distance over which the rod's diffused light falls to half
    double room{};         // the room light the acrylic itself carries, as source radiance
    double rear_reflector{};  // reflectance of a silvered back face (0: plain polished face)
};

// Geometry. Units: the key's base is 0.68 wide; z up; y away from the viewer.
struct Scene final {
    double half_base{0.34};
    double half_top{0.275};
    double bottom{-0.07};
    double top{0.30};
    double chamfer{0.05};      // top edge chamfer, measured along each face
    double corner{0.07};       // plan-view corner chamfer
    double softness{0.03};     // smooth-maximum radius: how soft every edge is
    double dish{0.012};        // depth of the cylindrical dish on top
    double press{0.0};         // how far the key is pushed down
    double well_half{0.385};
    double well_radius{0.06};
    double well_floor{-0.22};
    // The lamp: a coiled filament suspended in the lens on two lead wires.
    // rod_a–rod_b is the coil's axis; rod_radius is the coil's radius.
    Vec3 rod_a{-0.10, 0.0, 0.12};
    Vec3 rod_b{0.10, 0.0, 0.12};
    double rod_radius{0.014};
    double coil_pitch{0.011};
    double wire_radius{0.0032};
    double lead_radius{0.0045};
    double filament_radiance{24.0};  // white-hot: far above the room's light
    Grade grade{};
};

// Polynomial smooth maximum: max(a, b) raised by at most k/4 where they meet,
// which rounds the edge between two planes with a radius of about k.
double smooth_max(double a, double b, double k) {
    const double h = std::max(k - std::abs(a - b), 0.0) / k;
    return std::max(a, b) + h * h * k * 0.25;
}

double key_distance(const Scene& scene, Vec3 p) {
    const double z = p.z + scene.press;
    const double ax = std::abs(p.x);
    const double ay = std::abs(p.y);
    const double height = scene.top - scene.bottom;
    const double slope = (scene.half_base - scene.half_top) / height;
    const double side_norm = 1.0 / std::sqrt(1.0 + slope * slope);
    const double half_here = scene.half_base - slope * (z - scene.bottom);
    // top: a shallow cylindrical dish across x
    const double across = std::min(1.0, ax / scene.half_top);
    const double top_here = scene.top - scene.dish * (1.0 - across * across);
    const double k = scene.softness;
    double d = z - top_here;
    d = smooth_max(d, scene.bottom - z, k);
    d = smooth_max(d, (ax - half_here) * side_norm, k);
    d = smooth_max(d, (ay - half_here) * side_norm, k);
    // top edge chamfers: planes through (half_top − c, top) and (half_top, top − c)
    const double c = scene.chamfer;
    d = smooth_max(d, (ax + z - (scene.half_top + scene.top - c)) * 0.7071067811865476, k);
    d = smooth_max(d, (ay + z - (scene.half_top + scene.top - c)) * 0.7071067811865476, k);
    // plan-view corners
    d = smooth_max(d, (ax + ay - (2.0 * half_here - scene.corner)) * 0.7071067811865476, k);
    return d;
}

double rounded_rect(double x, double y, double half, double radius) {
    const double qx = std::abs(x) - half + radius;
    const double qy = std::abs(y) - half + radius;
    const double ox = std::max(qx, 0.0);
    const double oy = std::max(qy, 0.0);
    return std::sqrt(ox * ox + oy * oy) + std::min(std::max(qx, qy), 0.0) - radius;
}

// The plate with its well: solid below z = 0 outside the well, and below the
// well's floor.
double plate_distance(const Scene& scene, Vec3 p) {
    const double well = rounded_rect(p.x, p.y, scene.well_half, scene.well_radius);
    return std::min(std::max(p.z, -well), p.z - scene.well_floor);
}

Vec3 rod_offset(const Scene& scene) { return Vec3{0.0, 0.0, -scene.press}; }

// Distance along a ray to the rod (a capsule), or a negative value on a miss.
double rod_hit(const Scene& scene, Vec3 origin, Vec3 direction) {
    const Vec3 a = add(scene.rod_a, rod_offset(scene));
    const Vec3 b = add(scene.rod_b, rod_offset(scene));
    const Vec3 ba = sub(b, a);
    const Vec3 oa = sub(origin, a);
    const double baba = dot(ba, ba);
    const double bard = dot(ba, direction);
    const double baoa = dot(ba, oa);
    const double rdoa = dot(direction, oa);
    const double oaoa = dot(oa, oa);
    const double r = scene.rod_radius;
    const double qa = baba - bard * bard;
    const double qb = baba * rdoa - baoa * bard;
    const double qc = baba * oaoa - baoa * baoa - r * r * baba;
    const double h = qb * qb - qa * qc;
    if (h >= 0.0) {
        const double t = (-qb - std::sqrt(h)) / qa;
        const double y = baoa + t * bard;
        if (y > 0.0 && y < baba && t > 1e-6) {
            return t;
        }
        // the rounded ends
        const Vec3 oc = y <= 0.0 ? oa : sub(origin, b);
        const double b2 = dot(direction, oc);
        const double c2 = dot(oc, oc) - r * r;
        const double h2 = b2 * b2 - c2;
        if (h2 > 0.0) {
            const double t2 = -b2 - std::sqrt(h2);
            if (t2 > 1e-6) {
                return t2;
            }
        }
    }
    return -1.0;
}

Vec3 rod_normal(const Scene& scene, Vec3 p) {
    const Vec3 a = add(scene.rod_a, rod_offset(scene));
    const Vec3 b = add(scene.rod_b, rod_offset(scene));
    const Vec3 ba = sub(b, a);
    const double h = std::clamp(dot(sub(p, a), ba) / dot(ba, ba), 0.0, 1.0);
    return normalized(sub(p, add(a, scale(ba, h))));
}

enum class Surface : std::uint8_t {
    none,
    key,
    plate,
};

struct Hit final {
    Surface surface{Surface::none};
    double t{};
};

// March from outside the key through open space. The march stops at the key,
// the plate, or leaving the scene's bound.
Hit march_outside(const Scene& scene, Vec3 origin, Vec3 direction) {
    Hit hit{};
    double t = 0.0;
    for (int step = 0; step < 160; ++step) {
        const Vec3 p = add(origin, scale(direction, t));
        if (p.z > 0.8 && direction.z > 0.0) {
            return hit;
        }
        if (t > 8.0) {
            return hit;
        }
        const double dk = key_distance(scene, p);
        const double dp = plate_distance(scene, p);
        const double d = std::min(dk, dp);
        if (d < 2e-4) {
            hit.surface = dk < dp ? Surface::key : Surface::plate;
            hit.t = t;
            return hit;
        }
        t += d * 0.9;
    }
    return hit;
}

// Distance from inside the key to its boundary along a ray, marching no
// further than `limit` (returns limit or more when the boundary is beyond it).
double march_inside(const Scene& scene, Vec3 origin, Vec3 direction, double limit) {
    double t = 0.0;
    for (int step = 0; step < 200; ++step) {
        const double d = -key_distance(scene, add(origin, scale(direction, t)));
        if (d < 2e-4) {
            return t;
        }
        t += d * 0.9;
        if (t >= limit) {
            return t;
        }
    }
    return t;
}

Vec3 gradient(double (*field)(const Scene&, Vec3), const Scene& scene, Vec3 p) {
    const double e = 2e-4;
    const double dx = field(scene, Vec3{p.x + e, p.y, p.z}) - field(scene, Vec3{p.x - e, p.y, p.z});
    const double dy = field(scene, Vec3{p.x, p.y + e, p.z}) - field(scene, Vec3{p.x, p.y - e, p.z});
    const double dz = field(scene, Vec3{p.x, p.y, p.z + e}) - field(scene, Vec3{p.x, p.y, p.z - e});
    return normalized(Vec3{dx, dy, dz});
}

// ---- light and materials -------------------------------------------------------

// A dark room with one large softbox above and in front of the key.
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


// ---- participating child --------------------------------------------------------

Vec3 exp_neg(Vec3 sigma, double t) {
    return Vec3{std::exp(-sigma.x * t), std::exp(-sigma.y * t), std::exp(-sigma.z * t)};
}

double rod_axis_distance(const Scene& scene, Vec3 p) {
    const Vec3 a = add(scene.rod_a, rod_offset(scene));
    const Vec3 b = add(scene.rod_b, rod_offset(scene));
    const Vec3 ba = sub(b, a);
    const double h = std::clamp(dot(sub(p, a), ba) / dot(ba, ba), 0.0, 1.0);
    return length(sub(p, add(a, scale(ba, h))));
}

double smoothstep(double a, double b, double x) {
    const double t = std::clamp((x - a) / (b - a), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

// The filament and leads are opaque at this density.
constexpr double wire_density = 400.0;

// Distance to the filament's wire: the coil is a helix about rod_a–rod_b. The
// nearest point is taken among the three turns at the point's own angle, which
// is close to exact while the pitch is small against the coil radius.
double filament_distance(const Scene& scene, Vec3 p) {
    const Vec3 a = add(scene.rod_a, rod_offset(scene));
    const Vec3 b = add(scene.rod_b, rod_offset(scene));
    const double half = 0.5 * (b.x - a.x);
    const Vec3 c = scale(add(a, b), 0.5);
    const Vec3 q = sub(p, c);
    const double theta = std::atan2(q.z, q.y);
    const double turn = scene.coil_pitch;
    const double k0 = std::round(q.x / turn - theta / (2.0 * pi));
    double best = 1e9;
    for (int k = -1; k <= 1; ++k) {
        const double x = std::clamp(turn * (theta / (2.0 * pi) + k0 + k), -half, half);
        const double angle = 2.0 * pi * x / turn;
        const Vec3 h{x, scene.rod_radius * std::cos(angle), scene.rod_radius * std::sin(angle)};
        best = std::min(best, length(sub(q, h)));
    }
    return best - scene.wire_radius;
}

// The two lead wires run from the coil's ends down to the base of the lens.
double lead_distance(const Scene& scene, Vec3 p) {
    const Vec3 a = add(scene.rod_a, rod_offset(scene));
    const Vec3 b = add(scene.rod_b, rod_offset(scene));
    const double foot = scene.bottom - scene.press + 0.02;
    double best = 1e9;
    for (int side = 0; side < 2; ++side) {
        const Vec3 top = side == 0 ? a : b;
        const double z = std::clamp(p.z, foot, top.z);
        best = std::min(best, length(sub(p, Vec3{top.x, top.y, z})));
    }
    return best - scene.lead_radius;
}

// Extinction and source densities at a point inside the lens. `glow` and
// `room` are σ·S terms: radiance generated per unit length.
struct MediumPoint final {
    Vec3 extinction{};
    Vec3 glow{};
    Vec3 room{};
};

MediumPoint medium_at(const Scene& scene, Vec3 p) {
    const Grade& g = scene.grade;
    const double soft = 0.0008;
    const double wire = 1.0 - smoothstep(-soft, soft, filament_distance(scene, p));
    const double lead = 1.0 - smoothstep(-soft, soft, lead_distance(scene, p));
    const double solid = std::max(wire, lead);
    // the lamp's light diffused in the lens: falls to half at `reach` from the coil
    const double beyond = std::max(0.0, rod_axis_distance(scene, p) - scene.rod_radius);
    const double reach = g.glow_reach;
    const double diffused = reach * reach / (reach * reach + beyond * beyond);
    MediumPoint m{};
    const double dense = wire_density * solid;
    m.extinction = add(g.density, Vec3{dense, dense, dense});
    const double hot = wire_density * wire * scene.filament_radiance;
    m.glow = add(Vec3{hot, hot, hot}, scale(g.density, g.albedo * diffused * (1.0 - solid)));
    m.room = scale(g.density, g.albedo * g.room * (1.0 - solid));
    return m;
}

// How close the nearest wire is: the chord's quadrature steps no further than
// half of it, so a wire a pixel wide is never stepped over.
double wire_clearance(const Scene& scene, Vec3 p) {
    return std::max(0.0, std::min(filament_distance(scene, p), lead_distance(scene, p)));
}

struct Radiance final {
    Vec3 base{};
    Vec3 glow{};
};

Radiance radiance_add(Radiance a, Radiance b) { return Radiance{add(a.base, b.base), add(a.glow, b.glow)}; }
Radiance radiance_scale(Radiance a, Vec3 w) { return Radiance{mul(a.base, w), mul(a.glow, w)}; }

constexpr double acrylic_index = 1.49;

// One chord through the medium: midpoint quadrature of the source densities
// under the running transmittance, with steps shortened near the wires. Adds to
// `out`, returns the chord's T.
Vec3 integrate_chord(const Scene& scene, Vec3 origin, Vec3 direction, double length_of_chord, Radiance& out) {
    Vec3 transmittance{1.0, 1.0, 1.0};
    double t = 0.0;
    for (int step = 0; step < 800 && t < length_of_chord; ++step) {
        const double clearance = wire_clearance(scene, add(origin, scale(direction, t)));
        const double dt = std::min(length_of_chord - t, std::clamp(clearance * 0.5, 0.0006, 0.008));
        const Vec3 p = add(origin, scale(direction, t + 0.5 * dt));
        t += dt;
        const MediumPoint m = medium_at(scene, p);
        // exact for constant σ and S over the step: S·(1 − e^{−σ dt})
        const Vec3 step_t = exp_neg(m.extinction, dt);
        for (int c = 0; c < 3; ++c) {
            const double sigma = c == 0 ? m.extinction.x : (c == 1 ? m.extinction.y : m.extinction.z);
            const double factor = sigma > 1e-9 ? (1.0 - (c == 0 ? step_t.x : (c == 1 ? step_t.y : step_t.z))) / sigma : dt;
            const double tc = c == 0 ? transmittance.x : (c == 1 ? transmittance.y : transmittance.z);
            const double glow = c == 0 ? m.glow.x : (c == 1 ? m.glow.y : m.glow.z);
            const double room = c == 0 ? m.room.x : (c == 1 ? m.room.y : m.room.z);
            if (c == 0) { out.glow.x += tc * glow * factor; out.base.x += tc * room * factor; }
            else if (c == 1) { out.glow.y += tc * glow * factor; out.base.y += tc * room * factor; }
            else { out.glow.z += tc * glow * factor; out.base.z += tc * room * factor; }
        }
        transmittance = mul(transmittance, step_t);
    }
    return transmittance;
}

Radiance outside_radiance(const Scene& scene, Vec3 origin, Vec3 direction, int depth, bool gather);

// The participating child law (after ConvexOpticalChild::evaluate_law): the
// ray enters, each internal leg contributes its source and retains T, each exit
// returns a port, and the reflected remainder continues inside.
Radiance key_response(const Scene& scene, Vec3 position, Vec3 direction, int depth, bool gather) {
    Radiance out{};
    const Vec3 n = gradient(key_distance, scene, position);
    const double cos_i = std::max(0.0, -dot(direction, n));
    const double f_in = fresnel_dielectric(cos_i, 1.0, acrylic_index);
    if (depth > 0) {
        const Vec3 r = normalized(sub(direction, scale(n, 2.0 * dot(direction, n))));
        const Radiance reflected = outside_radiance(scene, add(position, scale(n, 4e-4)), r, depth - 1, gather);
        out = radiance_add(out, radiance_scale(reflected, Vec3{f_in, f_in, f_in}));
    } else {
        const double e = f_in * environment(normalized(sub(direction, scale(n, 2.0 * dot(direction, n)))));
        out.base = add(out.base, Vec3{e, e, e});
    }
    const double eta = 1.0 / acrylic_index;
    const double cos_t2 = 1.0 - eta * eta * (1.0 - cos_i * cos_i);
    Vec3 d = normalized(add(scale(direction, eta), scale(n, eta * cos_i - std::sqrt(std::max(0.0, cos_t2)))));
    Vec3 p = sub(position, scale(n, 4e-4));
    Vec3 weight{1.0 - f_in, 1.0 - f_in, 1.0 - f_in};
    for (int leg = 0; leg < 10 && max_component(weight) > 2e-3; ++leg) {
        const double t_exit = march_inside(scene, p, d, 10.0);
        Radiance chord{};
        const Vec3 retained = integrate_chord(scene, p, d, t_exit, chord);
        out = radiance_add(out, radiance_scale(chord, weight));
        weight = mul(weight, retained);
        p = add(p, scale(d, t_exit));
        const Vec3 outward = gradient(key_distance, scene, p);
        const double c = std::max(0.0, dot(d, outward));
        if (scene.grade.rear_reflector > 0.0 && outward.z < -0.7) {
            // a silvered back: everything reflects, at the mirror's reflectance
            weight = scale(weight, scene.grade.rear_reflector);
            d = normalized(sub(d, scale(outward, 2.0 * dot(d, outward))));
            p = sub(p, scale(outward, 4e-4));
            continue;
        }
        const double f_out = fresnel_dielectric(c, acrylic_index, 1.0);
        if (f_out < 1.0 && depth > 0) {
            const double e2 = acrylic_index;
            const double k = 1.0 - e2 * e2 * (1.0 - c * c);
            const Vec3 t = normalized(add(scale(d, e2), scale(outward, std::sqrt(std::max(0.0, k)) - e2 * c)));
            const Radiance beyond = outside_radiance(scene, add(p, scale(outward, 4e-4)), t, depth - 1, gather);
            out = radiance_add(out, radiance_scale(beyond, scale(weight, 1.0 - f_out)));
        }
        weight = scale(weight, f_out);
        d = normalized(sub(d, scale(outward, 2.0 * dot(d, outward))));
        p = sub(p, scale(outward, 4e-4));
    }
    return out;
}

// A fixed, well-spread set of cosine-weighted directions about +z, rotated per
// pixel; the plate gathers the room and the glowing key through them.
constexpr int gather_count = 24;

Vec3 gather_direction(int i, double rotation, Vec3 n) {
    const double golden = 2.399963229728653;
    const double r1 = (i + 0.5) / gather_count;
    const double r = std::sqrt(r1);
    const double phi = i * golden + rotation;
    Vec3 u{};
    Vec3 v{};
    basis(n, u, v);
    return normalized(add(add(scale(u, r * std::cos(phi)), scale(v, r * std::sin(phi))), scale(n, std::sqrt(1.0 - r1))));
}

Radiance plate_response(const Scene& scene, Vec3 position, Vec3 direction, int depth, bool gather) {
    Radiance out{};
    const Vec3 n = gradient(plate_distance, scene, position);
    const bool top_face = position.z > -1e-3 && n.z > 0.9;
    const double albedo = top_face ? 0.035 : 0.10;
    // satin coat: the room reflected, softly
    if (top_face) {
        const double coat = fresnel_dielectric(std::max(0.0, -dot(direction, n)), 1.0, 1.5);
        const Vec3 r = normalized(sub(direction, scale(n, 2.0 * dot(direction, n))));
        const double e = coat * 0.5 * (environment(r) + environment(normalized(add(r, Vec3{0.08, 0.05, 0.0}))));
        out.base = add(out.base, Vec3{e, e, e});
    }
    if (!gather || depth <= 0) {
        const double e = albedo * 0.05;
        out.base = add(out.base, Vec3{e, e, e});
        return out;
    }
    // rotation from an integer hash of the position on a fine lattice: a
    // linear function of position would band into stripes
    const std::uint32_t qx = static_cast<std::uint32_t>(static_cast<std::int32_t>(std::floor(position.x * 4096.0)));
    const std::uint32_t qy = static_cast<std::uint32_t>(static_cast<std::int32_t>(std::floor(position.y * 4096.0)));
    std::uint32_t h = qx * 0x27d4eb2dU ^ qy * 0x165667b1U;
    h = (h ^ (h >> 15U)) * 0x85ebca6bU;
    h = (h ^ (h >> 13U)) * 0xc2b2ae35U;
    h ^= h >> 16U;
    const double rotation = 6.283185307179586 * (h * (1.0 / 4294967296.0));
    Radiance sum{};
    const Vec3 lifted = add(position, scale(n, 4e-4));
    for (int i = 0; i < gather_count; ++i) {
        const Vec3 w = gather_direction(i, rotation, n);
        sum = radiance_add(sum, outside_radiance(scene, lifted, w, depth - 1, false));
    }
    const double k = albedo / gather_count;
    return radiance_add(out, radiance_scale(sum, Vec3{k, k, k}));
}

Radiance outside_radiance(const Scene& scene, Vec3 origin, Vec3 direction, int depth, bool gather) {
    const Hit hit = march_outside(scene, origin, direction);
    if (hit.surface == Surface::none) {
        const double e = environment(direction);
        return Radiance{Vec3{e, e, e}, Vec3{}};
    }
    const Vec3 p = add(origin, scale(direction, hit.t));
    if (hit.surface == Surface::key) {
        return key_response(scene, p, direction, depth, gather);
    }
    return plate_response(scene, p, direction, depth, gather);
}

// ---- camera and image ----------------------------------------------------------

struct Image final {
    int width{};
    int height{};
    std::vector<float> base;   // width × height × 3, linear
    std::vector<float> glow;   // width × height × 3, linear, per unit white rod
};

// Head-on, as a control is seen on screen: the camera looks down the key's
// axis from a little in front, so the key rises towards the viewer and its
// chamfers and a sliver of the front face show.
struct Camera final {
    Vec3 eye{0.0, -0.32, 2.6};
    Vec3 target{0.0, 0.0, 0.08};
    double field_of_view{21.0};  // degrees, vertical
};

void render_rows(const Scene& scene, const Camera& camera, int samples, int first_row, int row_step, Image& image) {
    // `samples` is the side of a stratified grid inside each pixel.
    const Vec3 forward = normalized(sub(camera.target, camera.eye));
    const Vec3 right = normalized(cross(forward, Vec3{0.0, 1.0, 0.0}));
    // forward is nearly −z, so screen-up is +y (away from the viewer's chin)
    const Vec3 up = cross(right, forward);
    const double half = std::tan(camera.field_of_view * pi / 360.0);
    const int w = image.width;
    const int h = image.height;
    for (int y = first_row; y < h; y += row_step) {
        for (int x = 0; x < w; ++x) {
            Vec3 base{};
            Vec3 glow{};
            const std::uint64_t pixel = static_cast<std::uint64_t>(y) * static_cast<std::uint64_t>(w) + static_cast<std::uint64_t>(x);
            for (int s = 0; s < samples * samples; ++s) {
                const double ox = ((s % samples) + 0.5) / samples;
                const double oy = ((s / samples) + 0.5) / samples;
                const double sx = (x + ox) / w * 2.0 - 1.0;
                const double sy = 1.0 - (y + oy) / h * 2.0;
                const Vec3 direction = normalized(add(forward, add(scale(right, sx * half), scale(up, sy * half))));
                const Radiance r = outside_radiance(scene, camera.eye, direction, 3, true);
                base = add(base, r.base);
                glow = add(glow, r.glow);
            }
            const double inv = 1.0 / (samples * samples);
            const std::size_t at = static_cast<std::size_t>(pixel) * 3U;
            image.base[at] = static_cast<float>(base.x * inv);
            image.base[at + 1U] = static_cast<float>(base.y * inv);
            image.base[at + 2U] = static_cast<float>(base.z * inv);
            image.glow[at] = static_cast<float>(glow.x * inv);
            image.glow[at + 1U] = static_cast<float>(glow.y * inv);
            image.glow[at + 2U] = static_cast<float>(glow.z * inv);
        }
    }
}

struct RowJob final {
    const Scene* scene{};
    const Camera* camera{};
    int samples{};
    int first_row{};
    int row_step{};
    Image* image{};
};

void run_job(RowJob job) {
    render_rows(*job.scene, *job.camera, job.samples, job.first_row, job.row_step, *job.image);
}

Image render(const Scene& scene, int size, int samples) {
    Image image{};
    image.width = size;
    image.height = size;
    image.base.assign(static_cast<std::size_t>(size) * static_cast<std::size_t>(size) * 3U, 0.0F);
    image.glow.assign(image.base.size(), 0.0F);
    const Camera camera{};
    const unsigned threads = std::max(1U, std::thread::hardware_concurrency());
    std::vector<std::thread> workers;
    workers.reserve(threads);
    for (unsigned i = 0; i < threads; ++i) {
        RowJob job{};
        job.scene = &scene;
        job.camera = &camera;
        job.samples = samples;
        job.first_row = static_cast<int>(i);
        job.row_step = static_cast<int>(threads);
        job.image = &image;
        workers.emplace_back(run_job, job);
    }
    for (std::thread& worker : workers) {
        worker.join();
    }
    return image;
}

// ---- output ---------------------------------------------------------------------

bool write_buffers(const std::string& path, const Image& image) {
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    const std::int32_t header[2] = {image.width, image.height};
    bool ok = std::fwrite(header, sizeof(header), 1U, file) == 1U;
    ok = ok && std::fwrite(image.base.data(), sizeof(float), image.base.size(), file) == image.base.size();
    ok = ok && std::fwrite(image.glow.data(), sizeof(float), image.glow.size(), file) == image.glow.size();
    return std::fclose(file) == 0 && ok;
}

std::uint8_t encode(double v) {
    const double c = std::clamp(v, 0.0, 1.0);
    const double s = c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
    return static_cast<std::uint8_t>(std::lround(s * 255.0));
}

double tone(double v) { return 1.0 - std::exp(-v * 1.3); }

// A 24-bit bottom-up BMP of tone(base + power × colour × glow).
bool write_preview(const std::string& path, const Image& image, Vec3 colour, double power) {
    const int w = image.width;
    const int h = image.height;
    const int row = (w * 3 + 3) & ~3;
    const std::uint32_t data_size = static_cast<std::uint32_t>(row * h);
    std::array<std::uint8_t, 54> head{};
    head[0] = 'B';
    head[1] = 'M';
    const std::uint32_t file_size = 54U + data_size;
    for (int i = 0; i < 4; ++i) {
        head[2 + i] = static_cast<std::uint8_t>(file_size >> (8 * i));
        head[18 + i] = static_cast<std::uint8_t>(static_cast<std::uint32_t>(w) >> (8 * i));
        head[22 + i] = static_cast<std::uint8_t>(static_cast<std::uint32_t>(h) >> (8 * i));
        head[34 + i] = static_cast<std::uint8_t>(data_size >> (8 * i));
    }
    head[10] = 54;
    head[14] = 40;
    head[26] = 1;
    head[28] = 24;
    std::vector<std::uint8_t> pixels(data_size, 0U);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const std::size_t at = (static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)) * 3U;
            const double r = tone(image.base[at] + power * colour.x * image.glow[at]);
            const double g = tone(image.base[at + 1U] + power * colour.y * image.glow[at + 1U]);
            const double b = tone(image.base[at + 2U] + power * colour.z * image.glow[at + 2U]);
            const std::size_t out = static_cast<std::size_t>(h - 1 - y) * static_cast<std::size_t>(row) + static_cast<std::size_t>(x) * 3U;
            pixels[out] = encode(b);
            pixels[out + 1U] = encode(g);
            pixels[out + 2U] = encode(r);
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

}  // namespace keycap

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: lit_key <output directory> <size>\n");
        return 2;
    }
    const std::string directory = argv[1];
    const int size = std::clamp(std::atoi(argv[2]), 16, 2048);
    const int grid = 2;  // 2 × 2 stratified samples per pixel
    // name, σ per channel, albedo, glow reach, room source, silvered back
    const std::array<keycap::Grade, 4> grades = {
        keycap::Grade{"clear", keycap::Vec3{0.06, 0.06, 0.06}, 0.9, 0.035, 0.02, 0.0},
        keycap::Grade{"silvered", keycap::Vec3{0.06, 0.06, 0.06}, 0.9, 0.035, 0.02, 0.88},
        keycap::Grade{"frosted", keycap::Vec3{5.0, 5.0, 5.2}, 0.92, 0.12, 0.06, 0.0},
        keycap::Grade{"smoked", keycap::Vec3{4.0, 4.0, 3.8}, 0.15, 0.06, 0.02, 0.0},
    };
    const keycap::Vec3 red{1.0, 0.16, 0.05};
    for (const keycap::Grade& grade : grades) {
        for (int pressed = 0; pressed < 2; ++pressed) {
            keycap::Scene scene{};
            scene.grade = grade;
            scene.press = pressed != 0 ? 0.06 : 0.0;
            const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            const keycap::Image image = keycap::render(scene, size, grid);
            const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            const double seconds = std::chrono::duration<double>(end - start).count();
            const std::string stem = directory + "/" + grade.name + (pressed != 0 ? "_down" : "_up");
            const bool written = keycap::write_buffers(stem + ".f32", image) &&
                                 keycap::write_preview(stem + "_off.bmp", image, red, 0.0) &&
                                 keycap::write_preview(stem + "_on.bmp", image, red, 1.0);
            std::printf("%-8s %-4s %dx%d  %.2f s%s\n", grade.name, pressed != 0 ? "down" : "up", size, size, seconds,
                        written ? "" : "  (write failed)");
            if (!written) {
                return 1;
            }
        }
    }
    return 0;
}
