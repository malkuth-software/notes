'use strict';
// Blasted-metal glints, one pixel at a time.
//
// Blasting turns a metal surface into a field of tiny flat facets with random
// tilts. A pixel shows a few or many of them. The pixel is bright when one of
// its facets faces the half-vector between the light and the eye, so what the
// eye sees is not relief but glint statistics:
//
//   - the average over many facets is a smooth specular lobe (Beckmann, slope
//     spread alpha) that sweeps across the surface with the light and view;
//   - with few facets per pixel (sand blast) the lobe breaks into sparse,
//     heavy-tailed specks; with many (bead blast) it averages to a satin
//     sheen with faint grain.
//
// Two regimes, blended where a facet is about one pixel wide:
//   facets smaller than a pixel   the count of facets that glint is drawn
//     from a Poisson distribution whose mean is facets × lobe density ×
//     facet tolerance, with a hash of (pixel, half-vector bin) as its
//     uniform; the bins are blended bilinearly, so moving the light makes
//     glints come and go (twinkle) instead of swelling in place;
//   facets larger than a pixel    each facet is a jittered cell with its own
//     hashed tilt, and the pixel takes that facet's mirror response.
//
// Everything is a function of the pixel's surface position, the scale and
// the parameters: no texture, no scratch rows, no neighbouring pixels.

// glint_fast.js: glint_kernel.js with the specks' work retained per tile.
// A speck point's facet count in a half-vector bin, its hue and its jittered
// position do not depend on the pixel asking; the original recomputes them for
// every pixel in reach (about nine times each). Here the surface is painted in
// 32 × 32 tiles; within a tile each (point, bin) is computed on first use and
// kept, and each cell's jitter once. Pixels combine them in the original order,
// so the output is byte-identical.

