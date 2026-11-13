// font-pack experiment: fuzzy-match simulation on a stream of numbers.
// Estimates, under one simple cost model, what greedy matching is worth when
// a match must be exact versus when it may carry small per-number corrections.
//   lab_fuzzy <values.i32> [max_residual] [max_bad_run]
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <vector>

static std::vector<std::int32_t> g_values;
static std::vector<double> g_literal_cost;   // bits to code each number as a literal

static double residual_cost(std::int32_t d, double zero_cost) {
    if (d == 0) {
        return zero_cost;
    }
    const double magnitude = std::fabs(static_cast<double>(d));
    return 2.5 + 2.0 * std::log2(magnitude + 1.0);   // flag, sign and a gamma-like magnitude
}

static double simulate(std::int32_t max_residual, std::uint32_t max_bad_run, double zero_cost, std::uint64_t& matched,
                       std::uint64_t& segments) {
    const std::size_t count = g_values.size();
    const std::uint32_t hash_bits = 20;
    std::vector<std::uint32_t> head(1u << hash_bits, 0);
    std::vector<std::uint32_t> chain(count, 0);
    double total = 0.0;
    matched = 0;
    segments = 0;
    std::size_t i = 0;
    std::size_t inserted = 0;
    const double header = 23.0;
    while (i < count) {
        double best_saving = 0.0;
        std::size_t best_length = 0;
        double best_cost = 0.0;
        if (i + 3 <= count) {
            const std::uint32_t h = (static_cast<std::uint32_t>(g_values[i]) * 2654435761u ^
                                     static_cast<std::uint32_t>(g_values[i + 1]) * 40503u ^
                                     static_cast<std::uint32_t>(g_values[i + 2]) * 2246822519u) >> (32 - hash_bits);
            std::uint32_t link = head[h];
            for (int depth = 0; depth < 48 && link != 0; depth += 1) {
                const std::size_t c = link - 1;
                link = chain[c];
                double cost = header;
                double literal = 0.0;
                std::uint32_t bad_run = 0;
                for (std::size_t j = 0; i + j < count && c + j < i; j += 1) {
                    const std::int32_t d = g_values[i + j] - g_values[c + j];
                    if (d > max_residual || d < -max_residual) {
                        break;
                    }
                    bad_run = d == 0 ? 0 : bad_run + 1;
                    if (bad_run > max_bad_run) {
                        break;
                    }
                    cost += residual_cost(d, zero_cost);
                    literal += g_literal_cost[i + j];
                    if (d == 0 && literal - cost > best_saving) {
                        best_saving = literal - cost;
                        best_length = j + 1;
                        best_cost = cost;
                    }
                }
            }
        }
        std::size_t advance = 1;
        if (best_length != 0) {
            total += best_cost;
            matched += best_length;
            segments += 1;
            advance = best_length;
        } else {
            total += g_literal_cost[i];
        }
        for (; inserted < i + advance && inserted + 3 <= count; inserted += 1) {
            const std::uint32_t h = (static_cast<std::uint32_t>(g_values[inserted]) * 2654435761u ^
                                     static_cast<std::uint32_t>(g_values[inserted + 1]) * 40503u ^
                                     static_cast<std::uint32_t>(g_values[inserted + 2]) * 2246822519u) >> (32 - hash_bits);
            chain[inserted] = head[h];
            head[h] = static_cast<std::uint32_t>(inserted + 1);
        }
        i += advance;
    }
    return total;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        return 2;
    }
    std::FILE* file = std::fopen(argv[1], "rb");
    if (file == nullptr) {
        return 1;
    }
    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    g_values.resize(static_cast<std::size_t>(size) / 4);
    if (std::fread(g_values.data(), 4, g_values.size(), file) != g_values.size()) {
        return 1;
    }
    std::fclose(file);
    // Literal cost: order-0 over the values (a stand-in for the context model).
    std::unordered_map<std::int32_t, std::uint32_t> tally;
    for (std::size_t k = 0; k < g_values.size(); k += 1) {
        tally[g_values[k]] += 1;
    }
    g_literal_cost.resize(g_values.size());
    double plain = 0.0;
    for (std::size_t k = 0; k < g_values.size(); k += 1) {
        // 0.8 scales order-0 down to roughly what the (operator, count, index) contexts reach.
        g_literal_cost[k] = 0.8 * std::log2(static_cast<double>(g_values.size()) / tally[g_values[k]]);
        plain += g_literal_cost[k];
    }
    std::printf("numbers %zu, all literal %.2f MB\n", g_values.size(), plain / 8e6);
    const std::int32_t max_residual = argc > 2 ? std::atoi(argv[2]) : 8;
    const std::uint32_t max_bad_run = argc > 3 ? static_cast<std::uint32_t>(std::atoi(argv[3])) : 3;
    std::uint64_t matched = 0;
    std::uint64_t segments = 0;
    double bits = simulate(0, 0, 0.0, matched, segments);
    std::printf("exact matches only:  %.2f MB (%llu numbers in %llu matches)\n", bits / 8e6,
                static_cast<unsigned long long>(matched), static_cast<unsigned long long>(segments));
    static const double zero_costs[3] = {0.3, 0.5, 0.8};
    for (int z = 0; z < 3; z += 1) {
        bits = simulate(max_residual, max_bad_run, zero_costs[z], matched, segments);
        std::printf("fuzzy (|d|<=%d, bad run<=%u, zero %.1f bit): %.2f MB (%llu numbers in %llu matches)\n", max_residual,
                    max_bad_run, zero_costs[z], bits / 8e6, static_cast<unsigned long long>(matched),
                    static_cast<unsigned long long>(segments));
    }
    return 0;
}
