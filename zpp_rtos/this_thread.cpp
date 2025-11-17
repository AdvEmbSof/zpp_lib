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
 * @file this_thread.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Implementation of various helper functions for accessing the running thread
 *        within ThisThread namespace
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

// zpp_lib
#include "zpp_include/this_thread.hpp"

// zephyr
#include <zephyr/kernel.h>

namespace zpp_lib {

namespace ThisThread {

PreemptableThreadPriority getPriority() {
  k_tid_t tid = k_current_get();
#if ASSERT
  __ASSERT(tid != nullptr, "Current thread has no tid");
#endif
  int prio = k_thread_priority_get(tid);
  return prio_to_preemptable_thread_priority(prio);
}

void setPriority(PreemptableThreadPriority priority) {
  k_tid_t tid = k_current_get();
#if ASSERT
  __ASSERT(tid != nullptr, "Current thread has no tid");
#endif
  k_thread_priority_set(tid, preemptable_thread_priority_to_zephyr_prio(priority));
}

void busyWait(const std::chrono::microseconds& waitTime) {
  // k_busy_wait takes usecs
  k_busy_wait(waitTime.count());
}

std::chrono::milliseconds sleep_for(const std::chrono::milliseconds& sleep_duration) {
  auto res = k_msleep(sleep_duration.count());
  return std::chrono::milliseconds(res);
}

std::chrono::milliseconds sleep_for(const std::chrono::microseconds& sleep_duration) {
  std::chrono::milliseconds ms_sleep_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(sleep_duration);
  auto res = k_msleep(ms_sleep_duration.count());
  return std::chrono::milliseconds(res);
}

std::chrono::milliseconds sleep_for(const std::chrono::seconds& sleep_duration) {
  std::chrono::milliseconds ms_sleep_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(sleep_duration);
  auto res = k_msleep(ms_sleep_duration.count());
  return std::chrono::milliseconds(res);
}

}  // namespace ThisThread

}  // namespace zpp_lib
