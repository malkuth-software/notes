#include "surface_kernels.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace industrial_surfaces {
namespace {

constexpr double pi = 3.141592653589793;

// ---- integer hash, noise and cells ----------------------------------------------------
//
// The arithmetic follows the JavaScript reference: 32-bit multiplies that wrap,
// logical shifts, doubles elsewhere.

// Wrap any integer-valued double into int32 the way JavaScript's `v | 0` does.
std::uint32_t wrap32(double v) {
    const double m = std::fmod(std::trunc(v), 4294967296.0);
    const double positive = m < 0.0 ? m + 4294967296.0 : m;
    return static_cast<std::uint32_t>(positive);
}

double hash_unit(std::uint32_t x, std::uint32_t y, std::uint32_t s) {
    std::uint32_t h = (x * 0x27d4eb2dU) ^ (y * 0x165667b1U) ^ (s * 0x9e3779b1U);
    h = (h ^ (h >> 15U)) * 0x85ebca6bU;
    h = (h ^ (h >> 13U)) * 0xc2b2ae35U;
    return static_cast<double>(h ^ (h >> 16U)) / 4294967296.0;
}

double hash_unit(double x, double y, double s) { return hash_unit(wrap32(x), wrap32(y), wrap32(s)); }

// Value noise with smoothstep interpolation; cells never smaller than a pixel.
double noise(double x, double y, double cell, double s) {
    const double c = cell < 1.0 ? 1.0 : cell;
    const double fx0 = x / c;
    const double fy0 = y / c;
    const double ix = std::floor(fx0);
    const double iy = std::floor(fy0);
    const double fx = fx0 - ix;
    const double fy = fy0 - iy;
    const double ux = fx * fx * (3.0 - 2.0 * fx);
    const double uy = fy * fy * (3.0 - 2.0 * fy);
    const double a = hash_unit(ix, iy, s);
    const double b = hash_unit(ix + 1.0, iy, s);
    const double cc = hash_unit(ix, iy + 1.0, s);
    const double d = hash_unit(ix + 1.0, iy + 1.0, s);
    return a + (b - a) * ux + (cc - a) * uy + (a - b - cc + d) * ux * uy;
}

// Jittered-cell distances in cell units: nearest, second nearest, and the
// nearest cell's identity hash.
struct CellDistances final {
    double nearest{};
    double second{};
    double identity{};
};

CellDistances cells(double x, double y, double cell, double s) {
    const double c = cell < 1.0 ? 1.0 : cell;
    const double gx = x / c;
    const double gy = y / c;
    const double ix = std::floor(gx);
    const double iy = std::floor(gy);
    double d1 = 9.0;
    double d2 = 9.0;
    double id = 0.0;
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            const double cx = ix + i;
            const double cy = iy + j;
            const double dx = gx - (cx + 0.15 + 0.7 * hash_unit(cx, cy, s));
            const double dy = gy - (cy + 0.15 + 0.7 * hash_unit(cx, cy, s + 1.0));
            const double d = dx * dx + dy * dy;
            if (d < d1) {
                d2 = d1;
                d1 = d;
                id = hash_unit(cx, cy, s + 2.0);
            } else if (d < d2) {
                d2 = d;
            }
        }
    }
    return CellDistances{std::sqrt(d1), std::sqrt(d2), id};
}

double smooth(double a, double b, double x) {
    double t = (x - a) / (b - a);
    t = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    return t * t * (3.0 - 2.0 * t);
}

// ---- colour tables ------------------------------------------------------------------------

// sRGB byte to linear, stored as float as the reference stores it.
struct LinearTable final {
    std::array<float, 256> value{};
};

LinearTable make_linear_table() {
    LinearTable table{};
    for (int i = 0; i < 256; ++i) {
        const double c = i / 255.0;
        table.value[static_cast<std::size_t>(i)] =
            static_cast<float>(c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4));
    }
    return table;
}

// Linear [0, 1] to sRGB byte through 4097 steps.
struct EncodeTable final {
    std::array<std::uint8_t, 4097> value{};
};

