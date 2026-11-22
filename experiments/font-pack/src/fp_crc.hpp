// font-pack experiment: CRC-32 check values for the fonts in a pack.
//
// STAND-IN for stx_zlib::crc32. Same polynomial and conventions as zlib, PNG
// and stx_zlib (reflected 0xEDB88320, initial and final XOR 0xFFFFFFFF), so a
// value written here equals stx_zlib::crc32 over the same bytes; stage 3
// replaces this file with calls into stx_zlib. Check value: crc32(0,
// "123456789", 9) == 0xCBF43926 (fontpack claims tests it).
//
// crc32 takes and returns finished values, like zlib: start with 0, feed
// the previous result back in to continue.
#ifndef FP_CRC_HPP
#define FP_CRC_HPP

#include <cstddef>
#include <cstdint>

namespace fp {

enum : std::uint32_t { kCrcPolynomial = 0xEDB88320u };

// Slicing-by-8 tables: row 0 is the ordinary byte table; row k advances a
// byte's contribution by k further zero bytes, so eight bytes are folded in
// with eight lookups and no loop-carried dependency between them.
struct CrcTables {
    std::uint32_t row[8][256];
};

inline CrcTables crc_tables_make() {
    CrcTables tables;
    for (std::uint32_t n = 0; n < 256; n += 1) {
        std::uint32_t c = n;
        for (std::uint32_t bit = 0; bit < 8; bit += 1) {
            c = (c & 1u) != 0 ? (c >> 1) ^ kCrcPolynomial : c >> 1;
        }
        tables.row[0][n] = c;
    }
    for (std::uint32_t n = 0; n < 256; n += 1) {
        std::uint32_t c = tables.row[0][n];
        for (std::uint32_t k = 1; k < 8; k += 1) {
            c = tables.row[0][c & 0xFFu] ^ (c >> 8);
            tables.row[k][n] = c;
        }
    }
    return tables;
}

// Built once, on first use (a function-local static is initialised once even
// with several threads asking at the same time).
inline const CrcTables& crc_tables() {
    static const CrcTables tables = crc_tables_make();
    return tables;
}

inline std::uint32_t crc32(std::uint32_t crc, const std::uint8_t* data, std::size_t size) {
    const CrcTables& t = crc_tables();
    std::uint32_t c = ~crc;
    // Eight bytes per step; no alignment is needed because bytes are read singly.
    while (size >= 8) {
        // The low four bytes are XORed into the register; the high four are
        // looked up directly. Byte order is spelled out, so this is the same
        // on any host.
        const std::uint32_t low = c ^ (static_cast<std::uint32_t>(data[0]) | static_cast<std::uint32_t>(data[1]) << 8 |
                                       static_cast<std::uint32_t>(data[2]) << 16 | static_cast<std::uint32_t>(data[3]) << 24);
        c = t.row[7][low & 0xFFu] ^ t.row[6][(low >> 8) & 0xFFu] ^ t.row[5][(low >> 16) & 0xFFu] ^ t.row[4][low >> 24] ^
            t.row[3][data[4]] ^ t.row[2][data[5]] ^ t.row[1][data[6]] ^ t.row[0][data[7]];
        data += 8;
        size -= 8;
    }
    // Tail.
    while (size != 0) {
        c = t.row[0][(c ^ *data) & 0xFFu] ^ (c >> 8);
        data += 1;
        size -= 1;
    }
    return ~c;
}

// Polynomial product a * b modulo the CRC polynomial, both in the reflected
// bit order (bit 31 is x^0). As in zlib's multmodp.
inline std::uint32_t crc_multiply(std::uint32_t a, std::uint32_t b) {
    std::uint32_t m = 1u << 31;
    std::uint32_t product = 0;
    for (;;) {
        if ((a & m) != 0) {
            product ^= b;
            if ((a & (m - 1)) == 0) {
                break;
            }
        }
        m >>= 1;
        b = (b & 1u) != 0 ? (b >> 1) ^ kCrcPolynomial : b >> 1;
    }
    return product;
}

// x^(8 * bytes) modulo the polynomial: the operator that advances a CRC
// register over that many zero bytes. power[k] holds x^(2^k), so the loop
// multiplies in x^(2^k) for every set bit k of 8 * bytes, starting at k = 3
// because the factor 8 is three doublings.
inline std::uint32_t crc_zeros_operator(std::uint64_t bytes) {
    std::uint32_t power[64];
    power[0] = 1u << 30;   // x^1
    for (std::uint32_t k = 1; k < 64; k += 1) {
        power[k] = crc_multiply(power[k - 1], power[k - 1]);
    }
    std::uint32_t result = 1u << 31;   // x^0
    std::uint32_t k = 3;
    // bytes below 2^60 keeps k inside power[]; the loader passes at most kCheckPieceBytes.
    while (bytes != 0 && k < 64) {
        if ((bytes & 1u) != 0) {
            result = crc_multiply(power[k], result);
        }
        bytes >>= 1;
        k += 1;
    }
    return result;
}

// CRC of A followed by B from crc(A), crc(B) and B's length, as zlib's
// crc32_combine. The loader checks large fonts in pieces on several threads
// and combines the pieces' values.
inline std::uint32_t crc32_combine(std::uint32_t crc_a, std::uint32_t crc_b, std::uint64_t size_b) {
    return crc_multiply(crc_zeros_operator(size_b), crc_a) ^ crc_b;
}

}  // namespace fp

#endif
