// An acrylic keycap lit from within by a glowing rod, path traced on the CPU.
//
// The scene: a black anodised plate with a square well, and an elevated
// acrylic key standing in it (tapered sides, a shallow dish on top, chamfers
// softened by a smooth maximum of the bounding planes). A diffuse glowing rod
// is embedded in the acrylic along the key's width. Light leaves the rod,
// scatters in the acrylic (frosted and opal grades), reflects totally inside
// the polished faces and refracts out, lighting the key, the well and the
// plate.
//
// Every light in the scene other than the room is the rod, and transport is
// linear in emitted power, so each path adds to two images: `base`, the room's
// light alone, and `glow`, the rod's light at unit white power. A frame is then
// tone(base + power × colour × glow), per channel, which is exact for any rod
// colour because the acrylic's absorption is applied per channel in `glow`.
//
// Usage: acrylic_key <output directory> <samples per pixel> <size in pixels>
// Writes <grade>_<up|down>.f32 (int32 width, int32 height, then base and glow
// as width × height × 3 float32 each) and <grade>_<state>.bmp previews.

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

// Uniform random numbers for one pixel sample stream (PCG32).
struct Random final {
    std::uint64_t state{};
    std::uint64_t increment{};
};

std::uint32_t next_u32(Random& random) {
    const std::uint64_t old = random.state;
    random.state = old * 6364136223846793005ULL + random.increment;
    const std::uint32_t shifted = static_cast<std::uint32_t>(((old >> 18U) ^ old) >> 27U);
    const std::uint32_t rotation = static_cast<std::uint32_t>(old >> 59U);
    return (shifted >> rotation) | (shifted << ((32U - rotation) & 31U));
}

double next_unit(Random& random) { return next_u32(random) * (1.0 / 4294967296.0); }

Random seeded(std::uint64_t pixel, std::uint64_t stream) {
    Random random{};
    random.increment = (stream << 1U) | 1U;
    next_u32(random);
    random.state += pixel * 0x9E3779B97F4A7C15ULL;
    next_u32(random);
    return random;
}

// The acrylic's grade: its volume scattering and absorption (per unit length,
// the key's base being 0.68 units wide) and its surface finish.
struct Grade final {
    const char* name{};
    double scattering{};       // σs
    double anisotropy{};       // Henyey–Greenstein g
    Vec3 absorption{};         // σa per channel
    double roughness{};        // surface normal jitter (0 = polished)
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
    Vec3 rod_a{-0.17, 0.0, 0.11};
    Vec3 rod_b{0.17, 0.0, 0.11};
    double rod_radius{0.026};
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

Vec3 cosine_direction(Vec3 n, Random& random) {
    const double r1 = next_unit(random);
    const double r2 = next_unit(random);
    const double r = std::sqrt(r1);
    const double phi = 2.0 * pi * r2;
    Vec3 u{};
    Vec3 v{};
    basis(n, u, v);
    return normalized(add(add(scale(u, r * std::cos(phi)), scale(v, r * std::sin(phi))), scale(n, std::sqrt(1.0 - r1))));
}

// Henyey–Greenstein phase function: density, and a sampled direction about w.
double phase_hg(double cos_theta, double g) {
    const double denom = 1.0 + g * g - 2.0 * g * cos_theta;
    return (1.0 - g * g) / (4.0 * pi * denom * std::sqrt(denom));
}

Vec3 sample_hg(Vec3 w, double g, Random& random) {
    const double xi = next_unit(random);
    double cos_theta = 1.0 - 2.0 * xi;
    if (std::abs(g) > 1e-3) {
        const double s = (1.0 - g * g) / (1.0 - g + 2.0 * g * xi);
        cos_theta = (1.0 + g * g - s * s) / (2.0 * g);
    }
    const double sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta * cos_theta));
    const double phi = 2.0 * pi * next_unit(random);
    Vec3 u{};
    Vec3 v{};
    basis(w, u, v);
    return normalized(add(add(scale(u, sin_theta * std::cos(phi)), scale(v, sin_theta * std::sin(phi))), scale(w, cos_theta)));
}

// Roughen a surface normal for a satin finish.
Vec3 jitter(Vec3 n, double roughness, Random& random) {
    if (roughness <= 0.0) {
        return n;
    }
    Vec3 u{};
    Vec3 v{};
    basis(n, u, v);
    const double r = roughness * std::sqrt(-std::log(std::max(1e-12, next_unit(random))));
    const double phi = 2.0 * pi * next_unit(random);
    return normalized(add(n, add(scale(u, r * std::cos(phi)), scale(v, r * std::sin(phi)))));
}

Vec3 exp_neg(Vec3 sigma, double t) {
    return Vec3{std::exp(-sigma.x * t), std::exp(-sigma.y * t), std::exp(-sigma.z * t)};
}