EncodeTable make_encode_table() {
    EncodeTable table{};
    for (int i = 0; i <= 4096; ++i) {
        const double c = i / 4096.0;
        const double v = c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
        table.value[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>(std::floor(std::clamp(v, 0.0, 1.0) * 255.0 + 0.5));
    }
    return table;
}

const LinearTable& linear_table() {
    static const LinearTable table = make_linear_table();
    return table;
}

const EncodeTable& encode_table() {
    static const EncodeTable table = make_encode_table();
    return table;
}

std::uint8_t encode(double v) {
    const double clamped = v <= 0.0 ? 0.0 : (v >= 1.0 ? 1.0 : v);
    return encode_table().value[static_cast<std::size_t>(clamped * 4096.0)];
}

struct Linear final {
    double red{};
    double green{};
    double blue{};
};

Linear linear_of(Rgb8 c) {
    const LinearTable& t = linear_table();
    return Linear{t.value[c.red], t.value[c.green], t.value[c.blue]};
}

void store_opaque(std::byte* pixel, double red, double green, double blue) {
    pixel[0] = static_cast<std::byte>(encode(blue));
    pixel[1] = static_cast<std::byte>(encode(green));
    pixel[2] = static_cast<std::byte>(encode(red));
    pixel[3] = static_cast<std::byte>(255U);
}

// The damage clipped to the surface and the target; empty when they miss.
PixelRect clip(FramebufferView target, PixelRect surface, PixelRect damage) {
    const int x0 = std::max({surface.x, damage.x, 0});
    const int y0 = std::max({surface.y, damage.y, 0});
    const int x1 = std::min({surface.x + surface.width, damage.x + damage.width, target.width});
    const int y1 = std::min({surface.y + surface.height, damage.y + damage.height, target.height});
    return PixelRect{x0, y0, std::max(0, x1 - x0), std::max(0, y1 - y0)};
}

// ---- finishes -----------------------------------------------------------------------------

// Shading constants of each finish, read once before the loop.
struct FinishConstants final {
    double relief{};
    double gloss{};
    double shininess{};
    double metallic{};
    bool own_colour{};
    Rgb8 colour{};
};

FinishConstants constants_of(FinishKind kind) {
    switch (kind) {
    case FinishKind::dusty_matte: return FinishConstants{0.35, 0.015, 6.0, 0.0, false, Rgb8{}};
    case FinishKind::blasted_under_paint: return FinishConstants{0.9, 0.14, 18.0, 0.0, false, Rgb8{}};
    case FinishKind::orange_peel: return FinishConstants{0.85, 0.22, 38.0, 0.0, false, Rgb8{}};
    case FinishKind::hammertone: return FinishConstants{2.1, 0.32, 22.0, 0.65, false, Rgb8{}};
    case FinishKind::wrinkle: return FinishConstants{1.7, 0.10, 14.0, 0.0, false, Rgb8{}};
    case FinishKind::bead_blast: return FinishConstants{0.65, 0.45, 7.0, 1.0, true, Rgb8{0x8b, 0x91, 0x94}};
    case FinishKind::sand_cast: return FinishConstants{1.35, 0.05, 9.0, 0.0, false, Rgb8{}};
    case FinishKind::flat:
    default: return FinishConstants{0.0, 0.0, 1.0, 0.0, false, Rgb8{}};
    }
}

// One finish evaluated at a device position relative to the surface's origin.
// The template argument is the declared choice of finish; each instantiation
// is the whole row loop for one finish, so nothing is dispatched per pixel.
template <FinishKind Kind>
FinishSample evaluate(double x, double y, double k, double s) {
    double h = 0.0;
    double a = 1.0;
    double d = 0.0;
    double sp = 1.0;
    if constexpr (Kind == FinishKind::dusty_matte) {
        h = 0.6 * noise(x, y, 3.0 * k, s) + 0.4 * noise(x, y, 1.2 * k, s + 1.0);
        a = 1.0 + (noise(x, y, 48.0 * k, s + 2.0) - 0.5) * 0.05;
        const double fleck = std::max(1.0, k * 0.8);
        const double speck = hash_unit(std::floor(x / fleck), std::floor(y / fleck), s + 3.0);
        const double haze = noise(x, y, 90.0 * k, s + 4.0);
        d = 0.10 * haze + (speck > 0.985 ? 0.35 * (speck - 0.985) / 0.015 : 0.0);
    } else if constexpr (Kind == FinishKind::blasted_under_paint) {
        const CellDistances coarse = cells(x, y, 4.2 * k, s);
        const double pit_a = 1.0 - smooth(0.0, 0.8, coarse.nearest);
        const double ridge_a = 1.0 - smooth(0.0, 0.10, coarse.second - coarse.nearest);
        const double size_a = 0.6 + 0.4 * coarse.identity;
        const CellDistances fine = cells(x, y, 2.3 * k, s + 11.0);
        const double pit_b = 1.0 - smooth(0.0, 0.8, fine.nearest);
        const double ridge_b = 1.0 - smooth(0.0, 0.12, fine.second - fine.nearest);
        const double film = 0.35 * noise(x, y, 16.0 * k, s + 12.0);
        h = -1.6 * pit_a * pit_a * size_a + 0.35 * ridge_a - (0.7 * pit_b * pit_b - 0.15 * ridge_b) * (1.0 - film);
        a = 1.0 + 0.025 * ridge_a - 0.02 * pit_a + (noise(x, y, 70.0 * k, s + 13.0) - 0.5) * 0.02;
        sp = 0.75 + 0.5 * ridge_a;
    } else if constexpr (Kind == FinishKind::orange_peel) {
        h = 0.7 * noise(x, y, 5.5 * k, s) + 0.3 * noise(x, y, 2.6 * k, s + 1.0);
        a = 1.0 + (noise(x, y, 60.0 * k, s + 2.0) - 0.5) * 0.025;
    } else if constexpr (Kind == FinishKind::hammertone) {
        const CellDistances large = cells(x, y, 8.5 * k, s);
        const double big = smooth(0.0, 0.95, large.nearest);
        const CellDistances little = cells(x, y, 4.1 * k, s + 9.0);
        const double small = smooth(0.0, 0.95, little.nearest);
        h = 0.7 * big * big + 0.3 * small * small + 0.06 * noise(x, y, 1.5 * k, s + 5.0);
        a = 1.0 + (large.identity - 0.5) * 0.05 + (noise(x, y, 40.0 * k, s + 4.0) - 0.5) * 0.06;
        const double flake_cell = std::max(1.0, k);
        const double flake = hash_unit(std::floor(x / flake_cell), std::floor(y / flake_cell), s + 6.0);
        sp = flake > 0.93 ? 2.6 : 0.8;
    } else if constexpr (Kind == FinishKind::wrinkle) {
        const double wx = x + (noise(x, y, 7.0 * k, s + 7.0) - 0.5) * 6.0 * k;
        const double wy = y + (noise(x, y, 7.0 * k, s + 8.0) - 0.5) * 6.0 * k;
        const double r1 = 1.0 - std::abs(2.0 * noise(wx, wy, 2.3 * k, s) - 1.0);
        const double r2 = 1.0 - std::abs(2.0 * noise(wx, wy, 1.1 * k, s + 1.0) - 1.0);
        h = 0.75 * r1 * r1 + 0.25 * r2;
        a = 1.0 - (1.0 - r1) * 0.05;
    } else if constexpr (Kind == FinishKind::bead_blast) {
        h = 0.5 * noise(x, y, 0.9 * k, s) + 0.5 * noise(x, y, 1.7 * k, s + 1.0);
        a = 1.0 + (noise(x, y, 30.0 * k, s + 2.0) - 0.5) * 0.04;
    } else if constexpr (Kind == FinishKind::sand_cast) {
        const CellDistances c = cells(x, y, 3.2 * k, s);
        const double pit = c.identity > 0.72 ? 1.0 - smooth(0.0, 0.32, c.nearest) : 0.0;
        h = 0.55 * noise(x, y, 1.1 * k, s + 1.0) + 0.45 * noise(x, y, 2.8 * k, s + 2.0) - 0.9 * pit;
        a = 1.0 - pit * 0.18 + (noise(x, y, 25.0 * k, s + 3.0) - 0.5) * 0.05;
        d = pit * 0.15;
    }
    return FinishSample{static_cast<float>(h), static_cast<float>(a), static_cast<float>(d), static_cast<float>(sp)};
}

template <FinishKind Kind>
void fill_row(std::vector<FinishSample>& row, int y, int x0, int count, PixelRect surface, double k, double s) {
    for (int i = 0; i <= count; ++i) {
        row[static_cast<std::size_t>(i)] = evaluate<Kind>(static_cast<double>(x0 + i - surface.x), static_cast<double>(y - surface.y), k, s);
    }
}

// The light and half vectors from azimuth and elevation, the eye straight above.
struct Lighting final {
    std::array<double, 3> light{};
    std::array<double, 3> half{};
};

Lighting lighting_of(double azimuth_degrees, double elevation_degrees) {
    const double az = azimuth_degrees * pi / 180.0;
    const double el = elevation_degrees * pi / 180.0;
    Lighting l{};
    l.light = {std::cos(el) * std::cos(az), -std::cos(el) * std::sin(az), std::sin(el)};
    const double hz = l.light[2] + 1.0;
    const double hn = std::hypot(l.light[0], l.light[1], hz);
    l.half = {l.light[0] / hn, l.light[1] / hn, hz / hn};
    return l;
}

template <FinishKind Kind>
std::size_t paint_rows(FramebufferView target, PixelRect surface, PixelRect area, const FinishParameters& p, FinishScratch& scratch) {
    const FinishConstants fc = constants_of(Kind);
    const double k = p.scale;
    const double s = static_cast<double>(p.seed);
    const Linear base = linear_of(fc.own_colour ? fc.colour : p.paint);
    const Linear dust = linear_of(p.dust);
    const Lighting lit = lighting_of(p.light_azimuth_degrees, p.light_elevation_degrees);
    const double ambient = p.ambient;
    const double kd = 1.0 - ambient;
    const double relief = fc.relief * k * p.relief_scale;
    const double flat_diffuse = ambient + kd * lit.light[2];
    const double gloss = fc.gloss * p.gloss_scale;
    const double shin = fc.shininess;
    const double flat_spec = gloss * std::pow(lit.half[2], shin);
    const double metal = fc.metallic;
    const int count = area.width;
    std::vector<FinishSample>* top = &scratch.upper;
    std::vector<FinishSample>* bottom = &scratch.lower;
    fill_row<Kind>(*top, area.y, area.x, count, surface, k, s);
    for (int y = area.y; y < area.y + area.height; ++y) {
        fill_row<Kind>(*bottom, y + 1, area.x, count, surface, k, s);
        const std::vector<FinishSample>& upper = *top;
        const std::vector<FinishSample>& lower = *bottom;
        std::byte* pixel = target.pixels + static_cast<std::size_t>(y) * target.row_bytes + static_cast<std::size_t>(area.x) * 4U;
        for (int i = 0; i < count; ++i, pixel += 4) {
            const std::size_t at = static_cast<std::size_t>(i);
            const double ht = upper[at].height;
            const double gx = (static_cast<double>(upper[at + 1U].height) - ht) * relief;
            const double gy = (static_cast<double>(lower[at].height) - ht) * relief;
            const double inv = 1.0 / std::sqrt(gx * gx + gy * gy + 1.0);
            const double nx = -gx * inv;
            const double ny = -gy * inv;
            const double nz = inv;
            const double ndl = nx * lit.light[0] + ny * lit.light[1] + nz * lit.light[2];
            const double diffuse = (ambient + kd * (ndl > 0.0 ? ndl : 0.0)) / flat_diffuse;
            const double ndh = nx * lit.half[0] + ny * lit.half[1] + nz * lit.half[2];
            const double spec = (gloss * (ndh > 0.0 ? std::pow(ndh, shin) : 0.0) - flat_spec * 0.6) * upper[at].specular;
            const double a = upper[at].albedo;
            const double d = upper[at].dust;
            const double r = (base.red * a) * (1.0 - d) + dust.red * d;
            const double g = (base.green * a) * (1.0 - d) + dust.green * d;
            const double b = (base.blue * a) * (1.0 - d) + dust.blue * d;
            const double sr = spec * (1.0 - metal + metal * r * 2.2);
            const double sg = spec * (1.0 - metal + metal * g * 2.2);
            const double sb = spec * (1.0 - metal + metal * b * 2.2);
            store_opaque(pixel, r * diffuse + sr, g * diffuse + sg, b * diffuse + sb);
        }
        std::vector<FinishSample>* swap = top;
        top = bottom;
        bottom = swap;
    }
    return static_cast<std::size_t>(count) * static_cast<std::size_t>(area.height);
}

// ---- glints -------------------------------------------------------------------------------

int poisson(double lambda, double u, double u2) {
    if (lambda <= 0.0) {
        return 0;
    }
    if (lambda > 24.0) {
        const double z = std::sqrt(-2.0 * std::log(std::max(u, 1e-12))) * std::cos(6.283185307 * u2);
        const double n = std::floor(lambda + std::sqrt(lambda) * z + 0.5);
        return n > 0.0 ? static_cast<int>(n) : 0;
    }
    double p = std::exp(-lambda);
    double f = p;
    int n = 0;
    while (u > f && n < 64) {
        ++n;
        p *= lambda / n;
        f += p;
    }
    return n;
}

// The same draw with e^(−λ) supplied (paint_glints_retained keeps it per bin).
int poisson_from(double lambda, double start, double u, double u2) {
    if (lambda <= 0.0) {
        return 0;
    }
    if (lambda > 24.0) {
        const double z = std::sqrt(-2.0 * std::log(std::max(u, 1e-12))) * std::cos(6.283185307 * u2);
        const double n = std::floor(lambda + std::sqrt(lambda) * z + 0.5);
        return n > 0.0 ? static_cast<int>(n) : 0;
    }
    double p = start;
    double f = p;
    int n = 0;
    while (u > f && n < 64) {
        ++n;
        p *= lambda / n;
        f += p;
    }
    return n;
}

// One tile's retained speck results (paint_glints_retained): points are named
// (cell x · 2 + d, cell y) as speck_density names them; bins by their indices.
struct GlintTile final {
    double facets{};
    int point_x0{};
    int point_y0{};
    int point_width{};
    int point_height{};
    int bin_x0{};
    int bin_y0{};
    int bin_width{};
    int bin_height{};
    int cell_x0{};
    int cell_y0{};
    int cell_width{};
    int cell_height{};
    std::int32_t* counts{};
    double* hues{};
    double* jitter{};
    double* lambdas{};  // per bin: the expected facet count, and e^(−λ) (NaN: not yet computed)
    double* starts{};
};

struct GlintState final {
    const GlintParameters* p{};
    double sigma2{};
    double tau_area{};
    double bin_width{};
    Linear metal{};
    Linear paint{};
    GlintTile* tile{};  // null: compute every speck result in place (paint_glints)
};

// Facet density estimate and the hue seed of its glints.
struct Density final {
    double value{};
    double hue{};
};

std::int32_t imul(std::int32_t a, std::uint32_t b) {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) * b);
}

