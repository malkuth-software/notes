// font-pack experiment: command line.
//   fontpack pack <out.fpk> [--manifest <file>] <font>...
//       pack (one group per manifest group; one eager group "all" without a
//       manifest), then load every group and compare every font
//   fontpack verify <in.fpk> <font>...    load every group and compare against the given files
//   fontpack bench <in.fpk> [--threads 1,4,0] [--rounds N] [--baseline <single.fpk>] [--only <label>]
//       pack ratio per group and for the whole set (against the baseline pack
//       when given), then load time and peak scratch per group, for the eager
//       groups together and for all groups, at each thread count (0: all cores)
//   fontpack unpack <in.fpk> <directory>
//   fontpack damage <in.fpk> [rounds]     load damaged copies; for sanitizer runs
//   fontpack claims <in.fpk>              wrong size claims and check values must be refused, unallocated
#include "fp_crc.hpp"
#include "fp_load.hpp"
#include "fp_pack.hpp"

#include <algorithm>
#include <chrono>
#include <string>
#include <sys/resource.h>
#include <thread>

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

static const char* refusal_name(fp::GroupRefusal refusal) {
    switch (refusal) {
        case fp::kGroupLoaded: return "loaded";
        case fp::kGroupBadIndex: return "bad index";
        case fp::kGroupBadClaims: return "bad claims";
        case fp::kGroupBadStreams: return "bad streams";
        case fp::kGroupBadCheck: return "bad check value";
        case fp::kGroupNoMemory: return "no memory";
        default: return "?";
    }
}

static std::string group_name(const fp::PackGroup& group) {
    return std::string(group.name, group.name_length);
}

