// font-pack experiment: command line.
//   fontpack pack <out.fpk> <font>...     pack, then load the pack and compare every font
//   fontpack verify <in.fpk> <font>...    load and compare against the given files
//   fontpack bench <in.fpk> [threads] [rounds]
//   fontpack unpack <in.fpk> <directory>
//   fontpack damage <in.fpk> [rounds]     load damaged copies; for sanitizer runs
#include "fp_load.hpp"
#include "fp_pack.hpp"

#include <algorithm>
#include <chrono>
#include <sys/resource.h>

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

static bool write_file(const char* path, const fp::Bytes& bytes) {
    std::FILE* file = std::fopen(path, "wb");
    if (file == nullptr) {
        return false;
    }
    const std::size_t put = std::fwrite(bytes.data(), 1, bytes.size(), file);
    std::fclose(file);
    return put == bytes.size();
}

#ifdef FP_TRACE
#include <atomic>
namespace fp {
extern std::atomic<long long> g_trace[8];
}
#endif

static double seconds_now() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static const char* base_name(const char* path) {
    const char* slash = std::strrchr(path, '/');
    return slash == nullptr ? path : slash + 1;
}

// Loads pack and compares each font with the input of the same name.
static bool verify(const fp::Bytes& pack, const std::vector<fp::PackInput>& fonts) {
    fp::LoadedPack loaded;
    if (!fp::pack_load(pack.data(), pack.size(), 0, loaded)) {
        std::printf("verify: the pack does not load\n");
        return false;
    }
    bool same = loaded.font_count == fonts.size();
    for (std::uint32_t f = 0; f < loaded.font_count && same; f += 1) {
        const fp::LoadedFont& font = loaded.fonts[f];
        if (font.size != fonts[f].data.size() || font.name_length != fonts[f].name.size() ||
            std::memcmp(font.name, fonts[f].name.data(), font.name_length) != 0 ||
            std::memcmp(font.data, fonts[f].data.data(), font.size) != 0) {
            std::printf("verify: %s differs\n", fonts[f].name.c_str());
            same = false;
        }
    }
    fp::pack_free(loaded);
    return same;
}

static bool read_fonts(int argc, char** argv, int first, std::vector<fp::PackInput>& fonts, std::size_t& total) {
    total = 0;
    for (int k = first; k < argc; k += 1) {
        fp::PackInput input;
        input.name = base_name(argv[k]);
        if (!read_file(argv[k], input.data)) {
            std::printf("cannot read %s\n", argv[k]);
            return false;
        }
        total += input.data.size();
        fonts.push_back(input);
    }
    return true;
}

