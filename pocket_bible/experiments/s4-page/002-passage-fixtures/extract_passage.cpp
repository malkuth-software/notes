// Cuts one passage out of a pinned source file, byte for byte, and writes it
// as a fixture that carries the source's hash. The proving slice's passage
// text comes from here and from nowhere else: no line of a fixture is typed,
// corrected or completed by hand or by a model.
//
//   page_extract_passage usfm --file MRK.usfm --archive bsb_usfm.zip --passage "MRK 7:14-23"
//                             --chapter 7 --first 14 --last 23 --output mrk-7-14-23.bsb.fixture
//   page_extract_passage step --file TAGNT_Mat-Jhn.txt --passage "MRK 7:14-23"
//                             --prefix Mrk.7. --first 14 --last 23 --output mrk-7-14-23.tagnt.fixture
//
// A fixture is header lines that begin with "#!" followed by the body: whole
// lines of the source, unchanged, in source order.
//
//   usfm  The lines of one chapter from the first line that opens a verse of
//         the range through the last, with the heading lines directly above
//         (and the \c line when the passage opens the chapter) and the
//         verse-less continuation lines directly below. The tool refuses a
//         range whose lines hold a verse outside it.
//   step  Every word row of a STEPBible amalgamated text whose reference is
//         PREFIX<verse># (or PREFIX<verse>(<Hebrew reference>)#) with the
//         verse in range. Comment and summary rows
//         between them are left out; the header records how many rows and
//         the first and last source line.
//
// The header records the SHA-256 of the file read, of the archive it was
// unpacked from when one is given (the tool hashes the archive; it does not
// unpack it), and of the body.

#include "core/file_io.hpp"
#include "core/sha256.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace {

constexpr std::uint64_t source_maximum_bytes = 256U * 1024U * 1024U;

struct Arguments final {
    std::string mode{};
    std::string file{};
    std::string archive{};
    std::string passage{};
    std::string prefix{};
    std::string output{};
    std::uint32_t chapter{0U};
    std::uint32_t first{0U};
    std::uint32_t last{0U};
    bool chapter_given{false};
    bool first_given{false};
    bool last_given{false};
};

// One line of the source: [begin, end) includes its terminator; text excludes
// it and any carriage return before it.
struct SourceLine final {
    std::size_t begin{0U};
    std::size_t end{0U};
    std::string_view text{};
};

void fail(const std::string& message) {
    throw std::runtime_error(message);
}

bool parse_unsigned(std::string_view text, std::uint32_t& destination) {
    std::uint32_t value = 0U;
    const char* first = text.data();
    const char* last = text.data() + text.size();
    const std::from_chars_result parsed = std::from_chars(first, last, value);
    if (parsed.ec != std::errc{} || parsed.ptr != last) {
        return false;
    }
    destination = value;
    return true;
}

Arguments parse_arguments(int argc, char** argv) {
    Arguments arguments;
    if (argc < 2) {
        fail("a mode is required: usfm or step");
    }
    arguments.mode = argv[1];
    for (int index = 2; index + 1 < argc; index += 2) {
        const std::string_view key(argv[index]);
        const std::string_view value(argv[index + 1]);
        if (key == "--file") {
            arguments.file = value;
        } else if (key == "--archive") {
            arguments.archive = value;
        } else if (key == "--passage") {
            arguments.passage = value;
        } else if (key == "--prefix") {
            arguments.prefix = value;
        } else if (key == "--output") {
            arguments.output = value;
        } else if (key == "--chapter") {
            arguments.chapter_given = parse_unsigned(value, arguments.chapter);
        } else if (key == "--first") {
            arguments.first_given = parse_unsigned(value, arguments.first);
        } else if (key == "--last") {
            arguments.last_given = parse_unsigned(value, arguments.last);
        } else {
            fail("unknown argument " + std::string(key));
        }
    }
    if (argc % 2 != 0) {
        fail("every argument takes one value");
    }
    if (arguments.file.empty() || arguments.passage.empty() || arguments.output.empty() ||
        !arguments.first_given || !arguments.last_given || arguments.first > arguments.last) {
        fail("--file, --passage, --output, --first and --last are required, with first <= last");
    }
    return arguments;
}

std::vector<std::uint8_t> read_source(const std::string& path) {
    std::vector<std::uint8_t> bytes;
    const bible::core::FileReadStatus status =
        bible::core::read_file_bounded(std::filesystem::path(path), source_maximum_bytes, bytes);
    if (status != bible::core::FileReadStatus::ok) {
        fail("cannot read " + path + ": " + std::string(bible::core::file_read_status_name(status)));
    }
    return bytes;
}

std::string digest_of(std::span<const std::uint8_t> bytes) {
    const bible::core::Sha256Digest digest = bible::core::sha256(bytes);
    std::string hex = bible::core::sha256_hex(digest);
    return hex;
}

