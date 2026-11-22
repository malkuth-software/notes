// Tracer bakes: the playground's driver. It runs the owner's engines on four
// control-sized scenes, times every bake at 1, 4 and all threads, and writes
// their linear light planes for tracer_bakes.html.
//
// Columns (README.md has what each engine is given and what it leaves out):
//   lamp_keys   today's baker, engines/lamp_keys/lamp_keys.cpp, on its own
//               families: keycap for the keycap scene, indicator for the lamp
//               scene. It has no geometry for the strip or the open well.
//   conv-teapot the teapot engine's own bake (open fraction, transfer gather,
//               exact shadow edges) through teapot_bake.cpp; fixed light only.
//   bfft        the photonic transport engine (regime_scene_native) with the
//               scenes in its own geometry (tracer_scenes.hpp): exact source
//               windows for every area light, retained diffuse field.
//   playground  not an engine: this file's own height-field version of the
//               source-window idea plus a 32-direction gather, kept as a
//               labelled extra.
//
// Every engine runs as its own process with TRACER_THREADS set to 1, 4 and the
// hardware count, three times each; the median wall time of the whole process
// is reported (scene build, bake and file writes included), and the planes of
// each thread count are compared byte for byte with the single-thread ones.
// The playground column is timed in process.
//
// Usage: tracer_bakes [size, even, 32..512, default 160]
// Run from this folder after `make` (see README.md). Writes build/tracer_bakes/:
// one <scene>_<column>.f32 per cell (int32 width, height; then fixed-light
// radiance RGB float32 and lamp radiance RGB float32, row 0 at the top),
// planes.js for the page and timings.json. Engine work files stay in
// build/tracer_bakes/work/.
//
// Determinism of the playground column: no FMA contraction and a fixed order
// in every sum, so its output is identical for any thread count on one host;
// libm supplies sin, cos, asin and atan, so bytes across hosts are not claimed.

#include "scenes.hpp"

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

constexpr double golden_angle = 2.39996322972865332;
constexpr int direction_count = 32;
constexpr int quarter_stride = 4;
constexpr int jacobi_passes = 6;
constexpr int repeats = 3;
constexpr double sky_radiance = 0.22;
constexpr double sun_angular_radius = 0.06;
constexpr double lamp_radiance = 1.0;
constexpr double far_distance = 3.0;

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
//
// Fixed light: n.l times the visible fraction of its disc. Lamp: a sphere of
// radius r at distance d gives irradiance Le * pi * sin^2(beta) * cos(theta)
// unoccluded, beta = asin(r / d); the plane stores irradiance / pi, times the
// sphere's visible fraction. Lens texels emit 1.

struct DirectLight final {
    double sun{};
    double lamp{};
};

