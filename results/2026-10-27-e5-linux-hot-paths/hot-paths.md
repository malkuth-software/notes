# GUI.Forms hot paths

Shares of perf samples per workload (self: in the function itself; inclusive: with its callees, where frame pointers allow). Not timings. The Skia archive is built without frame pointers, so a sample inside Skia may not reach its GUI.Forms callers: inclusive shares of GUI.Forms functions are lower bounds.

- Commit: `d16c7910cdaf80b8acc6ec5a8626a30cb1f612f7`
- Host: AMD EPYC 7763 64-Core Processor, 4 CPUs, Linux-6.17.0-1022-azure-x86_64-with-glibc2.39
- Runner: ubuntu-24.04 (ubuntu24 20261023.334.3)
- perf: perf version 6.17.13; sampling at 999 Hz with call graphs (frame pointers)
- Compiler: Ubuntu clang version 22.1.8 (++20260714014902+ca7933e47d3a-1~exp1~20260714135019.80); Release plus `-fno-omit-frame-pointer -mno-omit-leaf-frame-pointer -gline-tables-only`

### paint

About 11000 samples.

| Area | Self share |
|---|---:|
| skia | 47.13% |
| gui_forms | 40.19% |
| freetype | 3.13% |
| workload | 1.00% |
| other | 0.80% |
| runtime | 0.73% |
| kernel | 0.06% |

