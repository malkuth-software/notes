# Skeudoindustrialism: ingesting the industrial surfaces work

Status: planned (owner, 2026-11-01: "ingest the source and plan it"). Not
started. Source record: `experiments/industrial-surfaces/` on this branch
(see its `RECORD.md`), from the glintstone session.

## What it is

The owner's name for the look is **skeudoindustrialism**: industrial
equipment made legible on screen. Painted and blasted metal, lit acrylic keys,
jewelled indicator lamps and documented industrial paint colours. The demo is a
process-control dialog ("START TRANSFER / PUMP TO RESERVE TANK") with lamp
indicators and lit CANCEL/OK keys. The owner calls the pieces "independently
ingestable ideas": each can come into GUI.Forms on its own.

| Piece | Where | What it gives GUI.Forms |
|---|---|---|
| Surface finishes | `cpp/surface_kernels.{hpp,cpp}` (port of `finish_kernel.js`) | Dusty matte, orange peel, hammertone, wrinkle, bead blast, sand cast and flat finishes, painted per pixel with **no bitmap**: each device pixel is a pure function of its position on the surface, the scale and the parameters, so any damage rectangle repaints to exactly the full repaint's bytes. Scratch is two rows the width of the damage. |
| Glint | same files (port of `glint_kernel.js`, `glint_fast.js`) | Blasted-metal glints with presets, no scratch. An optional effect (owner: "itself merely an optional effect"). |
| Lit keys and lamps | `cpp/lamps.cpp`, `lamp_keys.cpp`, `lit_key.cpp`, `acrylic_key.cpp`, `key_mesh.cpp`, `caustic_tile.cpp` | Keycaps lit from within, faceted jewel indicators and split-legend annunciators, rendered offline with BFFT's photonic transport law (`bfft/`), path tracing, a Stillwater caustic tile and an stx_conv-rebuilt key mesh, cached as linear light planes (room, each lamp at unit power, coverage) and composited at run time. `cpp/bake_study/` measures cheaper bakes against a converged reference by CIEDE2000. |
| Colour styles | `research/soviet_paints.md` | Soviet industrial enamel colours (1950s to 1980s) tied to GOST standards, every value cited and graded by evidence, none sampled from photographs. |
| Composition | `cpp/dialog_demo.cpp`, `demo.html`, `dialog.html` | The target look, as one dialog. |

## Fit with GUI.Forms

- **Damage invariance is already the kernels' contract.** It is the property
  the AAA work (2026-10-31/11-01) made Skia's anti-aliasing satisfy for rows
  and columns, and banded paint (B1) needs. The surface kernels can be held to
  it by the same test: every frame equals its whole repaint and its repaint in
  uneven row and column bands.
- **"Texture a surface without a bitmap, efficiently"** is the theme engine's
  procedural-material job (T2's material, procedural and blur buffers; the
  owner's ADR-015/ADR-016 line on paying only for what changes).
- **Lamps and keys are animated-control DNA:** lit/unlit and pressed states,
  half-press, glow. They are natural first users of `AnimationTimer` and of
  part states in the theme language.
- **Colour styles** become named palettes that themes and schemes can use.

## Phases

Each phase lands on its own, through the normal worker and checkpoint path,
under the house style (section 8 for every kernel: reformulate first, no
decisions in the loop, corrections outside the interior, kept reference).

1. **Finishes as theme-engine material layers.** Port `surface_kernels` into
   the engine's material system behind the renderer-neutral drawing
   vocabulary (no Skia in public headers). The theme language gains a finish
   layer (kind, seed, scale, colours). Keep the JavaScript kernels as the
   reference (byte comparison, as `surface_check.cpp` does now). Acceptance:
   the damage-invariance test above at scales 1 to 3; a cost bound per painted
   pixel measured on the M4; scalar and NEON equal.
2. **Glint as an optional effect.** A theme property, off unless the theme
   asks for it, honouring reduced motion, with its own cost bound.
3. **Lamps and lit keys.** First decide how they are drawn at run time (see
   the open decisions): composited from shipped light-plane caches, as the
   demo does, or a cheaper closed-form approximation. Then: a lamp indicator
   (a control or a part, per the W and T3 owners), lit/unlit/pressed states
   for buttons through the theme language, animated with `AnimationTimer`.
4. **Colour styles.** The GOST palette as a named colour set themes can
   import, with its sources and evidence grades kept beside it.
5. **The look as a theme and a Gallery page.** A skeudoindustrial theme
   package (after user themes, T3 U1 to U4) and a Gallery page or template
   dialog showing it, as dogfood and as the visual target.

## Open decisions (owner)

- **Lamps: baked or computed. Answered (owner, 2026-11-01): bake on first
  use** (shipping caches is also acceptable). Two questions to answer before
  building it:
  1. **Bake once, use many.** The demo already splits each bake into separate
     linear light planes (room, each lamp at unit power, coverage). Can that
     decomposition be extended so one bake serves many uses (other sizes,
     scales, states, key shapes), the way the plane split lets one bake serve
     every lamp brightness?
  2. **Bake without colour, add colour after, same result.** Can the planes be
     baked achromatic and tinted afterwards while matching a bake done in
     colour? It holds exactly where light reaches the eye after one
     interaction with the tinted material (the result is linear in its
     colour), and fails where light bounces inside a tinted lens or body
     several times (the colour enters as a power). The study should measure
     the error against a coloured bake with the bake study's CIEDE2000 bar,
     and, if needed, keep a small number of extra planes (for example one per
     bounce order) so the tint can still be applied exactly afterwards.
- **Finishes: theme-language primitive or engine-only** (a theme names a
  finish kind and parameters, or only engine code uses them).
- **Where lamps live:** a new control, a part of existing controls, or both.
- **Phase order** relative to T4's default theme.
- **BFFT and stx_conv at run time:** the lamp renders use BFFT's photonic
  transport and stx_conv's mesh reconstruction offline; whether either is
  needed at run time is open.

## Not done here

- The reference photographs (`reference/` in the original folder) are not in
  this repository; their rights are unknown.
- No GUI.Forms code is changed by this plan. The experiment's code has not
  had a house-style pass.
