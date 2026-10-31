// SCRATCH (AAA column seams): primitives drawn whole, and again through two clip rectangles
// that split the surface at column c; counts pixels that differ, per primitive, over all c.
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradient.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <vector>
extern bool gSkForceRasterPipelineBlitter;
namespace {
constexpr int W = 160, H = 60;
void draw(SkCanvas& canvas, int kind) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF3366CCU);
    const SkRRect rr = SkRRect::MakeRectXY(SkRect::MakeXYWH(10.5F, 8.25F, 137.0F, 41.5F), 9.0F, 9.0F);
    if (kind == 0) canvas.drawRRect(rr, paint);
    if (kind == 1) { paint.setStyle(SkPaint::kStroke_Style); paint.setStrokeWidth(2.0F); canvas.drawRRect(rr, paint); }
    if (kind == 2) { SkPathBuilder p; p.moveTo(5, 50); p.cubicTo(40, -10, 120, 70, 155, 10); p.lineTo(155, 55); p.close(); canvas.drawPath(p.detach(), paint); }
    if (kind == 3) {
        const SkPoint pts[2] = {{0, 0}, {W, 0}};
        const SkColor4f colors[2] = {SkColor4f::FromColor(0xFF203040U), SkColor4f::FromColor(0xFFE0D0C0U)};
        paint.setAntiAlias(false);
        paint.setShader(SkShaders::LinearGradient(pts, SkGradient(SkGradient::Colors(SkSpan<const SkColor4f>(colors, 2), SkTileMode::kClamp), {})).shader);
        canvas.drawRect(SkRect::MakeWH(W, H), paint);
    }
    if (kind == 4) { paint.setStrokeWidth(2.0F); canvas.drawLine(3.5F, 5.25F, 150.25F, 52.5F, paint); }
}
void render(std::vector<std::uint32_t>& pixels, int kind, int cut) {
    std::fill(pixels.begin(), pixels.end(), 0xFFF0F0F0U);
    const SkImageInfo info = SkImageInfo::Make(W, H, kBGRA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, pixels.data(), W * 4U);
    SkCanvas& canvas = *(*surface).getCanvas();
    if (cut < 0) { draw(canvas, kind); return; }
    canvas.save(); canvas.clipRect(SkRect::MakeLTRB(0, 0, cut, H)); draw(canvas, kind); canvas.restore();
    canvas.save(); canvas.clipRect(SkRect::MakeLTRB(cut, 0, W, H)); draw(canvas, kind); canvas.restore();
}
}  // namespace
int main() {
    const char* names[5] = {"rrect fill", "rrect stroke", "cubic fill", "linear gradient", "2px line"};
    std::vector<std::uint32_t> whole(W * H), split(W * H);
    for (int pipeline = 0; pipeline < 2; ++pipeline) {
        gSkForceRasterPipelineBlitter = pipeline == 1;
        for (int kind = 0; kind < 5; ++kind) {
            render(whole, kind, -1);
            int px = 0, cuts = 0, delta = 0;
            for (int cut = 1; cut < W; ++cut) {
                render(split, kind, cut);
                int here = 0;
                for (int i = 0; i < W * H; ++i) if (whole[i] != split[i]) {
                    ++here;
                    for (int b = 0; b < 32; b += 8) { const int d = std::abs(int((whole[i] >> b) & 255) - int((split[i] >> b) & 255)); if (d > delta) delta = d; }
                }
                px += here; cuts += here > 0;
            }
            std::printf("pipeline=%d %-16s cuts_with_seam=%3d/159 pixels=%5d max_delta=%d\n", pipeline, names[kind], cuts, px, delta);
        }
    }
    return 0;
}