| # | Self | Inclusive | Area | Function | Object |
|---:|---:|---:|---|---|---|
| 1 | 28.62% | 28.66% | skia | `sse2::rect_memset32(unsigned int*, unsigned int, int, unsigned long, int)` | gui_forms_paint_profile_workload |
| 2 | 24.46% | 34.60% | gui_forms | `gui_forms::detail::composite_text_coverage(gui_forms::detail::TextCoverage, gui_forms::detail::TextCompositeTarget, gui_forms::detail::TextCompositeClip, gui...` | gui_forms_paint_profile_workload |
| 3 | 10.29% | 10.30% | gui_forms | `gui_forms::detail::(anonymous namespace)::prepared_clip_coverage(gui_forms::detail::(anonymous namespace)::PreparedClip const&, long, long)` | gui_forms_paint_profile_workload |
| 4 | 5.29% | 5.29% | skia | `sse2::lowp::evenly_spaced_2_stop_gradient(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsig...` | gui_forms_paint_profile_workload |
| 5 | 1.48% | 1.49% | skia | `aaa_fill_path(SkPathRaw const&, SkIRect const&, AdditiveBlitter*, int, int, bool, bool, bool)` | gui_forms_paint_profile_workload |
| 6 | 1.23% | 1.23% | skia | `sse2::lowp::store_8888(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vector(...` | gui_forms_paint_profile_workload |
| 7 | 1.17% | 1.18% | skia | `sse2::lowp::start_pipeline(unsigned long, unsigned long, unsigned long, unsigned long, SkRasterPipelineStage*, SkSpan<SkRasterPipelineContexts::MemoryCtxPatc...` | gui_forms_paint_profile_workload |
| 8 | 1.17% | 1.17% | skia | `sse2::lowp::bilerp_clamp_8888(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __...` | gui_forms_paint_profile_workload |
| 9 | 1.00% | 46.45% | workload | `(anonymous namespace)::measure(gui_forms::render::SkiaRaster&, gui_forms::Window&, gui_forms::Rect, double, unsigned long, unsigned long)` | gui_forms_paint_profile_workload |
| 10 | 0.84% | 0.84% | skia | `sse2::lowp::matrix_2x3(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vector(...` | gui_forms_paint_profile_workload |
| 11 | 0.81% | 0.98% | gui_forms | `gui_forms::Window::collect_paint_checkpoints(std::shared_ptr<gui_forms::Control> const&, std::vector<gui_forms::Window::PaintControlCheckpoint, std::allocato...` | gui_forms_paint_profile_workload |
| 12 | 0.75% | 0.76% | gui_forms | `gui_forms::Control::clear_dirty(gui_forms::Dirty)` | gui_forms_paint_profile_workload |
| 13 | 0.75% | 1.53% | gui_forms | `gui_forms::Window::clear_paint_dirty_subtree(std::shared_ptr<gui_forms::Control> const&)` | gui_forms_paint_profile_workload |
| 14 | 0.75% | 0.87% | freetype | `af_glyph_hints_reload` | gui_forms_paint_profile_workload |
| 15 | 0.71% | 0.71% | skia | `blit_trapezoid_row(AdditiveBlitter*, int, int, int, int, int, int, int, unsigned char, unsigned char*, bool)` | gui_forms_paint_profile_workload |
| 16 | 0.65% | 0.65% | gui_forms | `gui_forms::Window::display_cache_entries(std::shared_ptr<gui_forms::Control> const&) const` | gui_forms_paint_profile_workload |
| 17 | 0.60% | 0.60% | skia | `SkScan::FillPath(SkPathRaw const&, SkRegion const&, SkBlitter*)` | gui_forms_paint_profile_workload |
| 18 | 0.57% | 0.57% | freetype | `gray_render_line` | gui_forms_paint_profile_workload |
| 19 | 0.51% | 0.51% | skia | `sse2::lowp::load_8888_dst(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vect...` | gui_forms_paint_profile_workload |
| 20 | 0.45% | 3.54% | gui_forms | `gui_forms::Window::paint_recursive(std::shared_ptr<gui_forms::Control> const&, gui_forms::Painter&, gui_forms::Rect, gui_forms::PaintPlane, unsigned long&, u...` | gui_forms_paint_profile_workload |
| 21 | 0.39% | 0.39% | skia | `sse2::lowp::clamp_01(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vector(8)...` | gui_forms_paint_profile_workload |
| 22 | 0.37% | 0.37% | gui_forms | `gui_forms::Window::absolute_bounds_of(gui_forms::Control const&) const` | gui_forms_paint_profile_workload |
| 23 | 0.32% | 40.57% | gui_forms | `gui_forms::detail::replay_display_chunk(gui_forms::detail::DisplayChunk const&, gui_forms::Painter&)` | gui_forms_paint_profile_workload |
| 24 | 0.32% | 0.33% | skia | `sse2::lowp::seed_shader(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vector...` | gui_forms_paint_profile_workload |
| 25 | 0.30% | 0.30% | skia | `SkRasterPipelineBlitter::blitRect(int, int, int, int)` | gui_forms_paint_profile_workload |
| 26 | 0.27% | 0.27% | skia | `sse2::lowp::clamp_x_1(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vector(8...` | gui_forms_paint_profile_workload |
| 27 | 0.27% | 0.27% | skia | `sse2::lowp::lerp_1_float(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vecto...` | gui_forms_paint_profile_workload |
| 28 | 0.24% | 0.24% | freetype | `af_latin_hints_compute_segments` | gui_forms_paint_profile_workload |
| 29 | 0.24% | 0.24% | runtime | `0x0000000000188dd9` | libc.so.6 |
| 30 | 0.22% | 0.22% | skia | `SkRasterPipelineBlitter::blitAntiH(int, int, unsigned char const*, short const*)` | gui_forms_paint_profile_workload |
| 31 | 0.22% | 0.66% | runtime | `malloc` | libc.so.6 |
| 32 | 0.21% | 0.21% | skia | `SkRasterPipelineBlitter::blitH(int, int, int)` | gui_forms_paint_profile_workload |
| 33 | 0.20% | 0.22% | gui_forms | `gui_forms::detail::DisplayCommand::~DisplayCommand()` | gui_forms_paint_profile_workload |
| 34 | 0.20% | 0.20% | gui_forms | `void std::_Destroy_aux<false>::__destroy<gui_forms::Window::PaintControlCheckpoint*>(gui_forms::Window::PaintControlCheckpoint*, gui_forms::Window::PaintCont...` | gui_forms_paint_profile_workload |
| 35 | 0.19% | 0.19% | skia | `SkAAClipBlitter::blitH(int, int, int)` | gui_forms_paint_profile_workload |
| 36 | 0.19% | 0.19% | skia | `SkRasterPipelineBlitter::Create(SkPixmap const&, SkPaint const&, SkRGBA4f<(SkAlphaType)3> const&, SkArenaAlloc*, SkRasterPipeline const&, bool, bool, SkShade...` | gui_forms_paint_profile_workload |
| 37 | 0.19% | 0.21% | freetype | `af_glyph_hints_align_weak_points` | gui_forms_paint_profile_workload |
| 38 | 0.19% | 0.49% | gui_forms | `gui_forms::detail::DisplayCommand& std::vector<gui_forms::detail::DisplayCommand, std::allocator<gui_forms::detail::DisplayCommand> >::emplace_back<gui_forms...` | gui_forms_paint_profile_workload |
| 39 | 0.19% | 0.19% | skia | `sse2::lowp::srcover(SkRasterPipelineStage*, unsigned long, unsigned long, unsigned short __vector(8), unsigned short __vector(8), unsigned short __vector(8),...` | gui_forms_paint_profile_workload |
| 40 | 0.18% | 0.18% | freetype | `TT_Load_Simple_Glyph` | gui_forms_paint_profile_workload |

### text_engine

About 15000 samples.

| Area | Self share |
|---|---:|
| freetype | 71.80% |
| gui_forms | 15.48% |
| runtime | 4.34% |
| other | 3.26% |
| harfbuzz | 2.22% |
| workload | 0.05% |