int main(int argc, char** argv) {
    if (argc >= 4 && std::strcmp(argv[1], "verify") == 0) {
        fp::Bytes pack;
        std::vector<fp::PackInput> fonts;
        std::size_t total = 0;
        if (!read_file(argv[2], pack) || !read_fonts(argc, argv, 3, fonts, total)) {
            return 1;
        }
        const bool same = verify(pack, fonts);
        std::printf("%s: %zu fonts, %zu bytes from a %zu byte pack\n", same ? "IDENTICAL" : "DIFFERENT", fonts.size(), total,
                    pack.size());
        return same ? 0 : 1;
    }
    if (argc >= 3 && std::strcmp(argv[1], "bench") == 0) {
        fp::Bytes pack;
        if (!read_file(argv[2], pack)) {
            return 1;
        }
        const std::uint32_t threads = argc > 3 ? static_cast<std::uint32_t>(std::atoi(argv[3])) : 0;
        const int rounds = argc > 4 ? std::atoi(argv[4]) : 25;
        std::vector<double> times;
        fp::LoadTimings best;
        std::memset(&best, 0, sizeof(best));
        std::size_t scratch = 0;
        std::size_t storage = 0;
        for (int round = 0; round < rounds; round += 1) {
            fp::LoadedPack loaded;
            const double t0 = seconds_now();
            const bool ok = fp::pack_load(pack.data(), pack.size(), threads, loaded);
            const double t1 = seconds_now();
            if (!ok) {
                std::printf("load failed\n");
                return 1;
            }
            if (times.empty() || t1 - t0 < *std::min_element(times.begin(), times.end())) {
                best = loaded.timings;
            }
            times.push_back(t1 - t0);
            scratch = loaded.scratch_peak;
            storage = loaded.storage_bytes;
            fp::pack_free(loaded);
        }
#ifdef FP_TRACE
        static const char* const names[8] = {"classes",
                                             "chunk decode",
                                             "match copy",
                                             "font rebuild",
                                             "cff rebuild",
                                             "  byte stream sequences",
                                             "  number literals",
                                             "  number low bytes and corrections"};
        for (int k = 0; k < 8; k += 1) {
            std::printf("trace %-34s %.2f ms per load\n", names[k], fp::g_trace[k].load() / 1e6 / rounds);
        }
#endif
        std::sort(times.begin(), times.end());
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        std::printf("threads %u rounds %d: min %.2f ms, median %.2f ms, max %.2f ms\n", threads, rounds, times[0] * 1e3,
                    times[times.size() / 2] * 1e3, times[times.size() - 1] * 1e3);
        std::printf("best round: open %.2f, leading streams %.2f, other streams %.2f, rebuild %.2f ms\n", best.open_seconds * 1e3,
                    best.streams_seconds * 1e3, best.dependent_seconds * 1e3, best.rebuild_seconds * 1e3);
        std::printf("fonts %zu bytes, scratch peak %zu bytes (%.1f%% of the fonts), pack %zu bytes, process peak RSS %ld bytes\n",
                    storage, scratch, 100.0 * scratch / storage, pack.size(), static_cast<long>(usage.ru_maxrss));
        return 0;
    }
    if (argc >= 3 && std::strcmp(argv[1], "damage") == 0) {
        // Loads damaged copies of the pack. Every load must return (true or false)
        // without touching memory it does not own; run this under the sanitizers.
        fp::Bytes pack;
        if (!read_file(argv[2], pack)) {
            return 1;
        }
        const int rounds = argc > 3 ? std::atoi(argv[3]) : 200;
        std::uint64_t random = 0x9E3779B97F4A7C15ull;
        int loaded = 0;
        int refused = 0;
        for (int round = 0; round < rounds; round += 1) {
            fp::Bytes damaged = pack;
            random ^= random << 13;
            random ^= random >> 7;
            random ^= random << 17;
            const std::uint32_t kind = static_cast<std::uint32_t>(random >> 60);
            const std::uint32_t edits = 1 + static_cast<std::uint32_t>((random >> 52) & 7u);
            for (std::uint32_t e = 0; e < edits; e += 1) {
                random ^= random << 13;
                random ^= random >> 7;
                random ^= random << 17;
                // Half the edits land in the first 8 KiB, where the headers are.
                std::size_t where = static_cast<std::size_t>(random % damaged.size());
                if ((random >> 40 & 1u) != 0 && damaged.size() > 8192) {
                    where = static_cast<std::size_t>(random % 8192);
                }
                if (kind < 10) {
                    damaged[where] = static_cast<std::uint8_t>(random >> 32);
                } else if (kind < 13) {
                    const std::size_t span = static_cast<std::size_t>((random >> 20) % 4096);
                    for (std::size_t k = where; k < damaged.size() && k < where + span; k += 1) {
                        damaged[k] = 0;
                    }
                } else if (where > 64) {
                    damaged.resize(where);
                }
            }
            fp::LoadedPack result;
            if (fp::pack_load(damaged.data(), damaged.size(), 0, result)) {
                loaded += 1;
                fp::pack_free(result);
            } else {
                refused += 1;
            }
        }
        std::printf("damage: %d rounds, %d refused, %d still loaded\n", rounds, refused, loaded);
        return 0;
    }
    if (argc >= 4 && std::strcmp(argv[1], "unpack") == 0) {
        fp::Bytes pack;
        if (!read_file(argv[2], pack)) {
            return 1;
        }
        fp::LoadedPack loaded;
        if (!fp::pack_load(pack.data(), pack.size(), 0, loaded)) {
            std::printf("load failed\n");
            return 1;
        }
        for (std::uint32_t f = 0; f < loaded.font_count; f += 1) {
            std::string path = std::string(argv[3]) + "/" + std::string(loaded.fonts[f].name, loaded.fonts[f].name_length);
            fp::Bytes bytes(loaded.fonts[f].data, loaded.fonts[f].data + loaded.fonts[f].size);
            if (!write_file(path.c_str(), bytes)) {
                std::printf("cannot write %s\n", path.c_str());
                return 1;
            }
        }
        fp::pack_free(loaded);
        return 0;
    }
    if (argc >= 4 && std::strcmp(argv[1], "pack") == 0) {
        std::vector<fp::PackInput> fonts;
        std::size_t total = 0;
        if (!read_fonts(argc, argv, 3, fonts, total)) {
            return 1;
        }
        fp::Bytes pack;
        const double t0 = seconds_now();
        fp::pack_fonts(fonts, pack, true);
        const double t1 = seconds_now();
        if (!write_file(argv[2], pack)) {
            std::printf("cannot write %s\n", argv[2]);
            return 1;
        }
        std::printf("packed %zu fonts: %zu -> %zu bytes (%.2f%%) in %.1f s\n", fonts.size(), total, pack.size(),
                    100.0 * pack.size() / total, t1 - t0);
        if (!verify(pack, fonts)) {
            std::printf("the pack does NOT reproduce its inputs\n");
            return 1;
        }
        std::printf("verified: the pack loads to the identical bytes\n");
        return 0;
    }
    std::printf("usage: fontpack pack|verify|bench|unpack ...\n");
    return 2;
}
