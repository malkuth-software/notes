// Render the comparison cases with the JavaScript kernels (the reference for
// cpp/surface_kernels.cpp) and write each as raw RGBA bytes.
//
// Usage: node tools/js_reference.mjs <output directory>
// The cases must match cases_finish / cases_glint in cpp/surface_check.cpp.

import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.join(here, '..');
const out = process.argv[2];
fs.mkdirSync(out, { recursive: true });

const context = vm.createContext({ Math, Float32Array, Uint8Array, Uint8ClampedArray, parseInt });
const source = fs.readFileSync(path.join(root, 'finish_kernel.js'), 'utf8') + '\n' +
  fs.readFileSync(path.join(root, 'glint_kernel.js'), 'utf8') + '\nthis.Finish = Finish; this.Glint = Glint;';
vm.runInContext(source, context);
const { Finish, Glint } = context;

const W = 320, H = 200, K = 2;
const finishes = ['dusty_matte', 'sand_matte', 'orange_peel', 'hammertone', 'wrinkle', 'bead_blast', 'sand_cast', 'flat_baseline'];
for (const name of finishes) {
  const img = { data: new Uint8ClampedArray(W * H * 4), width: W };
  Finish.paint(img, { x: 0, y: 0, w: W, h: H }, { x: 0, y: 0, w: W, h: H }, {
    finish: name, k: K, seed: 7, colour: '#2f6e6c', light: Finish.lightOf(125, 38), ambient: 0.32,
  });
  fs.writeFileSync(path.join(out, `finish_${name}.rgba`), img.data);
}
const glints = ['bead_blast', 'sand_blast', 'iridescent', 'coarse_grit', 'painted_sand'];
for (const name of glints) {
  const preset = Glint.PRESETS[name];
  const img = { data: new Uint8ClampedArray(W * H * 4), width: W };
  Glint.paint(img, { x: 0, y: 0, w: W, h: H }, { x: 0, y: 0, w: W, h: H }, {
    ...preset, k: K, seed: 5, ambient: 0.35, filmAlpha: preset.filmAlpha || 0.3, paintColour: preset.paintColour || '#808080',
    lightX: 0.62 * W / K, lightY: 0.35 * H / K, lightZ: 900, eyeX: 0.5 * W / K, eyeY: 0.5 * H / K, eyeZ: 700,
  });
  fs.writeFileSync(path.join(out, `glint_${name}.rgba`), img.data);
}
console.log(`wrote ${finishes.length + glints.length} reference images to ${out}`);
