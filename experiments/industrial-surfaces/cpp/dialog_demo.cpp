// A demonstration dialog composed from the pieces of this experiment: the
// glint kernel paints the window (GOST 22133-86 colours over sand-blasted
// metal), cached lamp renders (lamps.cpp) supply the keys and indicator
// jewels, and a stroke font sets the labels.
//
// Usage: dialog_demo <lamp directory> <output .bmp> <idle|pressed>
// The lamp directory holds dialog_{cancel,ok}_{up,down}.f32 and
// indicator_backed_up.f32 from `lamps <dir> 280 dialog` and `lamps <dir> 112 indicator`.

#include "surface_kernels.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace demo {

namespace is = industrial_surfaces;

constexpr int width = 1000;
constexpr int height = 620;
constexpr double scale = 2.0;  // device pixels per logical unit

// ---- pixels ----------------------------------------------------------------------------

struct Linear final {
    double red{};
    double green{};
    double blue{};
};

double to_linear(std::uint8_t v) {
    const double c = v / 255.0;
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

std::uint8_t to_byte(double v) {
    const double c = std::clamp(v, 0.0, 1.0);
    const double s = c <= 0.0031308 ? c * 12.92 : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
    return static_cast<std::uint8_t>(std::lround(s * 255.0));
}

Linear linear_of(is::Rgb8 c) { return Linear{to_linear(c.red), to_linear(c.green), to_linear(c.blue)}; }

// The window's pixels: BGRA8 as the kernels write them, read and blended in
// linear light.
struct Canvas final {
    std::vector<std::byte> pixels;
};

Linear read(const Canvas& canvas, int x, int y) {
    const std::size_t at = (static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)) * 4U;
    return Linear{to_linear(std::to_integer<std::uint8_t>(canvas.pixels[at + 2U])),
                  to_linear(std::to_integer<std::uint8_t>(canvas.pixels[at + 1U])),
                  to_linear(std::to_integer<std::uint8_t>(canvas.pixels[at]))};
}

void write(Canvas& canvas, int x, int y, Linear c) {
    const std::size_t at = (static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)) * 4U;
    canvas.pixels[at] = static_cast<std::byte>(to_byte(c.blue));
    canvas.pixels[at + 1U] = static_cast<std::byte>(to_byte(c.green));
    canvas.pixels[at + 2U] = static_cast<std::byte>(to_byte(c.red));
    canvas.pixels[at + 3U] = static_cast<std::byte>(255U);
}

Linear mix(Linear a, Linear b, double t) {
    return Linear{a.red + (b.red - a.red) * t, a.green + (b.green - a.green) * t, a.blue + (b.blue - a.blue) * t};
}

is::FramebufferView view_of(Canvas& canvas) {
    return is::FramebufferView{canvas.pixels.data(), static_cast<std::size_t>(width) * 4U, width, height};
}

// ---- stroke font (the same angular letters as lamps.cpp) --------------------------------

struct Glyph final {
    char letter;
    const char* strokes;
};

