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
 * @file mutex.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Barrier class implementation
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#include "zpp_include/barrier.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>
#if CONFIG_USERSPACE
#include <zephyr/app_memory/app_memdomain.h>
#endif  // CONFIG_USERSPACE

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

#if CONFIG_USERSPACE
extern struct k_mem_partition zpp_lib_partition;
#define ZPP_LIB_DATA K_APP_DMEM(zpp_lib_partition)
#define ZPP_LIB_BSS K_APP_BMEM(zpp_lib_partition)
#else
#define ZPP_LIB_DATA
#define ZPP_LIB_BSS
#endif  // CONFIG_USERSPACE

namespace zpp_lib {

using std::literals::chrono_literals::operator""us;
ZPP_LIB_DATA std::chrono::microseconds Barrier::_startTime = 0us;

std::chrono::microseconds Barrier::wait() {    
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

#if CONFIG_USERSPACE
void Barrier::grant_access(k_tid_t tid) {
  // Grant access to the internal semaphore and mutex attributes
  LOG_DBG("Granting access to barrier for thread %p", static_cast<void*>(tid));
  _waitSemaphore.grant_access(tid);
  _mutex.grant_access(tid);
}  
#endif  // CONFIG_USERSPACE

}  // namespace zpp_lib
