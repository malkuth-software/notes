// Bake study: a compact store for lamp caches, and its decoder.
//
// Reads a standard cache (cpp/lamp_keys.cpp: int32 width, height, lamps; room;
// each lamp; RGB float32 premultiplied by coverage; coverage), stores it
// compactly, decodes the store back into a standard cache, and reports the
// store's size. The decoded cache is what a runtime would composite, so
// bake_study/compare measures exactly what the store loses.
//
// Choices (each optional):
//   --grey       one channel per plane (the planes are grey: neutral materials
//                under white lamps; the colour is applied at composite)
//   --merge      the lamps' planes summed into one (lamps that always share a
//                colour and power)
//   --crop       only the pixels the control covers: per row, its first and
//                last covered column
//   --half       IEEE binary16 values instead of binary32
//   --log BITS   light as log2(1 + v / scale) in BITS-bit integers (scale per
//                plane), coverage in 8 bits; replaces --half for light planes
//   --scale N    light planes stored at 1/N resolution (box mean), decoded by
//                bilinear interpolation; coverage stays full resolution
//   --store F    also write the store itself to F (with --grey --half, full
//                resolution, uncropped): int32 width, height, light planes; then
//                each light plane and the coverage as IEEE binary16, row by row.
//                Light is unpremultiplied (it is what the composite tones).
//
// Usage: pack <in.f32> <decoded out.f32> [choices]
// Prints: stored bytes, and the original's bytes.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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

bool save(const std::string& path, const Cache& cache) {
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    const std::int32_t header[3] = {cache.width, cache.height, cache.lamps};
    bool ok = std::fwrite(header, sizeof(header), 1U, file) == 1U;
    for (const std::vector<float>& plane : cache.planes) {
        ok = ok && std::fwrite(plane.data(), sizeof(float), plane.size(), file) == plane.size();
    }
    ok = ok && std::fwrite(cache.coverage.data(), sizeof(float), cache.coverage.size(), file) == cache.coverage.size();
    return std::fclose(file) == 0 && ok;
}

struct Choices final {
    bool grey{};
    bool merge{};
    bool crop{};
    bool half{};
    int log_bits{};  // 0: not used
    int scale{1};
    std::string store{};
};

// IEEE binary16 bits of a value already rounded through binary16.
std::uint16_t half_bits(float v) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    const std::uint32_t sign = (bits >> 16U) & 0x8000U;
    const float magnitude = std::abs(v);
    if (magnitude == 0.0F) {
        return static_cast<std::uint16_t>(sign);
    }
    if (magnitude < 6.103515625e-05F) {  // denormal: steps of 2^-24
        return static_cast<std::uint16_t>(sign | static_cast<std::uint32_t>(std::nearbyint(magnitude / 5.9604644775390625e-08F)));
    }
    int exponent = 0;
    const float mantissa = std::frexp(magnitude, &exponent);  // 0.5 <= m < 1, magnitude = m · 2^exponent
    const std::uint32_t e = static_cast<std::uint32_t>(exponent - 1 + 15);
    const std::uint32_t m = static_cast<std::uint32_t>(std::nearbyint((mantissa * 2.0F - 1.0F) * 1024.0F));
    return static_cast<std::uint16_t>(sign | (e << 10U) | (m & 0x3FFU));
}

// Round a value through IEEE binary16 (round to nearest even, no denormal flush).
float through_half(float v) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    const std::uint32_t sign = bits & 0x80000000U;
    const float magnitude = std::abs(v);
    if (magnitude < 6.103515625e-05F) {  // below the smallest normal half: denormal steps of 2^-24
        const float step = 5.9604644775390625e-08F;
        const float rounded = std::nearbyint(magnitude / step) * step;
        return sign != 0U ? -rounded : rounded;
    }
    int exponent = 0;
    const float mantissa = std::frexp(magnitude, &exponent);  // 0.5 <= mantissa < 1
    const float steps = 2048.0F;                               // 11 significant bits
    const float rounded = std::ldexp(std::nearbyint(mantissa * steps) / steps, exponent);
    return sign != 0U ? -rounded : rounded;
}

// One stored light plane: one or three channels, at full or reduced resolution.
struct Stored final {
    int width{};
    int height{};
    int channels{};
    std::vector<float> values;
};

