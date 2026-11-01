// Body study: paint the demo's body with two glint kernels and compare.
// Usage: node tools/body_study.mjs <reference kernel.js> <candidate kernel.js>
// Paints demo.html's two Glint.paint calls (1000 × 620 at scale 2: the 397
// panel below the title strip, film 0.55, specular 1.6; the strip in 656f74),
// times each kernel (median of three) and reports CIEDE2000 over the pixels.
import fs from 'node:fs';
import vm from 'node:vm';

function load(file) {
  const context = vm.createContext({ Math, Float32Array, Float64Array, Int32Array, Uint8Array, Uint8ClampedArray, Number, Infinity, NaN, parseInt });
  vm.runInContext(fs.readFileSync('finish_kernel.js', 'utf8') + '\n' + fs.readFileSync(file, 'utf8') + '\nthis.Glint = Glint;', context);
  return context.Glint;
}
const W = 1000, H = 620, K = 2;
function paintBody(Glint) {
  const img = { data: new Uint8ClampedArray(W * H * 4), width: W };
  const preset = Glint.PRESETS.painted_sand;
  const common = { ...preset, k: K, seed: 11, ambient: 0.35, filmAlpha: preset.filmAlpha, film: 0.55, specular: 1.6,
    lightX: 0.2 * W / K, lightY: 0.05 * H / K, lightZ: 900, eyeX: 0.5 * W / K, eyeY: 0.5 * H / K, eyeZ: 700 };
  Glint.paint(img, { x: 0, y: 0, w: W, h: H }, { x: 0, y: 78, w: W, h: H - 78 }, { ...common, paintColour: '#4d676d' });
  Glint.paint(img, { x: 0, y: 0, w: W, h: H }, { x: 0, y: 0, w: W, h: 78 }, { ...common, paintColour: '#656f74', film: 0.85 });
  return img.data;
}
function timed(Glint) {
  const times = []; let data;
  for (let r = 0; r < 3; r++) { const t0 = performance.now(); data = paintBody(Glint); times.push(performance.now() - t0); }
  times.sort((a, b) => a - b);
  return { data, ms: times[1], runs: times.map((t) => t.toFixed(0)).join(' ') };
}
function lab(r8, g8, b8) {
  const lin = (c) => { c /= 255; return c <= 0.04045 ? c / 12.92 : ((c + 0.055) / 1.055) ** 2.4; };
  const r = lin(r8), g = lin(g8), b = lin(b8);
  const x = (0.4124564 * r + 0.3575761 * g + 0.1804375 * b) / 0.95047, y = 0.2126729 * r + 0.7151522 * g + 0.0721750 * b;
  const z = (0.0193339 * r + 0.1191920 * g + 0.9503041 * b) / 1.08883;
  const f = (t) => (t > 216 / 24389 ? Math.cbrt(t) : (24389 / 27 * t + 16) / 116);
  return [116 * f(y) - 16, 500 * (f(x) - f(y)), 200 * (f(y) - f(z))];
}
function de2000(p, q) {
  const [L1, a1, b1] = p, [L2, a2, b2] = q, pi = Math.PI;
  const C1 = Math.hypot(a1, b1), C2 = Math.hypot(a2, b2), Cm = (C1 + C2) / 2, C7 = Cm ** 7;
  const G = 0.5 * (1 - Math.sqrt(C7 / (C7 + 25 ** 7)));
  const ap1 = (1 + G) * a1, ap2 = (1 + G) * a2, Cp1 = Math.hypot(ap1, b1), Cp2 = Math.hypot(ap2, b2);
  let h1 = Math.atan2(b1, ap1); if (h1 < 0) h1 += 2 * pi; let h2 = Math.atan2(b2, ap2); if (h2 < 0) h2 += 2 * pi;
  const dL = L2 - L1, dC = Cp2 - Cp1; let dh = 0;
  if (Cp1 * Cp2 !== 0) { dh = h2 - h1; if (dh > pi) dh -= 2 * pi; else if (dh < -pi) dh += 2 * pi; }
  const dH = 2 * Math.sqrt(Cp1 * Cp2) * Math.sin(dh / 2), Lm = (L1 + L2) / 2, Cpm = (Cp1 + Cp2) / 2;
  let hm = h1 + h2; if (Cp1 * Cp2 !== 0) hm = Math.abs(h1 - h2) > pi ? (h1 + h2 < 2 * pi ? (h1 + h2 + 2 * pi) / 2 : (h1 + h2 - 2 * pi) / 2) : (h1 + h2) / 2;
  const T = 1 - 0.17 * Math.cos(hm - pi / 6) + 0.24 * Math.cos(2 * hm) + 0.32 * Math.cos(3 * hm + pi / 30) - 0.2 * Math.cos(4 * hm - 63 * pi / 180);
  const dT = 30 * pi / 180 * Math.exp(-((((hm * 180 / pi) - 275) / 25) ** 2)), Cp7 = Cpm ** 7, Rc = 2 * Math.sqrt(Cp7 / (Cp7 + 25 ** 7));
  const l50 = (Lm - 50) ** 2, SL = 1 + 0.015 * l50 / Math.sqrt(20 + l50), SC = 1 + 0.045 * Cpm, SH = 1 + 0.015 * Cpm * T, RT = -Math.sin(2 * dT) * Rc;
  return Math.sqrt((dL / SL) ** 2 + (dC / SC) ** 2 + (dH / SH) ** 2 + RT * (dC / SC) * (dH / SH));
}
const ref = timed(load(process.argv[2])), cand = timed(load(process.argv[3]));
let same = 0, max = 0, sum = 0, over1 = 0; const d = [];
for (let i = 0; i < W * H * 4; i += 4) {
  if (ref.data[i] === cand.data[i] && ref.data[i + 1] === cand.data[i + 1] && ref.data[i + 2] === cand.data[i + 2]) { same++; d.push(0); continue; }
  const e = de2000(lab(ref.data[i], ref.data[i + 1], ref.data[i + 2]), lab(cand.data[i], cand.data[i + 1], cand.data[i + 2]));
  d.push(e); max = Math.max(max, e); sum += e; if (e > 1) over1++;
}
d.sort((a, b) => a - b);
console.log(`reference ${ref.ms.toFixed(0)} ms (${ref.runs}) · candidate ${cand.ms.toFixed(0)} ms (${cand.runs}) · ${(ref.ms / cand.ms).toFixed(2)}×`);
console.log(`identical pixels ${(100 * same / (W * H)).toFixed(3)}% · dE00 max ${max.toFixed(3)} p99.9 ${d[Math.floor(0.999 * d.length)].toFixed(3)} mean ${(sum / (W * H)).toFixed(4)} >1 ${(100 * over1 / (W * H)).toFixed(3)}%`);