const Glint = (() => {
  const hash = Finish.hash;

  function poisson(lambda, u, u2) {
    if (lambda <= 0) return 0;
    if (lambda > 24) {
      // normal approximation, second uniform for the angle
      const z = Math.sqrt(-2 * Math.log(Math.max(u, 1e-12))) * Math.cos(6.283185307 * u2);
      const n = Math.round(lambda + Math.sqrt(lambda) * z);
      return n > 0 ? n : 0;
    }
    let p = Math.exp(-lambda), F = p, n = 0;
    while (u > F && n < 64) { n++; p *= lambda / n; F += p; }
    return n;
  }

  function hsvToRgb(h, out) {
    const k = (n) => (n + h * 6) % 6;
    const f = (n) => 1 - Math.max(0, Math.min(k(n), 4 - k(n), 1));
    out[0] = f(5); out[1] = f(3); out[2] = f(1);
  }

  const rainbow = [0, 0, 0];
  const result = [0, 0, 0];

  // Prepared once per paint: everything that does not change per pixel.
  function prepare(p) {
    const sigma = p.alpha;
    return {
      p,
      sigma2: sigma * sigma,
      tau: p.tau,
      tauArea: Math.PI * p.tau * p.tau,
      binW: p.tau * 2,
      metal: Finish.linearOf(p.metalColour),
      paint: Finish.linearOf(p.paintColour || '#808080'),
      film: p.film || 0,
    };
  }

  // Facet density estimate at half-vector slope (hx, hy) for the pixel at
  // device (x, y); also leaves the glint's hue seed in `glintHue`.
  let glintHue = 0;
  // One point's facet count in one bin, and its hue hash (only used when the
  // count is positive): the original's inner step.
  function binCount(st, xi, yi, facets, cx, cy) {
    const w = st.binW, seed = st.p.seed | 0;
    const sx = (cx + 0.5) * w, sy = (cy + 0.5) * w;
    const pdf = Math.exp(-(sx * sx + sy * sy) / st.sigma2) / (Math.PI * st.sigma2);
    const lambda = facets * pdf * st.tauArea;
    const key = Math.imul(cx, 0x632be5ab) ^ Math.imul(cy, 0x85157af5);
    const u = hash(xi ^ key, yi + key, seed);
    const u2 = hash(xi + key, yi ^ key, seed + 1);
    binN = poisson(lambda, u, u2);
    binHue = binN > 0 ? hash(xi, yi, key + seed + 2) : 0;
  }
  let binN = 0, binHue = 0;

  // The tile's retained (point, bin) results: counts (−1: not yet computed)
  // and hues, for points with x in [px0, px0 + pw), y in [py0, py0 + ph) and
  // bins in [bx0, bx0 + bw) × [by0, by0 + bh).
  function lookup(st, xi, yi, facets, cx, cy) {
    const c = st.cache;
    if (c && facets === c.facets) {
      const a = xi - c.px0, b = yi - c.py0, u = cx - c.bx0, v = cy - c.by0;
      if (a >= 0 && b >= 0 && u >= 0 && v >= 0 && a < c.pw && b < c.ph && u < c.bw && v < c.bh) {
        const k = ((b * c.pw + a) * c.bh + v) * c.bw + u;
        if (c.n[k] < 0) { binCount(st, xi, yi, facets, cx, cy); c.n[k] = binN; c.hue[k] = binHue; }
        binN = c.n[k]; binHue = c.hue[k];
        return;
      }
    }
    binCount(st, xi, yi, facets, cx, cy);
  }

  function subPixelDensity(st, x, y, facets, hx, hy) {
    const w = st.binW;
    const bx = hx / w - 0.5, by = hy / w - 0.5;
    const ix = Math.floor(bx), iy = Math.floor(by);
    const fx = bx - ix, fy = by - iy;
    let acc = 0, hueAcc = 0, wAcc = 0;
    const xi = x | 0, yi = y | 0;
    for (let j = 0; j < 2; j++) {
      for (let i = 0; i < 2; i++) {
        lookup(st, xi, yi, facets, ix + i, iy + j);
        const n = binN;
        const wt = (i ? fx : 1 - fx) * (j ? fy : 1 - fy);
        acc += wt * n;
        if (n > 0) { hueAcc += wt * binHue; wAcc += wt; }
      }
    }
    glintHue = wAcc > 0 ? hueAcc / wAcc : 0;
    return acc / (facets * st.tauArea);
  }

  // Real specks are about one and a half pixels wide, not single pixels. A
  // square lattice blended with smoothstep shows its rows and columns, so the
  // specks are jittered points instead: two per cell of `speck` device
  // pixels, each drawing its own glint count for the facets under it, blended
  // by a radial weight (1 − d²/R²)² normalised over the points in reach. The
  // nearest point stands in if none is within reach.
  function speckDensity(st, x, y, speck, facetPx, hx, hy) {
    const perPixel = 1 / (facetPx * facetPx);
    if (speck <= 1.01) return subPixelDensity(st, x, y, perPixel, hx, hy);
    const gx = x / speck, gy = y / speck;
    const ix = Math.floor(gx), iy = Math.floor(gy);
    const facets = perPixel * speck * speck * 0.5;
    const seed = st.p.seed | 0;
    const R2 = 1.0;
    let acc = 0, wsum = 0, hue = 0, nearest = 9, nx = 0, ny = 0;
    for (let j = -1; j <= 1; j++) {
      for (let i = -1; i <= 1; i++) {
        const cx = ix + i, cy = iy + j;
        for (let d = 0; d < 2; d++) {
          let px, py;
          const c = st.cache;
          const a = cx - (c ? c.cx0 : 0), b = cy - (c ? c.cy0 : 0);
          if (c && a >= 0 && b >= 0 && a < c.cw && b < c.ch) {
            const k = ((b * c.cw + a) * 2 + d) * 2;
            if (Number.isNaN(c.jitter[k])) { c.jitter[k] = cx + hash(cx, cy, seed + 30 + d * 2); c.jitter[k + 1] = cy + hash(cx, cy, seed + 31 + d * 2); }
            px = c.jitter[k]; py = c.jitter[k + 1];
          } else {
            px = cx + hash(cx, cy, seed + 30 + d * 2); py = cy + hash(cx, cy, seed + 31 + d * 2);
          }
          const dx = gx - px, dy = gy - py;
          const q = (dx * dx + dy * dy) / R2;
          if (q < nearest) { nearest = q; nx = cx * 2 + d; ny = cy; }
          if (q < 1) {
            const w = (1 - q) * (1 - q);
            acc += w * subPixelDensity(st, cx * 2 + d, cy, facets, hx, hy);
            if (w > wsum * 0.5) hue = glintHue;
            wsum += w;
          }
        }
      }
    }
    if (wsum > 1e-6) { glintHue = hue; return acc / wsum; }
    return subPixelDensity(st, nx, ny, facets, hx, hy);
  }

  // Facets wider than a pixel: the nearest jittered facet centre owns the
  // pixel; its tilt is drawn from the Beckmann slope distribution.
  function cellDensity(st, x, y, size, hx, hy) {
    const gx = x / size, gy = y / size;
    const ix = Math.floor(gx), iy = Math.floor(gy);
    let best = 9, bx = 0, by = 0, second = 9;
    const seed = st.p.seed | 0;
    for (let j = -1; j <= 1; j++) {
      for (let i = -1; i <= 1; i++) {
        const cx = ix + i, cy = iy + j;
        const dx = gx - (cx + 0.1 + 0.8 * hash(cx, cy, seed + 10));
        const dy = gy - (cy + 0.1 + 0.8 * hash(cx, cy, seed + 11));
        const d = dx * dx + dy * dy;
        if (d < best) { second = best; best = d; bx = cx; by = cy; }
        else if (d < second) second = d;
      }
    }
    const u1 = Math.max(1e-9, hash(bx, by, seed + 12)), u2 = hash(bx, by, seed + 13);
    const r = Math.sqrt(st.sigma2 * -Math.log(u1));
    const sx = r * Math.cos(6.283185307 * u2), sy = r * Math.sin(6.283185307 * u2);
    const dx = hx - sx, dy = hy - sy;
    glintHue = hash(bx, by, seed + 14);
    // A thin dark seam where facets meet (the boundary is in shadow).
    const seam = Math.sqrt(second) - Math.sqrt(best);
    const edge = seam < 0.08 ? 0.6 + 5 * seam : 1;
    return Math.exp(-(dx * dx + dy * dy) / (st.tau * st.tau)) / st.tauArea * edge;
  }

  // Shade one pixel. (x, y) device pixels from the surface origin; geometry
  // in logical units through k. Writes linear RGB to `result`.
  function shadePixel(st, x, y, k) {
    const p = st.p;
    const lx = p.lightX - x / k, ly = p.lightY - y / k, lz = p.lightZ;
    const vx = p.eyeX - x / k, vy = p.eyeY - y / k, vz = p.eyeZ;
    const li = 1 / Math.hypot(lx, ly, lz), vi = 1 / Math.hypot(vx, vy, vz);
    const Lx = lx * li, Ly = ly * li, Lz = lz * li;
    const Vx = vx * vi, Vy = vy * vi, Vz = vz * vi;
    let Hx = Lx + Vx, Hy = Ly + Vy, Hz = Lz + Vz;
    const hi = 1 / Math.hypot(Hx, Hy, Hz);
    Hx *= hi; Hy *= hi; Hz *= hi;
    // the facet slope that would mirror L into V
    const hx = -Hx / Hz, hy = -Hy / Hz;
    const facetPx = p.grain * k;
    let density;
    const speck = Math.max(1, (p.speck || 0) * k);
    if (facetPx <= 0.7) {
      density = speckDensity(st, x, y, speck, facetPx, hx, hy);
    } else if (facetPx >= 1.3) {
      density = cellDensity(st, x, y, facetPx, hx, hy);
    } else {
      const t = (facetPx - 0.7) / 0.6;
      const a = subPixelDensity(st, x, y, 1 / (facetPx * facetPx), hx, hy);
      const ha = glintHue;
      const b = cellDensity(st, x, y, facetPx, hx, hy);
      density = a + (b - a) * t;
      glintHue = t < 0.5 ? ha : glintHue;
    }
    // Beckmann D = slope density / cos⁴θh; radiance = D F / (4 n·v)
    const cos2 = Hz * Hz;
    const D = density / (cos2 * cos2);
    const vdh = Lx * Hx + Ly * Hy + Lz * Hz;
    const film = st.film;
    const F0m = 0.6;
    const Fm = F0m + (1 - F0m) * Math.pow(1 - vdh, 5);
    const specM = D * Fm / (4 * Vz) * p.specular * (1 - film) * (1 - film);
    // a smooth dielectric lobe for a paint film over the profile
    let specP = 0;
    if (film > 0) {
      const a2 = p.filmAlpha * p.filmAlpha;
      const lobe = Math.exp(-(hx * hx + hy * hy) / a2) / (Math.PI * a2) / (cos2 * cos2);
      const Fp = 0.04 + 0.96 * Math.pow(1 - vdh, 5);
      specP = lobe * Fp / (4 * Vz) * p.specular * film;
    }
    const ndl = Lz > 0 ? Lz : 0;
    // faint large-scale mottle of the blast coverage
    const mottle = 1 + (Finish.noise(x, y, 40 * k, (p.seed | 0) + 20) - 0.5) * p.mottle;
    const diffuse = (p.ambient + (1 - p.ambient) * ndl) * mottle;
    let tintR = 1, tintG = 1, tintB = 1;
    if (p.iridescence > 0) {
      hsvToRgb(glintHue, rainbow);
      const m = p.iridescence;
      tintR = 1 - m + m * rainbow[0] * 1.6; tintG = 1 - m + m * rainbow[1] * 1.6; tintB = 1 - m + m * rainbow[2] * 1.6;
    }
    const base = film > 0 ? st.paint : st.metal;
    const db = p.diffuse;
    result[0] = base[0] * diffuse * db + specM * st.metal[0] * tintR + specP;
    result[1] = base[1] * diffuse * db + specM * st.metal[1] * tintG + specP;
    result[2] = base[2] * diffuse * db + specM * st.metal[2] * tintB + specP;
  }

  // Soft shoulder so glints saturate towards white instead of clipping hue.
  function tone(v) {
    return v < 0.8 ? v : 0.8 + 0.2 * (1 - Math.exp(-(v - 0.8) * 5));
  }

  // The half-vector slope at a surface position (as shadePixel takes it).
  function halfSlope(p, x, y, k, out) {
    const lx = p.lightX - x / k, ly = p.lightY - y / k, lz = p.lightZ;
    const vx = p.eyeX - x / k, vy = p.eyeY - y / k, vz = p.eyeZ;
    const li = 1 / Math.hypot(lx, ly, lz), vi = 1 / Math.hypot(vx, vy, vz);
    let Hx = lx * li + vx * vi, Hy = ly * li + vy * vi, Hz = lz * li + vz * vi;
    const hi = 1 / Math.hypot(Hx, Hy, Hz);
    Hx *= hi; Hy *= hi; Hz *= hi;
    out[0] = -Hx / Hz; out[1] = -Hy / Hz;
  }
  const corner = [0, 0];

  const TILE = 32;
  function paint(img, rect, damage, p) {
    const st = prepare(p);
    const k = p.k;
    const x0 = Math.max(rect.x, damage.x), y0 = Math.max(rect.y, damage.y);
    const x1 = Math.min(rect.x + rect.w, damage.x + damage.w), y1 = Math.min(rect.y + rect.h, damage.y + damage.h);
    const data = img.data, stride = img.width * 4;
    const ox = p.originX || 0, oy = p.originY || 0;
    const facetPx = p.grain * k, speck = Math.max(1, (p.speck || 0) * k);
    const retained = facetPx <= 0.7 && speck > 1.01;
    const facets = (1 / (facetPx * facetPx)) * speck * speck * 0.5;
    for (let ty = y0; ty < y1; ty += TILE) {
      for (let tx = x0; tx < x1; tx += TILE) {
        const tx1 = Math.min(x1, tx + TILE), ty1 = Math.min(y1, ty + TILE);
        st.cache = null;
        if (retained) {
          // surface coordinates of the tile, the speck cells in reach of it,
          // and the bins its pixels' half-vectors fall between (corners ± 1)
          const sx0 = tx - rect.x + ox, sy0 = ty - rect.y + oy, sx1 = tx1 - 1 - rect.x + ox, sy1 = ty1 - 1 - rect.y + oy;
          const cx0 = Math.floor(sx0 / speck) - 2, cy0 = Math.floor(sy0 / speck) - 2;
          const cw = Math.floor(sx1 / speck) + 2 - cx0 + 1, ch = Math.floor(sy1 / speck) + 2 - cy0 + 1;
          let bxa = Infinity, bxb = -Infinity, bya = Infinity, byb = -Infinity;
          for (const [cxs, cys] of [[sx0, sy0], [sx1, sy0], [sx0, sy1], [sx1, sy1]]) {
            halfSlope(p, cxs, cys, k, corner);
            const bx = Math.floor(corner[0] / st.binW - 0.5), by = Math.floor(corner[1] / st.binW - 0.5);
            bxa = Math.min(bxa, bx); bxb = Math.max(bxb, bx); bya = Math.min(bya, by); byb = Math.max(byb, by);
          }
          const bx0 = bxa - 1, by0 = bya - 1, bw = bxb + 2 - bx0 + 1, bh = byb + 2 - by0 + 1;
          // points are named (cx·2 + d, cy)
          const px0 = cx0 * 2, py0 = cy0, pw = cw * 2, ph = ch;
          const size = pw * ph * bw * bh;
          const c = st.cache = { facets, px0, py0, pw, ph, bx0, by0, bw, bh, cx0, cy0, cw, ch,
            n: new Int32Array(size).fill(-1), hue: new Float64Array(size), jitter: new Float64Array(cw * ch * 4).fill(NaN) };
          void c;
        }
        for (let y = ty; y < ty1; y++) {
          let q = y * stride + tx * 4;
          for (let x = tx; x < tx1; x++, q += 4) {
            shadePixel(st, x - rect.x + ox, y - rect.y + oy, k);
            const r = tone(result[0]), g = tone(result[1]), b = tone(result[2]);
            data[q] = Finish.encode(r);
            data[q + 1] = Finish.encode(g);
            data[q + 2] = Finish.encode(b);
            data[q + 3] = 255;
          }
        }
      }
    }
    st.cache = null;
    return (x1 - x0) * (y1 - y0);
  }

  // Fitted against photographs (glint_lab.html, "Fit"), at display scale 2,
  // light height 900 and the owner's slightly reduced τ (2026-10-31):
  // mean, 2–4 and 4–8 px spread and skew of each crop match to within 0.3
  // levels, 0.1 levels and 0.2. The 1–2 px band is left 0.6–0.9 low
  // because the photographs' finest band carries compression and sensor
  // noise. α and τ are not constrained by a flat crop; they set the sheen's
  // breadth and how quickly glints change as the light moves.
  const PRESETS = {
    bead_blast: {
      label: 'Bead blasted', note: 'Fitted to the bead-blasted disc: many facets per pixel, a satin with faint, even grain.',
      grain: 0.05, speck: 0.75, alpha: 0.32, tau: 0.03, diffuse: 0.79, specular: 0.57, mottle: 0.05, iridescence: 0, film: 0,
      metalColour: '#a5a5a5',
    },
    sand_blast: {
      label: 'Sand blasted', note: 'Fitted to the sand-blasted disc: the same fine grain with twice the glint energy.',
      grain: 0.05, speck: 0.75, alpha: 0.42, tau: 0.034, diffuse: 0.46, specular: 1.23, mottle: 0.06, iridescence: 0, film: 0,
      metalColour: '#a3a3a3',
    },
    iridescent: {
      label: 'Iridescent blast', note: 'Fitted to the iridescent plate: larger specks, strong glints, colour from a thin oxide film.',
      grain: 0.10, speck: 1.4, alpha: 0.4, tau: 0.034, diffuse: 0.37, specular: 2.82, mottle: 0.06, iridescence: 0.27, film: 0,
      metalColour: '#a6a5a7',
    },
    coarse_grit: {
      label: 'Coarse steel grit', note: 'Not fitted. Facets wider than a pixel: distinct, flat glinting chips.',
      grain: 0.9, speck: 0, alpha: 0.45, tau: 0.05, diffuse: 0.33, specular: 0.7, mottle: 0.08, iridescence: 0, film: 0,
      metalColour: '#9f9f9f',
    },
    painted_sand: {
      label: 'Thin paint over sand blast', note: 'Not fitted. The sand-blast fit under a thin satin coat of paint.',
      grain: 0.05, speck: 0.75, alpha: 0.42, tau: 0.034, diffuse: 0.9, specular: 1.23, mottle: 0.05, iridescence: 0, film: 0.7,
      filmAlpha: 0.3, metalColour: '#a3a3a3', paintColour: '#2f6e6c',
    },
  };

  return { paint, PRESETS };
})();
