#pragma once

#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>

// stl
#include <chrono>

namespace zpp_lib {

constexpr k_timeout_t milliseconds_to_ticks(const std::chrono::milliseconds& d) noexcept {
  return { k_ms_to_ticks_ceil32(d.count()) };
}

} // namespace zpp_lib