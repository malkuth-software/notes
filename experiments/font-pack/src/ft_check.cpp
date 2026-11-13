// font-pack experiment: FreeType smoke test.
// Loads a pack and opens every font straight from the loader's memory with
// FT_New_Memory_Face, then loads a sample of glyphs from each.
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
    fp::LoadedPack loaded;
    if (!fp::pack_load(pack.data(), pack.size(), 0, loaded)) {
        std::printf("the pack does not load\n");
        return 1;
    }
    const double t1 = seconds_now();
    std::vector<FT_Face> faces(loaded.font_count);
    int failures = 0;
    for (std::uint32_t f = 0; f < loaded.font_count; f += 1) {
        if (FT_New_Memory_Face(library, loaded.fonts[f].data, static_cast<FT_Long>(loaded.fonts[f].size), 0, &faces[f]) != 0) {
            faces[f] = nullptr;
            failures += 1;
        }
    }
    const double t2 = seconds_now();
    long glyphs_loaded = 0;
    for (std::uint32_t f = 0; f < loaded.font_count; f += 1) {
        if (faces[f] == nullptr) {
            std::printf("  %.*s: FreeType refuses it\n", static_cast<int>(loaded.fonts[f].name_length), loaded.fonts[f].name);
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
                    static_cast<int>(loaded.fonts[f].name_length), loaded.fonts[f].name,
                    (*faces[f]).family_name != nullptr ? (*faces[f]).family_name : "?",
                    (*faces[f]).style_name != nullptr ? (*faces[f]).style_name : "?", glyph_count, tried - bad, tried);
    }
    for (std::uint32_t f = 0; f < loaded.font_count; f += 1) {
        if (faces[f] != nullptr) {
            FT_Done_Face(faces[f]);
        }
    }
    FT_Done_FreeType(library);
    std::printf("pack to fonts %.2f ms, FT_New_Memory_Face for %u fonts %.2f ms, %ld sampled glyphs loaded, %d failures\n",
                (t1 - t0) * 1e3, loaded.font_count, (t2 - t1) * 1e3, glyphs_loaded, failures);
    fp::pack_free(loaded);
    return failures == 0 ? 0 : 1;
}
