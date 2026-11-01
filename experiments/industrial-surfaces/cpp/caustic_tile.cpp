// A caustic tile for a moulded acrylic key top.
//
// The trace is Stillwater's (playsuite-games shared/ambient/src/caustics.cpp,
// 0efc1f0): a tileable height field of travelling waves with integer wave
// vectors, photons on a regular grid bent by its slope and gathered where they
// land. Here the surface is the key's moulded top, whose waviness does not move,
// so one frame is kept. The light it gathers is the lamp's, passing out through
// the top, and the tile's mean is 1: it moves light into the web and takes it
// from between the lines, so the key is not brighter overall.
//
// Usage: caustic_tile <out.f32> [size] [focus] [seed]
// Output: int32 size, then size * size float32 sharp values, then size * size
// float32 soft values (one wrapped box blur), row-major, each with mean 1.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

constexpr int table_size = 4096;  // cosine table entries per cycle, a power of two
constexpr double two_pi = 6.283185307179586;

struct Wave {
    int kx{};
    int ky{};
    double amplitude{};  // surface height, tile units
    double phase{};      // 0..1
};

// Stillwater's generator, so a seed gives the same tile everywhere.
struct Mixer {
    std::uint32_t state{};
    std::uint32_t next() {
        state = state * 1664525U + 1013904223U;
        std::uint32_t x = state;
        x ^= x >> 16U;
        x *= 0x7FEB352DU;
        x ^= x >> 15U;
        return x;
    }
    double unit() {
        const double result = static_cast<double>(next() & 0xFFFFFFU) / 16777216.0;
        return result;
    }
};

std::vector<Wave> make_waves(std::uint32_t seed) {
    std::vector<Wave> waves{};
    Mixer mixer{seed * 2654435761U + 12345U};
    while (waves.size() < 14) {
        const int kx = static_cast<int>(mixer.next() % 15U) - 7;
        const int ky = static_cast<int>(mixer.next() % 15U) - 7;
        const double radius = std::sqrt(static_cast<double>(kx * kx + ky * ky));
        if (radius < 1.4 || radius > 7.2)
            continue;
        Wave wave{};
        wave.kx = kx;
        wave.ky = ky;
        wave.amplitude = 1.0 / std::pow(radius, 1.75);
        wave.phase = mixer.unit();
        waves.push_back(wave);
    }
    return waves;
}