// Light from the rod reaching a point inside the acrylic (next-event estimate),
// for unit emitted radiance, through the phase function. The key is convex and
// the rod lies inside it, so the segment stays in the acrylic; only the rod
// itself can hide the sampled point, which the cosine at the rod handles.
Vec3 rod_direct(const Scene& scene, Vec3 p, Vec3 incoming, Random& random) {
    const Vec3 a = add(scene.rod_a, rod_offset(scene));
    const Vec3 b = add(scene.rod_b, rod_offset(scene));
    const Vec3 axis = sub(b, a);
    const double len = length(axis);
    const Vec3 w = scale(axis, 1.0 / len);
    Vec3 u{};
    Vec3 v{};
    basis(w, u, v);
    const double s = next_unit(random);
    const double phi = 2.0 * pi * next_unit(random);
    const Vec3 radial = add(scale(u, std::cos(phi)), scale(v, std::sin(phi)));
    const Vec3 q = add(add(a, scale(axis, s)), scale(radial, scene.rod_radius));
    const Vec3 to_light = sub(q, p);
    const double d2 = dot(to_light, to_light);
    const double d = std::sqrt(d2);
    const Vec3 l = scale(to_light, 1.0 / d);
    const double cos_light = -dot(radial, l);
    if (cos_light <= 0.0) {
        return Vec3{};
    }
    const double area = 2.0 * pi * scene.rod_radius * len;
    const Grade& g = scene.grade;
    const Vec3 sigma_t = add(g.absorption, Vec3{g.scattering, g.scattering, g.scattering});
    const double geometric = cos_light * area / d2 * phase_hg(dot(incoming, l), g.anisotropy);
    return scale(exp_neg(sigma_t, d), geometric);
}

struct Radiance final {
    Vec3 base{};
    Vec3 glow{};
};

constexpr double acrylic_index = 1.49;

