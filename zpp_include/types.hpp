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
 * @file types.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration of various types used with zephyr OS
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

namespace zpp_lib {

// we expect CONFIG_NUM_PREEMPT_PRIORITIES to be at least 8
#if CONFIG_NUM_PREEMPT_PRIORITIES < 8
#error zpp_lib requires CONFIG_NUM_PREEMPT_PRIORITIES >= 8
#endif

// Preemptable thread priority values
enum class PreemptableThreadPriority {
  PriorityIdle        = CONFIG_NUM_PREEMPT_PRIORITIES - 1,  ///< Reserved for Idle thread.
  PriorityLow         = CONFIG_NUM_PREEMPT_PRIORITIES - 2,  ///< Priority: low
  PriorityBelowNormal = CONFIG_NUM_PREEMPT_PRIORITIES - 3,  ///< Priority: below normal
  PriorityNormal      = CONFIG_NUM_PREEMPT_PRIORITIES - 4,  ///< Priority: normal
  PriorityAboveNormal = CONFIG_NUM_PREEMPT_PRIORITIES - 5,  ///< Priority: above normal
  PriorityHigh        = CONFIG_NUM_PREEMPT_PRIORITIES - 6,  ///< Priority: high
  PriorityRealtime    = CONFIG_NUM_PREEMPT_PRIORITIES - 7,  ///< Priority: realtime
  PriorityISR = -CONFIG_NUM_PREEMPT_PRIORITIES  ///< Highest priority (Reserved for ISR
                                                ///< deferred thread)
};

constexpr PreemptableThreadPriority prio_to_preemptable_thread_priority(
    int prio) noexcept {
  return static_cast<PreemptableThreadPriority>(prio);
}

constexpr int preemptable_thread_priority_to_zephyr_prio(
    PreemptableThreadPriority prio) noexcept {
  return static_cast<int>(prio);
}

}  // namespace zpp_lib
