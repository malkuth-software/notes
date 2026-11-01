// A key lens as a small control grid, reconstructed into a mesh by stx_conv.
//
// As in Plan Paint's CONV cloth (examples/conv_cloth/GEOMETRY.md), only the
// control grid is the shape's data; stx_conv::stream::resize<double> with
// nodal sampling rebuilds the mesh from it at whatever subdivision is wanted,
// control sites exactly preserved.
//
// The lens is the experiment's caption shape: a rounded rectangle whose height
// depends only on the inset d from the base edge: a tapered side, a
// quarter-round chamfer, then the flat top. So the control rows are inset rings
// (dense over the side and the round, sparse over the top) and the control
// columns run round each ring by arc length; each node carries x, y, height and
// its inset d, the inset deciding afterwards which triangles are sidewall.
//
// Usage: key_mesh <output file> <subdivision> [sidewall height]
// Writes int32 triangle count, then per triangle 9 float64 (three key-space
// points: x right, y up the screen, h above the panel) and int32 tag
// (0 lens face, 1 sidewall, 2 back).

#include <stx_conv/stream.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <span>
#include <vector>

namespace key_mesh {

constexpr double pi = 3.141592653589793;

// The caption shape (lamps.cpp make_annunciator at this sidewall).
struct Shape final {
    double half_base{0.34};
    double aspect{0.62};
    double half_top{0.315};
    double top{0.12};
    double bottom{-0.07};
    double chamfer{0.022};
    double corner_radius{0.045};
};

// Height at inset d from the base edge.
double height_at(const Shape& s, double d) {
    const double taper = s.half_base - s.half_top;
    if (d <= taper) {
        return s.bottom + (s.top - s.chamfer - s.bottom) * (d / taper);
    }
    if (d <= taper + s.chamfer) {
        const double u = s.chamfer - (d - taper);
        return s.top - s.chamfer + std::sqrt(std::max(0.0, s.chamfer * s.chamfer - u * u));
    }
    return s.top;
}

// A point on the base outline inset by d, at fraction t ∈ [0, 1] of the way
// round it by arc length (t = 0 and t = 1 coincide).
std::array<double, 2> on_corner(double cx, double cy, double r, double start, double a);

std::array<double, 2> ring_point(const Shape& s, double d, double t) {
    const double hx = std::max(0.0, s.half_base - d);
    const double hy = std::max(0.0, s.half_base * s.aspect - d);
    const double r = std::min(std::max(0.0, s.corner_radius - d), std::min(hx, hy));
    const double sx = 2.0 * (hx - r);
    const double sy = 2.0 * (hy - r);
    const double arc = 0.5 * pi * r;
    const double perimeter = 2.0 * (sx + sy) + 4.0 * arc;
    double along = std::fmod(t, 1.0) * perimeter;
    // start at the middle of the right side, go counter-clockwise
    const std::array<double, 8> lengths = {0.5 * sy, arc, sx, arc, sy, arc, sx, arc + 0.5 * sy};
    int piece = 0;
    while (piece < 7 && along > lengths[static_cast<std::size_t>(piece)]) {
        along -= lengths[static_cast<std::size_t>(piece)];
        ++piece;
    }
    const double a = r > 0.0 ? along / r : 0.0;
    switch (piece) {
    case 0: return {hx, along};
    case 1: return on_corner(hx - r, hy - r, r, 0.0, a);
    case 2: return {hx - r - along, hy};
    case 3: return on_corner(-hx + r, hy - r, r, 0.5 * pi, a);
    case 4: return {-hx, hy - r - along};
    case 5: return on_corner(-hx + r, -hy + r, r, pi, a);
    case 6: return {-hx + r + along, -hy};
    default: {
        if (along <= arc) {
            return on_corner(hx - r, -hy + r, r, 1.5 * pi, a);
        }
        return {hx, -hy + r + (along - arc)};
    }
    }
}

std::array<double, 2> on_corner(double cx, double cy, double r, double start, double a) {
    return std::array<double, 2>{cx + r * std::cos(start + a), cy + r * std::sin(start + a)};
}

// A reconstructed mesh value: row r, column c, channel k.
double mesh_value(const std::vector<double>& mesh, std::size_t columns, std::size_t r, std::size_t c, std::size_t k) {
    return mesh[(r * columns + c) * 4U + k];
}

struct Triangle final {
    std::array<double, 9> points{};
    std::int32_t tag{};
};

}  // namespace key_mesh