// One channel of the reduced grid at (gx, gy).
double stored_value(const Stored& stored, int stored_width, int channels, int gx, int gy, int c) {
    const std::size_t index = (static_cast<std::size_t>(gy) * static_cast<std::size_t>(stored_width) + static_cast<std::size_t>(gx)) *
                                  static_cast<std::size_t>(channels) +
                              static_cast<std::size_t>(c);
    return static_cast<double>(stored.values[index]);
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: pack <in.f32> <decoded out.f32> [--grey] [--merge] [--crop] [--half] [--log BITS] [--scale N]\n");
        return 2;
    }
    Choices choices{};
    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--grey") {
            choices.grey = true;
        } else if (arg == "--merge") {
            choices.merge = true;
        } else if (arg == "--crop") {
            choices.crop = true;
        } else if (arg == "--half") {
            choices.half = true;
        } else if (arg == "--log" && i + 1 < argc) {
            choices.log_bits = std::atoi(argv[++i]);
        } else if (arg == "--store" && i + 1 < argc) {
            choices.store = argv[++i];
        } else if (arg == "--scale" && i + 1 < argc) {
            choices.scale = std::max(1, std::atoi(argv[++i]));
        } else {
            std::fprintf(stderr, "unknown choice %s\n", arg.c_str());
            return 2;
        }
    }
    Cache cache{};
    if (!load(argv[1], cache)) {
        std::fprintf(stderr, "cannot read %s\n", argv[1]);
        return 1;
    }
    const int w = cache.width;
    const int h = cache.height;
    const std::size_t pixels = static_cast<std::size_t>(w) * static_cast<std::size_t>(h);
    const std::size_t original_bytes = 12U + (cache.planes.size() * 3U + 1U) * pixels * sizeof(float);

    // ---- the light planes to store: merged, grey -------------------------------------------
    std::vector<std::vector<float>> light;  // each width × height × 3, as the cache holds them
    light.push_back(cache.planes[0]);
    if (choices.merge && cache.lamps > 1) {
        std::vector<float> sum(pixels * 3U, 0.0F);
        for (int l = 1; l <= cache.lamps; ++l) {
            for (std::size_t k = 0; k < sum.size(); ++k) {
                sum[k] += cache.planes[static_cast<std::size_t>(l)][k];
            }
        }
        light.push_back(sum);
    } else {
        for (int l = 1; l <= cache.lamps; ++l) {
            light.push_back(cache.planes[static_cast<std::size_t>(l)]);
        }
    }
    const int channels = choices.grey ? 1 : 3;

    // ---- which pixels are stored: all, or each row's covered span ---------------------------
    std::vector<int> first(static_cast<std::size_t>(h), 0);
    std::vector<int> last(static_cast<std::size_t>(h), w - 1);  // inclusive; last < first: an empty row
    if (choices.crop) {
        for (int y = 0; y < h; ++y) {
            int lo = w;
            int hi = -1;
            for (int x = 0; x < w; ++x) {
                if (cache.coverage[static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x)] > 0.0F) {
                    lo = std::min(lo, x);
                    hi = std::max(hi, x);
                }
            }
            first[static_cast<std::size_t>(y)] = lo;
            last[static_cast<std::size_t>(y)] = hi;
        }
    }
    std::size_t stored_pixels = 0;
    for (int y = 0; y < h; ++y) {
        stored_pixels += static_cast<std::size_t>(std::max(0, last[static_cast<std::size_t>(y)] - first[static_cast<std::size_t>(y)] + 1));
    }

    // ---- reduce, quantize and decode each light plane ----------------------------------------
    const int s = choices.scale;
    const int sw = (w + s - 1) / s;
    const int sh = (h + s - 1) / s;
    std::size_t light_bytes = 0;
    std::vector<std::vector<float>> decoded(light.size(), std::vector<float>(pixels * 3U, 0.0F));
    std::vector<std::vector<float>> kept;  // each stored plane's values, for --store
    for (std::size_t p = 0; p < light.size(); ++p) {
        // grey: the channels' mean (they agree to float precision)
        std::vector<float> plane(pixels * static_cast<std::size_t>(channels), 0.0F);
        for (std::size_t k = 0; k < pixels; ++k) {
            if (channels == 1) {
                plane[k] = (light[p][k * 3U] + light[p][k * 3U + 1U] + light[p][k * 3U + 2U]) / 3.0F;
            } else {
                for (std::size_t c = 0; c < 3U; ++c) {
                    plane[k * 3U + c] = light[p][k * 3U + c];
                }
            }
        }
        // Light is premultiplied by coverage. Reduced resolution is taken on
        // the unpremultiplied light over covered pixels (so edges do not
        // darken), and premultiplied again after decoding.
        Stored stored{};
        stored.width = s > 1 ? sw : w;
        stored.height = s > 1 ? sh : h;
        stored.channels = channels;
        stored.values.assign(static_cast<std::size_t>(stored.width) * static_cast<std::size_t>(stored.height) * static_cast<std::size_t>(channels), 0.0F);
        if (s > 1) {
            for (int by = 0; by < sh; ++by) {
                for (int bx = 0; bx < sw; ++bx) {
                    std::vector<double> sum(static_cast<std::size_t>(channels), 0.0);
                    double weight = 0.0;
                    for (int y = by * s; y < std::min(h, by * s + s); ++y) {
                        for (int x = bx * s; x < std::min(w, bx * s + s); ++x) {
                            const std::size_t k = static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x);
                            const double a = cache.coverage[k];
                            if (a <= 0.0) {
                                continue;
                            }
                            for (int c = 0; c < channels; ++c) {
                                sum[static_cast<std::size_t>(c)] += plane[k * static_cast<std::size_t>(channels) + static_cast<std::size_t>(c)];  // a × L
                            }
                            weight += a;
                        }
                    }
                    for (int c = 0; c < channels; ++c) {
                        stored.values[(static_cast<std::size_t>(by) * static_cast<std::size_t>(sw) + static_cast<std::size_t>(bx)) * static_cast<std::size_t>(channels) + static_cast<std::size_t>(c)] =
                            weight > 0.0 ? static_cast<float>(sum[static_cast<std::size_t>(c)] / weight) : 0.0F;
                    }
                }
            }
        } else {
            for (std::size_t k = 0; k < pixels; ++k) {
                const double a = cache.coverage[k];
                for (int c = 0; c < channels; ++c) {
                    const std::size_t i = k * static_cast<std::size_t>(channels) + static_cast<std::size_t>(c);
                    stored.values[i] = a > 0.0 ? static_cast<float>(plane[i] / a) : 0.0F;
                }
            }
        }
        // quantize
        float peak = 0.0F;
        for (const float v : stored.values) {
            peak = std::max(peak, v);
        }
        std::size_t value_bytes = 4U;
        if (choices.log_bits > 0) {
            // log2(1 + v / scale), the scale a 256th of the peak: fine steps in
            // the shadows, relative steps in the highlights
            const double scale = std::max(1e-6, static_cast<double>(peak) / 256.0);
            const double top = std::log2(1.0 + static_cast<double>(peak) / scale);
            const double levels = std::ldexp(1.0, choices.log_bits) - 1.0;
            for (float& v : stored.values) {
                const double code = std::nearbyint(std::log2(1.0 + std::max(0.0, static_cast<double>(v)) / scale) / top * levels);
                v = static_cast<float>((std::exp2(code / levels * top) - 1.0) * scale);
            }
            value_bytes = choices.log_bits <= 8 ? 1U : 2U;
        } else if (choices.half) {
            for (float& v : stored.values) {
                v = through_half(v);
            }
            value_bytes = 2U;
        }
        kept.push_back(stored.values);
        // count: covered pixels only when cropped at full resolution; the
        // reduced grid is small and kept whole
        const std::size_t count = s > 1 ? stored.values.size() : stored_pixels * static_cast<std::size_t>(channels);
        light_bytes += count * value_bytes;
        // decode: bilinear from the reduced grid (cell centres), premultiply
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const std::size_t k = static_cast<std::size_t>(y) * static_cast<std::size_t>(w) + static_cast<std::size_t>(x);
                const double a = cache.coverage[k];
                std::vector<double> value(static_cast<std::size_t>(channels), 0.0);
                if (s > 1) {
                    const double u = std::clamp((x + 0.5) / s - 0.5, 0.0, static_cast<double>(sw - 1));
                    const double v = std::clamp((y + 0.5) / s - 0.5, 0.0, static_cast<double>(sh - 1));
                    const int x0 = static_cast<int>(u);
                    const int y0 = static_cast<int>(v);
                    const int x1 = std::min(sw - 1, x0 + 1);
                    const int y1 = std::min(sh - 1, y0 + 1);
                    const double fx = u - x0;
                    const double fy = v - y0;
                    for (int c = 0; c < channels; ++c) {
                        const double v00 = stored_value(stored, sw, channels, x0, y0, c);
                        const double v10 = stored_value(stored, sw, channels, x1, y0, c);
                        const double v01 = stored_value(stored, sw, channels, x0, y1, c);
                        const double v11 = stored_value(stored, sw, channels, x1, y1, c);
                        value[static_cast<std::size_t>(c)] = (v00 * (1 - fx) + v10 * fx) * (1 - fy) + (v01 * (1 - fx) + v11 * fx) * fy;
                    }
                } else {
                    for (int c = 0; c < channels; ++c) {
                        value[static_cast<std::size_t>(c)] = stored.values[k * static_cast<std::size_t>(channels) + static_cast<std::size_t>(c)];
                    }
                }
                for (std::size_t c = 0; c < 3U; ++c) {
                    decoded[p][k * 3U + c] = static_cast<float>(a * value[channels == 1 ? 0U : c]);
                }
            }
        }
    }

    // ---- coverage: binary32, binary16 or 8 bits ------------------------------------------------
    std::size_t coverage_bytes = stored_pixels * 4U;
    std::vector<float> coverage = cache.coverage;
    if (choices.log_bits > 0) {
        for (float& a : coverage) {
            a = std::nearbyint(a * 255.0F) / 255.0F;
        }
        coverage_bytes = stored_pixels;
    } else if (choices.half) {
        for (float& a : coverage) {
            a = through_half(a);
        }
        coverage_bytes = stored_pixels * 2U;
    }
    const std::size_t span_bytes = choices.crop ? static_cast<std::size_t>(h) * 4U : 0U;  // two int16 per row
    const std::size_t stored_bytes = 16U + span_bytes + light_bytes + coverage_bytes;

    Cache out{};
    out.width = w;
    out.height = h;
    out.lamps = static_cast<int>(decoded.size()) - 1;
    out.planes = decoded;
    out.coverage = coverage;
    if (!choices.store.empty()) {
        if (!choices.grey || !choices.half || choices.scale != 1 || choices.crop || choices.log_bits > 0) {
            std::fprintf(stderr, "--store needs --grey --half at full resolution, uncropped\n");
            return 2;
        }
        std::FILE* file = std::fopen(choices.store.c_str(), "wb");
        if (file == nullptr) {
            std::fprintf(stderr, "cannot write %s\n", choices.store.c_str());
            return 1;
        }
        const std::int32_t header[3] = {w, h, static_cast<std::int32_t>(kept.size())};
        bool ok = std::fwrite(header, sizeof(header), 1U, file) == 1U;
        std::vector<std::uint16_t> bits(pixels, 0U);
        for (const std::vector<float>& plane : kept) {
            for (std::size_t k = 0; k < pixels; ++k) {
                bits[k] = half_bits(plane[k]);
            }
            ok = ok && std::fwrite(bits.data(), sizeof(std::uint16_t), pixels, file) == pixels;
        }
        for (std::size_t k = 0; k < pixels; ++k) {
            bits[k] = half_bits(coverage[k]);
        }
        ok = ok && std::fwrite(bits.data(), sizeof(std::uint16_t), pixels, file) == pixels;
        if (std::fclose(file) != 0 || !ok) {
            std::fprintf(stderr, "short write to %s\n", choices.store.c_str());
            return 1;
        }
    }
    if (!save(argv[2], out)) {
        std::fprintf(stderr, "cannot write %s\n", argv[2]);
        return 1;
    }
    std::printf("stored %zu bytes (original %zu, %.1f%%)\n", stored_bytes, original_bytes,
                100.0 * static_cast<double>(stored_bytes) / static_cast<double>(original_bytes));
    return 0;
}
