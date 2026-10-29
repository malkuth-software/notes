// SCRATCH: why does Arabic glyph 169 fail to render through the slot path?
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>
int main(int, char** argv) {
    std::ifstream stream(argv[1], std::ios::binary);
    std::vector<char> bytes((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    FT_Library library; FT_Init_FreeType(&library);
    FT_Face face; FT_New_Memory_Face(library, (const FT_Byte*)bytes.data(), (FT_Long)bytes.size(), 0, &face);
    FT_Set_Char_Size(face, 0, 13 * 64, 72, 72);
    const FT_UInt id = 169;
    FT_Matrix m{98304, -19661, 0, 98304};
    for (int delta_x = 0; delta_x < 64; delta_x += 21) {
        FT_Vector d{delta_x, -19};
        FT_Set_Transform(face, &m, &d);
        FT_Error load = FT_Load_Glyph(face, id, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING);
        FT_BBox box; FT_Outline_Get_CBox(&face->glyph->outline, &box);
        std::printf("slot dx %d load %d points %d contours %d flags %x format %x cbox %ld %ld %ld %ld\n", delta_x, load,
                    face->glyph->outline.n_points, face->glyph->outline.n_contours, face->glyph->outline.flags,
                    (unsigned)face->glyph->format, box.xMin, box.yMin, box.xMax, box.yMax);
        FT_Error render = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        std::printf("  render %d bitmap %ux%u\n", render, face->glyph->bitmap.width, face->glyph->bitmap.rows);
        FT_Set_Transform(face, nullptr, nullptr);
        FT_Load_Glyph(face, id, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING);
        FT_Glyph g; FT_Get_Glyph(face->glyph, &g);
        FT_Glyph_Transform(g, &m, &d);
        FT_Glyph b = g;
        FT_Error to = FT_Glyph_To_Bitmap(&b, FT_RENDER_MODE_NORMAL, nullptr, 0);
        std::printf("  glyph path %d format %x\n", to, (unsigned)b->format);
    }
    return 0;
}