constexpr std::array<Glyph, 26> glyphs = {
    Glyph{'A', "0,0 0,4.5 1.5,6 2.5,6 4,4.5 4,0|0,3 4,3"},
    Glyph{'B', "0,0 0,6 3,6 4,5 4,4 3,3 0,3|3,3 4,2 4,1 3,0 0,0"},
    Glyph{'C', "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1"},
    Glyph{'D', "0,0 0,6 3,6 4,5 4,1 3,0 0,0"},
    Glyph{'E', "4,6 0,6 0,0 4,0|0,3 3,3"},
    Glyph{'F', "4,6 0,6 0,0|0,3 3,3"},
    Glyph{'G', "4,5 3,6 1,6 0,5 0,1 1,0 3,0 4,1 4,3 2.5,3"},
    Glyph{'H', "0,0 0,6|4,0 4,6|0,3 4,3"},
    Glyph{'I', "2,0 2,6|1,6 3,6|1,0 3,0"},
    Glyph{'K', "0,0 0,6|4,6 0,2|1.3,3.3 4,0"},
    Glyph{'L', "0,6 0,0 4,0"},
    Glyph{'M', "0,0 0,6 2,3 4,6 4,0"},
    Glyph{'N', "0,0 0,6 4,0 4,6"},
    Glyph{'O', "1,0 0,1 0,5 1,6 3,6 4,5 4,1 3,0 1,0"},
    Glyph{'P', "0,0 0,6 3,6 4,5 4,4 3,3 0,3"},
    Glyph{'R', "0,0 0,6 3,6 4,5 4,4 3,3 0,3|2,3 4,0"},
    Glyph{'S', "4,5 3,6 1,6 0,5 0,4 1,3 3,3 4,2 4,1 3,0 1,0 0,1"},
    Glyph{'T', "0,6 4,6|2,6 2,0"},
    Glyph{'U', "0,6 0,1 1,0 3,0 4,1 4,6"},
    Glyph{'V', "0,6 2,0 4,6"},
    Glyph{'W', "0,6 1,0 2,3.5 3,0 4,6"},
    Glyph{'Y', "0,6 2,3 4,6|2,3 2,0"},
    Glyph{'-', "1,3 3,3"},
    Glyph{'.', "1.8,0 2.2,0"},
    Glyph{'/', "0,0 4,6"},
    Glyph{':', "2,1 2,1.2|2,4.8 2,5"},
};

struct Segment final {
    double ax{};
    double ay{};
    double bx{};
    double by{};
};

// Lay out a line of text with its baseline at (x, baseline), letters `size`
// device pixels tall; image y grows downwards.
std::vector<Segment> set_text(const std::string& text, double x, double baseline, double size) {
    std::vector<Segment> out;
    const double unit = size / 6.0;
    double pen = x;
    for (const char letter : text) {
        const Glyph* found = nullptr;
        for (const Glyph& g : glyphs) {
            if (g.letter == letter) {
                found = &g;
            }
        }
        if (found != nullptr) {
            const char* cursor = (*found).strokes;
            bool have_previous = false;
            double px = 0.0;
            double py = 0.0;
            while (*cursor != '\0') {
                if (*cursor == '|') {
                    have_previous = false;
                    ++cursor;
                    continue;
                }
                char* end = nullptr;
                const double gx = std::strtod(cursor, &end);
                cursor = end + 1;
                const double gy = std::strtod(cursor, &end);
                cursor = end;
                while (*cursor == ' ') {
                    ++cursor;
                }
                const double sx = pen + gx * unit;
                const double sy = baseline - gy * unit;
                if (have_previous) {
                    out.push_back(Segment{px, py, sx, sy});
                }
                px = sx;
                py = sy;
                have_previous = true;
            }
        }
        pen += 5.5 * unit;
    }
    return out;
}

double segment_distance(const Segment& s, double x, double y) {
    const double vx = s.bx - s.ax;
    const double vy = s.by - s.ay;
    const double wx = x - s.ax;
    const double wy = y - s.ay;
    const double t = std::clamp((wx * vx + wy * vy) / std::max(1e-12, vx * vx + vy * vy), 0.0, 1.0);
    return std::hypot(wx - vx * t, wy - vy * t);
}

// Paint stroked text: ink of `colour`, strokes `half_width` device pixels
// either side of the centre line, antialiased over one pixel.
void paint_text(Canvas& canvas, const std::vector<Segment>& strokes, double half_width, Linear colour, double opacity) {
    if (strokes.empty()) {
        return;
    }
    double x0 = 1e9;
    double y0 = 1e9;
    double x1 = -1e9;
    double y1 = -1e9;
    for (const Segment& s : strokes) {
        x0 = std::min({x0, s.ax, s.bx});
        y0 = std::min({y0, s.ay, s.by});
        x1 = std::max({x1, s.ax, s.bx});
        y1 = std::max({y1, s.ay, s.by});
    }
    const int px0 = std::max(0, static_cast<int>(x0 - half_width - 2.0));
    const int py0 = std::max(0, static_cast<int>(y0 - half_width - 2.0));
    const int px1 = std::min(width - 1, static_cast<int>(x1 + half_width + 2.0));
    const int py1 = std::min(height - 1, static_cast<int>(y1 + half_width + 2.0));
    for (int y = py0; y <= py1; ++y) {
        for (int x = px0; x <= px1; ++x) {
            double d = 1e9;
            for (const Segment& s : strokes) {
                d = std::min(d, segment_distance(s, x + 0.5, y + 0.5));
            }
            const double cover = std::clamp(half_width + 0.5 - d, 0.0, 1.0) * opacity;
            if (cover > 0.0) {
                write(canvas, x, y, mix(read(canvas, x, y), colour, cover));
            }
        }
    }
}

