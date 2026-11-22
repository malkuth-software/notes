# Provenance

conv-teapot and BFFT are MIT (owner, 2026-11-13). Only the teapot's and BFFT's own code is used here. Nothing
comes from the confidential AirSpy DDC method, which a Fable sibling studied when speeding up the teapot
engine, and no code derived from that study is used.

| File here | Origin | Changes |
|---|---|---|
| `engines/conv_teapot/src/render.hpp`, `raster.hpp`, `blend.hpp`, `surface.hpp`, `conv_line.hpp` | conv-teapot `src/` at fc0a609 (MIT; shallow clone `~/Developer/gui_forms_workers/hD5-study`) | None |
| `engines/conv_teapot/LICENSE` | conv-teapot `LICENSE` at fc0a609 | None |
| `engines/bfft/experiments/photonic_transport_field/native/regime_scene_native.cpp` | notes `experiments/industrial-surfaces/bfft/...` (bfft 6e64386, with the industrial-surfaces key and meter additions) | Inputs and outputs only (README, "Changes made to the engine copies") |
| `.../native/retained_transport.{h,cpp}`, `bruun_mag_angle_adapter.hpp`, `child_boundary_response.hpp`, `pixel_redundancy_audit.hpp`, `return_extinction.hpp`, `retained_{scene_update,camera,boundary,source}_checks.hpp` | the same, bfft 6e64386 | None |
| `.../native/key_scene.hpp`, `meter_scene.hpp` | the same: the industrial-surfaces additions to bfft (not upstream) | None |
| `.../native/tracer_scenes.hpp`, `worker_limit.hpp` | new, for this playground | — |
| `engines/bfft/src/detail/MAG_REPRESENT_KERNEL.hpp`, `generated/bruun_phase_slope_tables.hpp`, `generated/bruun_sincos_table256.hpp` | notes `experiments/industrial-surfaces/bfft/src/detail/` (bfft 6e64386) | None |
| `engines/bfft/standalone_conv_resize_demo/native/conv_native.c` | notes `experiments/industrial-surfaces/bfft/standalone_conv_resize_demo/native/` (bfft 6e64386) | None |
| `engines/bfft/PROVENANCE.txt` | the extract's own provenance note | None |
| `engines/lamp_keys/lamp_keys.cpp` | notes `experiments/industrial-surfaces/cpp/lamp_keys.cpp` at c9bef59: the std-only source that GUI.Forms' `tools/industrial_surfaces/lamp_keys.cpp` was ported from (main differs only in drawing legends through GUI.Drawing) | The worker count reads TRACER_THREADS |
| `scenes.hpp`, `tracer_bakes.cpp`, `teapot_bake.cpp`, `tracer_bakes.html`, `Makefile` | new, for this playground | — |
