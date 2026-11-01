// Bake study: what a lamp cache's planes contain.
//
// For each plane: its range; how far its three channels differ from their
// mean (a neutral material under a white lamp gives grey planes); and, for
// two lamps, how far the second differs from the first. The share of
// pixels inside, outside and on the edge of the coverage shows how much of a
// tile holds a control at all.
//
// Usage: inspect <cache.f32> [<other cache.f32>: how far it differs from the first]
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

struct Cache final {
    int width{};
    int height{};
    int lamps{};
    std::vector<std::vector<float>> planes;  // room, then each lamp: width × height × 3
    std::vector<float> coverage;             // width × height
};

bool load(const std::string& path, Cache& cache) {
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr) {
        return false;
    }
    std::int32_t header[3] = {};
    bool ok = std::fread(header, sizeof(header), 1U, file) == 1U;
    cache.width = header[0];
    cache.height = header[1];
    cache.lamps = header[2];
    const std::size_t pixels = static_cast<std::size_t>(cache.width) * static_cast<std::size_t>(cache.height);
    cache.planes.assign(static_cast<std::size_t>(cache.lamps) + 1U, std::vector<float>(pixels * 3U, 0.0F));
    for (std::vector<float>& plane : cache.planes) {
        ok = ok && std::fread(plane.data(), sizeof(float), plane.size(), file) == plane.size();
    }
    cache.coverage.assign(pixels, 0.0F);
    ok = ok && std::fread(cache.coverage.data(), sizeof(float), cache.coverage.size(), file) == cache.coverage.size();
    std::fclose(file);
    return ok;
}

void describe_plane(const char* name, const std::vector<float>& plane) {
    double low = 1e30;
    double high = -1e30;
    double spread = 0.0;   // largest |channel − mean of channels|
    double relative = 0.0; // the same over the mean, where the mean is not tiny
    for (std::size_t k = 0; k < plane.size(); k += 3U) {
        const double mean = (plane[k] + plane[k + 1U] + plane[k + 2U]) / 3.0;
        for (std::size_t c = 0; c < 3U; ++c) {
            const double v = plane[k + c];
            low = std::min(low, v);
            high = std::max(high, v);
            spread = std::max(spread, std::abs(v - mean));
            if (mean > 1e-3) {
                relative = std::max(relative, std::abs(v - mean) / mean);
            }
        }
    }
    std::printf("  %-6s range %.4g..%.4g  channel spread %.3g (relative %.3g)\n", name, low, high, spread, relative);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: inspect <cache.f32> [<other cache.f32>]\n");
        return 2;
    }
    Cache cache{};
    if (!load(argv[1], cache)) {
        std::fprintf(stderr, "cannot read %s\n", argv[1]);
        return 1;
    }
    std::printf("%s: %dx%d, %d lamps\n", argv[1], cache.width, cache.height, cache.lamps);
    describe_plane("room", cache.planes[0]);
    for (int i = 1; i <= cache.lamps; ++i) {
        const std::string name = "lamp" + std::to_string(i);
        describe_plane(name.c_str(), cache.planes[static_cast<std::size_t>(i)]);
    }
    if (cache.lamps == 2) {
        double difference = 0.0;
        double largest = 0.0;
        for (std::size_t k = 0; k < cache.planes[1].size(); ++k) {
            difference = std::max(difference, static_cast<double>(std::abs(cache.planes[1][k] - cache.planes[2][k])));
            largest = std::max(largest, static_cast<double>(cache.planes[1][k]));
        }
        std::printf("  lamps differ by up to %.4g (lamp 1 peaks at %.4g)\n", difference, largest);
    }
    std::size_t empty = 0;
    std::size_t full = 0;
    for (const float a : cache.coverage) {
        empty += a <= 0.0F ? 1U : 0U;
        full += a >= 1.0F ? 1U : 0U;
    }
    const double n = static_cast<double>(cache.coverage.size());
    std::printf("  coverage: empty %.1f%%, full %.1f%%, edge %.1f%%\n", 100.0 * static_cast<double>(empty) / n,
                100.0 * static_cast<double>(full) / n, 100.0 * static_cast<double>(cache.coverage.size() - empty - full) / n);
    if (argc > 2) {
        Cache other{};
        if (!load(argv[2], other) || other.width != cache.width || other.height != cache.height) {
            std::fprintf(stderr, "cannot compare with %s\n", argv[2]);
            return 1;
        }
        std::size_t changed = 0;
        const std::size_t pixels = cache.coverage.size();
        for (std::size_t k = 0; k < pixels; ++k) {
            bool differs = std::abs(cache.coverage[k] - other.coverage[k]) > 1e-4F;
            for (std::size_t p = 0; p < cache.planes.size() && !differs; ++p) {
                for (std::size_t c = 0; c < 3U; ++c) {
                    const float a = cache.planes[p][k * 3U + c];
                    const float b = other.planes[p][k * 3U + c];
                    differs = differs || std::abs(a - b) > 1e-3F * std::max(1.0F, std::abs(a));
                }
            }
            changed += differs ? 1U : 0U;
        }
        std::printf("  against %s: %.1f%% of pixels differ by more than 0.1%%\n", argv[2], 100.0 * static_cast<double>(changed) / static_cast<double>(pixels));
    }
    return 0;
}
