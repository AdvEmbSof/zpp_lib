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
 * @file events.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration for wrapping zephyr os Events
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

#if CONFIG_EVENTS == 1

// zephyr
#include <zephyr/kernel.h>

// stl
#include <chrono>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

class Event : private NonCopyable<Event> {
 public:
  /** Create and Initialize a Event object
   *
   * @note You cannot call this function from ISR context.
   */
  Event() noexcept;

#if CONFIG_USERSPACE == 1
  /*  When user mode is enabled, allow to construct an event
      based on the kernel object. This allows another thread
  *   to access the kernel object without accessing an unallowed
  *   memory section (such as another thread's stack).
  */
  explicit Event(k_event* pEvent) noexcept;
#endif

  /** Set an event flag in the event object. This unblocks any thread
   *  waiting on that flag.
   *
   *  @note This function is ISR-safe.
   */
  void set(uint32_t event_flag);

  /** Wait indefinitely until one of the specified event flags is set.
   *  Up to 32 flags can be waited on simultaneously.
   *
   *  @note Cannot be called from ISR context.
   */
  void wait_any(uint32_t events_flags) noexcept;

  /**
   * Wait timout until one of the specified event flags is set.
   * Up to 32 flags can be waited on simultaneously.
   *
   * @note Cannot be called from ISR context.
   */
  [[nodiscard]] ZephyrBoolResult try_wait_any_for(
      const std::chrono::milliseconds& timeout, uint32_t events_flags) noexcept;

#if CONFIG_USERSPACE == 1
  /**
   * Grants access to the k_event kernel object for a specific thread
   */
  void grant_access(k_tid_t tid);
#endif

  /** Event destructor
   *
   * @note You cannot call this function from ISR context.
   */
  ~Event();

 private:
#if CONFIG_USERSPACE == 1
  friend class Thread;
  static uint8_t _eventInstanceCount;
#else
  struct k_event _event;
#endif
  struct k_event* _p_event = nullptr;
};

}  // namespace zpp_lib

#endif  // CONFIG_EVENTS == 1
