// Bake study: how close is a candidate bake to the reference, as the demo shows it?
//
// Both bakes are lamp caches (cpp/lamp_keys.cpp): int32 width, height and lamp
// count, then the room plane, each lamp's plane (RGB float32, premultiplied by
// coverage) and the coverage plane. A key has an up and a down cache. Each is
// composited as demo.html does it, over the dialog body's colour (ГОСТ 397):
//   v = room + power × colour × Σ lamps, mixed up→down by the press,
//   lit = 1 − e^(−1.3 v / coverage), shown = lit × coverage + body × (1 − coverage),
// in the states the demo shows: off and up; lit and up; lit and down; lit,
// half pressed. Each state's sRGB images are compared by CIEDE2000.
//
// Usage: compare <colour r> <g> <b> <reference up> <reference down> <candidate up> <candidate down>
// Prints max, 99.9th percentile and mean ΔE00 over all states' pixels, and
// the share of pixels with ΔE00 above 1 (a just-noticeable difference) and 2.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

struct Cache final {
    int width{};
    int height{};
    int lamps{};
    std::vector<float> room;                // width × height × 3
    std::vector<std::vector<float>> glows;  // per lamp, width × height × 3
    std::vector<float> coverage;            // width × height
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
    cache.room.assign(pixels * 3U, 0.0F);
    ok = ok && std::fread(cache.room.data(), sizeof(float), cache.room.size(), file) == cache.room.size();
    cache.glows.assign(static_cast<std::size_t>(cache.lamps), std::vector<float>(pixels * 3U, 0.0F));
    for (std::vector<float>& glow : cache.glows) {
        ok = ok && std::fread(glow.data(), sizeof(float), glow.size(), file) == glow.size();
    }
    cache.coverage.assign(pixels, 0.0F);
    ok = ok && std::fread(cache.coverage.data(), sizeof(float), cache.coverage.size(), file) == cache.coverage.size();
    std::fclose(file);
    return ok;
}

