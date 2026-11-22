// font-pack experiment: packer interface.
#ifndef FP_PACK_HPP
#define FP_PACK_HPP

#include "fp_format.hpp"

#include <string>

namespace fp {

struct PackInput {
    std::string name;
    Bytes data;
};

// A group the packer is asked to build: fonts by index into the input list.
struct PackGroupSpec {
    std::string name;
    bool eager;
    std::vector<std::uint32_t> fonts;
};

// Reads a group manifest and places every font in the first group, in
// manifest order, that has a pattern (fnmatch) matching its file name:
//
//   # comment
//   group interface eager
//   Portsmouth*.ttf
//   group cjk lazy
//   NotoSansCJK*
//
// Fails with a message if a line is malformed, a font matches no group or
// a group matches no font.
bool manifest_groups(const std::string& text, const std::vector<PackInput>& fonts, std::vector<PackGroupSpec>& groups,
                     std::string& error);

// One group exactly as the container records it (fp_format.hpp).
struct ContainerFont {
    std::string name;
    std::uint64_t size;
    std::uint32_t crc;
};

struct ContainerGroup {
    std::string name;
    bool eager;
    std::vector<ContainerFont> fonts;
    std::uint64_t font_bytes;
    std::uint64_t stream_bytes;
    Bytes body;
};

// Writes the container as given, without checks: fontpack claims uses it to
// write packs whose claims are deliberately wrong.
void container_write(const std::vector<ContainerGroup>& groups, Bytes& out);

// Packs every group (each its own set of streams) into out. Fails, with a
// message, when a group would break one of the loader's limits, so every
// pack this writes loads. verbose prints each stream's raw and coded size.
bool pack_fonts(const std::vector<PackInput>& fonts, const std::vector<PackGroupSpec>& groups, Bytes& out, bool verbose);

}  // namespace fp

#endif
