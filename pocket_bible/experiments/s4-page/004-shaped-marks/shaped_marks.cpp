// The shaped test Stage 1 could not run (sources/MEASUREMENTS.md section 3):
// every base-plus-marks sequence of the Hebrew and the Greek, shaped with
// HarfBuzz in each face, with any missing glyph, unplaced mark or collision
// reported. See README.md beside this file for the procedure and results.
//
//   shaped_marks hebrew --face NAME=FILE [--face ...] --text TAHOT_FILE [--text ...] [--details FILE]
//   shaped_marks greek  --face NAME=FILE [--face ...] --text TAGNT_FILE [--text ...] [--details FILE]
//   shaped_marks glyph  --face NAME=FILE --code HEX [--code ...]
//
// A sequence is a code point that is not a combining mark followed by the
// combining marks directly after it, exactly as Stage 1's text_inventory
// counts them, read from the same word rows of STEPBible's files. For Greek
// every distinct word is shaped as well.
//
// What is tested, per sequence and face, at an em of 1000 units:
//   missing    a glyph the face does not have (glyph 0).
//   unplaced   a mark the face's own anchors did not move: its offsets are
//              the same with the 'mark' and 'mkmk' features on and off. A
//              mark the face composes with its base into one glyph is not a
//              separate glyph and is not counted.
//   collision  two marks of one sequence whose ink overlaps: both glyphs are
//              rasterized at 128 pixels to the em where the shaper puts them
//              and at least three pixels are more than half covered by both.
//   on base    a mark whose ink overlaps its base's ink in the same sense.
//   detached   a mark whose ink box lies wholly more than a tenth of an em
//              to one side of the width its base stands on.
// No scripture text is printed: sequences are reported as code point
// numbers.

#include "core/file_io.hpp"
#include "page/font_set.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <map>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using bible::page::FaceId;
using bible::page::FontSet;
using bible::page::GlyphBox;
using bible::page::GlyphCoverage;
using bible::page::LayoutUnit;
using bible::page::ShapedGlyph;
using bible::page::ShapedRun;
using bible::page::ShapeFeatures;
using bible::page::TextScript;

constexpr LayoutUnit test_em = 1000;
// Ink is compared at this many pixels to the em.
constexpr std::int32_t ink_pixels_per_em = 128;
constexpr std::uint32_t ink_overlap_pixels = 3U;
constexpr std::uint64_t file_maximum_bytes = 256U * 1024U * 1024U;

struct NamedFace final {
    std::string name{};
    std::string path{};
};

struct Arguments final {
    std::string mode{};
    std::vector<NamedFace> faces{};
    std::vector<std::string> texts{};
    std::vector<char32_t> codes{};
    std::string details{};
};

void fail(const std::string& message) {
    throw std::runtime_error(message);
}

// Stage 1's definition (sources/audit/audit_common.hpp:156-178), restated:
// the combining ranges the admitted texts use, with the Hebrew spacing marks
// (maqaf, paseq, sof pasuq, nun hafukha) left out.
bool is_combining_mark(char32_t code_point) noexcept {
    if (code_point >= 0x0300U && code_point <= 0x036fU) {
        return true;
    }
    if (code_point >= 0x0591U && code_point <= 0x05c7U) {
        const bool spacing =
            code_point == 0x05beU || code_point == 0x05c0U || code_point == 0x05c3U || code_point == 0x05c6U;
        return !spacing;
    }
    const bool other = (code_point >= 0x1ab0U && code_point <= 0x1affU) ||
                       (code_point >= 0x1dc0U && code_point <= 0x1dffU) ||
                       (code_point >= 0x20d0U && code_point <= 0x20ffU) || (code_point >= 0xfe20U && code_point <= 0xfe2fU);
    return other;
}

std::u32string decode(std::string_view text) {
    std::u32string code_points;
    std::size_t index = 0U;
    while (index < text.size()) {
        const unsigned char lead = static_cast<unsigned char>(text[index]);
        std::size_t length = 1U;
        char32_t value = lead;
        if ((lead & 0xe0U) == 0xc0U) {
            length = 2U;
            value = lead & 0x1fU;
        } else if ((lead & 0xf0U) == 0xe0U) {
            length = 3U;
            value = lead & 0x0fU;
        } else if ((lead & 0xf8U) == 0xf0U) {
            length = 4U;
            value = lead & 0x07U;
        }
        if (index + length > text.size()) {
            break;
        }
        for (std::size_t offset = 1U; offset < length; ++offset) {
            value = (value << 6U) | (static_cast<unsigned char>(text[index + offset]) & 0x3fU);
        }
        code_points.push_back(value);
        index += length;
    }
    return code_points;
}

