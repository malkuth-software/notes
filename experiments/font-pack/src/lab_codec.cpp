// font-pack experiment: codec lab. Round-trips files through the stream codec
// and reports size and timing.
//   lab_codec <file> [quantiser] [iterations] [chain_depth] [chunk_bytes]
#include "fp_codec.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>

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
        return 2;
    }
    fp::Bytes input;
    if (!read_file(argv[1], input)) {
        std::printf("cannot read %s\n", argv[1]);
        return 1;
    }
    fp::EncodeOptions options = fp::encode_options_default();
    if (argc > 2) options.quantiser = static_cast<std::uint32_t>(std::atoi(argv[2]));
    if (argc > 3) options.iterations = static_cast<std::uint32_t>(std::atoi(argv[3]));
    if (argc > 4) options.chain_depth = static_cast<std::uint32_t>(std::atoi(argv[4]));
    if (argc > 5) options.chunk_bytes = static_cast<std::uint32_t>(std::atoi(argv[5]));
    // Optional sixth argument: a file of little-endian 16-bit fine context ids, one per byte.
    std::vector<std::uint8_t> classes;
    if (argc > 6) {
        fp::Bytes fine_bytes;
        if (!read_file(argv[6], fine_bytes) || fine_bytes.size() != input.size() * 2) {
            std::printf("bad fine file\n");
            return 1;
        }
        std::vector<std::uint16_t> fine(input.size());
        std::memcpy(fine.data(), fine_bytes.data(), fine_bytes.size());
        std::uint32_t fine_count = 0;
        for (std::size_t k = 0; k < fine.size(); k += 1) {
            if (fine[k] >= fine_count) fine_count = fine[k] + 1u;
        }
        std::vector<std::uint8_t> map;
        options.class_count = fp::cluster_contexts(input.data(), fine.data(), input.size(), fine_count, map);
        classes.resize(input.size());
        for (std::size_t k = 0; k < fine.size(); k += 1) classes[k] = map[fine[k]];
        options.classes = classes.data();
        std::printf("fine contexts %u -> classes %u\n", fine_count, options.class_count);
    }
    fp::Bytes coded;
    const double t0 = seconds_now();
    fp::stream_encode(input.data(), input.size(), options, coded);
    const double t1 = seconds_now();
    const std::size_t coded_size = coded.size();
    coded.insert(coded.end(), fp::kStreamPadding, 0);
    fp::Bytes output(input.size() + 16);
    std::size_t consumed = 0;
    double best = 1e9;
    bool ok = true;
    for (int round = 0; round < 5; round += 1) {
        const double d0 = seconds_now();
        ok = fp::stream_decode_all(coded.data(), coded_size, output.data(), options.classes, consumed) && ok;
        const double d1 = seconds_now();
        if (d1 - d0 < best) best = d1 - d0;
    }
    // With FP_LAB_PHRASES set: how concentrated the matched contents are.
    if (std::getenv("FP_LAB_PHRASES") != nullptr) {
        fp::StreamDecoder decoder;
        std::size_t used = 0;
        if (fp::stream_open(decoder, coded.data(), coded_size, used) && !decoder.stored) {
            std::vector<std::uint8_t> scratch(options.chunk_bytes + 64);
            for (std::uint32_t c = 0; c < decoder.chunk_count; c += 1) {
                fp::stream_decode_chunk(decoder, c, output.data(), options.classes, scratch.data());
            }
            fp::stream_copy_matches(decoder, output.data());
            std::vector<std::uint64_t> hashes;
            std::vector<std::uint32_t> lengths;
            std::size_t position = 0;
            std::uint64_t literal_total = 0;
            std::uint64_t match_bytes = 0;
            for (std::uint32_t q = 0; q < decoder.sequence_total; q += 1) {
                position += decoder.sequences[q].literal_length;
                literal_total += decoder.sequences[q].literal_length;
                const std::uint32_t length = decoder.sequences[q].match_length;
                if (length == 0) continue;
                std::uint64_t h = 1469598103934665603ull;
                for (std::uint32_t k = 0; k < length; k += 1) h = (h ^ output[position + k]) * 1099511628211ull;
                hashes.push_back(h);
                lengths.push_back(length);
                match_bytes += length;
                position += length;
            }
            std::vector<std::uint64_t> sorted = hashes;
            std::sort(sorted.begin(), sorted.end());
            static const std::uint32_t edges[8] = {1, 2, 4, 16, 64, 256, 1024, 4096};
            std::uint64_t matches_at[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            std::uint64_t distinct_at[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            double ideal_bits = 0.0;
            for (std::size_t a = 0; a < sorted.size();) {
                std::size_t b = a;
                while (b < sorted.size() && sorted[b] == sorted[a]) b += 1;
                const std::uint64_t n = b - a;
                for (int e = 0; e < 8; e += 1) {
                    if (n >= edges[e]) { matches_at[e] += n; distinct_at[e] += 1; }
                }
                ideal_bits += static_cast<double>(n) * std::log2(static_cast<double>(sorted.size()) / static_cast<double>(n));
                a = b;
            }
            std::printf("literals %llu, matches %zu covering %llu bytes (mean length %.1f)\n",
                        static_cast<unsigned long long>(literal_total), hashes.size(),
                        static_cast<unsigned long long>(match_bytes), static_cast<double>(match_bytes) / hashes.size());
            for (int e = 0; e < 8; e += 1) {
                std::printf("  content seen >= %4u times: %8llu matches, %7llu distinct\n", edges[e],
                            static_cast<unsigned long long>(matches_at[e]), static_cast<unsigned long long>(distinct_at[e]));
            }
            std::printf("  entropy of match content identity: %.1f bits per match\n", ideal_bits / sorted.size());
            fp::stream_close(decoder);
        }
    }
    const bool same = ok && consumed == coded_size && std::memcmp(output.data(), input.data(), input.size()) == 0;
    std::printf("%s: %zu -> %zu (%.2f%%) encode %.2fs decode %.2fms (%.0f MB/s) %s\n", argv[1], input.size(), coded_size,
                100.0 * coded_size / input.size(), t1 - t0, best * 1000.0, input.size() / best / 1e6,
                same ? "OK" : "MISMATCH");
    return same ? 0 : 1;
}
