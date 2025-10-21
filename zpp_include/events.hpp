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

//zpp_lib
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

class Events {
public:
  /** Create and Initialize a Events object
   *
   * @note You cannot call this function from ISR context.
   */
  Events() noexcept; 

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
  
  /** Events destructor
   *
   * @note You cannot call this function from ISR context.
   */
  ~Events();

private:
    struct k_event _event_obj;
};

}  // namespace zpp_lib

#endif  // CONFIG_EVENTS == 1