std::vector<SourceLine> split_lines(std::string_view text) {
    std::vector<SourceLine> lines;
    std::size_t begin = 0U;
    while (begin < text.size()) {
        const std::size_t newline = text.find('\n', begin);
        const std::size_t end = newline == std::string_view::npos ? text.size() : newline + 1U;
        std::size_t text_end = newline == std::string_view::npos ? text.size() : newline;
        if (text_end > begin && text[text_end - 1U] == '\r') {
            --text_end;
        }
        SourceLine line;
        line.begin = begin;
        line.end = end;
        line.text = text.substr(begin, text_end - begin);
        lines.push_back(line);
        begin = end;
    }
    return lines;
}

bool starts_with(std::string_view text, std::string_view prefix) {
    const bool matches = text.substr(0U, prefix.size()) == prefix;
    return matches;
}

// The verse numbers a USFM line opens: every "\v N" followed by a space.
// "\fv" inside a footnote is not a verse of the running text and does not
// match, because the marker must be exactly "\v".
std::vector<std::uint32_t> verses_opened(std::string_view line) {
    std::vector<std::uint32_t> verses;
    std::size_t position = 0U;
    while (true) {
        const std::size_t marker = line.find("\\v ", position);
        if (marker == std::string_view::npos) {
            break;
        }
        std::size_t digits_end = marker + 3U;
        while (digits_end < line.size() && line[digits_end] >= '0' && line[digits_end] <= '9') {
            ++digits_end;
        }
        std::uint32_t verse = 0U;
        if (parse_unsigned(line.substr(marker + 3U, digits_end - marker - 3U), verse)) {
            verses.push_back(verse);
        }
        position = digits_end;
    }
    return verses;
}

struct Selection final {
    // Indices into the line table, last inclusive.
    std::size_t first_line{0U};
    std::size_t last_line{0U};
    std::size_t line_count{0U};
    std::string body{};
    std::string notes{};
};

Selection select_usfm(std::string_view text, const std::vector<SourceLine>& lines, const Arguments& arguments) {
    if (!arguments.chapter_given) {
        fail("usfm mode needs --chapter");
    }
    const std::string chapter_marker = "\\c " + std::to_string(arguments.chapter);
    std::size_t chapter_line = lines.size();
    for (std::size_t index = 0U; index < lines.size(); ++index) {
        if (lines[index].text == chapter_marker) {
            chapter_line = index;
            break;
        }
    }
    if (chapter_line == lines.size()) {
        fail("the file has no line '" + chapter_marker + "'");
    }
    std::size_t chapter_end = lines.size();
    for (std::size_t index = chapter_line + 1U; index < lines.size(); ++index) {
        if (starts_with(lines[index].text, "\\c ")) {
            chapter_end = index;
            break;
        }
    }

    bool found_first = false;
    bool found_last = false;
    std::size_t first_line = 0U;
    std::size_t last_line = 0U;
    for (std::size_t index = chapter_line + 1U; index < chapter_end; ++index) {
        const std::vector<std::uint32_t> verses = verses_opened(lines[index].text);
        for (const std::uint32_t verse : verses) {
            if (!found_first && verse >= arguments.first && verse <= arguments.last) {
                first_line = index;
                found_first = true;
            }
            if (verse >= arguments.first && verse <= arguments.last) {
                last_line = index;
                found_last = true;
            }
        }
    }
    if (!found_first || !found_last) {
        fail("no verse of the range opens in chapter " + std::to_string(arguments.chapter));
    }

    // Heading lines directly above, and the chapter line when nothing with a
    // verse stands between it and the passage.
    while (first_line > chapter_line + 1U && verses_opened(lines[first_line - 1U].text).empty()) {
        --first_line;
    }
    if (first_line == chapter_line + 1U) {
        first_line = chapter_line;
    }
    // Continuation lines directly below: lines that open no verse and are not
    // the heading of what follows.
    while (last_line + 1U < chapter_end) {
        const std::string_view next = lines[last_line + 1U].text;
        const bool heading = starts_with(next, "\\s") || starts_with(next, "\\r ") || starts_with(next, "\\ms");
        if (heading || !verses_opened(next).empty()) {
            break;
        }
        ++last_line;
    }
    // A trailing blank-line marker belongs to what follows.
    while (last_line > first_line && (lines[last_line].text == "\\b" || lines[last_line].text == "\\b ")) {
        --last_line;
    }

    std::string verse_list;
    for (std::size_t index = first_line; index <= last_line; ++index) {
        const std::vector<std::uint32_t> verses = verses_opened(lines[index].text);
        for (const std::uint32_t verse : verses) {
            if (verse < arguments.first || verse > arguments.last) {
                fail("line " + std::to_string(index + 1U) + " opens verse " + std::to_string(verse) +
                     ", outside the range; choose a range that ends with its paragraph");
            }
            if (!verse_list.empty()) {
                verse_list += " ";
            }
            verse_list += std::to_string(verse);
        }
    }

    std::string book_heading;
    for (const SourceLine& line : lines) {
        if (starts_with(line.text, "\\h ")) {
            book_heading = std::string(line.text.substr(3U));
            break;
        }
    }

    Selection selection;
    selection.first_line = first_line;
    selection.last_line = last_line;
    selection.line_count = last_line - first_line + 1U;
    selection.body = std::string(text.substr(lines[first_line].begin, lines[last_line].end - lines[first_line].begin));
    selection.notes = "#!book-heading " + book_heading + "\n#!verses-opened " + verse_list + "\n";
    return selection;
}

