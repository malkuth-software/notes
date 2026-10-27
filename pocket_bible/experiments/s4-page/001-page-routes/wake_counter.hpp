#pragma once

#include <cstdint>

// How often the process was scheduled, read from the operating system. The
// toolkit's own counters do not see a host timer that wakes the thread
// without entering the window model, and Windows charges processor time in
// 15.625 ms ticks, so a wake that costs microseconds is invisible to both.
//
//   cycles            Windows  QueryProcessCycleTime (exact, in processor cycles)
//                     POSIX    0: not read
//   context_switches  Windows  0: not read
//                     POSIX    getrusage ru_nvcsw + ru_nivcsw
struct WakeCounters final {
    // False when the platform call failed; the other fields are then zero.
    bool valid{false};
    std::uint64_t cycles{0U};
    std::uint64_t context_switches{0U};
};

[[nodiscard]] WakeCounters read_wake_counters() noexcept;