// ---- lamp caches ------------------------------------------------------------------------

// A cached lamp render (lamps.cpp): base, one plane per lamp, coverage;
// colour planes premultiplied by coverage.
struct LampCache final {
    int width{};
    int height{};
    int lamps{};
    std::vector<float> base;
    std::vector<std::vector<float>> glow;
    std::vector<float> alpha;
};

bool load_cache(const std::string& path, LampCache& cache) {
    std::FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr) {
        return false;
    }
    std::int32_t header[3] = {0, 0, 0};
    bool ok = std::fread(header, sizeof(header), 1U, file) == 1U;
    if (ok) {
        cache.width = header[0];
        cache.height = header[1];
        cache.lamps = header[2];
        const std::size_t n = static_cast<std::size_t>(cache.width) * static_cast<std::size_t>(cache.height);
        cache.base.resize(n * 3U);
        ok = std::fread(cache.base.data(), sizeof(float), n * 3U, file) == n * 3U;
        cache.glow.assign(static_cast<std::size_t>(cache.lamps), std::vector<float>(n * 3U));
        for (std::vector<float>& g : cache.glow) {
            ok = ok && std::fread(g.data(), sizeof(float), n * 3U, file) == n * 3U;
        }
        cache.alpha.resize(n);
        ok = ok && std::fread(cache.alpha.data(), sizeof(float), n, file) == n;
    }
    std::fclose(file);
    return ok;
}

// Engine renders are toned as the engine tones them (1 − e^(−0.75 L), whose
// gamma-2.2 encoding the engine writes); the indicator jewels keep the curve
// they were tuned with.
double tone(double v, double exposure) { return 1.0 - std::exp(-v * exposure); }

// The lamp's light at a cache pixel, unpremultiplied and toned, with all of
// its lamps at `power` in `colour`, seen through a coloured filter on the
// face: the room's light crosses the filter twice (in and out), the lamp's
// once.
Linear lamp_light(const LampCache& c, std::size_t at, Linear colour, double power, Linear filter, double exposure) {
    double r = c.base[at * 3U] * filter.red * filter.red;
    double g = c.base[at * 3U + 1U] * filter.green * filter.green;
    double b = c.base[at * 3U + 2U] * filter.blue * filter.blue;
    for (const std::vector<float>& glow : c.glow) {
        r += power * colour.red * filter.red * glow[at * 3U];
        g += power * colour.green * filter.green * glow[at * 3U + 1U];
        b += power * colour.blue * filter.blue * glow[at * 3U + 2U];
    }
    const double a = c.alpha[at];
    const double inv = a > 1e-6 ? 1.0 / a : 0.0;
    return Linear{tone(r * inv, exposure), tone(g * inv, exposure), tone(b * inv, exposure)};
}

