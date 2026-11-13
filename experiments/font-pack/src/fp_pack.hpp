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

// Packs the fonts into out. verbose prints each stream's raw and coded size.
bool pack_fonts(const std::vector<PackInput>& fonts, Bytes& out, bool verbose);

}  // namespace fp

#endif
