#include "zpp_include/time.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>
#include <zephyr/sys/time_units.h>

// stl

namespace zpp_lib {

std::chrono::microseconds Time::getUpTime() {
  // get the system uptime in system ticks
#if CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER 
  uint64_t cycles = sys_clock_cycle_get_64();
  uint64_t us = k_cyc_to_us_floor64(cycles);
  std::chrono::microseconds now(us);
#else
#endif

  return now;
};

}  // namespace zpp_lib