# T2 engine core: study for the owner's interview

Date: 2026-10-18. Worker 2. Not committed. This is preparation for the T2
interview. The owner decides; nothing here is assumed.

Sources:
- the T2 brief (`planning/stages/T2-engine-core/README.md`) and `FORWARD_PROGRAMME.md` §4 T2;
- the design language: §1.2, §2.6–§2.9, §3, §4, §5.8, §8, §9, §10, §12.3–§12.9;
- `T4-default-theme/RENDERING_REFERENCE.md`;
- the interviews: GF002, GF007 and GF018–GF019;
- resources §1–§2;
- the docs: `painting-and-damage.md`, `update-scopes-and-dirtiness.md` and `PAINT_PIPELINE_AVAILABILITY.md`;
- ADR-004;
- the waiting consumers' code: DK D3 #144, DK D5 #145, RB decision 15, W decision 7 (#153) and RB-37.

The draft build plan I wrote before the scope changed is
`planning/stages/T2-engine-core/PLAN.md` on branch `claude/t2-engine-core`
(PR #159, draft). It is summarized in section 4, and it stays a draft until
the interview. A first phase was also built before the scope changed (PR
#161, draft). Section 3 gives what it proved.

## 1. What the waiting consumers need from the engine

| Consumer | Today | What it needs from T2 |
|---|---|---|
| **By-role paint path** (`paint_part`, every W, RB and DK control) | `PartPaint` carries the part's role, bounds and states, the recipe family, the mark, the text, font and alignment, the image, an optional corner radius and an optional accent. The body draws from today's recipes. | An engine entry point taking the same description, with today's body kept as the built-in fallback. Before motion or caching can work, the engine also needs a stable **part identity** (which declared part, and which item). `PartPaint` has none today. |
| **Docking stamp** (D3) | It shrinks to about 100 units over about 220 ms, then follows the pointer and settles into its target. It is driven by its own `activate_surface` loop with a hand-rolled smoothstep. | A motion value the control owns, on one engine clock per window: retargetable, honouring reduced motion and occlusion, scheduling nothing once settled. |
| **Docking tile fade** (D3) | It is published as `expanding` for 100 ms but not drawn, because `paint_part` has no opacity (#144, "Owner decisions"). | An opacity for the whole part, driven by a transition. |
| **Docking flyout slide** (D5) | It slides over about 180 ms on `schedule_ui_timer`. | The same as the stamp. |
| **Ribbon dithered rise** (RB-15, RB phase 7) | Not drawn. The bulge is not drawn at all yet (no interim default before T4). | A `dithered_bulge` primitive with an amplitude, and a theme-owned transition on `hot` that raises it (a settling primitive). |
| **Overlay shadow** (RB-37) | Painted by `paint_part`'s `shadow` role through `Painter::draw_box_shadow`. The values are two layers in `ThemeDefinition::overlay_shadow`, and there is none in high contrast. | Since §12.9 calls it "an engine input", the engine has to own it: a first-party blur, so the shadow is identical on every host, with its damage outsets kept. |
| **Content colour** (W-7, #153) | A `PartPaint` value that `paint_part` honours for the colour picker's swatches. | The engine reads it as a value atom. Content is drawn exactly as given, and the theme styles only around it. |
| **Accent tint** (RB-22) | A provisional six-colour translucent ramp over the material. | The engine's tint (§8.5: "until … the engine (T2) the tint"). |

## 2. What exists today

- **Materials:**
  - `SurfaceMaterial`: 8 fill layers, 4 shadows, borders, 8 keylines and one radius, painted through `Painter`.
  - Theme records: 11 roles × 7 states, hand-authored high contrast, and motion tokens of 100, 180, 280 and 1400 ms.
- **Motion:**
  - `AnimationTimeline`, `EasingCurve` and `MotionPolicy`.
  - **A conflict:** `MotionPolicy::reduced` keeps animating at 0.35 speed with half the excursion, but §9.5 and the T2 gate say reduced motion stops ambient motion and holds a static pose.
- **The paint pipeline:**
  - display chunks, typed dirtiness, three paint planes (backplane, control, overlay), bounded damage, paint leases and receipts;
  - frame requests through `schedule_paint`, `activate_surface` and `schedule_ui_timer`;
  - idle windows schedule zero frames.
- **Identity precedent:** ADR-004 made text coverage byte-identical on every host by giving the Skia raster and the Windows DIB compositor one blend arithmetic.
- **The atom vocabulary:** part roles, states, marks, values, families and semantic roles. **There is no engine-input atom yet**, no density in the code, and no ceremonial-surface flag.

## 3. What the paused first phase proved (PR #161, draft)

The first phase built three things:
- first-party `power`, `exponential` and `logarithm`, built only from `+ − × ÷`, `sqrt`, `frexp` and `ldexp`, compiled without fused multiply-add;
- a primitive registry declared in vocabulary atoms;
- the `bulge` and `dithered_bulge` primitives, taken exactly from the rendering reference.

Results:
- **Identical pixels are achievable.** CI's native jobs passed on macOS arm64, Linux arm64, Linux x64 and Windows x64. The 36 reference-image hashes (9 cases at scales 1, 1.25, 1.5 and 2) and a 1536-value math table matched on all four. Two things made this work: avoiding the host `pow`, `exp` and `log`, which differ between libraries, and turning off contraction (Clang fuses `a·b + c` on arm64 but not on x64).
- **Finding: the third shaded dither step never appears.** With the reference's default gains (lit 1.0, shade 0.85, crown 0.25), the tone over the whole button runs from −0.63 to +1.05. The third shaded step `rgb(88,122,186)` needs −0.68, so the light default shows three lit steps and only two shaded ones. The dark variant's gains reach −0.80 and show all six.
- **Finding: the shade field does not reach zero inside the part.** Its ellipse reaches 1.10 w and 1.12 h, so at the middle of the bottom edge it is still at about 47 % of its peak. That fits "deepest at the lower right", but not §3.5's "melt back into the plane before the part's bounds".
- **A rule the reference leaves to the engine:** the dither at fractional scales. The draft keeps the lattice at exactly 2 logical px, with dots of `max(1, floor(s + 0.5))` device px. At 1.25× a dot's position varies by up to one device pixel.

## 4. Proposed phases (draft)

Each phase ends with tests on every CI platform.

| Phase | Content |
|---|---|
| E1 | Deterministic kernels, the primitive registry, the bulge pair (built as evidence, #161). |
| E2 | `paint_part` → `ThemeEngine` with today's body as the fallback; part keys and a part table per window; a part-raster composite with ADR-004's blend; per-part framebuffers. |
| E3 | One motion clock per window; control-owned motion values (follow, spring, decay, timed easings). The stamp and flyout move onto it. |
| E4 | The cascade (built-in, sheet, class, stable ID, inline, forced high contrast), load-time validation, the engine version check, derived high contrast, and the schema's `engine` export. |
| E5 | Bounded expressions. |
| E6 | Engine inputs and damage by dependency (needs B1). |
| E7 | State machines and transitions (tile fade, ribbon rise), ambient effects within the accepted tokens, reduced motion. |
| E8 | Relief primitives (pill plate, well, pressed, gold, perimeter, disabled, etched, seam, actuator), shapes and derived hit regions, text slots. |
| E9 | Gloss, glow, sheen, pop, translucency, reflection, glass, and the overlay shadow. |
| E10 | Procedural materials. |
| E11 | Gate evidence and the native specimen sheet. |

## 5. Interview questions

Each question is a short numbered option list. ★ marks my recommendation.

**A. Scope and order**

- **T2-A1. Which phase comes first?**
  1. ★ Kernels and the composite (E1, E2), then motion (E3), so the tile fade and the rise can actually be drawn.
  2. Motion first (E3 only), moving the stamp and flyout off their own timers, before any engine drawing.
  3. The cascade first (E4), so that T3 can start early.
- **T2-A2. What must the identity gate cover?**
  1. Each primitive's raster bytes only.
  2. ★ Raster bytes, plus the composite into the window through each host's painter (as ADR-004 does for text).
  3. Whole-window captures.
- **T2-A3. What happens to the paused PRs #159 (plan) and #161 (E1 code)?**
  1. ★ Keep them as drafts and revise them after the interview.
  2. Close them and restart after the interview.

**B. Atoms (§12.7: adding one is the owner's decision)**

- **T2-B1. Admit the engine inputs as atoms.**
  1. ★ The brief's set: `pointer_x` and `pointer_y` (published only while the part is hot), `time_in_state`, `display_scale`, `text_scale`, and published application values (named, numeric, read-only).
  2. Only `pointer_x`, `pointer_y` and `time_in_state` now; the rest when a theme needs them.
  3. Defer all of them to T3.
- **T2-B2. Pointer units.**
  1. ★ Part-local logical units.
  2. Normalized 0–1 across the part.
  3. Both, as two pairs of atoms.
- **T2-B3. A ceremonial surface (§10: "the application decides").**
  1. ★ The application sets a ceremonial flag on a surface, which lifts the 10 % ambient cap for that surface only.
  2. A whole-window flag.
  3. No flag; ceremony stays under the everyday cap.
- **T2-B4. Density (§8.4 names it; no code has it).**
  1. Admit compact, standard and touch now.
  2. ★ Leave it until stage L or T4 needs it.

**C. Opacity and the docking tiles**

- **T2-C1. Where does a part's opacity come from?**
  1. ★ It is an engine presentational property driven by transitions. The control passes only its states and part keys, and D3's tiles get their fade without a new atom.
  2. A control-supplied `opacity` value in `PartPaint`. This is a new value atom, and it puts motion back into controls.
  3. Both: control-supplied for control-owned motion, engine-owned for theme motion.
- **T2-C2. Should the tile fade be drawn before T4?**
  1. ★ Yes. The DK plan gives motion to T2, so the built-in rules draw it once transitions exist (E7).
  2. No; wait for the T4 theme.

**D. Motion**

- **T2-D1. Motion a control computes itself (the stamp, the flyout).**
  1. ★ Motion values the control owns, on one clock per window, replacing each control's own timers.
  2. Keep the controls' timers; the engine runs only theme transitions.
- **T2-D2. Reduced motion and the activity object.**
  1. ★ §9.5 governs: transitions jump to their end, ambient effects stop, and the activity object holds a static pose beside its stage text. Today's slowed `MotionPolicy` moves to this in T4.
  2. Keep today's slowed cadence for activity objects only.
- **T2-D3. The ribbon's rise easing** (taste, see the mockup).
  1. ★ Exponential approach, a fixed fraction of the gap per about 75 ms (§9.2 "lifts and pops that retarget").
  2. A critically damped spring.
  3. Smoothstep over *quick* (100 ms).
- **T2-D4. An occluded or minimized window.**
  1. ★ Every transition jumps to its end and nothing is scheduled.
  2. Transitions pause and resume on exposure.
- **T2-D5. A presentational scale for loud themes.** §9.1 rejects scale transitions; the T3 scenario shows `scale: 0.97` on press.
  1. ★ Allow it as a capability only: drawn about the part's centre and inside its granted overflow, with the hit region and layout unchanged, and unused by the default.
  2. Do not offer scale at all.

**E. Overlay shadow and content colour**

- **T2-E1. The overlay shadow (RB-37).**
  1. ★ Move it to an engine primitive with a first-party blur, so it is identical across hosts (E9), keeping today's values and outsets.
  2. Leave it on `Painter`'s box shadow until T4.
- **T2-E2. Content colour (W-7).**
  1. ★ The engine draws content colour exactly as given; a theme styles only what surrounds it (edge, well).
  2. A theme may adjust content colour (for example, dim it when disabled).

**F. Budgets** (new tokens, revisited after the specimen sheet)

- **T2-F1. Per-part cost.**
  1. ★ At most 512 declared operations per device pixel for a part's stack, and at most 4 million per frame for an animated part.
  2. Enforce only "cannot be bounded" for now, with no number.
- **T2-F2. Memory for cached part rasters.**
  1. ★ At most 8 MB per window, evicting the least recently painted parts first.
  2. No cache until the measurements say it is needed.

**G. Rendering reference findings** (taste, see the mockup)

- **T2-G1. The third shaded dither step.**
  1. Keep the reference as written, with two shaded steps in the light default.
  2. ★ Raise the default shade gain to about 0.95, so that the third step appears near the lower-right rim.
  3. Rescale quantization to the field's actual tone range.
- **T2-G2. The shade field at the edge.**
  1. ★ Keep the formula as the approved studies drew it.
  2. Shrink the shade ellipse so it melts before the bounds, as §3.5 words it.
- **T2-G3. Dither at fractional display scales.**
  1. ★ Keep the lattice at exactly 2 logical px; at 1.25× a dot's position varies by up to one device pixel.
  2. A regular device lattice of `round(2s)` px, so the logical period is not exactly 2.
  3. Draw dithered parts at the nearest integer scale.

## 6. Taste questions for a mockup page

Each panel shows the alternatives side by side. Native rasters from the engine
can be produced with `GUI_FORMS_ENGINE_DUMP` (#161).

- **TT1. The rise** (D3): a ribbon button rising under a simulated pointer three ways (exponential approach about 75 ms, spring, smoothstep 100 ms), replayable.
- **TT2. The tile fade** (C2): docking tiles fading in over 100 ms beside appearing instantly.
- **TT3. Fractional-scale dither** (G3): the three rules at 1.25× and 1.5×, magnified.
- **TT4. The third shaded step** (G1): the light default with the reference gains beside shade gain 0.95 and the rescaled quantization.
- **TT5. The shade edge** (G2): the formula as written beside a shade field that melts inside the bounds, in smooth and dithered forms.
- **TT6. Hover on the smooth bulge**: none, the studies' brightness 1.03–1.04 with saturation 1.15, and a stronger lift.