void append_utf8(std::string& text, char32_t code_point) {
    if (code_point < 0x80U) {
        text.push_back(static_cast<char>(code_point));
    } else if (code_point < 0x800U) {
        text.push_back(static_cast<char>(0xc0U | (code_point >> 6U)));
        text.push_back(static_cast<char>(0x80U | (code_point & 0x3fU)));
    } else if (code_point < 0x10000U) {
        text.push_back(static_cast<char>(0xe0U | (code_point >> 12U)));
        text.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3fU)));
        text.push_back(static_cast<char>(0x80U | (code_point & 0x3fU)));
    } else {
        text.push_back(static_cast<char>(0xf0U | (code_point >> 18U)));
        text.push_back(static_cast<char>(0x80U | ((code_point >> 12U) & 0x3fU)));
        text.push_back(static_cast<char>(0x80U | ((code_point >> 6U) & 0x3fU)));
        text.push_back(static_cast<char>(0x80U | (code_point & 0x3fU)));
    }
}

std::string encode(const std::u32string& code_points) {
    std::string text;
    for (const char32_t code_point : code_points) {
        append_utf8(text, code_point);
    }
    return text;
}

std::string code_point_list(const std::u32string& code_points) {
    static const char digits[] = "0123456789ABCDEF";
    std::string text;
    for (const char32_t code_point : code_points) {
        if (!text.empty()) {
            text.push_back(' ');
        }
        text += "U+";
        for (int shift = 12; shift >= 0; shift -= 4) {
            text.push_back(digits[(code_point >> static_cast<unsigned>(shift)) & 0xfU]);
        }
    }
    return text;
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::vector<std::uint8_t> bytes;
    const bible::core::FileReadStatus status =
        bible::core::read_file_bounded(std::filesystem::path(path), file_maximum_bytes, bytes);
    if (status != bible::core::FileReadStatus::ok) {
        fail("cannot read " + path + ": " + std::string(bible::core::file_read_status_name(status)));
    }
    return bytes;
}

// What the texts hold: each distinct sequence and, for Greek, each distinct
// word, with how often it occurs.
struct Inventory final {
    std::map<std::u32string, std::uint64_t> sequences{};
    std::map<std::string, std::uint64_t> words{};
};

void add_word(const std::string& word, Inventory& inventory) {
    inventory.words[word] += 1U;
    const std::u32string code_points = decode(word);
    std::size_t index = 0U;
    while (index < code_points.size()) {
        if (is_combining_mark(code_points[index])) {
            // A mark with no base before it: Stage 1 records it on a dotted
            // circle; it cannot be shaped against a base and is skipped.
            ++index;
            continue;
        }
        std::u32string sequence;
        sequence.push_back(code_points[index]);
        std::size_t next = index + 1U;
        while (next < code_points.size() && is_combining_mark(code_points[next])) {
            sequence.push_back(code_points[next]);
            ++next;
        }
        if (sequence.size() > 1U) {
            inventory.sequences[sequence] += 1U;
        }
        index = next;
    }
}

