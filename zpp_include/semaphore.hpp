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
 * @file semaphore.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration for wrapping zephyr os semaphore
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr
#include <zephyr/kernel.h>

// stl
#include <chrono>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {
/** The Semaphore class is used to manage and protect access to a set of shared resources.
 *
 * @note
 * Memory considerations: The semaphore control structures will be created on current
 * thread's stack, both for the mbed OS and underlying RTOS objects (static or dynamic
 * RTOS memory pools are not being used).
 */
class Semaphore : private NonCopyable<Semaphore> {
 public:
  /** Create and Initialize a Semaphore object used for managing resources.
    @param count number of available resources; maximum index value is (count-1).
    (default: 0).

    @note You cannot call this function from ISR context.
  */
  Semaphore(uint32_t initial_count, uint32_t max_count) noexcept;

  /** Wait until a Semaphore resource becomes available.
    @note You cannot call this function from ISR context.
  */
  [[nodiscard]] ZephyrResult acquire();

  /** Try to acquire a Semaphore resource, and return immediately
    @return true if a resource was acquired, false otherwise.
    @note equivalent to try_acquire_for(0)

  @note You may call this function from ISR context.
  */
  [[nodiscard]] ZephyrBoolResult try_acquire();

  /** Release a Semaphore resource that was obtain with Semaphore::acquire.
    @return status code that indicates the execution status of the function:
            @a osOK the token has been correctly released.
            @a osErrorResource the maximum token count has been reached.
            @a osErrorParameter internal error.

    @note You may call this function from ISR context.
  */
  [[nodiscard]] ZephyrResult release(void);

 private:
  struct k_sem _sem_obj;
};

}  // namespace zpp_lib