// A key: its own pixels only, with a soft contact shadow the dialog draws
// beneath it (the key itself never draws outside its edge).
void place_key(Canvas& canvas, const LampCache& c, int left, int top, Linear colour, double power, Linear filter) {
    const int blur = 7;
    const int dx = 3;
    const int dy = 7;
    for (int y = top - blur; y < top + c.height + blur + dy; ++y) {
        for (int x = left - blur; x < left + c.width + blur + dx; ++x) {
            if (x < 0 || y < 0 || x >= width || y >= height) {
                continue;
            }
            double shadow = 0.0;
            int samples = 0;
            for (int j = -blur; j <= blur; j += 2) {
                for (int i = -blur; i <= blur; i += 2) {
                    const int sx = x - left - dx + i;
                    const int sy = y - top - dy + j;
                    ++samples;
                    if (sx >= 0 && sy >= 0 && sx < c.width && sy < c.height) {
                        shadow += c.alpha[static_cast<std::size_t>(sy) * static_cast<std::size_t>(c.width) + static_cast<std::size_t>(sx)];
                    }
                }
            }
            shadow /= samples;
            if (shadow > 0.0) {
                const Linear under = read(canvas, x, y);
                write(canvas, x, y, mix(under, Linear{0.0, 0.0, 0.0}, 0.6 * shadow));
            }
        }
    }
    for (int y = 0; y < c.height; ++y) {
        for (int x = 0; x < c.width; ++x) {
            const std::size_t at = static_cast<std::size_t>(y) * static_cast<std::size_t>(c.width) + static_cast<std::size_t>(x);
            const double a = c.alpha[at];
            const int cx = left + x;
            const int cy = top + y;
            if (a <= 0.0 || cx < 0 || cy < 0 || cx >= width || cy >= height) {
                continue;
            }
            write(canvas, cx, cy, mix(read(canvas, cx, cy), lamp_light(c, at, colour, power, filter, 0.75), a));
        }
    }
}

// An indicator: the jewel's crown seen through a dot `radius` pixels across,
// sampled from the cached jewel (3 × 3 per pixel), with a small halo and a
// four-point flare around it.
void place_indicator(Canvas& canvas, const LampCache& c, double cx, double cy, double radius, Linear colour, double power) {
    // the cached hole is 0.19 units across a frame of ±2.6·tan(9.5°)
    const double hole_px = 0.19 / (2.6 * std::tan(9.5 * 3.141592653589793 / 180.0)) * (c.width / 2.0);
    const double step = hole_px / radius;
    const double reach = radius * 4.0;
    for (int y = static_cast<int>(cy - reach); y <= static_cast<int>(cy + reach); ++y) {
        for (int x = static_cast<int>(cx - reach); x <= static_cast<int>(cx + reach); ++x) {
            if (x < 0 || y < 0 || x >= width || y >= height) {
                continue;
            }
            const double ox = x + 0.5 - cx;
            const double oy = y + 0.5 - cy;
            const double d = std::hypot(ox, oy);
            Linear here = read(canvas, x, y);
            // the dark rim of the hole
            const double rim = std::clamp(radius + 2.0 - d, 0.0, 1.0) - std::clamp(radius - d, 0.0, 1.0);
            here = mix(here, Linear{0.004, 0.004, 0.004}, 0.8 * rim);
            const double inside = std::clamp(radius + 0.5 - d, 0.0, 1.0);
            if (inside > 0.0) {
                Linear sum{};
                for (int j = 0; j < 3; ++j) {
                    for (int i = 0; i < 3; ++i) {
                        const double sx = c.width / 2.0 + (ox - 0.5 + (i + 0.5) / 3.0) * step;
                        const double sy = c.height / 2.0 + (oy - 0.5 + (j + 0.5) / 3.0) * step;
                        const int ix = std::clamp(static_cast<int>(sx), 0, c.width - 1);
                        const int iy = std::clamp(static_cast<int>(sy), 0, c.height - 1);
                        const Linear l = lamp_light(c, static_cast<std::size_t>(iy) * static_cast<std::size_t>(c.width) + static_cast<std::size_t>(ix), colour, power, Linear{1.0, 1.0, 1.0}, 1.3);
                        sum.red += l.red / 9.0;
                        sum.green += l.green / 9.0;
                        sum.blue += l.blue / 9.0;
                    }
                }
                here = mix(here, sum, inside);
            }
            // light flaring past the dot: a halo and a four-point star
            const double beyond = std::max(0.0, d - radius * 0.6);
            const double halo = 0.22 * std::exp(-beyond * beyond / (2.0 * radius * radius * 0.6));
            const double star = 0.10 * (std::exp(-std::abs(ox) / (radius * 1.6)) * std::exp(-oy * oy / 1.2) +
                                        std::exp(-std::abs(oy) / (radius * 1.6)) * std::exp(-ox * ox / 1.2));
            const double add = power * (halo + star);
            here.red += add * colour.red;
            here.green += add * colour.green;
            here.blue += add * colour.blue;
            write(canvas, x, y, here);
        }
    }
}