// The word rows of a STEPBible amalgamated text: lines whose first field is
// a reference with '#', as Stage 1 reads them.
void add_step_file(const std::string& path, bool hebrew, Inventory& inventory) {
    const std::vector<std::uint8_t> bytes = read_file(path);
    const std::string_view text(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    std::size_t position = 0U;
    while (position < text.size()) {
        const std::size_t newline = text.find('\n', position);
        const std::size_t end = newline == std::string_view::npos ? text.size() : newline;
        const std::string_view line = text.substr(position, end - position);
        position = newline == std::string_view::npos ? text.size() : newline + 1U;
        if (line.size() < 8U || line[3] != '.') {
            continue;
        }
        const std::size_t first_tab = line.find('\t');
        const std::size_t hash = line.find('#');
        if (first_tab == std::string_view::npos || hash == std::string_view::npos || hash > first_tab) {
            continue;
        }
        const std::size_t second_tab = line.find('\t', first_tab + 1U);
        const std::string_view field = line.substr(
            first_tab + 1U, (second_tab == std::string_view::npos ? line.size() : second_tab) - first_tab - 1U);
        std::string word;
        if (hebrew) {
            for (const char character : field) {
                if (character != '/' && character != '\\') {
                    word.push_back(character);
                }
            }
        } else {
            word = std::string(field.substr(0U, field.find(" (")));
        }
        add_word(word, inventory);
    }
}

struct Box final {
    LayoutUnit x_min{0};
    LayoutUnit y_min{0};
    LayoutUnit x_max{0};
    LayoutUnit y_max{0};
    bool empty{true};
};

// The ink box of each glyph of a run where the run puts it.
std::vector<Box> ink_boxes(const FontSet& fonts, FaceId face, const ShapedRun& run) {
    std::vector<Box> boxes;
    boxes.reserve(run.glyphs.size());
    LayoutUnit pen = 0;
    for (const ShapedGlyph& glyph : run.glyphs) {
        const GlyphBox extents = fonts.glyph_box(face, test_em, glyph.glyph);
        Box box;
        box.empty = extents.empty();
        box.x_min = pen + glyph.x_offset + extents.x_min;
        box.x_max = pen + glyph.x_offset + extents.x_max;
        box.y_min = glyph.y_offset + extents.y_min;
        box.y_max = glyph.y_offset + extents.y_max;
        boxes.push_back(box);
        pen += glyph.x_advance;
    }
    return boxes;
}

// One glyph's ink as rasterized, with where its bitmap stands in pixels:
// `left` from the run's origin, `top` rows above the baseline.
struct Ink final {
    GlyphCoverage coverage{};
    std::int32_t left{0};
    std::int32_t top{0};
};

std::int32_t to_ink_pixels(LayoutUnit value) noexcept {
    const std::int32_t scaled = value * ink_pixels_per_em;
    const std::int32_t pixels = scaled >= 0 ? (scaled + test_em / 2) / test_em : -((-scaled + test_em / 2) / test_em);
    return pixels;
}

std::vector<Ink> ink_of(const FontSet& fonts, FaceId face, const ShapedRun& run) {
    std::vector<Ink> inks(run.glyphs.size());
    LayoutUnit pen = 0;
    for (std::size_t index = 0U; index < run.glyphs.size(); ++index) {
        const ShapedGlyph& glyph = run.glyphs[index];
        Ink& ink = inks[index];
        const bool drawn = fonts.rasterize(face, static_cast<std::uint32_t>(ink_pixels_per_em) * 64U, glyph.glyph, 0, 0,
                                           0, ink.coverage);
        if (drawn) {
            ink.left = to_ink_pixels(pen + glyph.x_offset) + ink.coverage.left;
            ink.top = to_ink_pixels(glyph.y_offset) + ink.coverage.top;
        }
        pen += glyph.x_advance;
    }
    return inks;
}

// Pixels more than half covered by both glyphs.
std::uint32_t shared_ink(const Ink& first, const Ink& second) noexcept {
    const std::int32_t left = std::max(first.left, second.left);
    const std::int32_t right = std::min(first.left + static_cast<std::int32_t>(first.coverage.width),
                                        second.left + static_cast<std::int32_t>(second.coverage.width));
    // Rows are counted downward from each bitmap's top row.
    const std::int32_t top = std::min(first.top, second.top);
    const std::int32_t bottom = std::max(first.top - static_cast<std::int32_t>(first.coverage.height),
                                         second.top - static_cast<std::int32_t>(second.coverage.height));
    std::uint32_t shared = 0U;
    for (std::int32_t y = top; y > bottom; --y) {
        for (std::int32_t x = left; x < right; ++x) {
            const std::size_t first_index =
                static_cast<std::size_t>(first.top - y) * first.coverage.width + static_cast<std::size_t>(x - first.left);
            const std::size_t second_index = static_cast<std::size_t>(second.top - y) * second.coverage.width +
                                             static_cast<std::size_t>(x - second.left);
            if (first.coverage.coverage[first_index] > 127U && second.coverage.coverage[second_index] > 127U) {
                ++shared;
            }
        }
    }
    return shared;
}

struct Verdict final {
    bool missing{false};
    std::uint32_t unplaced{0U};
    std::uint32_t collisions{0U};
    std::uint32_t on_base{0U};
    std::uint32_t detached{0U};
    // The marks concerned, as code points, for the details file.
    std::u32string unplaced_marks{};
    std::u32string colliding_marks{};
    std::u32string on_base_marks{};
    std::u32string detached_marks{};
    [[nodiscard]] bool clean() const noexcept {
        return !missing && unplaced == 0U && collisions == 0U && on_base == 0U && detached == 0U;
    }
};

Verdict test_sequence(const FontSet& fonts, FaceId face, TextScript script, const std::u32string& sequence) {
    const std::string text = encode(sequence);
    const bool right_to_left = script == TextScript::hebrew;
    ShapeFeatures placed_features;
    placed_features.cluster_per_character = true;
    ShapeFeatures unplaced_features = placed_features;
    unplaced_features.mark_positioning_off = true;
    ShapedRun placed;
    ShapedRun unplaced;
    fonts.shape(face, test_em, text, script, right_to_left, placed_features, placed);
    fonts.shape(face, test_em, text, script, right_to_left, unplaced_features, unplaced);

    Verdict verdict;
    const std::vector<Box> boxes = ink_boxes(fonts, face, placed);
    // With one cluster to the character, a glyph's cluster is the byte
    // offset of its character; this maps the offset back to the code point.
    std::vector<char32_t> code_point_at(text.size() + 1U, U'?');
    std::size_t byte_offset = 0U;
    for (const char32_t code_point : sequence) {
        code_point_at[byte_offset] = code_point;
        std::string encoded;
        append_utf8(encoded, code_point);
        byte_offset += encoded.size();
    }
    // Where the base stands: from its pen position to the end of its advance.
    LayoutUnit base_left = 0;
    LayoutUnit base_right = 0;
    // The base is the glyph of the first character (cluster 0); every glyph
    // of a later cluster is a mark the face kept as a glyph of its own.
    std::size_t base = placed.glyphs.size();
    for (std::size_t index = 0U; index < placed.glyphs.size(); ++index) {
        if (placed.glyphs[index].glyph == 0U) {
            verdict.missing = true;
        }
        if (placed.glyphs[index].cluster == 0U && base == placed.glyphs.size()) {
            base = index;
        }
    }
    LayoutUnit pen = 0;
    for (std::size_t index = 0U; index < placed.glyphs.size(); ++index) {
        if (index == base) {
            base_left = pen;
            base_right = pen + placed.glyphs[index].x_advance;
        }
        pen += placed.glyphs[index].x_advance;
    }
    // A mark may reach a tenth of an em past its base's own width.
    const LayoutUnit reach = test_em / 10;
    const std::vector<Ink> inks = ink_of(fonts, face, placed);
    if (verdict.missing || base == placed.glyphs.size()) {
        return verdict;
    }
    const bool comparable = placed.glyphs.size() == unplaced.glyphs.size();
    for (std::size_t index = 0U; index < placed.glyphs.size(); ++index) {
        const ShapedGlyph& glyph = placed.glyphs[index];
        if (glyph.cluster == 0U) {
            continue;
        }
        if (comparable && unplaced.glyphs[index].glyph == glyph.glyph &&
            unplaced.glyphs[index].x_offset == glyph.x_offset && unplaced.glyphs[index].y_offset == glyph.y_offset) {
            ++verdict.unplaced;
            verdict.unplaced_marks.push_back(code_point_at[glyph.cluster]);
        }
        if (!boxes[index].empty &&
            (boxes[index].x_max <= base_left - reach || boxes[index].x_min >= base_right + reach)) {
            ++verdict.detached;
            verdict.detached_marks.push_back(code_point_at[glyph.cluster]);
        }
        if (shared_ink(inks[index], inks[base]) >= ink_overlap_pixels) {
            ++verdict.on_base;
            verdict.on_base_marks.push_back(code_point_at[glyph.cluster]);
        }
        for (std::size_t other = index + 1U; other < placed.glyphs.size(); ++other) {
            if (placed.glyphs[other].cluster == 0U) {
                continue;
            }
            if (shared_ink(inks[index], inks[other]) >= ink_overlap_pixels) {
                ++verdict.collisions;
                verdict.colliding_marks.push_back(code_point_at[glyph.cluster]);
                verdict.colliding_marks.push_back(code_point_at[placed.glyphs[other].cluster]);
            }
        }
    }
    return verdict;
}

struct FaceTotals final {
    std::uint64_t distinct_missing{0U};
    std::uint64_t distinct_unplaced{0U};
    std::uint64_t distinct_collision{0U};
    std::uint64_t distinct_on_base{0U};
    std::uint64_t distinct_detached{0U};
    std::uint64_t distinct_clean{0U};
    std::uint64_t occurrences_missing{0U};
    std::uint64_t occurrences_unplaced{0U};
    std::uint64_t occurrences_collision{0U};
    std::uint64_t occurrences_on_base{0U};
    std::uint64_t occurrences_detached{0U};
};

int run_script(const Arguments& arguments, TextScript script) {
    const bool hebrew = script == TextScript::hebrew;
    Inventory inventory;
    for (const std::string& path : arguments.texts) {
        add_step_file(path, hebrew, inventory);
    }
    std::uint64_t occurrences = 0U;
    std::size_t longest = 0U;
    for (const std::pair<const std::u32string, std::uint64_t>& entry : inventory.sequences) {
        occurrences += entry.second;
        longest = std::max(longest, entry.first.size());
    }
    std::cout << "sequences: " << inventory.sequences.size() << " distinct, " << occurrences
              << " occurrences, longest " << longest << " code points; distinct words: " << inventory.words.size()
              << std::endl;

    std::string details;
    const FaceId slot = hebrew ? FaceId::ezra_sil : FaceId::gentium_book_regular;
    for (const NamedFace& named : arguments.faces) {
        const std::vector<std::uint8_t> bytes = read_file(named.path);
        FontSet fonts;
        if (!fonts.load(slot, bytes)) {
            fail(named.path + " is not a font the text stack accepts");
        }
        FaceTotals totals;
        for (const std::pair<const std::u32string, std::uint64_t>& entry : inventory.sequences) {
            const Verdict verdict = test_sequence(fonts, slot, script, entry.first);
            if (verdict.clean()) {
                ++totals.distinct_clean;
                continue;
            }
            if (verdict.missing) {
                ++totals.distinct_missing;
                totals.occurrences_missing += entry.second;
            }
            if (verdict.unplaced > 0U) {
                ++totals.distinct_unplaced;
                totals.occurrences_unplaced += entry.second;
            }
            if (verdict.collisions > 0U) {
                ++totals.distinct_collision;
                totals.occurrences_collision += entry.second;
            }
            if (verdict.on_base > 0U) {
                ++totals.distinct_on_base;
                totals.occurrences_on_base += entry.second;
            }
            if (verdict.detached > 0U) {
                ++totals.distinct_detached;
                totals.occurrences_detached += entry.second;
            }
            details += named.name + "\t" + code_point_list(entry.first) + "\t" + std::to_string(entry.second) + "\t" +
                       (verdict.missing ? "missing" : "") + "\tunplaced " + code_point_list(verdict.unplaced_marks) +
                       "\tcollision " + code_point_list(verdict.colliding_marks) + "\ton-base " +
                       code_point_list(verdict.on_base_marks) + "\tdetached " +
                       code_point_list(verdict.detached_marks) + "\n";
        }
        // Greek: every distinct word, for glyphs the face lacks.
        std::uint64_t words_missing = 0U;
        std::uint64_t word_occurrences_missing = 0U;
        if (!hebrew) {
            ShapedRun run;
            for (const std::pair<const std::string, std::uint64_t>& entry : inventory.words) {
                fonts.shape(slot, test_em, entry.first, script, false, ShapeFeatures{}, run);
                bool missing = false;
                for (const ShapedGlyph& glyph : run.glyphs) {
                    if (glyph.glyph == 0U) {
                        missing = true;
                    }
                }
                if (missing) {
                    ++words_missing;
                    word_occurrences_missing += entry.second;
                    details += named.name + "\tword\t" + code_point_list(decode(entry.first)) + "\t" +
                               std::to_string(entry.second) + "\tmissing\n";
                }
            }
        }
        std::cout << named.name << ": clean " << totals.distinct_clean << " of " << inventory.sequences.size()
                  << "; missing " << totals.distinct_missing << " (" << totals.occurrences_missing << " occurrences)"
                  << "; unplaced " << totals.distinct_unplaced << " (" << totals.occurrences_unplaced << ")"
                  << "; collision " << totals.distinct_collision << " (" << totals.occurrences_collision << ")"
                  << "; on base " << totals.distinct_on_base << " (" << totals.occurrences_on_base << ")"
                  << "; detached " << totals.distinct_detached << " (" << totals.occurrences_detached << ")";
        if (!hebrew) {
            std::cout << "; words with a missing glyph " << words_missing << " of " << inventory.words.size() << " ("
                      << word_occurrences_missing << " occurrences)";
        }
        std::cout << std::endl;
    }
    if (!arguments.details.empty()) {
        const std::span<const std::uint8_t> bytes(reinterpret_cast<const std::uint8_t*>(details.data()),
                                                  details.size());
        const bible::core::FileWriteStatus written =
            bible::core::write_file_atomic(std::filesystem::path(arguments.details), bytes);
        if (written != bible::core::FileWriteStatus::ok) {
            fail("cannot write " + arguments.details);
        }
    }
    return 0;
}

// The advance and ink box of single code points in a face, in thousandths
// of the em: how a suspect glyph is examined.
int run_glyph(const Arguments& arguments) {
    for (const NamedFace& named : arguments.faces) {
        const std::vector<std::uint8_t> bytes = read_file(named.path);
        FontSet fonts;
        if (!fonts.load(FaceId::eb_garamond_regular, bytes)) {
            fail(named.path + " is not a font the text stack accepts");
        }
        for (const char32_t code_point : arguments.codes) {
            std::u32string single;
            single.push_back(code_point);
            const std::uint32_t glyph = fonts.glyph_of(FaceId::eb_garamond_regular, code_point);
            const GlyphBox box = fonts.glyph_box(FaceId::eb_garamond_regular, test_em, glyph);
            std::cout << named.name << " " << code_point_list(single) << ": glyph " << glyph << ", advance "
                      << fonts.glyph_advance(FaceId::eb_garamond_regular, test_em, glyph) << ", ink x " << box.x_min
                      << ".." << box.x_max << " y " << box.y_min << ".." << box.y_max << std::endl;
        }
    }
    return 0;
}

Arguments parse_arguments(int argc, char** argv) {
    Arguments arguments;
    if (argc < 2 || argc % 2 != 0) {
        fail("usage: shaped_marks hebrew|greek|glyph --face NAME=FILE ... [--text FILE ...] [--code HEX ...] "
             "[--details FILE]");
    }
    arguments.mode = argv[1];
    for (int index = 2; index + 1 < argc; index += 2) {
        const std::string_view key(argv[index]);
        const std::string value(argv[index + 1]);
        if (key == "--face") {
            const std::size_t equals = value.find('=');
            if (equals == std::string::npos) {
                fail("--face takes NAME=FILE");
            }
            arguments.faces.push_back(NamedFace{value.substr(0U, equals), value.substr(equals + 1U)});
        } else if (key == "--text") {
            arguments.texts.push_back(value);
        } else if (key == "--code") {
            arguments.codes.push_back(static_cast<char32_t>(std::stoul(value, nullptr, 16)));
        } else if (key == "--details") {
            arguments.details = value;
        } else {
            fail("unknown argument " + std::string(key));
        }
    }
    return arguments;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const Arguments arguments = parse_arguments(argc, argv);
        if (arguments.mode == "hebrew") {
            return run_script(arguments, TextScript::hebrew);
        }
        if (arguments.mode == "greek") {
            return run_script(arguments, TextScript::greek);
        }
        if (arguments.mode == "glyph") {
            return run_glyph(arguments);
        }
        fail("the mode must be hebrew, greek or glyph");
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "shaped_marks: " << error.what() << std::endl;
        return 1;
    }
}