// Loads every group of pack and compares each font with the input of the same name.
static bool verify(const fp::Bytes& pack, const std::vector<fp::PackInput>& fonts) {
    fp::PackIndex* index = new fp::PackIndex;
    bool same = fp::pack_open(pack.data(), pack.size(), *index);
    if (!same) {
        std::printf("verify: the pack does not open\n");
    }
    same = same && (*index).font_count == fonts.size();
    std::vector<bool> seen(fonts.size(), false);
    for (std::uint32_t g = 0; g < (*index).group_count && same; g += 1) {
        fp::LoadedGroup loaded;
        if (!fp::pack_load_group(*index, g, 0, loaded)) {
            std::printf("verify: group %s does not load (%s)\n", group_name((*index).groups[g]).c_str(),
                        refusal_name(loaded.refusal));
            same = false;
            break;
        }
        for (std::uint32_t f = 0; f < loaded.font_count && same; f += 1) {
            const fp::LoadedFont& font = loaded.fonts[f];
            const std::string name(font.name, font.name_length);
            std::size_t k = 0;
            while (k < fonts.size() && (seen[k] || fonts[k].name != name)) {
                k += 1;
            }
            if (k == fonts.size() || font.size != fonts[k].data.size() ||
                std::memcmp(font.data, fonts[k].data.data(), font.size) != 0) {
                std::printf("verify: %s differs or is not an input\n", name.c_str());
                same = false;
            } else {
                seen[k] = true;
            }
        }
        fp::group_free(loaded);
    }
    delete index;
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

// ---- bench -------------------------------------------------------------------

struct BenchOptions {
    std::vector<std::uint32_t> threads;
    int rounds;
    const char* baseline;
    const char* only;   // measure only this label (a group name, "eager" or "all"), or null
};

// Parses "1,4,0".
static bool parse_threads(const char* text, std::vector<std::uint32_t>& threads) {
    threads.clear();
    const char* at = text;
    while (*at != 0) {
        char* end = nullptr;
        const long value = std::strtol(at, &end, 10);
        if (end == at || value < 0 || value > 64) {
            return false;
        }
        threads.push_back(static_cast<std::uint32_t>(value));
        at = *end == ',' ? end + 1 : end;
    }
    return !threads.empty();
}

static void print_ratios(const char* label, const fp::PackIndex& index) {
    std::uint64_t font_bytes = 0;
    for (std::uint32_t g = 0; g < index.group_count; g += 1) {
        font_bytes += index.groups[g].font_bytes;
    }
    std::printf("%s: %u groups, %u fonts, %llu font bytes -> %zu pack bytes (%.2f%%)\n", label, index.group_count,
                index.font_count, static_cast<unsigned long long>(font_bytes), index.pack_size,
                100.0 * static_cast<double>(index.pack_size) / static_cast<double>(font_bytes));
    for (std::uint32_t g = 0; g < index.group_count; g += 1) {
        const fp::PackGroup& group = index.groups[g];
        std::printf("  group %-12s %-5s %3u fonts %10llu -> %10zu body bytes (%.2f%%), streams %llu bytes\n",
                    group_name(group).c_str(), group.eager ? "eager" : "lazy", group.font_count,
                    static_cast<unsigned long long>(group.font_bytes), group.body_size,
                    100.0 * static_cast<double>(group.body_size) / static_cast<double>(group.font_bytes),
                    static_cast<unsigned long long>(group.stream_bytes));
    }
}

// What is loaded in one measured round: one group, the eager groups or all groups.
enum BenchSet : std::uint32_t { kBenchOneGroup = 0, kBenchEager, kBenchAll };

// Loads the set rounds times with the given thread count and prints one line.
// A round of several groups loads them one after another, as an application
// would; its scratch peak is the largest of the groups' peaks (they do not overlap).
static bool bench_set(const fp::PackIndex& index, BenchSet set, std::uint32_t group, std::uint32_t threads, int rounds,
                      const char* label) {
    std::vector<double> times;
    std::size_t scratch = 0;
    std::uint64_t font_bytes = 0;
    double best_check = 0;
    fp::LoadTimings best;
    std::memset(&best, 0, sizeof(best));
    for (int round = 0; round < rounds; round += 1) {
        double round_seconds = 0;
        double check_seconds = 0;
        font_bytes = 0;
        for (std::uint32_t g = 0; g < index.group_count; g += 1) {
            const bool wanted = set == kBenchAll || (set == kBenchEager && index.groups[g].eager) ||
                                (set == kBenchOneGroup && g == group);
            if (!wanted) {
                continue;
            }
            fp::LoadedGroup loaded;
            const double t0 = seconds_now();
            const bool ok = fp::pack_load_group(index, g, threads, loaded);
            const double t1 = seconds_now();
            if (!ok) {
                std::printf("load %s: group %s refused (%s)\n", label, group_name(index.groups[g]).c_str(),
                            refusal_name(loaded.refusal));
                return false;
            }
            round_seconds += t1 - t0;
            check_seconds += loaded.timings.check_seconds;
            scratch = std::max(scratch, loaded.scratch_peak);
            font_bytes += index.groups[g].font_bytes;
            if (set == kBenchOneGroup && (times.empty() || t1 - t0 < *std::min_element(times.begin(), times.end()))) {
                best = loaded.timings;
            }
            fp::group_free(loaded);
        }
        if (times.empty() || round_seconds < *std::min_element(times.begin(), times.end())) {
            best_check = check_seconds;
        }
        times.push_back(round_seconds);
    }
    if (times.empty() || font_bytes == 0) {
        std::printf("load %s: nothing to load\n", label);
        return true;
    }
    std::sort(times.begin(), times.end());
    std::printf("load %-12s threads %2u rounds %d: min %.2f ms, median %.2f ms, max %.2f ms, scratch peak %zu bytes "
                "(%.1f%% of the fonts), check %.2f ms\n",
                label, threads, rounds, times[0] * 1e3, times[times.size() / 2] * 1e3, times[times.size() - 1] * 1e3, scratch,
                100.0 * static_cast<double>(scratch) / static_cast<double>(font_bytes), best_check * 1e3);
    if (set == kBenchOneGroup) {
        std::printf("  best round: open %.2f, leading streams %.2f, other streams %.2f, rebuild %.2f, check %.2f ms\n",
                    best.open_seconds * 1e3, best.streams_seconds * 1e3, best.dependent_seconds * 1e3,
                    best.rebuild_seconds * 1e3, best.check_seconds * 1e3);
    }
    return true;
}

static int bench(const char* path, const BenchOptions& options) {
    fp::Bytes pack;
    if (!read_file(path, pack)) {
        std::printf("cannot read %s\n", path);
        return 1;
    }
    fp::PackIndex* index = new fp::PackIndex;
    if (!fp::pack_open(pack.data(), pack.size(), *index)) {
        std::printf("the pack does not open\n");
        delete index;
        return 1;
    }
    print_ratios("pack", *index);
    if (options.baseline != nullptr) {
        fp::Bytes baseline;
        fp::PackIndex* base_index = new fp::PackIndex;
        if (!read_file(options.baseline, baseline) || !fp::pack_open(baseline.data(), baseline.size(), *base_index)) {
            std::printf("the baseline pack does not open\n");
            delete base_index;
            delete index;
            return 1;
        }
        print_ratios("baseline", *base_index);
        std::uint64_t font_bytes = 0;
        for (std::uint32_t g = 0; g < (*index).group_count; g += 1) {
            font_bytes += (*index).groups[g].font_bytes;
        }
        const double difference = static_cast<double>(pack.size()) - static_cast<double>(baseline.size());
        std::printf("groups against the baseline: %+.0f bytes (%+.2f points of the font bytes, %+.2f%% of the baseline)\n",
                    difference, 100.0 * difference / static_cast<double>(font_bytes),
                    100.0 * difference / static_cast<double>(baseline.size()));
        delete base_index;
    }
    std::printf("hardware threads %u\n", std::thread::hardware_concurrency());
    bool ok = true;
    for (std::size_t t = 0; t < options.threads.size() && ok; t += 1) {
        const std::uint32_t threads = options.threads[t];
        for (std::uint32_t g = 0; g < (*index).group_count && ok; g += 1) {
            const std::string label = group_name((*index).groups[g]);
            if (options.only == nullptr || label == options.only) {
                ok = bench_set(*index, kBenchOneGroup, g, threads, options.rounds, label.c_str());
            }
        }
        if (ok && (options.only == nullptr || std::strcmp(options.only, "eager") == 0)) {
            ok = bench_set(*index, kBenchEager, 0, threads, options.rounds, "eager");
        }
        if (ok && (options.only == nullptr || std::strcmp(options.only, "all") == 0)) {
            ok = bench_set(*index, kBenchAll, 0, threads, options.rounds, "all");
        }
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
        std::printf("trace %-34s %.2f ms in all\n", names[k], fp::g_trace[k].load() / 1e6);
    }
#endif
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    std::printf("pack %zu bytes, process peak RSS %ld bytes\n", pack.size(), static_cast<long>(usage.ru_maxrss));
    delete index;
    return ok ? 0 : 1;
}

// ---- damage and claims ---------------------------------------------------------

// Loads damaged copies of the pack. Every load must return (true or false)
// without touching memory it does not own; run this under the sanitizers.
static int damage(const fp::Bytes& pack, int rounds) {
    std::uint64_t random = 0x9E3779B97F4A7C15ull;
    int unopened = 0;
    int groups_loaded = 0;
    int groups_refused = 0;
    fp::PackIndex* index = new fp::PackIndex;
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
        if (!fp::pack_open(damaged.data(), damaged.size(), *index)) {
            unopened += 1;
            continue;
        }
        for (std::uint32_t g = 0; g < (*index).group_count; g += 1) {
            fp::LoadedGroup result;
            if (fp::pack_load_group(*index, g, 0, result)) {
                groups_loaded += 1;
                fp::group_free(result);
            } else {
                groups_refused += 1;
            }
        }
    }
    delete index;
    std::printf("damage: %d rounds, %d refused at the index, %d groups refused, %d groups loaded (each checked by CRC-32)\n",
                rounds, unopened, groups_refused, groups_loaded);
    return 0;
}

