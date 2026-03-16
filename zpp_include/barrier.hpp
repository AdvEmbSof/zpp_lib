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

namespace zpp_lib {

class Barrier : NonCopyable<Barrier> {
 public:
  explicit Barrier(uint32_t nbrOfThreads)
      : _waitSemaphore{0, nbrOfThreads}, _count(nbrOfThreads), _total(nbrOfThreads) {}

  std::chrono::microseconds wait() {
    auto res = _mutex.lock();
    if (!res) {
      __ASSERT(false, "Cannot lock mutex: %d", static_cast<int>(res.error()));
    }
    _count--;
    if (_count == 0) {
      // Last thread to arrive — get start time and release all
      _startTime = zpp_lib::Time::get_uptime();
      for (uint32_t i = 0; i < _total; i++) {
        res = _waitSemaphore.release();
        if (!res) {
          __ASSERT(false, "Cannot release semaphore: %d", static_cast<int>(res.error()));
        }
      }
    }
    res = _mutex.unlock();
    if (!res) {
      __ASSERT(false, "Cannot unlock mutex: %d", static_cast<int>(res.error()));
    }

    res = _waitSemaphore.acquire();
    if (!res) {
      __ASSERT(false, "Cannot acquire semaphore: %d", static_cast<int>(res.error()));
    }

    // _startTime is the same value for all threads
    return _startTime;
  }

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
