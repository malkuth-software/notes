// The playground's four control-sized scenes as height fields over the unit
// chart (x right, y up, row 0 at the top; h out of the panel), shared by the
// conv-teapot adapter (teapot_bake.cpp, which hands them to the engine as a
// parametric surface) and the playground's own height-field column
// (tracer_bakes.cpp). The bfft engine takes the same scenes in its own
// geometry (engines/bfft/.../tracer_scenes.hpp).
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tracer_bakes {

constexpr double pi = 3.14159265358979323846;

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
    {"keycap", "Keycap in a well, inset lens", shape_keycap, Lamp{Vec3{0.68, 0.68, 0.075}, 0.03}},
    {"lamp", "Lamp lens and bezel", shape_lamp, Lamp{Vec3{0.5, 0.5, 0.05}, 0.10}},
    {"contact", "Contact-shadow strip and bar", shape_contact, Lamp{Vec3{0.78, 0.5, 0.056}, 0.02}},
    {"well", "Inter-reflection in a key well", shape_well, Lamp{Vec3{0.5, 0.5, -0.265}, 0.05}},
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

} // namespace tracer_bakes