bool write_bmp(const std::string& path, const Canvas& canvas) {
    const int row = (width * 3 + 3) & ~3;
    const std::uint32_t data_size = static_cast<std::uint32_t>(row * height);
    std::array<std::uint8_t, 54> head{};
    head[0] = 'B';
    head[1] = 'M';
    const std::uint32_t file_size = 54U + data_size;
    for (int i = 0; i < 4; ++i) {
        head[static_cast<std::size_t>(2 + i)] = static_cast<std::uint8_t>(file_size >> (8 * i));
        head[static_cast<std::size_t>(18 + i)] = static_cast<std::uint8_t>(static_cast<std::uint32_t>(width) >> (8 * i));
        head[static_cast<std::size_t>(22 + i)] = static_cast<std::uint8_t>(static_cast<std::uint32_t>(height) >> (8 * i));
        head[static_cast<std::size_t>(34 + i)] = static_cast<std::uint8_t>(data_size >> (8 * i));
    }
    head[10] = 54;
    head[14] = 40;
    head[26] = 1;
    head[28] = 24;
    std::vector<std::uint8_t> out(data_size, 0U);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const std::size_t from = (static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)) * 4U;
            const std::size_t to = static_cast<std::size_t>(height - 1 - y) * static_cast<std::size_t>(row) + static_cast<std::size_t>(x) * 3U;
            out[to] = std::to_integer<std::uint8_t>(canvas.pixels[from]);
            out[to + 1U] = std::to_integer<std::uint8_t>(canvas.pixels[from + 1U]);
            out[to + 2U] = std::to_integer<std::uint8_t>(canvas.pixels[from + 2U]);
        }
    }
    std::FILE* file = std::fopen(path.c_str(), "wb");
    if (file == nullptr) {
        return false;
    }
    bool ok = std::fwrite(head.data(), 1U, head.size(), file) == head.size();
    ok = ok && std::fwrite(out.data(), 1U, out.size(), file) == out.size();
    return std::fclose(file) == 0 && ok;
}

}  // namespace demo

