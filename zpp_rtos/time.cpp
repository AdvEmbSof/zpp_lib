// Copyright 2025 Haute école d'ingénierie et d'architecture de Fribourg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file time.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class implementation wrapping zephyr OS uptime function
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

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
  uint64_t cycles =
      sys_clock_cycle_get_64();  // CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC25_000_000
#else
  uint32_t cycles = sys_clock_cycle_get_32();
#endif
  uint64_t us = k_cyc_to_us_floor64(cycles);
#endif
  std::chrono::microseconds now(us);
  return now;
}

}  // namespace zpp_lib