// Box blur with wrapping, along rows then columns; scratch is the caller's.
void blur(std::vector<float>& values, std::vector<float>& scratch, int size, int radius) {
    const float inverse = 1.0F / static_cast<float>(2 * radius + 1);
    const int mask = size - 1;
    for (int pass = 0; pass < 2; ++pass) {
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                float sum = 0;
                for (int d = -radius; d <= radius; ++d) {
                    const int sx = pass == 0 ? ((x + d) & mask) : x;
                    const int sy = pass == 0 ? y : ((y + d) & mask);
                    sum += values[static_cast<std::size_t>(sy) * static_cast<std::size_t>(size) + static_cast<std::size_t>(sx)];
                }
                scratch[static_cast<std::size_t>(y) * static_cast<std::size_t>(size) + static_cast<std::size_t>(x)] = sum * inverse;
            }
        }
        values.swap(scratch);
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: caustic_tile <out.f32> [size] [focus] [seed]\n");
        return 2;
    }
    const int size = argc > 2 ? std::atoi(argv[2]) : 128;
    const double focus = argc > 3 ? std::atof(argv[3]) : 0.0055;
    const std::uint32_t seed = argc > 4 ? static_cast<std::uint32_t>(std::atoi(argv[4])) : 1U;
    if (size < 16 || size > 256 || (size & (size - 1)) != 0) {
        std::fprintf(stderr, "size must be a power of two in 16..256\n");
        return 2;
    }

    std::vector<double> cosine(table_size);
    for (int index = 0; index < table_size; ++index)
        cosine[static_cast<std::size_t>(index)] = std::cos(two_pi * index / table_size);
    const std::vector<Wave> waves = make_waves(seed);
    // Two photons per texel side: enough that the web's thinnest lines are continuous.
    const int photons = size * 2;
    const int step = table_size / photons;
    const int mask = size - 1;
    const std::size_t texels = static_cast<std::size_t>(size) * static_cast<std::size_t>(size);
    std::vector<float> sharp(texels, 0.0F);
    std::vector<float> scratch(texels, 0.0F);
    std::vector<int> offsets(waves.size());
    std::vector<double> gains(waves.size());
    for (std::size_t w = 0; w < waves.size(); ++w) {
        offsets[w] = static_cast<int>(std::lround(waves[w].phase * table_size)) & (table_size - 1);
        // d/dx of a sin(2 pi k.x) is 2 pi k a cos(...), in tile units.
        gains[w] = two_pi * waves[w].amplitude * focus * size;
    }
    for (int j = 0; j < photons; ++j) {
        for (int i = 0; i < photons; ++i) {
            double gx = 0;
            double gy = 0;
            for (std::size_t w = 0; w < waves.size(); ++w) {
                const int index = (waves[w].kx * i * step + waves[w].ky * j * step + offsets[w]) & (table_size - 1);
                const double c = cosine[static_cast<std::size_t>(index)] * gains[w];
                gx += c * waves[w].kx;
                gy += c * waves[w].ky;
            }
            // Where the photon lands, in texels, and a bilinear splat there.
            const double x = (static_cast<double>(i) + 0.5) * 0.5 - gx;
            const double y = (static_cast<double>(j) + 0.5) * 0.5 - gy;
            const double fx = std::floor(x);
            const double fy = std::floor(y);
            const float ax = static_cast<float>(x - fx);
            const float ay = static_cast<float>(y - fy);
            const int x0 = static_cast<int>(fx) & mask;
            const int y0 = static_cast<int>(fy) & mask;
            const int x1 = (x0 + 1) & mask;
            const int y1 = (y0 + 1) & mask;
            const std::size_t row0 = static_cast<std::size_t>(y0) * static_cast<std::size_t>(size);
            const std::size_t row1 = static_cast<std::size_t>(y1) * static_cast<std::size_t>(size);
            sharp[row0 + static_cast<std::size_t>(x0)] += (1 - ax) * (1 - ay);
            sharp[row0 + static_cast<std::size_t>(x1)] += ax * (1 - ay);
            sharp[row1 + static_cast<std::size_t>(x0)] += (1 - ax) * ay;
            sharp[row1 + static_cast<std::size_t>(x1)] += ax * ay;
        }
    }
    // Four photons land per texel where the light is even: scale to mean 1.
    const float photon_weight = static_cast<float>(texels) / static_cast<float>(photons * photons);
    for (float& value : sharp)
        value *= photon_weight;
    std::vector<float> soft = sharp;
    blur(soft, scratch, size, 1);

    std::FILE* file = std::fopen(argv[1], "wb");
    if (file == nullptr) {
        std::fprintf(stderr, "cannot write %s\n", argv[1]);
        return 1;
    }
    const std::int32_t header = size;
    const bool written = std::fwrite(&header, sizeof header, 1, file) == 1 &&
                         std::fwrite(sharp.data(), sizeof(float), texels, file) == texels &&
                         std::fwrite(soft.data(), sizeof(float), texels, file) == texels;
    const bool closed = std::fclose(file) == 0;
    if (!written || !closed) {
        std::fprintf(stderr, "short write to %s\n", argv[1]);
        return 1;
    }
    float low = sharp[0];
    float high = sharp[0];
    double sum = 0;
    for (const float value : sharp) {
        low = value < low ? value : low;
        high = value > high ? value : high;
        sum += value;
    }
    std::printf("%d x %d, focus %.4f: sharp mean %.3f, range %.3f..%.3f\n", size, size, focus, sum / static_cast<double>(texels), low, high);
    return 0;
}