| # | Self | Inclusive | Area | Function | Object |
|---:|---:|---:|---|---|---|
| 1 | 10.64% | 12.82% | freetype | `af_glyph_hints_reload` | gui_forms_text_engine_benchmark |
| 2 | 7.98% | 7.98% | freetype | `gray_render_line` | gui_forms_text_engine_benchmark |
| 3 | 7.15% | 10.23% | gui_forms | `gui_forms::detail::composite_text_coverage(gui_forms::detail::TextCoverage, gui_forms::detail::TextCompositeTarget, gui_forms::detail::TextCompositeClip, gui...` | gui_forms_text_engine_benchmark |
| 4 | 4.79% | 4.79% | freetype | `af_latin_hints_compute_segments` | gui_forms_text_engine_benchmark |
| 5 | 4.70% | 33.67% | freetype | `af_latin_hints_apply` | gui_forms_text_engine_benchmark |
| 6 | 4.26% | 4.26% | freetype | `TT_Load_Simple_Glyph` | gui_forms_text_engine_benchmark |
| 7 | 4.04% | 9.45% | freetype | `gray_conic_to` | gui_forms_text_engine_benchmark |
| 8 | 3.59% | 4.26% | freetype | `af_glyph_hints_align_weak_points` | gui_forms_text_engine_benchmark |
| 9 | 3.08% | 3.08% | gui_forms | `gui_forms::detail::(anonymous namespace)::prepared_clip_coverage(gui_forms::detail::(anonymous namespace)::PreparedClip const&, long, long)` | gui_forms_text_engine_benchmark |
| 10 | 2.76% | 2.76% | runtime | `0x000000000018984a` | libc.so.6 |
| 11 | 2.67% | 77.55% | gui_forms | `gui_forms::render::text::GlyphRunRasterizer::Impl::Runner::run(gui_forms::render::text::GlyphRunRasterizer::Impl&, gui_forms::render::text::ShapedText const&...` | gui_forms_text_engine_benchmark |
| 12 | 2.58% | 17.54% | freetype | `gray_convert_glyph` | gui_forms_text_engine_benchmark |
| 13 | 2.43% | 2.43% | freetype | `FT_Outline_Transform` | gui_forms_text_engine_benchmark |
| 14 | 2.40% | 14.34% | freetype | `FT_Outline_Decompose` | gui_forms_text_engine_benchmark |
| 15 | 2.34% | 2.34% | freetype | `ft_glyphslot_preset_bitmap` | gui_forms_text_engine_benchmark |
| 16 | 2.06% | 2.06% | freetype | `FT_Outline_Get_Orientation` | gui_forms_text_engine_benchmark |
| 17 | 2.00% | 8.94% | freetype | `af_latin_hints_detect_features` | gui_forms_text_engine_benchmark |
| 18 | 1.82% | 1.82% | freetype | `FT_Outline_Get_CBox` | gui_forms_text_engine_benchmark |
| 19 | 1.69% | 1.69% | freetype | `tt_cmap4_char_map_binary` | gui_forms_text_engine_benchmark |
| 20 | 1.57% | 1.57% | freetype | `af_latin_hints_link_segments` | gui_forms_text_engine_benchmark |
| 21 | 1.50% | 46.19% | freetype | `af_autofitter_load_glyph` | gui_forms_text_engine_benchmark |
| 22 | 1.19% | 1.40% | gui_forms | `gui_forms::detail::extended_grapheme_boundaries(std::basic_string_view<char, std::char_traits<char> >, unsigned long)` | gui_forms_text_engine_benchmark |
| 23 | 1.10% | 1.10% | freetype | `FT_DivFix` | gui_forms_text_engine_benchmark |
| 24 | 1.06% | 1.20% | freetype | `af_glyph_hints_align_strong_points` | gui_forms_text_engine_benchmark |
| 25 | 0.95% | 53.96% | freetype | `FT_Load_Glyph` | gui_forms_text_engine_benchmark |
| 26 | 0.86% | 0.89% | freetype | `af_latin_hint_edges` | gui_forms_text_engine_benchmark |
| 27 | 0.83% | 0.83% | freetype | `FT_Outline_Translate` | gui_forms_text_engine_benchmark |
| 28 | 0.82% | 7.32% | other | `load_truetype_glyph` | gui_forms_text_engine_benchmark |
| 29 | 0.65% | 0.72% | other | `SBAlgorithmCreateParagraph` | gui_forms_text_engine_benchmark |
| 30 | 0.59% | 0.72% | freetype | `ft_hash_num_lookup` | gui_forms_text_engine_benchmark |
| 31 | 0.57% | 0.66% | runtime | `malloc` | libc.so.6 |
| 32 | 0.56% | 0.56% | freetype | `af_axis_hints_new_edge` | gui_forms_text_engine_benchmark |
| 33 | 0.46% | 1.50% | other | `apply_forward(OT::hb_ot_apply_context_t*, OT::hb_ot_layout_lookup_accelerator_t const&)` | gui_forms_text_engine_benchmark |
| 34 | 0.45% | 0.79% | harfbuzz | `OT::Layout::GPOS_impl::PairPosFormat2_4<OT::Layout::SmallTypes>::apply(OT::hb_ot_apply_context_t*, void*) const` | gui_forms_text_engine_benchmark |
| 35 | 0.43% | 0.75% | freetype | `TT_Access_Glyph_Frame` | gui_forms_text_engine_benchmark |
| 36 | 0.42% | 0.89% | runtime | `cfree` | libc.so.6 |
| 37 | 0.37% | 10.58% | gui_forms | `gui_forms::render::text::HarfBuzzFontEngine::shape(std::basic_string_view<char, std::char_traits<char> >, gui_forms::FontSpec)` | gui_forms_text_engine_benchmark |
| 38 | 0.34% | 0.34% | freetype | `TT_Load_Glyph_Header` | gui_forms_text_engine_benchmark |
| 39 | 0.32% | 0.33% | freetype | `TT_Process_Simple_Glyph` | gui_forms_text_engine_benchmark |
| 40 | 0.32% | 9.58% | freetype | `tt_glyph_load` | gui_forms_text_engine_benchmark |

