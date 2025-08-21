#include "zpp_include/time.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>
#include <zephyr/sys/time_units.h>

// stl

namespace zpp_lib {

std::chrono::microseconds Time::getUpTime() {
  // get the system uptime in system ticks
#if CONFIG_QEMU_TARGET
  // k_uptime_ticks (on Qemu, the tick duration is 1 msec)
  uint64_t us = (k_uptime_ticks() * USEC_PER_SEC) / CONFIG_SYS_CLOCK_TICKS_PER_SEC;
#else
#if CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER 
  uint64_t cycles = sys_clock_cycle_get_64(); // CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC25_000_000 
#else
  uint32_t cycles = sys_clock_cycle_get_32();  
#endif
  uint64_t us = k_cyc_to_us_floor64(cycles);
#endif
  std::chrono::microseconds now(us);
  return now;
};

}  // namespace zpp_lib