// font-pack experiment: loader interface.
//
// pack_open reads a pack's group and font table and checks every size claim
// in it; it allocates nothing. pack_load_group then turns one group into its
// original font files, byte for byte, in one allocation the caller owns.
// Each LoadedFont's bytes can be handed straight to FT_New_Memory_Face; they
// stay valid until group_free.
//
// LoadedSet is the application's view: it decodes the eager groups when it
// opens and every other group the first time one of its fonts is asked for.
// A group is decoded at most once.
#ifndef FP_LOAD_HPP
#define FP_LOAD_HPP

#include "fp_format.hpp"

namespace fp {

// One font as the container describes it. Names borrow from the pack.
struct PackFont {
    const char* name;            // not terminated
    std::uint32_t name_length;
    std::uint32_t size;
    std::uint32_t crc;           // CRC-32 of the original file
    std::uint32_t group;
};

struct PackGroup {
    const char* name;            // not terminated; borrows from the pack
    std::uint32_t name_length;
    bool eager;
    std::uint32_t font_first;    // into PackIndex::fonts
    std::uint32_t font_count;
    std::uint64_t font_bytes;    // checked: the sum of the fonts' sizes
    std::uint64_t stream_bytes;  // checked against the body before any stream is opened
    const std::uint8_t* body;    // followed by kStreamPadding readable bytes
    std::size_t body_size;
};

// Fixed capacity, so that opening a pack allocates nothing whatever it claims.
struct PackIndex {
    const std::uint8_t* pack;
    std::size_t pack_size;
    PackGroup groups[kMaxGroups];
    std::uint32_t group_count;
    PackFont fonts[kMaxFonts];
    std::uint32_t font_count;
};

// Reads and checks the container. pack must stay valid while the index or
// anything loaded from it is in use. On failure index is empty.
bool pack_open(const std::uint8_t* pack, std::size_t pack_size, PackIndex& index);

struct LoadedFont {
    const char* name;            // not terminated; borrows from the pack
    std::uint32_t name_length;
    const std::uint8_t* data;    // borrows from LoadedGroup::storage
    std::size_t size;
};

struct LoadTimings {
    double open_seconds;         // body header, size checks, stream tables and buffers
    double streams_seconds;      // leading streams and the rebuild plan
    double dependent_seconds;    // every other stream
    double rebuild_seconds;      // fonts written out
    double check_seconds;        // CRC-32 of every rebuilt font
    double total_seconds;
};

// Why a group did not load. A refusal leaves the LoadedGroup empty.
enum GroupRefusal : std::uint32_t {
    kGroupLoaded = 0,
    kGroupBadIndex,              // no such group, or an index that did not come from pack_open
    kGroupBadClaims,             // a size in the body disagrees with the container's claims or limits
    kGroupBadStreams,            // a stream or rebuild step found damage
    kGroupBadCheck,              // a rebuilt font's CRC-32 differs from the stored one
    kGroupNoMemory
};

struct LoadedGroup {
    std::uint32_t group;
    std::uint8_t* storage;       // every font's bytes, owned
    std::size_t storage_bytes;   // sum of the font sizes (each rounded up to 8)
    LoadedFont* fonts;           // owned; the group's fonts in index order
    std::uint32_t font_count;
    GroupRefusal refusal;
    // Kept on refusal too, so a test can see how far a damaged claim got.
    std::size_t scratch_peak;        // most temporary memory held at once while loading
    std::size_t storage_requested;   // bytes asked for font storage; 0 if refused before that
    LoadTimings timings;
};

// Decodes one group. threads 0 picks the hardware concurrency.
bool pack_load_group(const PackIndex& index, std::uint32_t group, std::uint32_t threads, LoadedGroup& out);

void group_free(LoadedGroup& group);

// Lazy loading over one pack. Not safe for concurrent use: the caller keeps
// it on one execution context (the stage 3 version runs decodes on the
// library's thread pool and owns that rule itself).
struct LoadedSet {
    PackIndex index;
    LoadedGroup groups[kMaxGroups];
    bool loaded[kMaxGroups];
    bool refused[kMaxGroups];    // a refused group is not retried: the pack does not change
    std::uint32_t threads;
};

// Opens the index and decodes every eager group. Fails if any eager group
// is refused; the set is then empty.
bool set_open(const std::uint8_t* pack, std::size_t pack_size, std::uint32_t threads, LoadedSet& set);

// Decodes group if it is not loaded yet. Returns whether it is loaded.
bool set_need_group(LoadedSet& set, std::uint32_t group);

// The font named name (first match in index order), decoding its group on
// first need; null if there is no such font or its group is refused.
const LoadedFont* set_find_font(LoadedSet& set, const char* name, std::uint32_t name_length);

void set_free(LoadedSet& set);

}  // namespace fp

#endif