int main(int argc, char** argv) {
    namespace km = key_mesh;
    if (argc < 3) {
        std::fprintf(stderr, "usage: key_mesh <output> <subdivision> [sidewall]\n");
        return 2;
    }
    const int subdivision = std::clamp(std::atoi(argv[2]), 1, 16);
    km::Shape shape{};
    if (argc > 3) {
        const double sidewall = std::atof(argv[3]);
        shape.top = sidewall;
        shape.half_top = 0.315 - 0.12 * std::max(0.0, sidewall - 0.12);
    }
    const double taper = shape.half_base - shape.half_top;
    const double centre = shape.half_base * shape.aspect;  // the inset at which the ring closes to a line
    // control rings: dense over the side and the round, sparse over the top
    const std::vector<double> insets = {0.0, 0.5 * taper, taper, taper + 0.3 * shape.chamfer, taper + 0.65 * shape.chamfer,
                                        taper + shape.chamfer, taper + shape.chamfer + 0.03, 0.5 * (taper + shape.chamfer + centre),
                                        centre - 0.02, centre};
    const std::size_t rows = insets.size();
    const std::size_t columns = 37;  // 36 round the ring, the last repeating the first
    constexpr std::size_t channels = 4;  // x, y, h, d
    std::vector<double> control(rows * columns * channels);
    for (std::size_t r = 0; r < rows; ++r) {
        for (std::size_t c = 0; c < columns; ++c) {
            const std::array<double, 2> p = km::ring_point(shape, insets[r], static_cast<double>(c) / (columns - 1));
            const std::size_t at = (r * columns + c) * channels;
            control[at] = p[0];
            control[at + 1] = p[1];
            control[at + 2] = km::height_at(shape, insets[r]);
            control[at + 3] = insets[r];
        }
    }
    const std::size_t out_rows = (rows - 1) * static_cast<std::size_t>(subdivision) + 1;
    const std::size_t out_columns = (columns - 1) * static_cast<std::size_t>(subdivision) + 1;
    std::vector<double> mesh(out_rows * out_columns * channels);
    stx_conv::stream::Workspace<double> workspace(columns, rows, out_columns, out_rows, channels, stx_conv::Sampling::nodal);
    const stx_conv::Status status = stx_conv::stream::resize<double>(
        workspace, {std::span<const double>(control), columns * channels}, {std::span<double>(mesh), out_columns * channels});
    if (status != stx_conv::Status::ok) {
        std::fprintf(stderr, "CONV reconstruction failed (%d)\n", static_cast<int>(status));
        return 1;
    }
    // The heights: CONV can overshoot near the chamfer's turn; the lens must
    // stay convex for the engine, so no height may exceed the flat top.
    for (std::size_t i = 0; i < out_rows * out_columns; ++i) {
        mesh[i * channels + 2] = std::min(mesh[i * channels + 2], shape.top);
    }
    std::vector<km::Triangle> triangles;
    for (std::size_t r = 0; r + 1 < out_rows; ++r) {
        for (std::size_t c = 0; c + 1 < out_columns; ++c) {
            const double d = 0.25 * (km::mesh_value(mesh, out_columns, r, c, 3) + km::mesh_value(mesh, out_columns, r + 1, c, 3) + km::mesh_value(mesh, out_columns, r, c + 1, 3) + km::mesh_value(mesh, out_columns, r + 1, c + 1, 3));
            const std::int32_t tag = d < taper - 1e-6 ? 1 : 0;
            km::Triangle a{};
            km::Triangle b{};
            const std::array<std::array<std::size_t, 2>, 4> q = {{{r, c}, {r, c + 1}, {r + 1, c + 1}, {r + 1, c}}};
            for (int k = 0; k < 3; ++k) {
                a.points[static_cast<std::size_t>(0 + k)] = km::mesh_value(mesh, out_columns, q[0][0], q[0][1], static_cast<std::size_t>(k));
                a.points[static_cast<std::size_t>(3 + k)] = km::mesh_value(mesh, out_columns, q[1][0], q[1][1], static_cast<std::size_t>(k));
                a.points[static_cast<std::size_t>(6 + k)] = km::mesh_value(mesh, out_columns, q[2][0], q[2][1], static_cast<std::size_t>(k));
                b.points[static_cast<std::size_t>(0 + k)] = km::mesh_value(mesh, out_columns, q[0][0], q[0][1], static_cast<std::size_t>(k));
                b.points[static_cast<std::size_t>(3 + k)] = km::mesh_value(mesh, out_columns, q[2][0], q[2][1], static_cast<std::size_t>(k));
                b.points[static_cast<std::size_t>(6 + k)] = km::mesh_value(mesh, out_columns, q[3][0], q[3][1], static_cast<std::size_t>(k));
            }
            a.tag = tag;
            b.tag = tag;
            triangles.push_back(a);
            triangles.push_back(b);
        }
    }
    // the back: a fan over the base ring
    for (std::size_t c = 1; c + 2 < out_columns; ++c) {
        km::Triangle t{};
        const std::array<std::size_t, 3> cols = {0, c, c + 1};
        for (std::size_t v = 0; v < 3U; ++v) {
            for (std::size_t k = 0; k < 3U; ++k) {
                t.points[v * 3U + k] = km::mesh_value(mesh, out_columns, 0, cols[v], k);
            }
        }
        t.tag = 2;
        triangles.push_back(t);
    }
    std::FILE* file = std::fopen(argv[1], "wb");
    if (file == nullptr) {
        return 1;
    }
    const std::int32_t count = static_cast<std::int32_t>(triangles.size());
    bool ok = std::fwrite(&count, sizeof(count), 1U, file) == 1U;
    for (const km::Triangle& t : triangles) {
        ok = ok && std::fwrite(t.points.data(), sizeof(double), 9U, file) == 9U;
        ok = ok && std::fwrite(&t.tag, sizeof(t.tag), 1U, file) == 1U;
    }
    ok = std::fclose(file) == 0 && ok;
    std::printf("control %zux%zu → mesh %zux%zu, %d triangles\n", columns, rows, out_columns, out_rows, count);
    return ok ? 0 : 1;
}
