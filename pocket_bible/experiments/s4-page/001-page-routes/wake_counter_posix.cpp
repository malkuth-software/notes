#include "wake_counter.hpp"

#include <sys/resource.h>

WakeCounters read_wake_counters() noexcept {
    WakeCounters counters;
    struct rusage usage {};
    const int result = getrusage(RUSAGE_SELF, &usage);
    if (result != 0) {
        return counters;
    }
    counters.valid = true;
    counters.context_switches =
        static_cast<std::uint64_t>(usage.ru_nvcsw) + static_cast<std::uint64_t>(usage.ru_nivcsw);
    return counters;
}