// One speck point's facet count in one half-vector bin, and the hue hash of
// its glints (computed only when the count is positive).
struct BinResult final {
    int count{};
    double hue{};
};

BinResult compute_bin(const GlintState& st, std::int32_t xi, std::int32_t yi, double facets, double cx, double cy, double seed) {
    const double w = st.bin_width;
    const double sx = (cx + 0.5) * w;
    const double sy = (cy + 0.5) * w;
    const double pdf = std::exp(-(sx * sx + sy * sy) / st.sigma2) / (pi * st.sigma2);
    const double lambda = facets * pdf * st.tau_area;
    const std::int32_t key = imul(static_cast<std::int32_t>(wrap32(cx)), 0x632be5abU) ^ imul(static_cast<std::int32_t>(wrap32(cy)), 0x85157af5U);
    const double u = hash_unit(static_cast<double>(xi ^ key), static_cast<double>(yi) + key, seed);
    const double u2 = hash_unit(static_cast<double>(xi) + key, static_cast<double>(yi ^ key), seed + 1.0);
    BinResult result{};
    result.count = poisson(lambda, u, u2);
    if (result.count > 0) {
        result.hue = hash_unit(static_cast<double>(xi), static_cast<double>(yi), static_cast<double>(key) + seed + 2.0);
    }
    return result;
}

