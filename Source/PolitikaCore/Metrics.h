#pragma once

// Portable measurement helpers for Development network evidence (MP-2, F36). They carry no
// gameplay meaning; the UE adapter feeds them sorted samples and engine counters.

#include <cstddef>
#include <cstdint>

namespace politika::metrics {

// Nearest-rank percentile of an ascending-sorted sample set: the smallest sample with at
// least `percent`% of the set at or below it. Empty sets report the zero value.
template <typename T>
[[nodiscard]] constexpr T NearestRank(const T* sorted, std::size_t count, unsigned percent) noexcept {
    if (count == 0 || sorted == nullptr) return T{};
    if (percent == 0) return sorted[0];
    if (percent >= 100) return sorted[count - 1];
    const std::size_t rank = (static_cast<std::size_t>(percent) * count + 99) / 100;
    return sorted[rank - 1];
}

// Average rate over an interval; a non-positive interval or a negative delta reports 0, so
// a counter reset between samples never produces a negative bandwidth.
[[nodiscard]] constexpr double RatePerSecond(std::int64_t delta, double seconds) noexcept {
    return seconds > 0.0 && delta >= 0 ? static_cast<double>(delta) / seconds : 0.0;
}

} // namespace politika::metrics
