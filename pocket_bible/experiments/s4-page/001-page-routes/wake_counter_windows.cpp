#include "wake_counter.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

WakeCounters read_wake_counters() noexcept {
    WakeCounters counters;
    ULONG64 cycles = 0U;
    const BOOL read = QueryProcessCycleTime(GetCurrentProcess(), &cycles);
    if (read == FALSE) {
        return counters;
    }
    counters.valid = true;
    counters.cycles = static_cast<std::uint64_t>(cycles);
    return counters;
}