double srgb_to_linear(double c) {
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

double linear_to_srgb(double v) {
    const double c = std::clamp(v, 0.0, 1.0);
    return c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
}

struct Lab final {
    double l{};
    double a{};
    double b{};
};

// sRGB (0..1, encoded) to CIELAB (D65), through 8-bit quantization as shown.
Lab to_lab(double r8, double g8, double b8) {
    const double r = srgb_to_linear(r8 / 255.0);
    const double g = srgb_to_linear(g8 / 255.0);
    const double b = srgb_to_linear(b8 / 255.0);
    const double x = (0.4124564 * r + 0.3575761 * g + 0.1804375 * b) / 0.95047;
    const double y = 0.2126729 * r + 0.7151522 * g + 0.0721750 * b;
    const double z = (0.0193339 * r + 0.1191920 * g + 0.9503041 * b) / 1.08883;
    const double e = 216.0 / 24389.0;
    const double k = 24389.0 / 27.0;
    const double fx = x > e ? std::cbrt(x) : (k * x + 16.0) / 116.0;
    const double fy = y > e ? std::cbrt(y) : (k * y + 16.0) / 116.0;
    const double fz = z > e ? std::cbrt(z) : (k * z + 16.0) / 116.0;
    return Lab{116.0 * fy - 16.0, 500.0 * (fx - fy), 200.0 * (fy - fz)};
}

// CIEDE2000 (Sharma, Wu and Dalal's formulation).
double delta_e_2000(Lab p, Lab q) {
    const double pi = 3.14159265358979323846;
    const double c1 = std::hypot(p.a, p.b);
    const double c2 = std::hypot(q.a, q.b);
    const double c_mean = 0.5 * (c1 + c2);
    const double c7 = std::pow(c_mean, 7.0);
    const double g = 0.5 * (1.0 - std::sqrt(c7 / (c7 + std::pow(25.0, 7.0))));
    const double a1 = (1.0 + g) * p.a;
    const double a2 = (1.0 + g) * q.a;
    const double cp1 = std::hypot(a1, p.b);
    const double cp2 = std::hypot(a2, q.b);
    double h1 = std::atan2(p.b, a1);
    double h2 = std::atan2(q.b, a2);
    if (h1 < 0.0) {
        h1 += 2.0 * pi;
    }
    if (h2 < 0.0) {
        h2 += 2.0 * pi;
    }
    const double dl = q.l - p.l;
    const double dc = cp2 - cp1;
    double dh = 0.0;
    if (cp1 * cp2 != 0.0) {
        dh = h2 - h1;
        if (dh > pi) {
            dh -= 2.0 * pi;
        } else if (dh < -pi) {
            dh += 2.0 * pi;
        }
    }
    const double dhh = 2.0 * std::sqrt(cp1 * cp2) * std::sin(dh / 2.0);
    const double l_mean = 0.5 * (p.l + q.l);
    const double cp_mean = 0.5 * (cp1 + cp2);
    double h_mean = h1 + h2;
    if (cp1 * cp2 != 0.0) {
        if (std::abs(h1 - h2) > pi) {
            h_mean = h1 + h2 < 2.0 * pi ? (h1 + h2 + 2.0 * pi) / 2.0 : (h1 + h2 - 2.0 * pi) / 2.0;
        } else {
            h_mean = (h1 + h2) / 2.0;
        }
    }
    const double t = 1.0 - 0.17 * std::cos(h_mean - pi / 6.0) + 0.24 * std::cos(2.0 * h_mean) +
                     0.32 * std::cos(3.0 * h_mean + pi / 30.0) - 0.20 * std::cos(4.0 * h_mean - 63.0 * pi / 180.0);
    const double d_theta = 30.0 * pi / 180.0 * std::exp(-std::pow((h_mean * 180.0 / pi - 275.0) / 25.0, 2.0));
    const double cp7 = std::pow(cp_mean, 7.0);
    const double rc = 2.0 * std::sqrt(cp7 / (cp7 + std::pow(25.0, 7.0)));
    const double l50 = (l_mean - 50.0) * (l_mean - 50.0);
    const double sl = 1.0 + 0.015 * l50 / std::sqrt(20.0 + l50);
    const double sc = 1.0 + 0.045 * cp_mean;
    const double sh = 1.0 + 0.015 * cp_mean * t;
    const double rt = -std::sin(2.0 * d_theta) * rc;
    const double tl = dl / sl;
    const double tc = dc / sc;
    const double th = dhh / sh;
    return std::sqrt(tl * tl + tc * tc + th * th + rt * tc * th);
}

struct State final {
    double power{};
    double press{};
};

// One state's displayed sRGB bytes, written to `out` (width × height × 3).
void composite(const Cache& up, const Cache& down, std::array<double, 3> colour, State state, std::vector<double>& out) {
    const std::array<double, 3> body = {srgb_to_linear(0x4d / 255.0), srgb_to_linear(0x67 / 255.0), srgb_to_linear(0x6d / 255.0)};
    const std::size_t pixels = up.coverage.size();
    out.assign(pixels * 3U, 0.0);
    for (std::size_t k = 0; k < pixels; ++k) {
        const double cover = up.coverage[k] + (down.coverage[k] - up.coverage[k]) * state.press;
        const double inverse = cover > 1e-6 ? 1.0 / cover : 0.0;
        for (std::size_t c = 0; c < 3U; ++c) {
            const std::size_t i = k * 3U + c;
            double a = up.room[i];
            double b = down.room[i];
            for (std::size_t l = 0; l < up.glows.size(); ++l) {
                a += state.power * colour[c] * up.glows[l][i];
                b += state.power * colour[c] * down.glows[l][i];
            }
            const double lit = 1.0 - std::exp(-1.3 * (a + (b - a) * state.press) * inverse);
            const double shown = lit * cover + body[c] * (1.0 - cover);
            out[i] = std::round(linear_to_srgb(shown) * 255.0);
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 8) {
        std::fprintf(stderr, "usage: compare <r> <g> <b> <reference up> <reference down> <candidate up> <candidate down>\n");
        return 2;
    }
    const std::array<double, 3> colour = {std::atof(argv[1]), std::atof(argv[2]), std::atof(argv[3])};
    Cache reference_up{};
    Cache reference_down{};
    Cache candidate_up{};
    Cache candidate_down{};
    if (!load(argv[4], reference_up) || !load(argv[5], reference_down) || !load(argv[6], candidate_up) || !load(argv[7], candidate_down)) {
        std::fprintf(stderr, "cannot read a cache\n");
        return 1;
    }
    if (candidate_up.width != reference_up.width || candidate_up.height != reference_up.height) {
        std::fprintf(stderr, "size mismatch: reference %dx%d, candidate %dx%d\n", reference_up.width, reference_up.height,
                     candidate_up.width, candidate_up.height);
        return 1;
    }
    const std::array<State, 4> states = {State{0.0, 0.0}, State{1.0, 0.0}, State{1.0, 1.0}, State{1.0, 0.5}};
    std::vector<double> differences;
    std::vector<double> expected;
    std::vector<double> got;
    for (const State& state : states) {
        composite(reference_up, reference_down, colour, state, expected);
        composite(candidate_up, candidate_down, colour, state, got);
        for (std::size_t k = 0; k < expected.size(); k += 3U) {
            const Lab p = to_lab(expected[k], expected[k + 1U], expected[k + 2U]);
            const Lab q = to_lab(got[k], got[k + 1U], got[k + 2U]);
            differences.push_back(delta_e_2000(p, q));
        }
    }
    std::vector<double> sorted = differences;
    std::sort(sorted.begin(), sorted.end());
    double sum = 0.0;
    std::size_t over1 = 0;
    std::size_t over2 = 0;
    for (const double d : sorted) {
        sum += d;
        over1 += d > 1.0 ? 1U : 0U;
        over2 += d > 2.0 ? 1U : 0U;
    }
    const std::size_t n = sorted.size();
    const double p999 = sorted[std::min(n - 1U, static_cast<std::size_t>(0.999 * static_cast<double>(n)))];
    std::printf("dE00 max %.3f  p99.9 %.3f  mean %.4f  >1: %.3f%%  >2: %.3f%%\n", sorted[n - 1U], p999, sum / static_cast<double>(n),
                100.0 * static_cast<double>(over1) / static_cast<double>(n), 100.0 * static_cast<double>(over2) / static_cast<double>(n));
    return 0;
}