// The container as the packer would write it, rebuilt from an opened index,
// so that one claim can be changed and the pack written again.
static void container_from_index(const fp::PackIndex& index, std::vector<fp::ContainerGroup>& groups) {
    groups.resize(index.group_count);
    for (std::uint32_t g = 0; g < index.group_count; g += 1) {
        const fp::PackGroup& source = index.groups[g];
        fp::ContainerGroup& group = groups[g];
        group.name = group_name(source);
        group.eager = source.eager;
        group.font_bytes = source.font_bytes;
        group.stream_bytes = source.stream_bytes;
        group.body.assign(source.body, source.body + source.body_size);
        group.fonts.resize(source.font_count);
        for (std::uint32_t f = 0; f < source.font_count; f += 1) {
            const fp::PackFont& font = index.fonts[source.font_first + f];
            group.fonts[f].name.assign(font.name, font.name_length);
            group.fonts[f].size = font.size;
            group.fonts[f].crc = font.crc;
        }
    }
}

struct ClaimsTally {
    int cases;
    int failures;
};

// Opens damaged and loads group. expect_open false: pack_open must refuse
// (it allocates nothing). Otherwise the group must be refused for the
// expected reason (kGroupLoaded meaning "any refusal before storage"), and
// when the reason is not a check value, before any font storage was asked
// for and with no more scratch than the genuine load of that group. Loads
// run on one thread, so a refused load's work is an exact prefix of the
// genuine load's and the scratch comparison is deterministic.
static void claims_case(const char* what, const fp::Bytes& damaged, std::uint32_t group, bool expect_open,
                        fp::GroupRefusal expected, std::size_t genuine_scratch, ClaimsTally& tally) {
    tally.cases += 1;
    fp::PackIndex* index = new fp::PackIndex;
    const bool opened = fp::pack_open(damaged.data(), damaged.size(), *index);
    bool pass = false;
    std::string detail;
    if (!expect_open) {
        pass = !opened;
        detail = opened ? "opened" : "refused at the index";
    } else if (!opened) {
        detail = "refused at the index (a load was expected to refuse it)";
    } else {
        fp::LoadedGroup loaded;
        const bool ok = fp::pack_load_group(*index, group, 1, loaded);
        if (ok) {
            fp::group_free(loaded);
            detail = "loaded";
        } else if (expected == fp::kGroupBadCheck) {
            pass = loaded.refusal == fp::kGroupBadCheck;
            detail = refusal_name(loaded.refusal);
        } else {
            pass = (expected == fp::kGroupLoaded || loaded.refusal == expected) && loaded.storage_requested == 0 &&
                   loaded.scratch_peak <= genuine_scratch;
            detail = std::string(refusal_name(loaded.refusal)) + ", storage asked " +
                     std::to_string(loaded.storage_requested) + ", scratch " + std::to_string(loaded.scratch_peak);
        }
    }
    if (!pass) {
        tally.failures += 1;
    }
    std::printf("  %-4s %-58s %s\n", pass ? "ok" : "FAIL", what, detail.c_str());
    delete index;
}

