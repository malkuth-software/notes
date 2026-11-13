// Tracer bakes: three ways to light four control-sized scenes, each timed at
// 1, 4 and all threads, written as linear light planes for tracer_bakes.html.
//
// Methods (see README.md for the derivations and what each leaves out):
//   relief  today's approach: Lambert shading, a k-pixel drop shadow (the
//           relief primitive's contact-shadow rule), unoccluded hemisphere
//           ambient and an SDF-style lamp glow that ignores occlusion.
//   teapot  conv-teapot's light: hard point visibility of each source, the
//           open fraction from 32 cosine-weighted Fibonacci directions rotated
//           per texel by a hash (quarter first), and the transfer gather: six
//           Jacobi passes of B = B0 + (1/N) sum over blocked rays of rho*B_land.
//   window  as teapot, but each source (the fixed light's disc and the
//           spherical lamp) is integrated over an exact visibility partition in
//           the spirit of bfft's source windows: for every column of the source
//           the horizon decides exactly where the visible part ends, class
//           changes across columns are located by bisection, fully visible runs
//           are integrated in closed form and only partial runs by quadrature.
//
// Scene model: a height field over the unit chart, seen from the front; x
// right, y up (row 0 is the top), z out of the panel. Plane values are the
// multiplier on a texel's albedo: outgoing radiance = albedo x plane. The
// fixed light has unit irradiance at normal incidence, the sky radiance 0.22
// of it; the lamp is a sphere of unit radiance, and lens texels emit 1.
//
// Usage: tracer_bakes <output directory> [size, even, 32..512, default 160]
// Writes, per scene, <scene>_albedo.f32 (int32 width, height, 3; then RGB
// float32 interleaved) and <scene>_<method>.f32 (int32 width, height, 2; then
// the fixed-light plane and the lamp plane, float32), plus planes.js for the
// page and timings.json. Every bake is repeated three times per thread count
// and the median is reported; the planes of every thread count are compared
// byte for byte with the single-thread planes.
//
// Build: c++ -std=c++20 -O2 -ffp-contract=off -pthread -o build/tracer_bakes tracer_bakes.cpp
// Determinism: no FMA contraction and a fixed order in every sum, so output is
// identical for any thread count on one host. Bytes across hosts are not
// claimed: std::sin, std::cos, std::asin and std::atan come from libm (the
// engine port replaces them with tables, as the finish kernels do).

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace tracer_bakes {

constexpr double pi = 3.14159265358979323846;
constexpr double golden_angle = 2.39996322972865332;
constexpr int direction_count = 32;
constexpr int quarter_stride = 4;
constexpr int jacobi_passes = 6;
constexpr int repeats = 3;
constexpr double sky_radiance = 0.22;
constexpr double sun_angular_radius = 0.06;
constexpr double lamp_radiance = 1.0;
constexpr double far_distance = 3.0;

struct Vec3 final {
    double x{};
    double y{};
    double z{};
};

Vec3 add(Vec3 a, Vec3 b) { return Vec3{a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 sub(Vec3 a, Vec3 b) { return Vec3{a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 scale(Vec3 a, double s) { return Vec3{a.x * s, a.y * s, a.z * s}; }
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 cross(Vec3 a, Vec3 b) { return Vec3{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
double length(Vec3 a) { return std::sqrt(dot(a, a)); }
Vec3 normalized(Vec3 a) { return scale(a, 1.0 / length(a)); }

double smoothstep(double a, double b, double x) {
    const double t = std::clamp((x - a) / (b - a), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

// The one fixed light, from the upper left (pointing towards the light).
Vec3 fixed_light_direction() { return normalized(Vec3{-0.45, 0.55, 0.70}); }

// ---- scenes ------------------------------------------------------------------------------

enum class Material : std::uint8_t { plate = 0, floor = 1, cap = 2, bezel = 3, wall = 4, lens = 5 };

constexpr std::array<std::array<float, 3>, 6> material_albedo{{
    {0.40f, 0.43f, 0.38f},  // plate: grey-green enamel
    {0.16f, 0.16f, 0.15f},  // floor: dark well floor
    {0.80f, 0.77f, 0.68f},  // cap: ivory keycap
    {0.52f, 0.53f, 0.55f},  // bezel: grey metal
    {0.86f, 0.85f, 0.82f},  // wall: white enamel well
    {0.92f, 0.92f, 0.90f},  // lens: white diffuser
}};

double albedo_luma(Material material) {
    const std::array<float, 3>& rgb = material_albedo[static_cast<int>(material)];
    return 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2];
}

struct Lamp final {
    Vec3 centre{};   // sphere centre
    double radius{}; // sphere radius
    double glow{};   // relief method only: SDF spill width beyond the sphere
};

struct Scene final {
    std::string name{};
    std::string title{};
    int size{};
    std::vector<double> height{};     // size x size, row-major, row 0 at the top
    std::vector<Material> material{}; // size x size
    std::vector<Vec3> position{};     // texel centre on the surface
    std::vector<Vec3> normal{};
    Lamp lamp{};
    double height_max{};
};

struct TexelShape final {
    double height{};
    Material material{Material::plate};
};

double rounded_box(double x, double y, double cx, double cy, double hx, double hy, double r) {
    const double qx = std::abs(x - cx) - hx + r;
    const double qy = std::abs(y - cy) - hy + r;
    const double ox = std::max(qx, 0.0);
    const double oy = std::max(qy, 0.0);
    const double outside = std::sqrt(ox * ox + oy * oy);
    const double inside = std::min(std::max(qx, qy), 0.0);
    return outside + inside - r;
}

// Quarter-circle rise over `width` inwards from an edge at signed distance d.
double rounded_rise(double d, double width) {
    const double s = std::clamp(-d / width, 0.0, 1.0);
    const double from_top = 1.0 - s;
    return std::sqrt(std::max(0.0, 1.0 - from_top * from_top));
}

double dome(double r, double radius) {
    const double u = r / radius;
    return std::sqrt(std::max(0.0, 1.0 - u * u));
}

// A keycap in a well, with an indicator lens inset at its upper right.
TexelShape shape_keycap(double x, double y, double wall) {
    TexelShape shape{};
    const double well = rounded_box(x, y, 0.5, 0.5, 0.36, 0.36, 0.05);
    shape.height = -0.12 * (1.0 - smoothstep(-wall, wall, well));
    shape.material = well < 0.0 ? Material::floor : Material::plate;
    const double cap = rounded_box(x, y, 0.5, 0.5, 0.29, 0.29, 0.06);
    if (cap < 0.0) {
        const double rx = x - 0.5;
        const double ry = y - 0.5;
        const double dish = 0.012 * (1.0 - std::clamp((rx * rx + ry * ry) / 0.09, 0.0, 1.0));
        const double top = -0.12 + 0.18 * rounded_rise(cap, 0.035) - dish;
        shape.height = std::max(shape.height, top);
        shape.material = Material::cap;
    }
    const double lens_r = std::hypot(x - 0.68, y - 0.68);
    if (lens_r < 0.035) {
        shape.height += 0.008 * dome(lens_r, 0.035);
        shape.material = Material::lens;
    }
    return shape;
}

// A domed lamp lens in a seat, ringed by a raised bezel.
TexelShape shape_lamp(double x, double y, double) {
    TexelShape shape{};
    const double r = std::hypot(x - 0.5, y - 0.5);
    if (r < 0.20) {
        shape.height = 0.015;
        shape.material = Material::bezel;
        if (r < 0.17) {
            shape.height = 0.015 + 0.075 * dome(r, 0.17);
            shape.material = Material::lens;
        }
    } else if (r < 0.32) {
        shape.height = 0.10 * dome(r - 0.26, 0.06);
        shape.material = Material::bezel;
    }
    return shape;
}

// A bar lying on the plate (contact crease) with a slot lamp at its right end,
// and a low strip below it (a one-to-two-pixel contact shadow).
TexelShape shape_contact(double x, double y, double) {
    TexelShape shape{};
    const double strip = rounded_box(x, y, 0.5, 0.2, 0.35, 0.03, 0.01);
    if (strip < 0.0) {
        shape.height = 0.012 * rounded_rise(strip, 0.008);
        shape.material = Material::bezel;
    }
    const double bar = rounded_box(x, y, 0.5, 0.5, 0.35, 0.08, 0.03);
    if (bar < 0.0) {
        shape.height = 0.05 * rounded_rise(bar, 0.02);
        shape.material = Material::cap;
    }
    const double lens_r = std::hypot(x - 0.78, y - 0.5);
    if (lens_r < 0.025) {
        shape.height += 0.006 * dome(lens_r, 0.025);
        shape.material = Material::lens;
    }
    return shape;
}

// A deep white key well with a lamp in its floor: walls lit directly, the floor
// around the lamp only by light returned from the walls.
TexelShape shape_well(double x, double y, double wall) {
    TexelShape shape{};
    const double well = rounded_box(x, y, 0.5, 0.5, 0.30, 0.30, 0.06);
    shape.height = -0.30 * (1.0 - smoothstep(-wall, wall, well));
    shape.material = well < 0.0 ? Material::wall : Material::plate;
    if (std::hypot(x - 0.5, y - 0.5) < 0.06) {
        shape.material = Material::lens;
    }
    return shape;
}

using ShapeFunction = TexelShape (*)(double x, double y, double wall);

struct SceneSpec final {
    const char* name{};
    const char* title{};
    ShapeFunction shape{};
    Lamp lamp{};
};

const std::array<SceneSpec, 4> scene_specs{{
    {"keycap", "Keycap in a well, inset lens", shape_keycap, Lamp{Vec3{0.68, 0.68, 0.075}, 0.03, 0.08}},
    {"lamp", "Lamp lens and bezel", shape_lamp, Lamp{Vec3{0.5, 0.5, 0.05}, 0.10, 0.10}},
    {"contact", "Contact-shadow strip and bar", shape_contact, Lamp{Vec3{0.78, 0.5, 0.056}, 0.02, 0.06}},
    {"well", "Inter-reflection in a key well", shape_well, Lamp{Vec3{0.5, 0.5, -0.265}, 0.05, 0.10}},
}};

int texel_index(const Scene& scene, int i, int j) { return j * scene.size + i; }

Scene make_scene(const SceneSpec& spec, int size) {
    Scene scene{};
    scene.name = spec.name;
    scene.title = spec.title;
    scene.size = size;
    scene.lamp = spec.lamp;
    const std::size_t count = static_cast<std::size_t>(size) * static_cast<std::size_t>(size);
    scene.height.resize(count);
    scene.material.resize(count);
    scene.position.resize(count);
    scene.normal.resize(count);
    const double wall = 1.5 / size; // walls are softened over three texels
    scene.height_max = -1.0e300;
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i < size; ++i) {
            const double x = (i + 0.5) / size;
            const double y = 1.0 - (j + 0.5) / size;
            const TexelShape shape = spec.shape(x, y, wall);
            const int index = texel_index(scene, i, j);
            scene.height[index] = shape.height;
            scene.material[index] = shape.material;
            scene.position[index] = Vec3{x, y, shape.height};
            scene.height_max = std::max(scene.height_max, shape.height);
        }
    }
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i < size; ++i) {
            const int il = std::max(i - 1, 0);
            const int ir = std::min(i + 1, size - 1);
            const int ju = std::max(j - 1, 0); // the row above has the larger y
            const int jd = std::min(j + 1, size - 1);
            const double dhdx = (scene.height[texel_index(scene, ir, j)] - scene.height[texel_index(scene, il, j)]) *
                                size / (ir - il);
            const double dhdy = (scene.height[texel_index(scene, i, ju)] - scene.height[texel_index(scene, i, jd)]) *
                                size / (jd - ju);
            scene.normal[texel_index(scene, i, j)] = normalized(Vec3{-dhdx, -dhdy, 1.0});
        }
    }
    return scene;
}

// ---- geometry queries ---------------------------------------------------------------------

// Bilinear height at a chart point; outside the chart the edge texels extend.
double height_at(const Scene& scene, double x, double y) {
    const int n = scene.size;
    const double fx = std::clamp(x * n - 0.5, 0.0, n - 1.0);
    const double fy = std::clamp((1.0 - y) * n - 0.5, 0.0, n - 1.0);
    const int i0 = std::min(static_cast<int>(fx), n - 2);
    const int j0 = std::min(static_cast<int>(fy), n - 2);
    const double tx = fx - i0;
    const double ty = fy - j0;
    const double h00 = scene.height[texel_index(scene, i0, j0)];
    const double h10 = scene.height[texel_index(scene, i0 + 1, j0)];
    const double h01 = scene.height[texel_index(scene, i0, j0 + 1)];
    const double h11 = scene.height[texel_index(scene, i0 + 1, j0 + 1)];
    const double top = h00 + (h10 - h00) * tx;
    const double bottom = h01 + (h11 - h01) * tx;
    return top + (bottom - top) * ty;
}

int texel_at(const Scene& scene, double x, double y) {
    const int n = scene.size;
    const int i = std::clamp(static_cast<int>(std::floor(x * n)), 0, n - 1);
    const int j = std::clamp(static_cast<int>(std::floor((1.0 - y) * n)), 0, n - 1);
    return texel_index(scene, i, j);
}

struct Hit final {
    bool blocked{};
    int texel{-1};
};

// March a ray in half-texel steps; a rising ray above every height escapes.
Hit march(const Scene& scene, Vec3 origin, Vec3 direction, double max_distance) {
    const double step = 0.5 / scene.size;
    const int steps = static_cast<int>(max_distance / step);
    for (int k = 1; k <= steps; ++k) {
        const Vec3 p = add(origin, scale(direction, step * k));
        if (direction.z >= 0.0 && p.z > scene.height_max) {
            return Hit{false, -1};
        }
        if (p.z < height_at(scene, p.x, p.y)) {
            return Hit{true, texel_at(scene, p.x, p.y)};
        }
    }
    return Hit{false, -1};
}

// Largest tangent of elevation of the height field seen from origin along a
// horizontal azimuth, up to max_distance. Stops once nothing ahead can be higher.
double horizon_slope(const Scene& scene, Vec3 origin, double ax, double ay, double max_distance) {
    const double step = 0.5 / scene.size;
    const int steps = std::max(1, static_cast<int>(max_distance / step));
    double best = -1.0e300;
    for (int k = 1; k <= steps; ++k) {
        const double d = step * k;
        const double ground = height_at(scene, origin.x + ax * d, origin.y + ay * d);
        best = std::max(best, (ground - origin.z) / d);
        if ((scene.height_max - origin.z) / d <= best) {
            break;
        }
    }
    return best;
}

Vec3 surface_origin(const Scene& scene, int index) {
    return add(scene.position[index], scale(scene.normal[index], 0.75 / scene.size));
}

// ---- the source-window partition (method window) ------------------------------------------
//
// A disc source of angular radius r centred at elevation e0 is cut into
// columns by lateral angle u in [-r, r]; column u spans elevations
// e0 +- sqrt(r^2 - u^2) along azimuth e0's azimuth turned by u / cos(e0) (small
// angles). The horizon H(u) along that azimuth decides exactly how much of the
// column is visible: clamp(top - max(bottom, H), 0, top - bottom). Columns are
// classed blocked (0), partial (1) or full (2); where the class changes the
// boundary is found by bisection, so quadrature never decides where a shadow
// starts. Full runs integrate in closed form: the integral of 2 sqrt(r^2 - u^2)
// is u sqrt(r^2 - u^2) + r^2 asin(u / r).

struct ConeWindow final {
    const Scene* scene{};
    Vec3 origin{};
    double azimuth_x{};
    double azimuth_y{};
    double elevation{};
    double radius{};
    double azimuth_scale{};
    double max_distance{};
};

constexpr int class_blocked = 0;
constexpr int class_partial = 1;
constexpr int class_full = 2;

struct Column final {
    double visible{};
    int column_class{};
};

Column evaluate_column(const ConeWindow& window, double u) {
    const double turn = u * window.azimuth_scale;
    const double c = std::cos(turn);
    const double s = std::sin(turn);
    const double ax = window.azimuth_x * c - window.azimuth_y * s;
    const double ay = window.azimuth_x * s + window.azimuth_y * c;
    const double horizon = std::atan(horizon_slope(*window.scene, window.origin, ax, ay, window.max_distance));
    const double half = std::sqrt(std::max(0.0, window.radius * window.radius - u * u));
    const double top = window.elevation + half;
    const double bottom = window.elevation - half;
    Column column{};
    column.visible = std::clamp(top - std::max(bottom, horizon), 0.0, 2.0 * half);
    if (horizon >= top) {
        column.column_class = class_blocked;
    } else if (horizon <= bottom) {
        column.column_class = class_full;
    } else {
        column.column_class = class_partial;
    }
    return column;
}

double full_columns(double r, double a, double b) {
    const double fa = a * std::sqrt(std::max(0.0, r * r - a * a)) + r * r * std::asin(std::clamp(a / r, -1.0, 1.0));
    const double fb = b * std::sqrt(std::max(0.0, r * r - b * b)) + r * r * std::asin(std::clamp(b / r, -1.0, 1.0));
    return fb - fa;
}

// Four-point Gauss-Legendre over a run of columns.
double gauss_columns(const ConeWindow& window, double a, double b) {
    constexpr std::array<double, 4> nodes{-0.8611363115940526, -0.3399810435848563, 0.3399810435848563,
                                          0.8611363115940526};
    constexpr std::array<double, 4> weights{0.3478548451374538, 0.6521451548625461, 0.6521451548625461,
                                            0.3478548451374538};
    const double half = 0.5 * (b - a);
    const double middle = 0.5 * (a + b);
    double sum = 0.0;
    for (int k = 0; k < 4; ++k) {
        sum += weights[k] * evaluate_column(window, middle + half * nodes[k]).visible;
    }
    return sum * half;
}

double uniform_run(const ConeWindow& window, double a, double b, int column_class) {
    if (column_class == class_full) {
        return full_columns(window.radius, a, b);
    }
    if (column_class == class_blocked) {
        return 0.0;
    }
    return gauss_columns(window, a, b);
}

// Integrate visible column length over [a, b]. The class change is located to
// 1/1024 of the run; that sliver is left out.
double integrate_run(const ConeWindow& window, double a, double b, int class_a, int class_b, int depth) {
    if (class_a == class_b) {
        return uniform_run(window, a, b, class_a);
    }
    if (depth == 0) {
        return gauss_columns(window, a, b);
    }
    double lo = a;
    double hi = b;
    int class_hi = class_b;
    for (int k = 0; k < 10; ++k) {
        const double mid = 0.5 * (lo + hi);
        const int class_mid = evaluate_column(window, mid).column_class;
        if (class_mid == class_a) {
            lo = mid;
        } else {
            hi = mid;
            class_hi = class_mid;
        }
    }
    const double left = uniform_run(window, a, lo, class_a);
    return left + integrate_run(window, hi, b, class_hi, class_b, depth - 1);
}

// Visible fraction of a disc source (axis, angular radius) from origin.
double cone_visible_fraction(const Scene& scene, Vec3 origin, Vec3 axis, double radius, double max_distance) {
    const double horizontal = std::sqrt(axis.x * axis.x + axis.y * axis.y);
    const double elevation = std::asin(std::clamp(axis.z, -1.0, 1.0));
    if (horizontal < 1.0e-9 || std::abs(elevation) + radius > 0.5 * pi - 0.02) {
        // Near the zenith the azimuth columns degenerate: fall back to the centre ray.
        return march(scene, origin, axis, max_distance).blocked ? 0.0 : 1.0;
    }
    ConeWindow window{};
    window.scene = &scene;
    window.origin = origin;
    window.azimuth_x = axis.x / horizontal;
    window.azimuth_y = axis.y / horizontal;
    window.elevation = elevation;
    window.radius = radius;
    window.azimuth_scale = 1.0 / std::cos(elevation);
    window.max_distance = max_distance;
    constexpr int runs = 8;
    std::array<double, runs + 1> u{};
    std::array<int, runs + 1> classes{};
    for (int k = 0; k <= runs; ++k) {
        u[k] = -radius + 2.0 * radius * k / runs;
        classes[k] = evaluate_column(window, u[k]).column_class;
    }
    double visible = 0.0;
    for (int k = 0; k < runs; ++k) {
        visible += integrate_run(window, u[k], u[k + 1], classes[k], classes[k + 1], 3);
    }
    return std::clamp(visible / (pi * radius * radius), 0.0, 1.0);
}

// ---- direct light -------------------------------------------------------------------------

enum class Method : int { relief = 0, teapot = 1, window = 2 };

constexpr std::array<Method, 3> methods{Method::relief, Method::teapot, Method::window};

const char* method_name(Method method) {
    switch (method) {
    case Method::relief:
        return "relief";
    case Method::teapot:
        return "teapot";
    case Method::window:
        return "window";
    default:
        return "unknown";
    }
}

// Relief method: the contact-shadow rule k = max(1, floor(min(s, 2) + 0.5))
// pixels, applied as a drop shadow offset towards the light. s is the relief
// size in pixels at this chart size (2 at 160 px).
double relief_shadow(const Scene& scene, int i, int j) {
    const Vec3 light = fixed_light_direction();
    const double s = scene.size / 80.0;
    const int k = std::max(1, static_cast<int>(std::floor(std::min(s, 2.0) + 0.5)));
    const double horizontal = std::sqrt(light.x * light.x + light.y * light.y);
    const int oi = std::clamp(i + static_cast<int>(std::lround(light.x / horizontal * k)), 0, scene.size - 1);
    const int oj = std::clamp(j - static_cast<int>(std::lround(light.y / horizontal * k)), 0, scene.size - 1);
    const double own = scene.height[texel_index(scene, i, j)];
    const double other = scene.height[texel_index(scene, oi, oj)];
    return other > own + 0.01 ? 0.45 : 1.0;
}

struct DirectLight final {
    double sun{};
    double lamp{};
};

DirectLight direct_light(const Scene& scene, Method method, int i, int j) {
    const int index = texel_index(scene, i, j);
    const Vec3 n = scene.normal[index];
    const Vec3 origin = surface_origin(scene, index);
    const Vec3 light = fixed_light_direction();
    const double n_dot_l = dot(n, light);
    DirectLight result{};
    if (n_dot_l > 0.0) {
        if (method == Method::relief) {
            result.sun = n_dot_l * relief_shadow(scene, i, j);
        } else if (method == Method::teapot) {
            result.sun = march(scene, origin, light, far_distance).blocked ? 0.0 : n_dot_l;
        } else {
            result.sun = n_dot_l * cone_visible_fraction(scene, origin, light, sun_angular_radius, far_distance);
        }
    }
    if (scene.material[index] == Material::lens) {
        result.lamp = lamp_radiance; // the lens emits; albedo x 1 shows the lamp colour
        return result;
    }
    // Sphere of radius r at distance d: irradiance Le * pi * sin^2(beta) * cos(theta)
    // unoccluded, beta = asin(r / d); the plane stores irradiance / pi.
    const Vec3 to_lamp = sub(scene.lamp.centre, origin);
    const double distance = length(to_lamp);
    const Vec3 direction = scale(to_lamp, 1.0 / distance);
    const double n_dot = dot(n, direction);
    if (n_dot <= 0.0 || distance <= scene.lamp.radius) {
        return result;
    }
    const double sin_beta = scene.lamp.radius / distance;
    const double unoccluded = lamp_radiance * sin_beta * sin_beta * n_dot;
    const double to_surface = distance - scene.lamp.radius;
    if (method == Method::relief) {
        const double reach = std::hypot(to_lamp.x, to_lamp.y) - scene.lamp.radius;
        result.lamp = unoccluded * (1.0 - smoothstep(0.0, scene.lamp.glow, reach));
    } else if (method == Method::teapot) {
        result.lamp = march(scene, origin, direction, to_surface).blocked ? 0.0 : unoccluded;
    } else {
        const double beta = std::asin(sin_beta);
        result.lamp = unoccluded * cone_visible_fraction(scene, origin, direction, beta, to_surface);
    }
    return result;
}

// ---- the bake ----------------------------------------------------------------------------

std::uint32_t texel_hash(int i, int j) {
    std::uint32_t h = static_cast<std::uint32_t>(i) * 73856093u ^ static_cast<std::uint32_t>(j) * 19349663u;
    h ^= h >> 13;
    h *= 0x5bd1e995u;
    h ^= h >> 15;
    return h;
}

struct Bake final {
    const Scene* scene{};
    Method method{};
    int coarse{};                      // coarse side: texels four times the area of the fine ones
    std::vector<double> sun{};         // fine
    std::vector<double> lamp{};        // fine
    std::vector<double> open{};        // coarse open fraction
    std::vector<int> landing{};        // coarse x direction_count; -1 = open
    std::vector<std::uint8_t> weight{};// coarse x direction_count
    std::vector<double> fixed_base{};  // fine B0 of the fixed plane (sun + sky)
    std::array<std::vector<double>, 2> bounce_fixed{};
    std::array<std::vector<double>, 2> bounce_lamp{};
    int read{};                        // which bounce buffer a pass reads
};

using RowFunction = void (*)(Bake* bake, int first_row, int end_row);

// Run rows [0, rows) on `threads` threads in contiguous bands. Every texel is
// written by one band only and reads only data finished before this call.
void run_rows(int threads, int rows, RowFunction function, Bake* bake) {
    const int count = std::clamp(threads, 1, rows);
    if (count == 1) {
        function(bake, 0, rows);
        return;
    }
    std::vector<std::thread> workers{};
    workers.reserve(static_cast<std::size_t>(count - 1));
    for (int t = 1; t < count; ++t) {
        workers.emplace_back(function, bake, rows * t / count, rows * (t + 1) / count);
    }
    function(bake, 0, rows / count);
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void direct_rows(Bake* bake, int first_row, int end_row) {
    const Scene& scene = *(*bake).scene;
    for (int j = first_row; j < end_row; ++j) {
        for (int i = 0; i < scene.size; ++i) {
            const DirectLight light = direct_light(scene, (*bake).method, i, j);
            const int index = texel_index(scene, i, j);
            (*bake).sun[index] = light.sun;
            (*bake).lamp[index] = light.lamp;
        }
    }
}

// Open fraction and landings at the coarse texel represented by fine (2ci, 2cj):
// 32 cosine-weighted directions on a Fibonacci spiral, r = sqrt((k + 0.5) / 32),
// angle = golden_angle * k + a per-texel rotation. Every fourth direction is
// cast first; if those eight agree, they stand for all 32 with weight 4.
void open_rows(Bake* bake, int first_row, int end_row) {
    const Scene& scene = *(*bake).scene;
    for (int cj = first_row; cj < end_row; ++cj) {
        for (int ci = 0; ci < (*bake).coarse; ++ci) {
            const int index = texel_index(scene, 2 * ci, 2 * cj);
            const Vec3 n = scene.normal[index];
            const Vec3 origin = surface_origin(scene, index);
            const Vec3 helper = std::abs(n.z) < 0.9 ? Vec3{0.0, 0.0, 1.0} : Vec3{1.0, 0.0, 0.0};
            const Vec3 tangent = normalized(cross(helper, n));
            const Vec3 bitangent = cross(n, tangent);
            const double rotation = 2.0 * pi * (texel_hash(ci, cj) / 4294967296.0);
            const int base = (cj * (*bake).coarse + ci) * direction_count;
            int quarter_open = 0;
            for (int k = 0; k < direction_count; ++k) {
                const bool quarter = k % quarter_stride == 0;
                (*bake).landing[base + k] = -1;
                (*bake).weight[base + k] = 0;
                if (!quarter) {
                    continue;
                }
                const double r = std::sqrt((k + 0.5) / direction_count);
                const double angle = golden_angle * k + rotation;
                const Vec3 local = add(scale(tangent, r * std::cos(angle)), scale(bitangent, r * std::sin(angle)));
                const Vec3 direction = add(local, scale(n, std::sqrt(1.0 - r * r)));
                const Hit hit = march(scene, origin, direction, far_distance);
                (*bake).landing[base + k] = hit.blocked ? hit.texel : -1;
                (*bake).weight[base + k] = quarter_stride;
                quarter_open += hit.blocked ? 0 : 1;
            }
            const int quarter_count = direction_count / quarter_stride;
            if (quarter_open == 0 || quarter_open == quarter_count) {
                (*bake).open[cj * (*bake).coarse + ci] = static_cast<double>(quarter_open) / quarter_count;
                continue;
            }
            int open_count = 0;
            for (int k = 0; k < direction_count; ++k) {
                if (k % quarter_stride == 0) {
                    (*bake).weight[base + k] = 1;
                    open_count += (*bake).landing[base + k] < 0 ? 1 : 0;
                    continue;
                }
                const double r = std::sqrt((k + 0.5) / direction_count);
                const double angle = golden_angle * k + rotation;
                const Vec3 local = add(scale(tangent, r * std::cos(angle)), scale(bitangent, r * std::sin(angle)));
                const Vec3 direction = add(local, scale(n, std::sqrt(1.0 - r * r)));
                const Hit hit = march(scene, origin, direction, far_distance);
                (*bake).landing[base + k] = hit.blocked ? hit.texel : -1;
                (*bake).weight[base + k] = 1;
                open_count += hit.blocked ? 0 : 1;
            }
            (*bake).open[cj * (*bake).coarse + ci] = static_cast<double>(open_count) / direction_count;
        }
    }
}

// Bilinear sample of a coarse plane at fine texel (i, j).
double coarse_sample(const std::vector<double>& plane, int coarse, int i, int j) {
    const double fx = std::clamp(i * 0.5, 0.0, coarse - 1.0);
    const double fy = std::clamp(j * 0.5, 0.0, coarse - 1.0);
    const int x0 = std::min(static_cast<int>(fx), coarse - 2);
    const int y0 = std::min(static_cast<int>(fy), coarse - 2);
    const double tx = fx - x0;
    const double ty = fy - y0;
    const double a = plane[y0 * coarse + x0];
    const double b = plane[y0 * coarse + x0 + 1];
    const double c = plane[(y0 + 1) * coarse + x0];
    const double d = plane[(y0 + 1) * coarse + x0 + 1];
    const double top = a + (b - a) * tx;
    const double bottom = c + (d - c) * tx;
    return top + (bottom - top) * ty;
}

void base_rows(Bake* bake, int first_row, int end_row) {
    const Scene& scene = *(*bake).scene;
    for (int j = first_row; j < end_row; ++j) {
        for (int i = 0; i < scene.size; ++i) {
            const int index = texel_index(scene, i, j);
            const double open = coarse_sample((*bake).open, (*bake).coarse, i, j);
            (*bake).fixed_base[index] = (*bake).sun[index] + sky_radiance * open;
        }
    }
}

// One Jacobi pass: bounce(t) = (1/32) sum_k w_k rho(land) (B0(land) + bounce_prev(land)).
// In the lamp plane, landings on the lens are left out: the direct term
// already counts the lamp's light.
void gather_rows(Bake* bake, int first_row, int end_row) {
    const Scene& scene = *(*bake).scene;
    const int coarse = (*bake).coarse;
    const std::vector<double>& prev_fixed = (*bake).bounce_fixed[(*bake).read];
    const std::vector<double>& prev_lamp = (*bake).bounce_lamp[(*bake).read];
    std::vector<double>& next_fixed = (*bake).bounce_fixed[1 - (*bake).read];
    std::vector<double>& next_lamp = (*bake).bounce_lamp[1 - (*bake).read];
    for (int cj = first_row; cj < end_row; ++cj) {
        for (int ci = 0; ci < coarse; ++ci) {
            const int base = (cj * coarse + ci) * direction_count;
            double sum_fixed = 0.0;
            double sum_lamp = 0.0;
            for (int k = 0; k < direction_count; ++k) {
                const int land = (*bake).landing[base + k];
                if (land < 0) {
                    continue;
                }
                const double w = (*bake).weight[base + k];
                const Material material = scene.material[land];
                const double rho = albedo_luma(material);
                const int li = land % scene.size;
                const int lj = land / scene.size;
                const int land_coarse = std::min(lj / 2, coarse - 1) * coarse + std::min(li / 2, coarse - 1);
                sum_fixed += w * rho * ((*bake).fixed_base[land] + prev_fixed[land_coarse]);
                if (material != Material::lens) {
                    sum_lamp += w * rho * ((*bake).lamp[land] + prev_lamp[land_coarse]);
                }
            }
            next_fixed[cj * coarse + ci] = sum_fixed / direction_count;
            next_lamp[cj * coarse + ci] = sum_lamp / direction_count;
        }
    }
}

std::vector<double> box_smooth(const std::vector<double>& plane, int side) {
    std::vector<double> out(plane.size());
    for (int y = 0; y < side; ++y) {
        for (int x = 0; x < side; ++x) {
            double sum = 0.0;
            int count = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const int sx = x + dx;
                    const int sy = y + dy;
                    if (sx >= 0 && sx < side && sy >= 0 && sy < side) {
                        sum += plane[sy * side + sx];
                        count += 1;
                    }
                }
            }
            out[y * side + x] = sum / count;
        }
    }
    return out;
}

struct Planes final {
    std::vector<float> fixed{};
    std::vector<float> lamp{};
};

Planes bake_scene(const Scene& scene, Method method, int threads) {
    Bake bake{};
    bake.scene = &scene;
    bake.method = method;
    bake.coarse = scene.size / 2;
    const std::size_t fine = scene.height.size();
    const std::size_t coarse = static_cast<std::size_t>(bake.coarse) * static_cast<std::size_t>(bake.coarse);
    bake.sun.assign(fine, 0.0);
    bake.lamp.assign(fine, 0.0);
    run_rows(threads, scene.size, direct_rows, &bake);
    Planes planes{};
    planes.fixed.resize(fine);
    planes.lamp.resize(fine);
    if (method == Method::relief) {
        for (std::size_t t = 0; t < fine; ++t) {
            const double ambient = sky_radiance * (0.5 + 0.5 * scene.normal[t].z);
            planes.fixed[t] = static_cast<float>(bake.sun[t] + ambient);
            planes.lamp[t] = static_cast<float>(bake.lamp[t]);
        }
        return planes;
    }
    bake.open.assign(coarse, 0.0);
    bake.landing.assign(coarse * direction_count, -1);
    bake.weight.assign(coarse * direction_count, 0);
    run_rows(threads, bake.coarse, open_rows, &bake);
    bake.fixed_base.assign(fine, 0.0);
    run_rows(threads, scene.size, base_rows, &bake);
    for (int b = 0; b < 2; ++b) {
        bake.bounce_fixed[b].assign(coarse, 0.0);
        bake.bounce_lamp[b].assign(coarse, 0.0);
    }
    for (int pass = 0; pass < jacobi_passes; ++pass) {
        bake.read = pass % 2;
        run_rows(threads, bake.coarse, gather_rows, &bake);
    }
    const int last = jacobi_passes % 2;
    const std::vector<double> smooth_fixed = box_smooth(bake.bounce_fixed[last], bake.coarse);
    const std::vector<double> smooth_lamp = box_smooth(bake.bounce_lamp[last], bake.coarse);
    for (int j = 0; j < scene.size; ++j) {
        for (int i = 0; i < scene.size; ++i) {
            const int t = texel_index(scene, i, j);
            const double bounce_fixed = coarse_sample(smooth_fixed, bake.coarse, i, j);
            const double bounce_lamp = coarse_sample(smooth_lamp, bake.coarse, i, j);
            planes.fixed[t] = static_cast<float>(bake.fixed_base[t] + bounce_fixed);
            const bool lens = scene.material[t] == Material::lens;
            planes.lamp[t] = static_cast<float>(lens ? bake.lamp[t] : bake.lamp[t] + bounce_lamp);
        }
    }
    return planes;
}

// ---- output ------------------------------------------------------------------------------

bool write_planes(const std::string& path, int width, int height, const std::vector<const std::vector<float>*>& planes,
                  int channels) {
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    const std::array<std::int32_t, 3> header{width, height, channels};
    bool ok = std::fwrite(header.data(), sizeof(std::int32_t), header.size(), file) == header.size();
    for (const std::vector<float>* plane : planes) {
        ok = ok && std::fwrite((*plane).data(), sizeof(float), (*plane).size(), file) == (*plane).size();
    }
    ok = (std::fclose(file) == 0) && ok;
    return ok;
}

std::string base64(const std::vector<float>& values) {
    static const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<std::uint8_t> bytes(values.size() * sizeof(float));
    std::memcpy(bytes.data(), values.data(), bytes.size()); // little-endian hosts only
    std::string out{};
    out.reserve((bytes.size() + 2) / 3 * 4);
    for (std::size_t k = 0; k < bytes.size(); k += 3) {
        const std::uint32_t b0 = bytes[k];
        const std::uint32_t b1 = k + 1 < bytes.size() ? bytes[k + 1] : 0u;
        const std::uint32_t b2 = k + 2 < bytes.size() ? bytes[k + 2] : 0u;
        const std::uint32_t triple = (b0 << 16) | (b1 << 8) | b2;
        out.push_back(alphabet[(triple >> 18) & 63u]);
        out.push_back(alphabet[(triple >> 12) & 63u]);
        out.push_back(k + 1 < bytes.size() ? alphabet[(triple >> 6) & 63u] : '=');
        out.push_back(k + 2 < bytes.size() ? alphabet[triple & 63u] : '=');
    }
    return out;
}

bool same_bytes(const Planes& a, const Planes& b) {
    return a.fixed.size() == b.fixed.size() && a.lamp.size() == b.lamp.size() &&
           std::memcmp(a.fixed.data(), b.fixed.data(), a.fixed.size() * sizeof(float)) == 0 &&
           std::memcmp(a.lamp.data(), b.lamp.data(), a.lamp.size() * sizeof(float)) == 0;
}

struct Timed final {
    Planes planes{};
    double median_ms{};
};

Timed timed_bake(const Scene& scene, Method method, int threads) {
    std::array<double, repeats> times{};
    Timed timed{};
    for (int r = 0; r < repeats; ++r) {
        const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        timed.planes = bake_scene(scene, method, threads);
        const std::chrono::steady_clock::time_point stop = std::chrono::steady_clock::now();
        times[r] = std::chrono::duration<double, std::milli>(stop - start).count();
    }
    std::sort(times.begin(), times.end());
    timed.median_ms = times[repeats / 2];
    return timed;
}

} // namespace tracer_bakes

int main(int argc, char** argv) {
    using namespace tracer_bakes;
    if (argc < 2) {
        std::fprintf(stderr, "usage: tracer_bakes <output directory> [size]\n");
        return 2;
    }
    const std::string directory = argv[1];
    const int size = argc > 2 ? std::atoi(argv[2]) : 160;
    if (size < 32 || size > 512 || size % 2 != 0) {
        std::fprintf(stderr, "size must be even and in 32..512\n");
        return 2;
    }
    const int hardware = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
    const std::array<int, 3> thread_counts{1, 4, hardware};
    std::string js = "window.TRACER_BAKES = {\n";
    js += "\"size\": " + std::to_string(size) + ",\n\"threads\": [1, 4, " + std::to_string(hardware) + "],\n";
    js += "\"scenes\": [\n";
    std::string timings = "{\"size\": " + std::to_string(size) + ", \"threads\": [1, 4, " + std::to_string(hardware) +
                          "], \"bakes\": [\n";
    bool all_identical = true;
    bool written = true;
    std::printf("size %d, threads 1 / 4 / %d, median of %d, ms\n", size, hardware, repeats);
    for (std::size_t s = 0; s < scene_specs.size(); ++s) {
        const Scene scene = make_scene(scene_specs[s], size);
        std::vector<float> albedo(scene.height.size() * 3);
        for (std::size_t t = 0; t < scene.height.size(); ++t) {
            const std::array<float, 3>& rgb = material_albedo[static_cast<int>(scene.material[t])];
            albedo[t * 3] = rgb[0];
            albedo[t * 3 + 1] = rgb[1];
            albedo[t * 3 + 2] = rgb[2];
        }
        const std::vector<const std::vector<float>*> albedo_planes{&albedo};
        written = written && write_planes(directory + "/" + scene.name + "_albedo.f32", size, size, albedo_planes, 3);
        js += "{\"name\": \"" + scene.name + "\", \"title\": \"" + scene.title + "\",\n\"albedo\": \"" +
              base64(albedo) + "\",\n\"methods\": {\n";
        for (std::size_t m = 0; m < methods.size(); ++m) {
            const Method method = methods[m];
            std::array<double, 3> ms{};
            Planes reference{};
            bool identical = true;
            for (std::size_t c = 0; c < thread_counts.size(); ++c) {
                Timed timed = timed_bake(scene, method, thread_counts[c]);
                ms[c] = timed.median_ms;
                if (c == 0) {
                    reference = std::move(timed.planes);
                } else {
                    identical = identical && same_bytes(reference, timed.planes);
                }
            }
            all_identical = all_identical && identical;
            std::printf("%-8s %-7s %9.2f %9.2f %9.2f  %s\n", scene.name.c_str(), method_name(method), ms[0], ms[1],
                        ms[2], identical ? "identical" : "DIFFERENT ACROSS THREAD COUNTS");
            const std::vector<const std::vector<float>*> planes{&reference.fixed, &reference.lamp};
            written = written && write_planes(directory + "/" + scene.name + "_" + method_name(method) + ".f32", size,
                                              size, planes, 2);
            std::array<char, 160> numbers{};
            std::snprintf(numbers.data(), numbers.size(), "[%.3f, %.3f, %.3f]", ms[0], ms[1], ms[2]);
            js += std::string{"\""} + method_name(method) + "\": {\"ms\": " + numbers.data() +
                  ", \"identical\": " + (identical ? "true" : "false") + ",\n\"fixed\": \"" +
                  base64(reference.fixed) + "\",\n\"lamp\": \"" + base64(reference.lamp) + "\"}" +
                  (m + 1 < methods.size() ? ",\n" : "\n");
            timings += std::string{"{\"scene\": \""} + scene.name + "\", \"method\": \"" + method_name(method) +
                       "\", \"ms\": " + numbers.data() + ", \"identical\": " + (identical ? "true" : "false") + "}" +
                       (s + 1 < scene_specs.size() || m + 1 < methods.size() ? ",\n" : "\n");
        }
        js += std::string{"}}"} + (s + 1 < scene_specs.size() ? ",\n" : "\n");
    }
    js += "]};\n";
    timings += "]}\n";
    const std::string js_path = directory + "/planes.js";
    const std::string timings_path = directory + "/timings.json";
    for (int f = 0; f < 2; ++f) {
        const std::string& path = f == 0 ? js_path : timings_path;
        const std::string& text = f == 0 ? js : timings;
        std::FILE* file = std::fopen(path.c_str(), "wb");
        if (file == nullptr) {
            written = false;
            continue;
        }
        written = std::fwrite(text.data(), 1, text.size(), file) == text.size() && written;
        written = (std::fclose(file) == 0) && written;
    }
    if (!written) {
        std::fprintf(stderr, "could not write every file under %s (does the directory exist?)\n", directory.c_str());
        return 1;
    }
    std::printf("wrote %s\n", directory.c_str());
    return all_identical ? 0 : 3;
}
