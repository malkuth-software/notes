// conv-teapot's own light bake on a playground scene.
//
// The engine (engines/conv_teapot/src, copied unchanged from conv-teapot at
// fc0a609) bakes parametric surfaces. The playground's height field is handed
// to it as the geometry it expects: one LinearSurface (the engine's bilinear
// surface over a lattice of nodes) whose nodes are the scene's heights. The
// engine then does everything itself, through its public entry points:
// build_mesh makes the mesh and its light-map charts, and
// Renderer::bake_light bakes the diffuse light (its lit fraction under the sun
// with exact shadow edges), bake_open the open fraction and bent normal (32
// cosine-weighted Fibonacci directions, quarter first), and gather_bounce the
// transfer gather (six Jacobi passes, albedo 0.9). Only the readout is ours:
// the baked maps are sampled at every chart pixel and combined exactly as the
// engine's shade_baked combines them for a diffuse texel, with the scene's own
// albedo in place of the porcelain colour.
//
// The engine knows one sun and the room's sky; it has no local emitter for
// diffuse light, so the lamp plane carries the lens's own emission only.
//
// Usage: teapot_bake <scene> <size> <output.f32>
// TRACER_THREADS sets RenderOptions::threads (unset or 0: all threads).
// Writes int32 width, height, then the fixed-light radiance (RGB float32,
// row 0 at the top) and the lamp radiance (RGB float32). Prints the bake time.
//
// Build: c++ -std=c++20 -O2 -ffp-contract=off -pthread -o build/bin/teapot_bake teapot_bake.cpp

#include "engines/conv_teapot/src/render.hpp"
#include "scenes.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace teapot_bake {

// Chart (x, y, h) in the engine's frame: y up, the chart's y along -z.
tconv::Vec3 engine_point(double x, double y, double h) { return tconv::Vec3{x - 0.5, h, 0.5 - y}; }

// The engine's room light on a surface facing n (Renderer::ambient_of, which
// is private): repeated here for the readout only.
tconv::Vec3 engine_ambient(tconv::Vec3 n) {
    const double up = 0.5 * (n.y + 1.0);
    return tconv::Vec3{0.30, 0.28, 0.26} * (1.0 - up) + tconv::Vec3{0.46, 0.50, 0.56} * up;
}

