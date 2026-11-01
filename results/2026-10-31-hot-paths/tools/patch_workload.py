# SCRATCH: add a pixel dump and a band-invariance check to a copy of
# benchmarks/paint_profile_workload.cpp. Usage: python3 patch_workload.py <file>
import sys
from pathlib import Path
path = Path(sys.argv[1])
text = path.read_text()
helper = r'''
// SCRATCH probe (AAA clip invariance): with GUI_FORMS_PAINT_DUMP_DIR set, each
// case writes its raster and then repaints the frame whole and in uneven row
// bands, recording both hashes.
void probe_case(SkiaRaster& raster, Window& window, Size size, double scale, const std::string& name) {
    const char* directory = std::getenv("GUI_FORMS_PAINT_DUMP_DIR");
    if (directory == nullptr) return;
    const std::filesystem::path base(directory);
    {
        std::ofstream dump(base / (name + ".raw"), std::ios::binary);
        dump << raster.pixel_width() << ' ' << raster.pixel_height() << ' ' << raster.row_bytes() << '\n';
        dump.write(static_cast<const char*>(raster.pixels()), static_cast<std::streamsize>(raster.byte_size()));
    }
    const std::uint64_t measured = pixel_hash(raster);
    paint_once(raster, window, {0.0, 0.0, size.width, size.height}, scale);
    const std::uint64_t whole = pixel_hash(raster);
    constexpr std::array<double, 8> cuts{0.0, 0.131, 0.293, 0.417, 0.583, 0.671, 0.829, 1.0};
    std::size_t index = 0;
    for (index = 0; index + 1 < cuts.size(); ++index) {
        const double top = size.height * cuts[index];
        const double bottom = size.height * cuts[index + 1];
        paint_once(raster, window, {0.0, top, size.width, bottom - top}, scale);
    }
    const std::uint64_t banded = pixel_hash(raster);
    paint_once(raster, window, {0.0, 0.0, size.width, size.height}, scale);
    for (index = 0; index + 1 < cuts.size(); ++index) {
        const double left = size.width * cuts[index];
        const double right = size.width * cuts[index + 1];
        paint_once(raster, window, {left, 0.0, right - left, size.height}, scale);
    }
    const std::uint64_t columns = pixel_hash(raster);
    if (columns != whole) {
        std::ofstream dump(base / (name + ".columns.raw"), std::ios::binary);
        dump << raster.pixel_width() << ' ' << raster.pixel_height() << ' ' << raster.row_bytes() << '\n';
        dump.write(static_cast<const char*>(raster.pixels()), static_cast<std::streamsize>(raster.byte_size()));
    }
    paint_once(raster, window, {0.0, 0.0, size.width, size.height}, scale);
    std::ofstream probe(base / "probe.txt", std::ios::app);
    probe << name << ' ' << std::hex << measured << ' ' << whole << ' ' << banded << std::dec
          << " measured_eq_whole=" << (measured == whole) << " banded_eq_whole=" << (banded == whole)
          << " columns_eq_whole=" << (columns == whole) << '\n';
}

// SCRATCH probe (AAA cost): a rounded fill and stroke `height` logical pixels tall,
// centred on the raster, painted under a 100-row damage band; ns per frame.
void probe_tall(SkiaRaster& raster, Window& window, double height, const char* label) {
    static_cast<void>(window);
    const Rect band{0.0, 300.0, damage_window_size.width, 100.0};
    const Rect shape{50.0, 310.0 - height / 2.0, 800.0, height};
    const gui_forms::Color color = gui_forms::Color::rgba(40, 90, 200, 255);
    constexpr std::size_t frames = 400;
    std::size_t frame = 0;
    std::uint64_t best = ~std::uint64_t{0};
    std::size_t round = 0;
    for (round = 0; round < 5; ++round) {
        const Clock::time_point started = Clock::now();
        for (frame = 0; frame < frames; ++frame) {
            DamageRegion region;
            region.add(band);
            raster.begin_frame(region);
            raster.fill_rounded_rect(shape, 12.0, color);
            raster.stroke_rounded_rect(shape, 12.0, color, 2.0);
            raster.end_frame();
        }
        const Clock::duration elapsed = Clock::now() - started;
        const std::uint64_t ns = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count()) / frames;
        best = std::min(best, ns);
    }
    std::cout << "TALL " << label << " height=" << height << " best_ns_per_frame=" << best << '\n';
}
'''
anchor = 'std::string case_record('
assert anchor in text
text = text.replace(anchor, helper + '\n' + anchor, 1)
old_d = 'write_record(records, case_record("damage", page, scale, percent, damage_measured_frames, result));'
assert old_d in text
text = text.replace(old_d, old_d + '\n            probe_case(raster, window, damage_window_size, scale, "damage_" + std::to_string(static_cast<int>(scale)) + "_" + std::to_string(static_cast<int>(percent)));', 1)
old_p = 'write_record(records, case_record("pages", spec.id, scale, 100.0, page_measured_frames, result));'
assert old_p in text
text = text.replace(old_p, old_p + '\n            probe_case(raster, window, size, scale, "pages_" + std::to_string(static_cast<int>(scale)) + "_" + std::string(spec.id));', 1)
old_m = '        run_damage_phase(raster, *gallery, records);'
assert old_m in text
text = text.replace(old_m, '''        if (std::getenv("GUI_FORMS_PAINT_TALL") != nullptr) {
            prepare_scale(raster, *gallery, damage_window_size, 1.0);
            probe_tall(raster, *gallery, 600.0, "short");
            probe_tall(raster, *gallery, 6000.0, "medium");
            probe_tall(raster, *gallery, 30000.0, "tall");
            probe_tall(raster, *gallery, 200000.0, "huge");
            return 0;
        }
''' + old_m, 1)
if '#include <cstdlib>' not in text:
    text = text.replace('#include', '#include <cstdlib>\n#include', 1)
path.write_text(text)
