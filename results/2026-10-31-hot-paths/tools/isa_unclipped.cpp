// SCRATCH (AAA clip invariance): the isa workload's paths case drawn at 320x200, and again
// into a 360x260 surface translated by (20, 30) so no path meets the surface edge; the
// interior crop of the second must equal the first if the fill is clip-invariant.
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkSurface.h"
#include <cstdint>
#include <cstdio>
#include <vector>
extern bool gSkForceRasterPipelineBlitter;
namespace {
SkColor panel_colour(int index, std::uint8_t alpha) {
    const std::uint32_t mix = static_cast<std::uint32_t>(index) * 0x9E3779B9U;
    return (static_cast<std::uint32_t>(alpha) << 24) | (mix & 0x00FFFFFFU);
}
void draw_paths(SkCanvas& canvas, float fw, float fh) {
    SkPaint paint;
    paint.setAntiAlias(true);
    for (int i = 0; i < 12; ++i) {
        const float x = fw * static_cast<float>(i % 4) / 4.0F;
        const float y = fh * static_cast<float>(i / 4) / 3.0F;
        const float w = fw / 4.0F;
        const float h = fh / 3.0F;
        SkPathBuilder path;
        path.moveTo(x + w * 0.1F, y + h * 0.8F);
        path.cubicTo(x + w * 0.2F, y - h * 0.1F, x + w * 0.9F, y + h * 0.1F, x + w * 0.85F, y + h * 0.7F);
        path.quadTo(x + w * 0.5F, y + h * 1.05F, x + w * 0.1F, y + h * 0.8F);
        path.close();
        path.addOval(SkRect::MakeXYWH(x + w * 0.3F, y + h * 0.3F, w * 0.35F, h * 0.35F));
        paint.setColor(panel_colour(i + 3, i % 2 == 0 ? 0xFF : 0x99));
        canvas.drawPath(path.detach(), paint);
    }
}
std::vector<std::uint32_t> render(int w, int h, float dx, float dy) {
    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(w * h));
    const SkImageInfo info = SkImageInfo::Make(w, h, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, pixels.data(), static_cast<std::size_t>(w) * 4U);
    SkCanvas& canvas = *(*surface).getCanvas();
    canvas.clear(0xFFF0F0F0U);
    canvas.translate(dx, dy);
    draw_paths(canvas, 320.0F, 200.0F);
    return pixels;
}
}  // namespace
int main() {
    int failures = 0;
    for (int pipeline = 0; pipeline < 2; ++pipeline) {
        gSkForceRasterPipelineBlitter = pipeline == 1;
        const std::vector<std::uint32_t> small = render(320, 200, 0.0F, 0.0F);
        const std::vector<std::uint32_t> large = render(360, 260, 20.0F, 30.0F);
        int differ = 0;
        for (int y = 0; y < 200; ++y)
            for (int x = 0; x < 320; ++x)
                if (small[static_cast<std::size_t>(y * 320 + x)] != large[static_cast<std::size_t>((y + 30) * 360 + x + 20)]) ++differ;
        std::printf("pipeline=%d clipped-vs-unclipped differing pixels: %d\n", pipeline, differ);
        failures += differ;
    }
    return failures == 0 ? 0 : 1;
}