static int claims(const fp::Bytes& pack) {
    static const std::uint8_t check_input[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    const std::uint32_t check = fp::crc32(0, check_input, 9);
    const std::uint32_t joined = fp::crc32_combine(fp::crc32(0, check_input, 4), fp::crc32(0, check_input + 4, 5), 5);
    std::printf("crc32 check value %08x (expected cbf43926), combined %08x\n", check, joined);
    if (check != 0xCBF43926u || joined != check) {
        return 1;
    }
    fp::PackIndex* index = new fp::PackIndex;
    if (!fp::pack_open(pack.data(), pack.size(), *index)) {
        std::printf("the genuine pack does not open\n");
        delete index;
        return 1;
    }
    std::vector<fp::ContainerGroup> genuine;
    container_from_index(*index, genuine);
    ClaimsTally tally;
    tally.cases = 0;
    tally.failures = 0;
    for (std::uint32_t g = 0; g < (*index).group_count; g += 1) {
        const fp::PackGroup& group = (*index).groups[g];
        fp::LoadedGroup loaded;
        if (!fp::pack_load_group(*index, g, 1, loaded)) {
            std::printf("the genuine group %s does not load\n", group_name(group).c_str());
            delete index;
            return 1;
        }
        const std::size_t genuine_scratch = loaded.scratch_peak;
        fp::group_free(loaded);
        std::printf("group %s (genuine: storage %zu bytes, scratch peak %zu bytes)\n", group_name(group).c_str(),
                    loaded.storage_requested, genuine_scratch);
        fp::Bytes damaged;
        std::vector<fp::ContainerGroup> changed;

        changed = genuine;
        changed[g].fonts[0].size = 0x7FFFFFFFu;
        changed[g].font_bytes = genuine[g].font_bytes - genuine[g].fonts[0].size + 0x7FFFFFFFu;
        fp::container_write(changed, damaged);
        claims_case("font size 2^31 - 1, group total to match", damaged, g, false, fp::kGroupLoaded, genuine_scratch, tally);

        changed = genuine;
        changed[g].fonts[0].size += 4096;
        changed[g].font_bytes += 4096;
        fp::container_write(changed, damaged);
        claims_case("font size + 4096, group total to match (within limits)", damaged, g, true, fp::kGroupLoaded,
                    genuine_scratch, tally);

        changed = genuine;
        changed[g].font_bytes += 1;
        fp::container_write(changed, damaged);
        claims_case("group font total + 1", damaged, g, false, fp::kGroupLoaded, genuine_scratch, tally);

        changed = genuine;
        changed[g].font_bytes = fp::kMaxGroupFontBytes + 1ull;
        changed[g].fonts[0].size += changed[g].font_bytes - genuine[g].font_bytes;
        fp::container_write(changed, damaged);
        claims_case("group font total above kMaxGroupFontBytes", damaged, g, false, fp::kGroupLoaded, genuine_scratch, tally);

        changed = genuine;
        changed[g].stream_bytes += 1;
        fp::container_write(changed, damaged);
        claims_case("group stream total + 1", damaged, g, true, fp::kGroupBadClaims, genuine_scratch, tally);

        changed = genuine;
        changed[g].stream_bytes = fp::kMaxStreamExpansion * genuine[g].font_bytes + 1;
        fp::container_write(changed, damaged);
        claims_case("group stream total above kMaxStreamExpansion", damaged, g, false, fp::kGroupLoaded, genuine_scratch,
                    tally);

        changed = genuine;
        changed[g].fonts[0].crc ^= 1u;
        fp::container_write(changed, damaged);
        claims_case("first font's CRC-32 with one bit flipped", damaged, g, true, fp::kGroupBadCheck, genuine_scratch, tally);

        // Each stream's own decoded-size claim, raised in place to the largest
        // value its varint length can hold: the body keeps its length, so only
        // that claim changes.
        fp::Reader body = fp::reader_make(group.body, group.body_size);
        for (std::uint32_t k = 0; k < group.font_count * fp::kStreamCount; k += 1) {
            fp::get_varint(body);
        }
        std::size_t blob_sizes[fp::kStreamCount];
        for (std::uint32_t s = 0; s < fp::kStreamCount; s += 1) {
            blob_sizes[s] = static_cast<std::size_t>(fp::get_varint(body));
        }
        std::size_t at = static_cast<std::size_t>(body.cursor - pack.data());
        for (std::uint32_t s = 0; s < fp::kStreamCount && !body.failed; s += 1) {
            damaged = pack;
            std::size_t length = 1;
            while (length < blob_sizes[s] && length < 5 && (damaged[at + length - 1] & 0x80u) != 0) {
                length += 1;
            }
            for (std::size_t b = 0; b + 1 < length; b += 1) {
                damaged[at + b] = 0xFF;
            }
            damaged[at + length - 1] = 0x7F;
            if (blob_sizes[s] != 0 && damaged != pack) {
                const std::string what = "stream " + std::to_string(s) + " decoded size raised to its varint's largest";
                claims_case(what.c_str(), damaged, g, true, fp::kGroupBadClaims, genuine_scratch, tally);
            }
            at += blob_sizes[s];
        }
    }
    fp::Bytes damaged = pack;
    damaged[4] = 0x7F;
    claims_case("group count 127", damaged, 0, false, fp::kGroupLoaded, 0, tally);
    std::printf("claims: %d cases, %d failures\n", tally.cases, tally.failures);
    delete index;
    return tally.failures == 0 ? 0 : 1;
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
        BenchOptions options;
        options.threads.push_back(1);
        options.threads.push_back(4);
        options.threads.push_back(0);
        options.rounds = 25;
        options.baseline = nullptr;
        options.only = nullptr;
        for (int k = 3; k < argc; k += 1) {
            const bool has_value = k + 1 < argc;
            if (std::strcmp(argv[k], "--threads") == 0 && has_value && parse_threads(argv[k + 1], options.threads)) {
                k += 1;
            } else if (std::strcmp(argv[k], "--rounds") == 0 && has_value && std::atoi(argv[k + 1]) > 0) {
                options.rounds = std::atoi(argv[k + 1]);
                k += 1;
            } else if (std::strcmp(argv[k], "--baseline") == 0 && has_value) {
                options.baseline = argv[k + 1];
                k += 1;
            } else if (std::strcmp(argv[k], "--only") == 0 && has_value) {
                options.only = argv[k + 1];
                k += 1;
            } else {
                std::printf("bench: unknown or incomplete option %s\n", argv[k]);
                return 2;
            }
        }
        return bench(argv[2], options);
    }
    if (argc >= 3 && std::strcmp(argv[1], "damage") == 0) {
        fp::Bytes pack;
        if (!read_file(argv[2], pack)) {
            return 1;
        }
        return damage(pack, argc > 3 ? std::atoi(argv[3]) : 200);
    }
    if (argc >= 3 && std::strcmp(argv[1], "claims") == 0) {
        fp::Bytes pack;
        if (!read_file(argv[2], pack)) {
            return 1;
        }
        return claims(pack);
    }
    if (argc >= 4 && std::strcmp(argv[1], "unpack") == 0) {
        fp::Bytes pack;
        if (!read_file(argv[2], pack)) {
            return 1;
        }
        fp::LoadedSet* set = new fp::LoadedSet;
        bool ok = fp::set_open(pack.data(), pack.size(), 0, *set);
        for (std::uint32_t g = 0; ok && g < (*set).index.group_count; g += 1) {
            ok = fp::set_need_group(*set, g);
        }
        for (std::uint32_t g = 0; ok && g < (*set).index.group_count; g += 1) {
            const fp::LoadedGroup& loaded = (*set).groups[g];
            for (std::uint32_t f = 0; ok && f < loaded.font_count; f += 1) {
                const fp::LoadedFont& font = loaded.fonts[f];
                const std::string path = std::string(argv[3]) + "/" + std::string(font.name, font.name_length);
                const fp::Bytes bytes(font.data, font.data + font.size);
                if (!write_file(path.c_str(), bytes)) {
                    std::printf("cannot write %s\n", path.c_str());
                    ok = false;
                }
            }
        }
        if (!ok) {
            std::printf("unpack failed\n");
        }
        fp::set_free(*set);
        delete set;
        return ok ? 0 : 1;
    }
    if (argc >= 4 && std::strcmp(argv[1], "pack") == 0) {
        int first = 3;
        const char* manifest = nullptr;
        if (argc >= 6 && std::strcmp(argv[3], "--manifest") == 0) {
            manifest = argv[4];
            first = 5;
        }
        std::vector<fp::PackInput> fonts;
        std::size_t total = 0;
        if (!read_fonts(argc, argv, first, fonts, total) || fonts.empty()) {
            return 1;
        }
        std::vector<fp::PackGroupSpec> groups;
        if (manifest != nullptr) {
            fp::Bytes text;
            std::string error;
            if (!read_file(manifest, text)) {
                std::printf("cannot read %s\n", manifest);
                return 1;
            }
            if (!fp::manifest_groups(std::string(text.begin(), text.end()), fonts, groups, error)) {
                std::printf("%s: %s\n", manifest, error.c_str());
                return 1;
            }
        } else {
            // The single-group baseline: every font in one eager group.
            fp::PackGroupSpec all;
            all.name = "all";
            all.eager = true;
            for (std::size_t f = 0; f < fonts.size(); f += 1) {
                all.fonts.push_back(static_cast<std::uint32_t>(f));
            }
            groups.push_back(all);
        }
        fp::Bytes pack;
        const double t0 = seconds_now();
        const bool packed = fp::pack_fonts(fonts, groups, pack, true);
        const double t1 = seconds_now();
        if (!packed) {
            return 1;
        }
        if (!write_file(argv[2], pack)) {
            std::printf("cannot write %s\n", argv[2]);
            return 1;
        }
        std::printf("packed %zu fonts in %zu groups: %zu -> %zu bytes (%.2f%%) in %.1f s\n", fonts.size(), groups.size(), total,
                    pack.size(), 100.0 * pack.size() / total, t1 - t0);
        if (!verify(pack, fonts)) {
            std::printf("the pack does NOT reproduce its inputs\n");
            return 1;
        }
        std::printf("verified: the pack loads to the identical bytes\n");
        return 0;
    }
    std::printf("usage: fontpack pack|verify|bench|unpack|damage|claims ...\n");
    return 2;
}
