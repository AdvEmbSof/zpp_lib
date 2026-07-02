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

// zephyr
#if CONFIG_USERSPACE
#include <zephyr/app_memory/app_memdomain.h>
#endif  // CONFIG_USERSPACE

#if CONFIG_SEGGER_SYSTEMVIEW
#include "SEGGER_SYSVIEW.h"
#endif  // CONFIG_SEGGER_SYSTEMVIEW

// zpp_lib
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_log.hpp"

ZPP_LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

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
ZPP_LIB_DATA std::chrono::microseconds Barrier::_start_time = 0us;

Barrier::Barrier(uint32_t nbr_of_threads) : _wait_semaphore{0, nbr_of_threads}, _count(nbr_of_threads), _total(nbr_of_threads) {}

#if CONFIG_TEST
std::chrono::microseconds Barrier::wait(const Barrier::ZeroTimeCB& zero_time_cb) {
#else
std::chrono::microseconds Barrier::wait() {
#endif
  auto res = _mutex.lock();
  if (!res) {
    ZPP_ASSERT(false, "Cannot lock mutex: %d", static_cast<int>(res.error()));
  }
  _count--;
  if (_count == 0) {
    // Last thread to arrive — get start time and release all
    _start_time = zpp_lib::Time::get_uptime();

#if CONFIG_TEST
    zero_time_cb(_start_time);
#endif

#if CONFIG_SEGGER_SYSTEMVIEW
#define SYSVIEW_MARK_TIME_ZERO 255U
    SEGGER_SYSVIEW_Mark(SYSVIEW_MARK_TIME_ZERO);
#endif  // CONFIG_SEGGER_SYSTEMVIEW

    for (uint32_t i = 0; i < _total; i++) {
      res = _wait_semaphore.release();
      if (!res) {
        ZPP_ASSERT(false, "Cannot release semaphore: %d", static_cast<int>(res.error()));
      }
    }
  }
  res = _mutex.unlock();
  if (!res) {
    ZPP_ASSERT(false, "Cannot unlock mutex: %d", static_cast<int>(res.error()));
  }

  res = _wait_semaphore.acquire();
  if (!res) {
    ZPP_ASSERT(false, "Cannot acquire semaphore: %d", static_cast<int>(res.error()));
  }

  // _start_time is the same value for all threads
  return _start_time;
}

#if CONFIG_USERSPACE
void Barrier::grant_access(k_tid_t tid) {
  // Grant access to the internal semaphore and mutex attributes
  ZPP_LOG_DBG("Granting access to barrier for thread %p", static_cast<void*>(tid));
  _wait_semaphore.grant_access(tid);
  _mutex.grant_access(tid);
}
#endif  // CONFIG_USERSPACE

}  // namespace zpp_lib
