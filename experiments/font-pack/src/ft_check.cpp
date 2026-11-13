// font-pack experiment: FreeType smoke test.
// Opens a pack as an application would (eager groups first, then every
// lazy group on demand), opens every font straight from the loader's memory
// with FT_New_Memory_Face, then loads a sample of glyphs from each.
//   ft_check <in.fpk>
#include "fp_load.hpp"

#include <chrono>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

static bool read_file(const char* path, fp::Bytes& bytes) {
    std::FILE* file = std::fopen(path, "rb");
    if (file == nullptr) {
        return false;
    }
    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    bytes.resize(static_cast<std::size_t>(size));
    const std::size_t got = std::fread(bytes.data(), 1, bytes.size(), file);
    std::fclose(file);
    return got == bytes.size();
}

static double seconds_now() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::printf("usage: ft_check <in.fpk>\n");
        return 2;
    }
    fp::Bytes pack;
    if (!read_file(argv[1], pack)) {
        std::printf("cannot read %s\n", argv[1]);
        return 1;
    }
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0) {
        std::printf("FreeType does not start\n");
        return 1;
    }
    const double t0 = seconds_now();
    fp::LoadedSet* set = new fp::LoadedSet;
    if (!fp::set_open(pack.data(), pack.size(), 0, *set)) {
        std::printf("the pack does not open, or an eager group is refused\n");
        delete set;
        return 1;
    }
    const double t_eager = seconds_now();
    std::printf("eager groups %.2f ms\n", (t_eager - t0) * 1e3);
    for (std::uint32_t g = 0; g < (*set).index.group_count; g += 1) {
        const double g0 = seconds_now();
        const bool was_loaded = (*set).loaded[g];
        if (!fp::set_need_group(*set, g)) {
            std::printf("group %.*s refused (reason %u)\n", static_cast<int>((*set).index.groups[g].name_length),
                        (*set).index.groups[g].name, static_cast<unsigned>((*set).groups[g].refusal));
            fp::set_free(*set);
            delete set;
            return 1;
        }
        std::printf("group %-12.*s %s %.2f ms\n", static_cast<int>((*set).index.groups[g].name_length),
                    (*set).index.groups[g].name, was_loaded ? "eager, already loaded" : "lazy, first need",
                    (seconds_now() - g0) * 1e3);
    }
    const double t1 = seconds_now();
    // Every font of every group, in index order.
    std::vector<const fp::LoadedFont*> fonts;
    for (std::uint32_t g = 0; g < (*set).index.group_count; g += 1) {
        for (std::uint32_t f = 0; f < (*set).groups[g].font_count; f += 1) {
            fonts.push_back(&(*set).groups[g].fonts[f]);
        }
    }
    // set_find_font must hand back the same memory without decoding again.
    for (std::size_t k = 0; k < fonts.size(); k += 1) {
        if (fp::set_find_font(*set, (*fonts[k]).name, (*fonts[k]).name_length) != fonts[k]) {
            std::printf("set_find_font disagrees for %.*s\n", static_cast<int>((*fonts[k]).name_length), (*fonts[k]).name);
            fp::set_free(*set);
            delete set;
            return 1;
        }
    }
    const std::uint32_t font_count = static_cast<std::uint32_t>(fonts.size());
    std::vector<FT_Face> faces(font_count);
    int failures = 0;
    for (std::uint32_t f = 0; f < font_count; f += 1) {
        if (FT_New_Memory_Face(library, (*fonts[f]).data, static_cast<FT_Long>((*fonts[f]).size), 0, &faces[f]) != 0) {
            faces[f] = nullptr;
            failures += 1;
        }
    }
    const double t2 = seconds_now();
    long glyphs_loaded = 0;
    for (std::uint32_t f = 0; f < font_count; f += 1) {
        if (faces[f] == nullptr) {
            std::printf("  %.*s: FreeType refuses it\n", static_cast<int>((*fonts[f]).name_length), (*fonts[f]).name);
            continue;
        }
        const long glyph_count = (*faces[f]).num_glyphs;
        const long step = glyph_count > 400 ? glyph_count / 400 : 1;
        long bad = 0;
        long tried = 0;
        for (long g = 0; g < glyph_count; g += step) {
            tried += 1;
            if (FT_Load_Glyph(faces[f], static_cast<FT_UInt>(g), FT_LOAD_NO_SCALE | FT_LOAD_NO_BITMAP) != 0) {
                bad += 1;
            }
        }
        glyphs_loaded += tried - bad;
        if (bad != 0) {
            failures += 1;
        }
        std::printf("  %-34.*s %-22s %-12s %6ld glyphs, %ld of %ld sampled glyphs load\n",
                    static_cast<int>((*fonts[f]).name_length), (*fonts[f]).name,
                    (*faces[f]).family_name != nullptr ? (*faces[f]).family_name : "?",
                    (*faces[f]).style_name != nullptr ? (*faces[f]).style_name : "?", glyph_count, tried - bad, tried);
    }
    for (std::uint32_t f = 0; f < font_count; f += 1) {
        if (faces[f] != nullptr) {
            FT_Done_Face(faces[f]);
        }
    }
    FT_Done_FreeType(library);
    std::printf("pack to fonts %.2f ms, FT_New_Memory_Face for %u fonts %.2f ms, %ld sampled glyphs loaded, %d failures\n",
                (t1 - t0) * 1e3, font_count, (t2 - t1) * 1e3, glyphs_loaded, failures);
    fp::set_free(*set);
    delete set;
    return failures == 0 ? 0 : 1;
}