// The same with the bin's λ and e^(−λ) supplied: the point's hashes alone.
BinResult compute_bin_from(std::int32_t xi, std::int32_t yi, double cx, double cy, double seed, double lambda, double start) {
    const std::int32_t key = imul(static_cast<std::int32_t>(wrap32(cx)), 0x632be5abU) ^ imul(static_cast<std::int32_t>(wrap32(cy)), 0x85157af5U);
    const double u = hash_unit(static_cast<double>(xi ^ key), static_cast<double>(yi) + key, seed);
    const double u2 = hash_unit(static_cast<double>(xi) + key, static_cast<double>(yi ^ key), seed + 1.0);
    BinResult result{};
    result.count = poisson_from(lambda, start, u, u2);
    if (result.count > 0) {
        result.hue = hash_unit(static_cast<double>(xi), static_cast<double>(yi), static_cast<double>(key) + seed + 2.0);
    }
    return result;
}

// The same, from the tile's retained results when the point and bin lie in it.
BinResult bin_result(const GlintState& st, std::int32_t xi, std::int32_t yi, double facets, double cx, double cy, double seed) {
    GlintTile* tile = st.tile;
    if (tile != nullptr && facets == (*tile).facets) {
        const int a = xi - (*tile).point_x0;
        const int b = yi - (*tile).point_y0;
        const int u = static_cast<int>(cx) - (*tile).bin_x0;
        const int v = static_cast<int>(cy) - (*tile).bin_y0;
        if (a >= 0 && b >= 0 && u >= 0 && v >= 0 && a < (*tile).point_width && b < (*tile).point_height && u < (*tile).bin_width &&
            v < (*tile).bin_height) {
            const std::size_t index = ((static_cast<std::size_t>(b) * static_cast<std::size_t>((*tile).point_width) + static_cast<std::size_t>(a)) *
                                           static_cast<std::size_t>((*tile).bin_height) +
                                       static_cast<std::size_t>(v)) *
                                          static_cast<std::size_t>((*tile).bin_width) +
                                      static_cast<std::size_t>(u);
            if ((*tile).counts[index] < 0) {
                const std::size_t bin = static_cast<std::size_t>(v) * static_cast<std::size_t>((*tile).bin_width) + static_cast<std::size_t>(u);
                if (std::isnan((*tile).lambdas[bin])) {
                    const double w = st.bin_width;
                    const double sx = (cx + 0.5) * w;
                    const double sy = (cy + 0.5) * w;
                    const double pdf = std::exp(-(sx * sx + sy * sy) / st.sigma2) / (pi * st.sigma2);
                    (*tile).lambdas[bin] = facets * pdf * st.tau_area;
                    (*tile).starts[bin] = std::exp(-(*tile).lambdas[bin]);
                }
                const BinResult computed = compute_bin_from(xi, yi, cx, cy, seed, (*tile).lambdas[bin], (*tile).starts[bin]);
                (*tile).counts[index] = computed.count;
                (*tile).hues[index] = computed.hue;
            }
            return BinResult{(*tile).counts[index], (*tile).hues[index]};
        }
    }
    return compute_bin(st, xi, yi, facets, cx, cy, seed);
}