// One light path from the camera. Contributions from the room go to `base`,
// from the rod (unit white radiance) to `glow`.
Radiance trace(const Scene& scene, Vec3 origin, Vec3 direction, Random& random) {
    Radiance out{};
    Vec3 throughput{1.0, 1.0, 1.0};
    bool inside = false;
    bool counted_by_next_event = false;  // last vertex was a medium scatter
    const Grade& g = scene.grade;
    for (int depth = 0; depth < 400; ++depth) {
        if (depth > 6) {
            const double keep = std::min(0.95, max_component(throughput));
            if (next_unit(random) > keep) {
                break;
            }
            throughput = scale(throughput, 1.0 / keep);
        }
        if (inside) {
            const double t_rod = rod_hit(scene, origin, direction);
            double t_free = 1e30;
            if (g.scattering > 0.0) {
                t_free = -std::log(std::max(1e-12, next_unit(random))) / g.scattering;
            }
            const double limit = std::min(t_free, t_rod > 0.0 ? t_rod : 1e30);
            const double t_wall = march_inside(scene, origin, direction, limit);
            if (t_free < t_wall && (t_rod < 0.0 || t_free < t_rod)) {
                // scatter in the volume
                origin = add(origin, scale(direction, t_free));
                throughput = mul(throughput, exp_neg(g.absorption, t_free));
                out.glow = add(out.glow, mul(throughput, rod_direct(scene, origin, direction, random)));
                direction = sample_hg(direction, g.anisotropy, random);
                counted_by_next_event = true;
                continue;
            }
            if (t_rod > 0.0 && t_rod < t_wall) {
                origin = add(origin, scale(direction, t_rod));
                throughput = mul(throughput, exp_neg(g.absorption, t_rod));
                if (!counted_by_next_event) {
                    out.glow = add(out.glow, throughput);
                }
                // the rod is frosted glass: it also reflects diffusely
                const Vec3 n = rod_normal(scene, origin);
                throughput = scale(throughput, 0.6);
                direction = cosine_direction(n, random);
                origin = add(origin, scale(n, 1e-4));
                counted_by_next_event = false;
                continue;
            }
            // reach the acrylic's surface from inside
            origin = add(origin, scale(direction, t_wall));
            throughput = mul(throughput, exp_neg(g.absorption, t_wall));
            const Vec3 outward = jitter(gradient(key_distance, scene, origin), g.roughness, random);
            const double cos_i = std::max(0.0, dot(direction, outward));
            const double reflect = fresnel_dielectric(cos_i, acrylic_index, 1.0);
            if (next_unit(random) < reflect) {
                direction = normalized(sub(direction, scale(outward, 2.0 * dot(direction, outward))));
                origin = sub(origin, scale(outward, 4e-4));
            } else {
                const double eta = acrylic_index;
                const double cos_t2 = 1.0 - eta * eta * (1.0 - cos_i * cos_i);
                direction = normalized(add(scale(direction, eta), scale(outward, std::sqrt(std::max(0.0, cos_t2)) - eta * cos_i)));
                origin = add(origin, scale(outward, 4e-4));
                inside = false;
            }
            counted_by_next_event = false;
            continue;
        }
        const Hit hit = march_outside(scene, origin, direction);
        if (hit.surface == Surface::none) {
            out.base = add(out.base, scale(throughput, environment(direction)));
            break;
        }
        origin = add(origin, scale(direction, hit.t));
        if (hit.surface == Surface::key) {
            const Vec3 n = jitter(gradient(key_distance, scene, origin), g.roughness, random);
            const double cos_i = std::max(0.0, -dot(direction, n));
            const double reflect = fresnel_dielectric(cos_i, 1.0, acrylic_index);
            if (next_unit(random) < reflect) {
                direction = normalized(sub(direction, scale(n, 2.0 * dot(direction, n))));
                origin = add(origin, scale(n, 4e-4));
            } else {
                const double eta = 1.0 / acrylic_index;
                const double cos_t2 = 1.0 - eta * eta * (1.0 - cos_i * cos_i);
                direction = normalized(add(scale(direction, eta), scale(n, eta * cos_i - std::sqrt(std::max(0.0, cos_t2)))));
                origin = sub(origin, scale(n, 4e-4));
                inside = true;
            }
            counted_by_next_event = false;
            continue;
        }
        // plate: black anodised top with a faint satin coat; dark grey well
        const Vec3 n = gradient(plate_distance, scene, origin);
        const bool top_face = origin.z > -1e-3 && n.z > 0.9;
        const double albedo = top_face ? 0.035 : 0.10;
        const double coat = fresnel_dielectric(std::max(0.0, -dot(direction, n)), 1.0, 1.5);
        if (top_face && next_unit(random) < coat) {
            const Vec3 m = jitter(n, 0.18, random);
            direction = normalized(sub(direction, scale(m, 2.0 * dot(direction, m))));
            if (dot(direction, n) <= 0.0) {
                break;
            }
        } else {
            throughput = scale(throughput, albedo);
            direction = cosine_direction(n, random);
        }
        origin = add(origin, scale(n, 4e-4));
        counted_by_next_event = false;
    }
    return out;
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
            Random random = seeded(pixel, 17U);
            for (int s = 0; s < samples; ++s) {
                const double sx = (x + next_unit(random)) / w * 2.0 - 1.0;
                const double sy = 1.0 - (y + next_unit(random)) / h * 2.0;
                const Vec3 direction = normalized(add(forward, add(scale(right, sx * half), scale(up, sy * half))));
                const Radiance r = trace(scene, camera.eye, direction, random);
                base = add(base, r.base);
                glow = add(glow, r.glow);
            }
            const double inv = 1.0 / samples;
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
    if (argc < 4) {
        std::fprintf(stderr, "usage: acrylic_key <output directory> <samples per pixel> <size>\n");
        return 2;
    }
    const std::string directory = argv[1];
    const int samples = std::max(1, std::atoi(argv[2]));
    const int size = std::clamp(std::atoi(argv[3]), 16, 2048);
    const std::array<keycap::Grade, 4> grades = {
        keycap::Grade{"clear", 0.0, 0.0, keycap::Vec3{0.03, 0.03, 0.03}, 0.0},
        keycap::Grade{"frosted", 9.0, 0.6, keycap::Vec3{0.04, 0.04, 0.05}, 0.05},
        keycap::Grade{"opal", 26.0, 0.0, keycap::Vec3{0.25, 0.25, 0.27}, 0.02},
        keycap::Grade{"smoked", 3.0, 0.7, keycap::Vec3{5.0, 5.0, 4.7}, 0.0},
    };
    const keycap::Vec3 red{1.0, 0.16, 0.05};
    for (const keycap::Grade& grade : grades) {
        for (int pressed = 0; pressed < 2; ++pressed) {
            keycap::Scene scene{};
            scene.grade = grade;
            scene.press = pressed != 0 ? 0.06 : 0.0;
            const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            const keycap::Image image = keycap::render(scene, size, samples);
            const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            const double seconds = std::chrono::duration<double>(end - start).count();
            const std::string stem = directory + "/" + grade.name + (pressed != 0 ? "_down" : "_up");
            const bool written = keycap::write_buffers(stem + ".f32", image) &&
                                 keycap::write_preview(stem + "_off.bmp", image, red, 0.0) &&
                                 keycap::write_preview(stem + "_on.bmp", image, red, 2.5);
            std::printf("%-8s %-4s %dx%d %d spp  %.2f s%s\n", grade.name, pressed != 0 ? "down" : "up", size, size, samples, seconds,
                        written ? "" : "  (write failed)");
            if (!written) {
                return 1;
            }
        }
    }
    return 0;
}
