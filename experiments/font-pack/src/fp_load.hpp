// font-pack experiment: loader interface.
//
// pack_load turns a pack into the original font files, byte for byte, in one
// allocation the caller owns. Each LoadedFont's bytes can be handed straight
// to FT_New_Memory_Face; they stay valid until pack_free.
#ifndef FP_LOAD_HPP
#define FP_LOAD_HPP

#include "fp_base.hpp"

namespace fp {

struct LoadedFont {
    const char* name;            // not terminated; borrows from the pack
    std::uint32_t name_length;
    const std::uint8_t* data;    // borrows from LoadedPack::storage
    std::size_t size;
};

struct LoadTimings {
    double open_seconds;         // header parse, tables, buffers
    double streams_seconds;      // leading streams and the rebuild plan
    double dependent_seconds;    // every other stream
    double rebuild_seconds;      // fonts written out
    double total_seconds;
};

struct LoadedPack {
    std::uint8_t* storage;       // every font's bytes, owned
    std::size_t storage_bytes;   // sum of the font sizes (each rounded up to 8)
    LoadedFont* fonts;           // owned
    std::uint32_t font_count;
    std::size_t scratch_peak;    // most temporary memory held at once while loading
    LoadTimings timings;
};

// pack must stay valid until pack_free (font names borrow from it).
// threads 0 picks the hardware concurrency. On failure out is empty.
bool pack_load(const std::uint8_t* pack, std::size_t pack_size, std::uint32_t threads, LoadedPack& out);

void pack_free(LoadedPack& pack);

}  // namespace fp

#endif