DirectLight direct_light(const Scene& scene, int i, int j) {
    const int index = texel_index(scene, i, j);
    const Vec3 n = scene.normal[index];
    const Vec3 origin = surface_origin(scene, index);
    const Vec3 light = fixed_light_direction();
    const double n_dot_l = dot(n, light);
    DirectLight result{};
    if (n_dot_l > 0.0) {
        result.sun = n_dot_l * cone_visible_fraction(scene, origin, light, sun_angular_radius, far_distance);
    }
    if (scene.material[index] == Material::lens) {
        result.lamp = lamp_radiance;
        return result;
    }
    const Vec3 to_lamp = sub(scene.lamp.centre, origin);
    const double distance = length(to_lamp);
    const Vec3 direction = scale(to_lamp, 1.0 / distance);
    const double n_dot = dot(n, direction);
    if (n_dot <= 0.0 || distance <= scene.lamp.radius) {
        return result;
    }
    const double sin_beta = scene.lamp.radius / distance;
    const double unoccluded = lamp_radiance * sin_beta * sin_beta * n_dot;
    const double beta = std::asin(sin_beta);
    result.lamp = unoccluded * cone_visible_fraction(scene, origin, direction, beta, distance - scene.lamp.radius);
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
            const DirectLight light = direct_light(scene, i, j);
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

Planes bake_scene(const Scene& scene, int threads) {
    Bake bake{};
    bake.scene = &scene;
    bake.coarse = scene.size / 2;
    const std::size_t fine = scene.height.size();
    const std::size_t coarse = static_cast<std::size_t>(bake.coarse) * static_cast<std::size_t>(bake.coarse);
    bake.sun.assign(fine, 0.0);
    bake.lamp.assign(fine, 0.0);
    run_rows(threads, scene.size, direct_rows, &bake);
    Planes planes{};
    planes.fixed.resize(fine);
    planes.lamp.resize(fine);
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

// ---- cells -------------------------------------------------------------------------------

// One cell of the page: radiance planes at unit white light (albedo included).
struct Cell final {
    bool present{};
    std::string note{};
    int width{};
    int height{};
    std::vector<float> fixed{}; // width x height x 3
    std::vector<float> lamp{};  // width x height x 3
    std::array<double, 3> ms{};
    bool identical{true};
    int bakes_per_run{1};
};

bool same_planes(const Cell& a, const Cell& b) {
    return a.width == b.width && a.height == b.height && a.fixed.size() == b.fixed.size() && a.lamp.size() == b.lamp.size() &&
           std::memcmp(a.fixed.data(), b.fixed.data(), a.fixed.size() * sizeof(float)) == 0 &&
           std::memcmp(a.lamp.data(), b.lamp.data(), a.lamp.size() * sizeof(float)) == 0;
}

std::vector<char> read_file(const std::string& path) {
    std::vector<char> bytes{};
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr) {
        return bytes;
    }
    std::array<char, 65536> chunk{};
    while (true) {
        const std::size_t got = std::fread(chunk.data(), 1, chunk.size(), file);
        bytes.insert(bytes.end(), chunk.data(), chunk.data() + got);
        if (got < chunk.size()) {
            break;
        }
    }
    std::fclose(file);
    return bytes;
}

std::int32_t read_int(const std::vector<char>& bytes, std::size_t offset) {
    std::int32_t value = 0;
    std::memcpy(&value, bytes.data() + offset, sizeof(value));
    return value;
}

// Read `count` float32 values at `offset`; false if the file is too short.
bool read_floats(const std::vector<char>& bytes, std::size_t offset, std::size_t count, std::vector<float>& out) {
    if (offset + count * sizeof(float) > bytes.size()) {
        return false;
    }
    out.resize(count);
    std::memcpy(out.data(), bytes.data() + offset, count * sizeof(float));
    return true;
}

// The playground's own cell format and teapot_bake's: int32 width, height;
// fixed RGB; lamp RGB.
bool read_pair(const std::string& path, Cell& cell) {
    const std::vector<char> bytes = read_file(path);
    if (bytes.size() < 8) {
        return false;
    }
    cell.width = read_int(bytes, 0);
    cell.height = read_int(bytes, 4);
    const std::size_t count = static_cast<std::size_t>(cell.width) * static_cast<std::size_t>(cell.height) * 3;
    return read_floats(bytes, 8, count, cell.fixed) && read_floats(bytes, 8 + count * sizeof(float), count, cell.lamp);
}

// bfft's --out-linear: int32 width, height; RGB float32.
bool read_linear(const std::string& path, int& width, int& height, std::vector<float>& rgb) {
    const std::vector<char> bytes = read_file(path);
    if (bytes.size() < 8) {
        return false;
    }
    width = read_int(bytes, 0);
    height = read_int(bytes, 4);
    return read_floats(bytes, 8, static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 3, rgb);
}

// lamp_keys' buffers: int32 width, height, lamps; the room image, each lamp's
// image (RGB float32, premultiplied by coverage) and the coverage plane. The
// lamp plane is the sum of the lamps' images (all lamps at unit power).
bool read_lamp_keys(const std::string& path, Cell& cell) {
    const std::vector<char> bytes = read_file(path);
    if (bytes.size() < 12) {
        return false;
    }
    cell.width = read_int(bytes, 0);
    cell.height = read_int(bytes, 4);
    const int lamps = read_int(bytes, 8);
    const std::size_t count = static_cast<std::size_t>(cell.width) * static_cast<std::size_t>(cell.height) * 3;
    if (!read_floats(bytes, 12, count, cell.fixed)) {
        return false;
    }
    cell.lamp.assign(count, 0.0f);
    std::vector<float> glow{};
    for (int l = 0; l < lamps; ++l) {
        if (!read_floats(bytes, 12 + (1 + static_cast<std::size_t>(l)) * count * sizeof(float), count, glow)) {
            return false;
        }
        for (std::size_t k = 0; k < count; ++k) {
            cell.lamp[k] += glow[k];
        }
    }
    return true;
}

bool write_pair(const std::string& path, const Cell& cell) {
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    const std::array<std::int32_t, 2> header{cell.width, cell.height};
    bool ok = std::fwrite(header.data(), sizeof(std::int32_t), header.size(), file) == header.size();
    ok = ok && std::fwrite(cell.fixed.data(), sizeof(float), cell.fixed.size(), file) == cell.fixed.size();
    ok = ok && std::fwrite(cell.lamp.data(), sizeof(float), cell.lamp.size(), file) == cell.lamp.size();
    ok = (std::fclose(file) == 0) && ok;
    return ok;
}

double median_of(std::array<double, repeats> times) {
    std::sort(times.begin(), times.end());
    return times[repeats / 2];
}

// Run a shell command (POSIX sh) and return its wall time, or a negative
// value when it fails. Its output goes to the log file named.
double timed_command(const std::string& command, const std::string& log) {
    const std::string line = command + " >>" + log + " 2>&1";
    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    const int status = std::system(line.c_str());
    const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
    return status == 0 ? ms : -1.0;
}

// ---- the engines ---------------------------------------------------------------------------

struct Run final {
    const std::array<int, 3>* threads{};
    int size{};
    std::string work{};
    std::string log{};
};

std::string with_threads(int threads, const std::string& command) {
    return "TRACER_THREADS=" + std::to_string(threads) + " " + command;
}

using ReadFunction = bool (*)(const Run& run, const std::string& scene, int threads, Cell& cell);
using CommandFunction = std::string (*)(const Run& run, const std::string& scene, int threads, int pass);

// Run one engine's commands for a scene at every thread count, `passes`
// commands per bake, and read the result of each thread count.
Cell run_engine(const Run& run, const std::string& scene, int passes, CommandFunction command, ReadFunction read) {
    Cell reference{};
    reference.present = true;
    for (std::size_t c = 0; c < (*run.threads).size(); ++c) {
        const int threads = (*run.threads)[c];
        std::array<double, repeats> times{};
        for (int r = 0; r < repeats; ++r) {
            double total = 0.0;
            for (int pass = 0; pass < passes; ++pass) {
                const double ms = timed_command(command(run, scene, threads, pass), run.log);
                if (ms < 0.0) {
                    Cell failed{};
                    failed.note = "engine run failed; see build/tracer_bakes/work/engines.log";
                    return failed;
                }
                total += ms;
            }
            times[r] = total;
        }
        Cell cell{};
        if (!read(run, scene, threads, cell)) {
            Cell failed{};
            failed.note = "engine output missing or short; see build/tracer_bakes/work/engines.log";
            return failed;
        }
        if (c == 0) {
            reference.width = cell.width;
            reference.height = cell.height;
            reference.fixed = std::move(cell.fixed);
            reference.lamp = std::move(cell.lamp);
        } else {
            reference.identical = reference.identical && same_planes(reference, cell);
        }
        reference.ms[c] = median_of(times);
    }
    return reference;
}

std::string teapot_output(const Run& run, const std::string& scene, int threads) {
    return run.work + "/teapot_" + scene + "_" + std::to_string(threads) + ".f32";
}

std::string teapot_command(const Run& run, const std::string& scene, int threads, int) {
    return with_threads(threads, "build/bin/teapot_bake " + scene + " " + std::to_string(run.size) + " " +
                                     teapot_output(run, scene, threads));
}

bool teapot_read(const Run& run, const std::string& scene, int threads, Cell& cell) {
    return read_pair(teapot_output(run, scene, threads), cell);
}

std::string bfft_output(const Run& run, const std::string& scene, int threads, int pass) {
    return run.work + "/bfft_" + scene + "_" + (pass == 0 ? "fixed" : "lamp") + "_" + std::to_string(threads);
}

std::string bfft_command(const Run& run, const std::string& scene, int threads, int pass) {
    const std::string stem = bfft_output(run, scene, threads, pass);
    const std::string size = std::to_string(run.size);
    return with_threads(threads, "build/bin/regime_scene_native --scene tracer --tracer-scene " + scene + " --tracer-pass " +
                                     (pass == 0 ? "fixed" : "lamp") + " --width " + size + " --height " + size +
                                     " --max-primitives 4000 --out " + stem + ".ppm --out-linear " + stem + ".f32");
}

bool bfft_read(const Run& run, const std::string& scene, int threads, Cell& cell) {
    int width = 0;
    int height = 0;
    if (!read_linear(bfft_output(run, scene, threads, 0) + ".f32", width, height, cell.fixed)) {
        return false;
    }
    int lamp_width = 0;
    int lamp_height = 0;
    if (!read_linear(bfft_output(run, scene, threads, 1) + ".f32", lamp_width, lamp_height, cell.lamp)) {
        return false;
    }
    cell.width = width;
    cell.height = height;
    return lamp_width == width && lamp_height == height;
}

// lamp_keys bakes a whole family per run (each style, up and down); the page
// shows the first style's unpressed bake.
struct LampKeysChoice final {
    const char* scene{};
    const char* family{};
    const char* style{};
    int bakes{};
};

constexpr std::array<LampKeysChoice, 2> lamp_keys_choices{{
    {"keycap", "keycap", "clear", 4},
    {"lamp", "indicator", "backed", 4},
}};

const LampKeysChoice* lamp_keys_choice(const std::string& scene) {
    for (const LampKeysChoice& choice : lamp_keys_choices) {
        if (scene == choice.scene) {
            return &choice;
        }
    }
    return nullptr;
}

std::string lamp_keys_folder(const Run& run, int threads) { return run.work + "/lamp_keys_" + std::to_string(threads); }

std::string lamp_keys_command(const Run& run, const std::string& scene, int threads, int) {
    const LampKeysChoice* choice = lamp_keys_choice(scene);
    const std::string folder = lamp_keys_folder(run, threads);
    return "mkdir -p " + folder + " && " +
           with_threads(threads, "build/bin/lamp_keys " + folder + " " + std::to_string(run.size) + " " + (*choice).family);
}

bool lamp_keys_read(const Run& run, const std::string& scene, int threads, Cell& cell) {
    const LampKeysChoice* choice = lamp_keys_choice(scene);
    return read_lamp_keys(lamp_keys_folder(run, threads) + "/" + (*choice).family + "_" + (*choice).style + "_up.f32", cell);
}

// ---- the playground's own column -----------------------------------------------------------

Cell playground_cell(const Scene& scene, const std::array<int, 3>& thread_counts) {
    Cell cell{};
    cell.present = true;
    cell.width = scene.size;
    cell.height = scene.size;
    Planes reference{};
    for (std::size_t c = 0; c < thread_counts.size(); ++c) {
        std::array<double, repeats> times{};
        Planes planes{};
        for (int r = 0; r < repeats; ++r) {
            const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            planes = bake_scene(scene, thread_counts[c]);
            times[r] = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        }
        cell.ms[c] = median_of(times);
        if (c == 0) {
            reference = std::move(planes);
        } else {
            cell.identical = cell.identical && reference.fixed == planes.fixed && reference.lamp == planes.lamp;
        }
    }
    const std::size_t count = scene.height.size();
    cell.fixed.resize(count * 3);
    cell.lamp.resize(count * 3);
    for (std::size_t t = 0; t < count; ++t) {
        const std::array<float, 3>& rgb = material_albedo[static_cast<int>(scene.material[t])];
        for (int k = 0; k < 3; ++k) {
            cell.fixed[t * 3 + k] = rgb[k] * reference.fixed[t];
            cell.lamp[t * 3 + k] = rgb[k] * reference.lamp[t];
        }
    }
    return cell;
}

// ---- output ------------------------------------------------------------------------------

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

std::string cell_json(const Cell& cell) {
    if (!cell.present) {
        return "{\"present\": false, \"note\": \"" + cell.note + "\"}";
    }
    std::array<char, 200> numbers{};
    std::snprintf(numbers.data(), numbers.size(), "[%.3f, %.3f, %.3f]", cell.ms[0], cell.ms[1], cell.ms[2]);
    return std::string{"{\"present\": true, \"note\": \""} + cell.note + "\", \"width\": " + std::to_string(cell.width) +
           ", \"height\": " + std::to_string(cell.height) + ", \"ms\": " + numbers.data() +
           ", \"bakes_per_run\": " + std::to_string(cell.bakes_per_run) +
           ", \"identical\": " + (cell.identical ? "true" : "false") + ",\n\"fixed\": \"" + base64(cell.fixed) +
           "\",\n\"lamp\": \"" + base64(cell.lamp) + "\"}";
}

bool write_text(const std::string& path, const std::string& text) {
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    bool ok = std::fwrite(text.data(), 1, text.size(), file) == text.size();
    ok = (std::fclose(file) == 0) && ok;
    return ok;
}

constexpr std::array<const char*, 4> column_names{"lamp_keys", "teapot", "bfft", "playground"};

} // namespace tracer_bakes