### document_view

About 13000 samples.

| Area | Self share |
|---|---:|
| workload | 52.23% |
| gui_forms | 43.46% |
| kernel | 1.97% |
| runtime | 0.33% |
| other | 0.22% |

| # | Self | Inclusive | Area | Function | Object |
|---:|---:|---:|---|---|---|
| 1 | 52.23% | 98.58% | workload | `main` | gui_forms_document_view_benchmark |
| 2 | 29.74% | 29.74% | gui_forms | `gui_forms::validate_utf8(std::basic_string_view<char, std::char_traits<char> >)` | gui_forms_document_view_benchmark |
| 3 | 8.45% | 38.74% | gui_forms | `gui_forms::DocumentViewState::publish(gui_forms::DocumentPage&&)` | gui_forms_document_view_benchmark |
| 4 | 2.71% | 2.71% | gui_forms | `gui_forms::DocumentViewState::display_position(gui_forms::DocumentPageRequest const&, gui_forms::SourceByteOffset) const` | gui_forms_document_view_benchmark |
| 5 | 2.56% | 2.56% | gui_forms | `gui_forms::DocumentViewState::source_position(gui_forms::DocumentPageRequest const&, gui_forms::DisplayByteOffset) const` | gui_forms_document_view_benchmark |
| 6 | 0.48% | 2.14% | kernel | `do_user_addr_fault` | [kernel.kallsyms] |
| 7 | 0.31% | 0.31% | kernel | `_raw_spin_unlock_irqrestore` | [kernel.kallsyms] |
| 8 | 0.26% | 0.26% | runtime | `0x000000000000fa22` | ld-linux-x86-64.so.2 |
| 9 | 0.25% | 0.25% | kernel | `clear_page_erms` | [kernel.kallsyms] |
| 10 | 0.20% | 0.28% | kernel | `zap_present_ptes.constprop.0` | [kernel.kallsyms] |
| 11 | 0.15% | 0.15% | other | `0x0000000000000bfb` | [vdso] |
| 12 | 0.11% | 1.27% | kernel | `do_anonymous_page` | [kernel.kallsyms] |
| 13 | 0.09% | 0.10% | kernel | `free_unref_folios` | [kernel.kallsyms] |
| 14 | 0.09% | 1.63% | kernel | `handle_mm_fault` | [kernel.kallsyms] |
| 15 | 0.09% | 1.44% | kernel | `handle_pte_fault` | [kernel.kallsyms] |
| 16 | 0.08% | 0.63% | kernel | `alloc_pages_mpol` | [kernel.kallsyms] |
| 17 | 0.08% | 0.08% | kernel | `queued_spin_unlock` | [kernel.kallsyms] |
| 18 | 0.07% | 0.07% | other | `void std::__introsort_loop<__gnu_cxx::__normal_iterator<double*, std::vector<double, std::allocator<double> > >, long, __gnu_cxx::__ops::_Iter_less_iter>(__g...` | gui_forms_document_view_benchmark |
| 19 | 0.07% | 0.07% | runtime | `0x000000000000a87f` | ld-linux-x86-64.so.2 |
| 20 | 0.07% | 0.12% | kernel | `__lruvec_stat_mod_folio` | [kernel.kallsyms] |
| 21 | 0.06% | 0.77% | kernel | `alloc_anon_folio` | [kernel.kallsyms] |
| 22 | 0.06% | 0.48% | kernel | `get_page_from_freelist` | [kernel.kallsyms] |