int main(int argc, char** argv) {
    namespace is = industrial_surfaces;
    if (argc < 4) {
        std::fprintf(stderr, "usage: dialog_demo <lamp directory> <output .bmp> <idle|pressed>\n");
        return 2;
    }
    const std::string lamps = argv[1];
    const bool pressed = std::string(argv[3]) == "pressed";
    demo::LampCache cancel{};
    demo::LampCache ok{};
    demo::LampCache jewel{};
    if (!demo::load_cache(lamps + "/dialog_cancel_up.f32", cancel) ||
        !demo::load_cache(lamps + (pressed ? "/dialog_ok_down.f32" : "/dialog_ok_up.f32"), ok) ||
        !demo::load_cache(lamps + "/indicator_backed_up.f32", jewel)) {
        std::fprintf(stderr, "missing lamp caches in %s\n", lamps.c_str());
        return 1;
    }
    demo::Canvas canvas{};
    canvas.pixels.assign(static_cast<std::size_t>(demo::width) * demo::height * 4U, std::byte{0});
    const is::FramebufferView view = demo::view_of(canvas);

    // The window body: sand-blasted metal under a thin coat of ГОСТ 22133-86
    // сине-зелёный 397, lit from the upper left.
    is::GlintParameters body = is::glint_preset(is::GlintPreset::painted_sand);
    body.seed = 11;
    body.scale = demo::scale;
    body.paint = is::Rgb8{0x4d, 0x67, 0x6d};
    body.light_x = 0.2 * demo::width / demo::scale;
    body.light_y = 0.05 * demo::height / demo::scale;
    body.light_z = 900.0;
    body.eye_x = 0.5 * demo::width / demo::scale;
    body.eye_y = 0.5 * demo::height / demo::scale;
    is::paint_glints(view, is::PixelRect{0, 0, demo::width, demo::height}, is::PixelRect{0, 78, demo::width, demo::height - 78}, body);
    // The title band: тёмно-серый 832, a heavier coat.
    is::GlintParameters band = body;
    band.paint = is::Rgb8{0x65, 0x6f, 0x74};
    band.film = 0.85;
    is::paint_glints(view, is::PixelRect{0, 0, demo::width, demo::height}, is::PixelRect{0, 0, demo::width, 78}, band);
    // a groove under the band, and a lit lip below it
    for (int x = 0; x < demo::width; ++x) {
        demo::write(canvas, x, 76, demo::mix(demo::read(canvas, x, 76), demo::Linear{0.0, 0.0, 0.0}, 0.7));
        demo::write(canvas, x, 77, demo::mix(demo::read(canvas, x, 77), demo::Linear{0.0, 0.0, 0.0}, 0.45));
        demo::write(canvas, x, 78, demo::mix(demo::read(canvas, x, 78), demo::Linear{1.0, 1.0, 1.0}, 0.12));
    }
    // a raised border round the window
    for (int y = 0; y < demo::height; ++y) {
        for (int x = 0; x < demo::width; ++x) {
            const int edge = std::min({x, y, demo::width - 1 - x, demo::height - 1 - y});
            if (edge < 3) {
                const bool lit = (x < 3 || y < 3) && x < demo::width - 3 && y < demo::height - 3;
                demo::write(canvas, x, y, demo::mix(demo::read(canvas, x, y), lit ? demo::Linear{1, 1, 1} : demo::Linear{0, 0, 0}, lit ? 0.18 : 0.5));
            }
        }
    }

    // Lettering in слоновая кость 224, as stencilled paint.
    const demo::Linear ivory = demo::linear_of(is::Rgb8{0xcc, 0xc7, 0xac});
    demo::paint_text(canvas, demo::set_text("PROCESS CONTROL", 34.0, 52.0, 26.0), 1.7, ivory, 0.95);
    demo::paint_text(canvas, demo::set_text("START TRANSFER", 48.0, 160.0, 36.0), 2.4, ivory, 1.0);
    demo::paint_text(canvas, demo::set_text("PUMP TO RESERVE TANK", 48.0, 205.0, 20.0), 1.5, ivory, 0.85);

    // Status: three indicator jewels with labels.
    const std::array<const char*, 3> labels = {"PUMP READY", "HEATING", "FAULT"};
    const std::array<demo::Linear, 3> colours = {demo::Linear{0.16, 1.0, 0.25}, demo::Linear{1.0, 0.50, 0.08}, demo::Linear{1.0, 0.12, 0.04}};
    for (std::size_t i = 0; i < 3U; ++i) {
        const double y = 290.0 + 58.0 * static_cast<double>(i);
        demo::place_indicator(canvas, jewel, 72.0, y, 12.0, colours[i], 1.6);
        demo::paint_text(canvas, demo::set_text(labels[i], 104.0, y + 9.0, 18.0), 1.4, ivory, 0.9);
    }

    // Keys: CANCEL lit red; OK green, lit only while it is held down.
    const int keys_top = demo::height - cancel.height - 36;
    // Keys rendered by the bfft engine (key scene): their colours are in the
    // render (CANCEL's pigmented diffuser, OK's tinted acrylic), lit by white
    // lamps at the power the engine rendered; OK is lit only while pressed.
    const demo::Linear white{1.0, 1.0, 1.0};
    demo::place_key(canvas, cancel, demo::width - 2 * ok.width - 70, keys_top, white, 1.0, white);
    demo::place_key(canvas, ok, demo::width - ok.width - 40, keys_top, white, pressed ? 1.0 : 0.0, white);

    if (!demo::write_bmp(argv[2], canvas)) {
        std::fprintf(stderr, "could not write %s\n", argv[2]);
        return 1;
    }
    return 0;
}