int main(int argc, char** argv) {
    using namespace tracer_bakes;
    const int size = argc > 1 ? std::atoi(argv[1]) : 160;
    if (size < 32 || size > 512 || size % 2 != 0) {
        std::fprintf(stderr, "size must be even and in 32..512\n");
        return 2;
    }
    const std::string directory = "build/tracer_bakes";
    if (std::system(("mkdir -p " + directory + "/work").c_str()) != 0) {
        std::fprintf(stderr, "cannot create %s/work\n", directory.c_str());
        return 1;
    }
    const int hardware = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
    const std::array<int, 3> thread_counts{1, 4, hardware};
    Run run{};
    run.threads = &thread_counts;
    run.size = size;
    run.work = directory + "/work";
    run.log = run.work + "/engines.log";
    const std::string threads_json = "[1, 4, " + std::to_string(hardware) + "]";
    std::string js = "window.TRACER_BAKES = {\n\"size\": " + std::to_string(size) + ",\n\"threads\": " + threads_json +
                     ",\n\"scenes\": [\n";
    std::string timings = "{\"size\": " + std::to_string(size) + ", \"threads\": " + threads_json + ", \"cells\": [\n";
    bool written = true;
    std::printf("size %d; median ms of %d runs at 1 / 4 / %d threads\n", size, repeats, hardware);
    for (std::size_t s = 0; s < scene_specs.size(); ++s) {
        const Scene scene = make_scene(scene_specs[s], size);
        std::array<Cell, 4> cells{};
        const LampKeysChoice* choice = lamp_keys_choice(scene.name);
        if (choice != nullptr) {
            cells[0] = run_engine(run, scene.name, 1, lamp_keys_command, lamp_keys_read);
            cells[0].bakes_per_run = (*choice).bakes;
            if (cells[0].present) {
                cells[0].note = std::string{"its own "} + (*choice).family + " family (" + (*choice).style +
                                ", unpressed); a run bakes " + std::to_string((*choice).bakes);
            }
        } else {
            cells[0].note = "lamp_keys has no geometry for this scene: it bakes a lens in a well";
        }
        cells[1] = run_engine(run, scene.name, 1, teapot_command, teapot_read);
        if (cells[1].present) {
            cells[1].note = "fixed light only: the engine has no local emitter; the lamp plane is the lens's own emission";
        }
        cells[2] = run_engine(run, scene.name, 2, bfft_command, bfft_read);
        if (cells[2].present) {
            cells[2].note = "two engine runs per bake: the fixed-light pass and the lamp pass";
        }
        cells[3] = playground_cell(scene, thread_counts);
        cells[3].note = "the playground's own height-field version, not an engine";
        js += "{\"name\": \"" + scene.name + "\", \"title\": \"" + scene.title + "\", \"cells\": {\n";
        for (std::size_t c = 0; c < cells.size(); ++c) {
            const Cell& cell = cells[c];
            if (cell.present) {
                written = write_pair(directory + "/" + scene.name + "_" + column_names[c] + ".f32", cell) && written;
                std::printf("%-8s %-10s %10.1f %10.1f %10.1f  %s\n", scene.name.c_str(), column_names[c], cell.ms[0], cell.ms[1],
                            cell.ms[2], cell.identical ? "identical" : "differs by thread count");
            } else {
                std::printf("%-8s %-10s  %s\n", scene.name.c_str(), column_names[c], cell.note.c_str());
            }
            js += std::string{"\""} + column_names[c] + "\": " + cell_json(cell) + (c + 1 < cells.size() ? ",\n" : "\n");
            std::array<char, 200> numbers{};
            std::snprintf(numbers.data(), numbers.size(), "[%.3f, %.3f, %.3f]", cell.ms[0], cell.ms[1], cell.ms[2]);
            timings += std::string{"{\"scene\": \""} + scene.name + "\", \"column\": \"" + column_names[c] +
                       "\", \"present\": " + (cell.present ? "true" : "false") + ", \"ms\": " + numbers.data() +
                       ", \"bakes_per_run\": " + std::to_string(cell.bakes_per_run) +
                       ", \"identical\": " + (cell.identical ? "true" : "false") + "}" +
                       (s + 1 < scene_specs.size() || c + 1 < cells.size() ? ",\n" : "\n");
        }
        js += std::string{"}}"} + (s + 1 < scene_specs.size() ? ",\n" : "\n");
    }
    js += "]};\n";
    timings += "]}\n";
    written = write_text(directory + "/planes.js", js) && written;
    written = write_text(directory + "/timings.json", timings) && written;
    if (!written) {
        std::fprintf(stderr, "could not write every file under %s\n", directory.c_str());
        return 1;
    }
    std::printf("wrote %s\n", directory.c_str());
    return 0;
}