Density sub_pixel_density(const GlintState& st, double x, double y, double facets, double hx, double hy) {
    const double w = st.bin_width;
    const double bx = hx / w - 0.5;
    const double by = hy / w - 0.5;
    const double ix = std::floor(bx);
    const double iy = std::floor(by);
    const double fx = bx - ix;
    const double fy = by - iy;
    double acc = 0.0;
    double hue_acc = 0.0;
    double weight_acc = 0.0;
    const std::int32_t xi = static_cast<std::int32_t>(wrap32(x));
    const std::int32_t yi = static_cast<std::int32_t>(wrap32(y));
    const double seed = static_cast<double>((*st.p).seed);
    for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < 2; ++i) {
            const double cx = ix + i;
            const double cy = iy + j;
            const BinResult bin = bin_result(st, xi, yi, facets, cx, cy, seed);
            const int n = bin.count;
            const double wt = (i != 0 ? fx : 1.0 - fx) * (j != 0 ? fy : 1.0 - fy);
            acc += wt * n;
            if (n > 0) {
                hue_acc += wt * bin.hue;
                weight_acc += wt;
            }
        }
    }
    return Density{acc / (facets * st.tau_area), weight_acc > 0.0 ? hue_acc / weight_acc : 0.0};
}

// A speck cell's jittered point d, from the tile's retained jitter when the
// cell lies in it.
void speck_point(const GlintState& st, double cx, double cy, int d, double seed, double& px, double& py) {
    GlintTile* tile = st.tile;
    if (tile != nullptr) {
        const int a = static_cast<int>(cx) - (*tile).cell_x0;
        const int b = static_cast<int>(cy) - (*tile).cell_y0;
        if (a >= 0 && b >= 0 && a < (*tile).cell_width && b < (*tile).cell_height) {
            const std::size_t index = ((static_cast<std::size_t>(b) * static_cast<std::size_t>((*tile).cell_width) + static_cast<std::size_t>(a)) * 2U +
                                       static_cast<std::size_t>(d)) *
                                      2U;
            if (std::isnan((*tile).jitter[index])) {
                (*tile).jitter[index] = cx + hash_unit(cx, cy, seed + 30.0 + d * 2);
                (*tile).jitter[index + 1U] = cy + hash_unit(cx, cy, seed + 31.0 + d * 2);
            }
            px = (*tile).jitter[index];
            py = (*tile).jitter[index + 1U];
            return;
        }
    }
    px = cx + hash_unit(cx, cy, seed + 30.0 + d * 2);
    py = cy + hash_unit(cx, cy, seed + 31.0 + d * 2);
}

Density speck_density(const GlintState& st, double x, double y, double speck, double facet_px, double hx, double hy) {
    const double per_pixel = 1.0 / (facet_px * facet_px);
    if (speck <= 1.01) {
        return sub_pixel_density(st, x, y, per_pixel, hx, hy);
    }
    const double gx = x / speck;
    const double gy = y / speck;
    const double ix = std::floor(gx);
    const double iy = std::floor(gy);
    const double facets = per_pixel * speck * speck * 0.5;
    const double seed = static_cast<double>((*st.p).seed);
    double acc = 0.0;
    double wsum = 0.0;
    double hue = 0.0;
    double nearest = 9.0;
    double nx = 0.0;
    double ny = 0.0;
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            const double cx = ix + i;
            const double cy = iy + j;
            for (int d = 0; d < 2; ++d) {
                double px = 0.0;
                double py = 0.0;
                speck_point(st, cx, cy, d, seed, px, py);
                const double dx = gx - px;
                const double dy = gy - py;
                const double q = dx * dx + dy * dy;
                if (q < nearest) {
                    nearest = q;
                    nx = cx * 2.0 + d;
                    ny = cy;
                }
                if (q < 1.0) {
                    const double w = (1.0 - q) * (1.0 - q);
                    const Density one = sub_pixel_density(st, cx * 2.0 + d, cy, facets, hx, hy);
                    acc += w * one.value;
                    if (w > wsum * 0.5) {
                        hue = one.hue;
                    }
                    wsum += w;
                }
            }
        }
    }
    if (wsum > 1e-6) {
        return Density{acc / wsum, hue};
    }
    return sub_pixel_density(st, nx, ny, facets, hx, hy);
}

Density cell_density(const GlintState& st, double x, double y, double size, double hx, double hy) {
    const double gx = x / size;
    const double gy = y / size;
    const double ix = std::floor(gx);
    const double iy = std::floor(gy);
    double best = 9.0;
    double second = 9.0;
    double bx = 0.0;
    double by = 0.0;
    const double seed = static_cast<double>((*st.p).seed);
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            const double cx = ix + i;
            const double cy = iy + j;
            const double dx = gx - (cx + 0.1 + 0.8 * hash_unit(cx, cy, seed + 10.0));
            const double dy = gy - (cy + 0.1 + 0.8 * hash_unit(cx, cy, seed + 11.0));
            const double d = dx * dx + dy * dy;
            if (d < best) {
                second = best;
                best = d;
                bx = cx;
                by = cy;
            } else if (d < second) {
                second = d;
            }
        }
    }
    const double u1 = std::max(1e-9, hash_unit(bx, by, seed + 12.0));
    const double u2 = hash_unit(bx, by, seed + 13.0);
    const double r = std::sqrt(st.sigma2 * -std::log(u1));
    const double sx = r * std::cos(6.283185307 * u2);
    const double sy = r * std::sin(6.283185307 * u2);
    const double dx = hx - sx;
    const double dy = hy - sy;
    const double seam = std::sqrt(second) - std::sqrt(best);
    const double edge = seam < 0.08 ? 0.6 + 5.0 * seam : 1.0;
    const double tau = (*st.p).tau;
    return Density{std::exp(-(dx * dx + dy * dy) / (tau * tau)) / st.tau_area * edge, hash_unit(bx, by, seed + 14.0)};
}