Selection select_step(std::string_view text, const std::vector<SourceLine>& lines, const Arguments& arguments) {
    if (arguments.prefix.empty()) {
        fail("step mode needs --prefix, for example Mrk.7.");
    }
    Selection selection;
    bool any = false;
    for (std::size_t index = 0U; index < lines.size(); ++index) {
        const std::string_view line = lines[index].text;
        if (!starts_with(line, arguments.prefix)) {
            continue;
        }
        std::size_t digits_end = arguments.prefix.size();
        while (digits_end < line.size() && line[digits_end] >= '0' && line[digits_end] <= '9') {
            ++digits_end;
        }
        std::uint32_t verse = 0U;
        const bool numbered =
            parse_unsigned(line.substr(arguments.prefix.size(), digits_end - arguments.prefix.size()), verse);
        // A psalm title carries the Hebrew reference in parentheses between
        // the verse and the word number: Psa.87.0(87.1)#01.
        std::size_t marker = digits_end;
        if (marker < line.size() && line[marker] == '(') {
            const std::size_t close = line.find(')', marker);
            marker = close == std::string_view::npos ? line.size() : close + 1U;
        }
        if (!numbered || marker >= line.size() || line[marker] != '#') {
            continue;
        }
        if (verse < arguments.first || verse > arguments.last) {
            continue;
        }
        if (!any) {
            selection.first_line = index;
            any = true;
        }
        selection.last_line = index;
        ++selection.line_count;
        selection.body += std::string(text.substr(lines[index].begin, lines[index].end - lines[index].begin));
    }
    if (!any) {
        fail("no word row begins with " + arguments.prefix + "<verse># in the range");
    }
    return selection;
}

std::string file_name_of(const std::string& path) {
    const std::filesystem::path file(path);
    const std::u8string name = file.filename().generic_u8string();
    std::string text(name.begin(), name.end());
    return text;
}

int run(int argc, char** argv) {
    const Arguments arguments = parse_arguments(argc, argv);
    const std::vector<std::uint8_t> source = read_source(arguments.file);
    const std::string_view text(reinterpret_cast<const char*>(source.data()), source.size());
    const std::vector<SourceLine> lines = split_lines(text);

    Selection selection;
    if (arguments.mode == "usfm") {
        selection = select_usfm(text, lines, arguments);
    } else if (arguments.mode == "step") {
        selection = select_step(text, lines, arguments);
    } else {
        fail("the mode must be usfm or step");
    }

    const std::span<const std::uint8_t> body_bytes(reinterpret_cast<const std::uint8_t*>(selection.body.data()),
                                                   selection.body.size());
    std::string fixture = "#!pocket-bible-fixture 1\n";
    fixture += "#!kind " + arguments.mode + "\n";
    fixture += "#!passage " + arguments.passage + "\n";
    fixture += "#!source-file " + file_name_of(arguments.file) + " sha256 " + digest_of(source) + " bytes " +
               std::to_string(source.size()) + "\n";
    if (!arguments.archive.empty()) {
        const std::vector<std::uint8_t> archive = read_source(arguments.archive);
        fixture += "#!source-archive " + file_name_of(arguments.archive) + " sha256 " + digest_of(archive) +
                   " bytes " + std::to_string(archive.size()) + "\n";
    }
    fixture += "#!source-lines " + std::to_string(selection.first_line + 1U) + "-" +
               std::to_string(selection.last_line + 1U) + " rows " + std::to_string(selection.line_count) + "\n";
    fixture += selection.notes;
    fixture += "#!body sha256 " + digest_of(body_bytes) + " bytes " + std::to_string(selection.body.size()) + "\n";
    fixture += selection.body;

    const std::span<const std::uint8_t> fixture_bytes(reinterpret_cast<const std::uint8_t*>(fixture.data()),
                                                      fixture.size());
    const bible::core::FileWriteStatus written =
        bible::core::write_file_atomic(std::filesystem::path(arguments.output), fixture_bytes);
    if (written != bible::core::FileWriteStatus::ok) {
        fail("cannot write " + arguments.output + ": " + std::string(bible::core::file_write_status_name(written)));
    }
    std::cout << arguments.output << ": " << arguments.passage << ", source lines " << (selection.first_line + 1U)
              << "-" << (selection.last_line + 1U) << ", " << selection.line_count << " rows, "
              << selection.body.size() << " bytes" << std::endl;
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const int exit_code = run(argc, argv);
        return exit_code;
    } catch (const std::exception& error) {
        std::cerr << "page_extract_passage: " << error.what() << std::endl;
        return 1;
    }
}
