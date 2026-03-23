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
 * @file car_system.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Car System header file
 *
 * @date 2025-07-01
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// stl
#include <chrono>

// zpp_lib
#include "zpp_include/mutex.hpp"
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/semaphore.hpp"
#include "zpp_include/time.hpp"

#if CONFIG_USERSPACE
#include <zephyr/syscalls/time_syscalls.h>
#endif

namespace zpp_lib {

class Barrier : NonCopyable<Barrier> {
 public:
  explicit Barrier(uint32_t nbrOfThreads)
      : _waitSemaphore{0, nbrOfThreads}, _count(nbrOfThreads), _total(nbrOfThreads) {}

  /** Wait for all thread to reach the barrier, last thread gets the time and
   *  all threads get the same synchronized time
   *
   *  @note This function is NOT ISR-safe.
   */
  std::chrono::microseconds wait();

#if CONFIG_USERSPACE
  void grant_access(k_tid_t tid);
#endif  // CONFIG_USERSPACE

 private:
  zpp_lib::Semaphore _waitSemaphore;
  zpp_lib::Mutex _mutex;
  // number of threads still waiting
  uint32_t _count;
  // total thread count
  uint32_t _total;
  // shared start time (same for all threads)
  static std::chrono::microseconds _startTime;
};

}  // namespace zpp_lib