void hsv_rainbow(double h, double& red, double& green, double& blue) {
    const std::array<double, 3> offsets = {5.0, 3.0, 1.0};
    std::array<double, 3> out{};
    for (std::size_t i = 0; i < 3U; ++i) {
        const double k = std::fmod(offsets[i] + h * 6.0, 6.0);
        out[i] = 1.0 - std::max(0.0, std::min({k, 4.0 - k, 1.0}));
    }
    red = out[0];
    green = out[1];
    blue = out[2];
}

double glint_tone(double v) { return v < 0.8 ? v : 0.8 + 0.2 * (1.0 - std::exp(-(v - 0.8) * 5.0)); }

void shade_glint(const GlintState& st, double x, double y, std::byte* pixel) {
    const GlintParameters& p = *st.p;
    const double k = p.scale;
    const double lx = p.light_x - x / k;
    const double ly = p.light_y - y / k;
    const double lz = p.light_z;
    const double vx = p.eye_x - x / k;
    const double vy = p.eye_y - y / k;
    const double vz = p.eye_z;
    const double li = 1.0 / std::hypot(lx, ly, lz);
    const double vi = 1.0 / std::hypot(vx, vy, vz);
    const double lux = lx * li;
    const double luy = ly * li;
    const double luz = lz * li;
    const double vux = vx * vi;
    const double vuy = vy * vi;
    const double vuz = vz * vi;
    double hx3 = lux + vux;
    double hy3 = luy + vuy;
    double hz3 = luz + vuz;
    const double hi = 1.0 / std::hypot(hx3, hy3, hz3);
    hx3 *= hi;
    hy3 *= hi;
    hz3 *= hi;
    // the facet slope that would mirror the light into the eye
    const double hx = -hx3 / hz3;
    const double hy = -hy3 / hz3;
    const double facet_px = p.grain * k;
    const double speck = std::max(1.0, p.speck * k);
    Density density{};
    if (facet_px <= 0.7) {
        density = speck_density(st, x, y, speck, facet_px, hx, hy);
    } else if (facet_px >= 1.3) {
        density = cell_density(st, x, y, facet_px, hx, hy);
    } else {
        const double t = (facet_px - 0.7) / 0.6;
        const Density a = sub_pixel_density(st, x, y, 1.0 / (facet_px * facet_px), hx, hy);
        const Density b = cell_density(st, x, y, facet_px, hx, hy);
        density = Density{a.value + (b.value - a.value) * t, t < 0.5 ? a.hue : b.hue};
    }
    const double cos2 = hz3 * hz3;
    const double d = density.value / (cos2 * cos2);
    const double vdh = lux * hx3 + luy * hy3 + luz * hz3;
    const double f_metal = 0.6 + 0.4 * std::pow(1.0 - vdh, 5.0);
    const double spec_metal = d * f_metal / (4.0 * vuz) * p.specular * (1.0 - p.film) * (1.0 - p.film);
    double spec_paint = 0.0;
    if (p.film > 0.0) {
        const double a2 = p.film_alpha * p.film_alpha;
        const double lobe = std::exp(-(hx * hx + hy * hy) / a2) / (pi * a2) / (cos2 * cos2);
        const double f_paint = 0.04 + 0.96 * std::pow(1.0 - vdh, 5.0);
        spec_paint = lobe * f_paint / (4.0 * vuz) * p.specular * p.film;
    }
    const double ndl = luz > 0.0 ? luz : 0.0;
    const double mottle = 1.0 + (noise(x, y, 40.0 * k, static_cast<double>(p.seed) + 20.0) - 0.5) * p.mottle;
    const double diffuse = (p.ambient + (1.0 - p.ambient) * ndl) * mottle;
    double tint_r = 1.0;
    double tint_g = 1.0;
    double tint_b = 1.0;
    if (p.iridescence > 0.0) {
        double rr = 0.0;
        double rg = 0.0;
        double rb = 0.0;
        hsv_rainbow(density.hue, rr, rg, rb);
        const double m = p.iridescence;
        tint_r = 1.0 - m + m * rr * 1.6;
        tint_g = 1.0 - m + m * rg * 1.6;
        tint_b = 1.0 - m + m * rb * 1.6;
    }
    const Linear base = p.film > 0.0 ? st.paint : st.metal;
    const double r = base.red * diffuse * p.diffuse + spec_metal * st.metal.red * tint_r + spec_paint;
    const double g = base.green * diffuse * p.diffuse + spec_metal * st.metal.green * tint_g + spec_paint;
    const double b = base.blue * diffuse * p.diffuse + spec_metal * st.metal.blue * tint_b + spec_paint;
    store_opaque(pixel, glint_tone(r), glint_tone(g), glint_tone(b));
}

}  // namespace

void reserve_finish_scratch(FinishScratch& scratch, int damage_width) {
    const std::size_t needed = static_cast<std::size_t>(std::max(0, damage_width)) + 1U;
    if (scratch.upper.size() < needed) {
        scratch.upper.resize(needed);
        scratch.lower.resize(needed);
    }
}