int threads_from_environment() {
    const char* text = std::getenv("TRACER_THREADS");
    if (text == nullptr) {
        return 0;
    }
    const int wanted = std::atoi(text);
    return wanted > 0 ? wanted : 0;
}

} // namespace teapot_bake

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: teapot_bake <keycap|lamp|contact|well> <size> <output.f32>\n");
        return 2;
    }
    const std::string name = argv[1];
    const int size = std::atoi(argv[2]);
    const std::string output = argv[3];
    if (size < 32 || size > 512 || size % 2 != 0) {
        std::fprintf(stderr, "size must be even and in 32..512\n");
        return 2;
    }
    const tracer_bakes::SceneSpec* spec = nullptr;
    for (const tracer_bakes::SceneSpec& candidate : tracer_bakes::scene_specs) {
        if (name == candidate.name) {
            spec = &candidate;
        }
    }
    if (spec == nullptr) {
        std::fprintf(stderr, "unknown scene %s\n", name.c_str());
        return 2;
    }
    const tracer_bakes::Scene chart = tracer_bakes::make_scene(*spec, size);

    // The geometry the engine expects: a lattice of nodes, one per two chart
    // texels each way, node v rising with the chart's y so that the surface's
    // normal cross(du, dv) points out of the panel.
    const std::size_t nodes = static_cast<std::size_t>(size / 2 + 1);
    tconv::Lattice lattice{};
    lattice.nu = nodes;
    lattice.nv = nodes;
    lattice.wrap_u = false;
    lattice.nodes.resize(nodes * nodes);
    const double wall = 1.5 / size;
    for (std::size_t iv = 0; iv < nodes; ++iv) {
        for (std::size_t iu = 0; iu < nodes; ++iu) {
            const double x = static_cast<double>(iu) / static_cast<double>(nodes - 1);
            const double y = static_cast<double>(iv) / static_cast<double>(nodes - 1);
            const tracer_bakes::TexelShape shape = (*spec).shape(x, y, wall);
            lattice.at(iu, iv) = teapot_bake::engine_point(x, y, shape.height);
        }
    }
    tconv::LinearSurface surface{};
    surface.lattice = &lattice;

    tconv::RenderOptions options{};
    options.method = tconv::Method::raster;
    options.threads = teapot_bake::threads_from_environment();
    options.ambient = 3;            // open fraction and the transfer gather
    options.live_reflection = true; // bake the diffuse light only (RenderOptions::diffuse_only)
    options.gloss = false;
    options.ground = false;         // no room floor: the chart is the whole scene
    options.shadows = true;
    options.lighting = true;

    tconv::Scene scene{};
    tconv::RenderPart part{};
    part.surface = &surface;
    part.orientation = 1.0;
    part.two_sided = true;
    part.solid = false;
    scene.parts.push_back(part);
    scene.stamp = 1;
    // The playground's fixed light, chart direction (-0.45, 0.55, 0.70).
    scene.light = tconv::normalize(teapot_bake::engine_point(0.5 - 0.45, 0.5 + 0.55, 0.70) - teapot_bake::engine_point(0.5, 0.5, 0.0));

    tconv::Camera camera{};
    camera.eye = tconv::Vec3{0.0, 6.0, 0.0};
    camera.target = tconv::Vec3{0.0, 0.0, 0.0};
    camera.up = tconv::Vec3{0.0, 0.0, -1.0};
    camera.fov_y = 0.2;

    tconv::MeshOptions mesh_options{};
    mesh_options.layout = tconv::Layout::tree;
    mesh_options.tolerance = 1e-3;
    mesh_options.angle = 20.0;

    const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    const std::vector<const tconv::Surface*> surfaces{&surface};
    const std::vector<std::vector<const tconv::Trim*>> trims{std::vector<const tconv::Trim*>{}};
    tconv::Mesh mesh = tconv::build_mesh(surfaces, trims, mesh_options);
    scene.mesh = &mesh;
    tconv::Renderer renderer{};
    renderer.bake_light(scene, camera, options, size, size, 1.0 / size);
    const double bake_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();

    // Readout: shade_baked's diffuse composite at every chart pixel.
    const tconv::MeshPart& baked = mesh.parts[0];
    const std::size_t pixels = static_cast<std::size_t>(size) * static_cast<std::size_t>(size);
    std::vector<float> fixed(pixels * 3, 0.0f);
    std::vector<float> lamp(pixels * 3, 0.0f);
    const double cells = static_cast<double>(surface.cells_u());
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i < size; ++i) {
            const double x = (i + 0.5) / size;
            const double y = 1.0 - (j + 0.5) / size;
            const double u = x * cells;
            const double v = y * static_cast<double>(surface.cells_v());
            const float s = static_cast<float>(u / cells);
            const float t = static_cast<float>(baked.t_of(v));
            const tconv::SurfacePoint point = surface.eval(u, v);
            const tconv::Vec3 normal = tconv::normalize(tconv::cross(point.du, point.dv));
            float light[4];
            baked.light.sample_level(0, s, t, light);
            const double lit = static_cast<double>(light[3]) / 255.0;
            const double diffuse = std::fmax(0.0, tconv::dot(normal, scene.light)) * lit;
            tconv::Vec3 ambient = teapot_bake::engine_ambient(normal);
            if (!baked.open.levels.empty()) {
                float seen[4];
                baked.open.sample_level(0, s, t, seen);
                const double open = static_cast<double>(seen[3]) / 255.0;
                tconv::Vec3 bent{static_cast<double>(seen[0]) / 127.5 - 1.0, static_cast<double>(seen[1]) / 127.5 - 1.0,
                                 static_cast<double>(seen[2]) / 127.5 - 1.0};
                bent = tconv::length(bent) > 0.2 ? tconv::normalize(bent) : normal;
                tconv::Vec3 from_body{};
                if (!baked.bounce.levels.empty()) {
                    float thrown[4];
                    baked.bounce.sample_level(0, s, t, thrown);
                    from_body = tconv::Vec3{static_cast<double>(thrown[0] * thrown[0]) / 32512.5,
                                            static_cast<double>(thrown[1] * thrown[1]) / 32512.5,
                                            static_cast<double>(thrown[2] * thrown[2]) / 32512.5};
                }
                // The room's light through what the point sees of it, plus what the body throws on it.
                ambient = teapot_bake::engine_ambient(bent) * open + from_body;
            }
            const int index = tracer_bakes::texel_index(chart, i, j);
            const tracer_bakes::Material material = chart.material[index];
            const std::array<float, 3>& albedo = tracer_bakes::material_albedo[static_cast<int>(material)];
            const std::size_t o = static_cast<std::size_t>(index) * 3;
            fixed[o] = static_cast<float>(albedo[0] * (ambient.x + 0.80 * diffuse));
            fixed[o + 1] = static_cast<float>(albedo[1] * (ambient.y + 0.78 * diffuse));
            fixed[o + 2] = static_cast<float>(albedo[2] * (ambient.z + 0.74 * diffuse));
            if (material == tracer_bakes::Material::lens) {
                lamp[o] = albedo[0];
                lamp[o + 1] = albedo[1];
                lamp[o + 2] = albedo[2];
            }
        }
    }

    std::FILE* file = std::fopen(output.c_str(), "wb");
    if (file == nullptr) {
        std::fprintf(stderr, "cannot write %s\n", output.c_str());
        return 1;
    }
    const std::array<std::int32_t, 2> header{size, size};
    bool ok = std::fwrite(header.data(), sizeof(std::int32_t), header.size(), file) == header.size();
    ok = ok && std::fwrite(fixed.data(), sizeof(float), fixed.size(), file) == fixed.size();
    ok = ok && std::fwrite(lamp.data(), sizeof(float), lamp.size(), file) == lamp.size();
    ok = (std::fclose(file) == 0) && ok;
    std::printf("{\"engine\": \"conv-teapot\", \"scene\": \"%s\", \"bake_ms\": %.3f, \"threads\": %d}\n", name.c_str(), bake_ms,
                options.threads);
    return ok ? 0 : 1;
}