std::size_t paint_finish(FramebufferView target, PixelRect surface, PixelRect damage, const FinishParameters& parameters,
                         FinishScratch& scratch) {
    const PixelRect area = clip(target, surface, damage);
    if (area.width == 0 || area.height == 0) {
        return 0U;
    }
    if (scratch.upper.size() < static_cast<std::size_t>(area.width) + 1U) {
        return 0U;  // the caller reserved too little: write nothing
    }
    switch (parameters.kind) {
    case FinishKind::dusty_matte: return paint_rows<FinishKind::dusty_matte>(target, surface, area, parameters, scratch);
    case FinishKind::blasted_under_paint: return paint_rows<FinishKind::blasted_under_paint>(target, surface, area, parameters, scratch);
    case FinishKind::orange_peel: return paint_rows<FinishKind::orange_peel>(target, surface, area, parameters, scratch);
    case FinishKind::hammertone: return paint_rows<FinishKind::hammertone>(target, surface, area, parameters, scratch);
    case FinishKind::wrinkle: return paint_rows<FinishKind::wrinkle>(target, surface, area, parameters, scratch);
    case FinishKind::bead_blast: return paint_rows<FinishKind::bead_blast>(target, surface, area, parameters, scratch);
    case FinishKind::sand_cast: return paint_rows<FinishKind::sand_cast>(target, surface, area, parameters, scratch);
    case FinishKind::flat:
    default: return paint_rows<FinishKind::flat>(target, surface, area, parameters, scratch);
    }
}

GlintParameters glint_preset(GlintPreset preset) {
    GlintParameters p{};
    switch (preset) {
    case GlintPreset::bead_blast:
        p.grain = 0.05; p.speck = 0.75; p.alpha = 0.32; p.tau = 0.03; p.diffuse = 0.79; p.specular = 0.57; p.mottle = 0.05;
        p.metal = Rgb8{0xa5, 0xa5, 0xa5};
        return p;
    case GlintPreset::iridescent:
        p.grain = 0.10; p.speck = 1.4; p.alpha = 0.4; p.tau = 0.034; p.diffuse = 0.37; p.specular = 2.82; p.mottle = 0.06;
        p.iridescence = 0.27; p.metal = Rgb8{0xa6, 0xa5, 0xa7};
        return p;
    case GlintPreset::coarse_grit:
        p.grain = 0.9; p.speck = 0.0; p.alpha = 0.45; p.tau = 0.05; p.diffuse = 0.33; p.specular = 0.7; p.mottle = 0.08;
        p.metal = Rgb8{0x9f, 0x9f, 0x9f};
        return p;
    case GlintPreset::painted_sand:
        p.grain = 0.05; p.speck = 0.75; p.alpha = 0.42; p.tau = 0.034; p.diffuse = 0.9; p.specular = 1.23; p.mottle = 0.05;
        p.film = 0.7; p.film_alpha = 0.3; p.metal = Rgb8{0xa3, 0xa3, 0xa3}; p.paint = Rgb8{0x2f, 0x6e, 0x6c};
        return p;
    case GlintPreset::sand_blast:
    default:
        p.grain = 0.05; p.speck = 0.75; p.alpha = 0.42; p.tau = 0.034; p.diffuse = 0.46; p.specular = 1.23; p.mottle = 0.06;
        p.metal = Rgb8{0xa3, 0xa3, 0xa3};
        return p;
    }
}

std::size_t paint_glints(FramebufferView target, PixelRect surface, PixelRect damage, const GlintParameters& parameters) {
    const PixelRect area = clip(target, surface, damage);
    if (area.width == 0 || area.height == 0) {
        return 0U;
    }
    GlintState st{};
    st.p = &parameters;
    st.sigma2 = parameters.alpha * parameters.alpha;
    st.tau_area = pi * parameters.tau * parameters.tau;
    st.bin_width = parameters.tau * 2.0;
    st.metal = linear_of(parameters.metal);
    st.paint = linear_of(parameters.paint);
    for (int y = area.y; y < area.y + area.height; ++y) {
        std::byte* pixel = target.pixels + static_cast<std::size_t>(y) * target.row_bytes + static_cast<std::size_t>(area.x) * 4U;
        for (int x = area.x; x < area.x + area.width; ++x, pixel += 4) {
            shade_glint(st, static_cast<double>(x - surface.x), static_cast<double>(y - surface.y), pixel);
        }
    }
    return static_cast<std::size_t>(area.width) * static_cast<std::size_t>(area.height);
}

namespace {

constexpr int glint_tile = 32;

// The half-vector slope at a surface position, as shade_glint takes it.
void half_slope(const GlintParameters& p, double x, double y, double& hx, double& hy) {
    const double k = p.scale;
    const double lx = p.light_x - x / k;
    const double ly = p.light_y - y / k;
    const double vx = p.eye_x - x / k;
    const double vy = p.eye_y - y / k;
    const double li = 1.0 / std::hypot(lx, ly, p.light_z);
    const double vi = 1.0 / std::hypot(vx, vy, p.eye_z);
    double hx3 = lx * li + vx * vi;
    double hy3 = ly * li + vy * vi;
    double hz3 = p.light_z * li + p.eye_z * vi;
    const double hi = 1.0 / std::hypot(hx3, hy3, hz3);
    hx3 *= hi;
    hy3 *= hi;
    hz3 *= hi;
    hx = -hx3 / hz3;
    hy = -hy3 / hz3;
}

// The speck cells a tile's pixels can reach and the points they name: the
// tile's cells ± 2 (speck_density reaches ± 1 cell; one more for rounding).
int tile_cells(double speck) {
    return static_cast<int>(std::ceil(glint_tile / speck)) + 5;
}

// The half-vector bins a tile's pixels can meet: their corners' bins ± 1 on
// each side (the slope is monotone across a tile; bilinear blending reaches
// one bin beyond). Capped: a tile whose slope sweeps more falls back to
// computing in place beyond the cap.
constexpr int tile_bins = 6;

}  // namespace

void reserve_glint_scratch(GlintScratch& scratch, const GlintParameters& parameters) {
    const double speck = std::max(1.0, parameters.speck * parameters.scale);
    const std::size_t cells = static_cast<std::size_t>(tile_cells(speck));
    const std::size_t points = cells * 2U * cells;
    const std::size_t bins = static_cast<std::size_t>(tile_bins) * static_cast<std::size_t>(tile_bins);
    if (scratch.counts.size() < points * bins) {
        scratch.counts.resize(points * bins);
        scratch.hues.resize(points * bins);
    }
    if (scratch.jitter.size() < cells * cells * 4U) {
        scratch.jitter.resize(cells * cells * 4U);
    }
    if (scratch.lambdas.size() < bins) {
        scratch.lambdas.resize(bins);
        scratch.starts.resize(bins);
    }
}

std::size_t paint_glints_retained(FramebufferView target, PixelRect surface, PixelRect damage, const GlintParameters& parameters,
                                  GlintScratch& scratch) {
    const PixelRect area = clip(target, surface, damage);
    if (area.width == 0 || area.height == 0) {
        return 0U;
    }
    const double k = parameters.scale;
    const double facet_px = parameters.grain * k;
    const double speck = std::max(1.0, parameters.speck * k);
    const bool retained = facet_px <= 0.7 && speck > 1.01;  // the speck path; others gain nothing
    const int cells = tile_cells(speck);
    if (retained && (scratch.counts.size() < static_cast<std::size_t>(cells) * 2U * static_cast<std::size_t>(cells) * tile_bins * tile_bins ||
                     scratch.jitter.size() < static_cast<std::size_t>(cells) * static_cast<std::size_t>(cells) * 4U ||
                     scratch.lambdas.size() < static_cast<std::size_t>(tile_bins) * tile_bins)) {
        return 0U;  // the caller reserved too little: write nothing
    }
    GlintState st{};
    st.p = &parameters;
    st.sigma2 = parameters.alpha * parameters.alpha;
    st.tau_area = pi * parameters.tau * parameters.tau;
    st.bin_width = parameters.tau * 2.0;
    st.metal = linear_of(parameters.metal);
    st.paint = linear_of(parameters.paint);
    GlintTile tile{};
    tile.facets = (1.0 / (facet_px * facet_px)) * speck * speck * 0.5;  // as speck_density names its points' facets
    tile.counts = scratch.counts.data();
    tile.hues = scratch.hues.data();
    tile.jitter = scratch.jitter.data();
    tile.lambdas = scratch.lambdas.data();
    tile.starts = scratch.starts.data();
    for (int ty = area.y; ty < area.y + area.height; ty += glint_tile) {
        for (int tx = area.x; tx < area.x + area.width; tx += glint_tile) {
            const int tx1 = std::min(area.x + area.width, tx + glint_tile);
            const int ty1 = std::min(area.y + area.height, ty + glint_tile);
            st.tile = nullptr;
            if (retained) {
                // the tile in surface coordinates
                const double sx0 = tx - surface.x;
                const double sy0 = ty - surface.y;
                const double sx1 = tx1 - 1 - surface.x;
                const double sy1 = ty1 - 1 - surface.y;
                tile.cell_x0 = static_cast<int>(std::floor(sx0 / speck)) - 2;
                tile.cell_y0 = static_cast<int>(std::floor(sy0 / speck)) - 2;
                tile.cell_width = std::min(cells, static_cast<int>(std::floor(sx1 / speck)) + 2 - tile.cell_x0 + 1);
                tile.cell_height = std::min(cells, static_cast<int>(std::floor(sy1 / speck)) + 2 - tile.cell_y0 + 1);
                tile.point_x0 = tile.cell_x0 * 2;
                tile.point_y0 = tile.cell_y0;
                tile.point_width = tile.cell_width * 2;
                tile.point_height = tile.cell_height;
                int low_x = 1 << 30;
                int high_x = -(1 << 30);
                int low_y = 1 << 30;
                int high_y = -(1 << 30);
                const std::array<double, 4> xs = {sx0, sx1, sx0, sx1};
                const std::array<double, 4> ys = {sy0, sy0, sy1, sy1};
                for (std::size_t c = 0; c < 4U; ++c) {
                    double hx = 0.0;
                    double hy = 0.0;
                    half_slope(parameters, xs[c], ys[c], hx, hy);
                    const int bx = static_cast<int>(std::floor(hx / st.bin_width - 0.5));
                    const int by = static_cast<int>(std::floor(hy / st.bin_width - 0.5));
                    low_x = std::min(low_x, bx);
                    high_x = std::max(high_x, bx);
                    low_y = std::min(low_y, by);
                    high_y = std::max(high_y, by);
                }
                tile.bin_x0 = low_x - 1;
                tile.bin_y0 = low_y - 1;
                tile.bin_width = std::min(tile_bins, high_x + 2 - tile.bin_x0 + 1);
                tile.bin_height = std::min(tile_bins, high_y + 2 - tile.bin_y0 + 1);
                const std::size_t used = static_cast<std::size_t>(tile.point_width) * static_cast<std::size_t>(tile.point_height) *
                                         static_cast<std::size_t>(tile.bin_width) * static_cast<std::size_t>(tile.bin_height);
                std::fill(scratch.counts.begin(), scratch.counts.begin() + static_cast<std::ptrdiff_t>(used), -1);
                std::fill(scratch.jitter.begin(),
                          scratch.jitter.begin() + static_cast<std::ptrdiff_t>(static_cast<std::size_t>(tile.cell_width) * static_cast<std::size_t>(tile.cell_height) * 4U),
                          std::numeric_limits<double>::quiet_NaN());
                std::fill(scratch.lambdas.begin(), scratch.lambdas.begin() + static_cast<std::ptrdiff_t>(tile.bin_width * tile.bin_height),
                          std::numeric_limits<double>::quiet_NaN());
                st.tile = &tile;
            }
            for (int y = ty; y < ty1; ++y) {
                std::byte* pixel = target.pixels + static_cast<std::size_t>(y) * target.row_bytes + static_cast<std::size_t>(tx) * 4U;
                for (int x = tx; x < tx1; ++x, pixel += 4) {
                    shade_glint(st, static_cast<double>(x - surface.x), static_cast<double>(y - surface.y), pixel);
                }
            }
        }
    }
    return static_cast<std::size_t>(area.width) * static_cast<std::size_t>(area.height);
}

}  // namespace industrial_surfaces
