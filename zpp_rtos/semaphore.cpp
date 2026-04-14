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
 * @file semaphore.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class implementation wrapping zephyr OS semaphore
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#include "zpp_include/semaphore.hpp"

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
#else  // CONFIG_USERSPACE
#define ZPP_LIB_DATA
#define ZPP_LIB_BSS
#endif  // CONFIG_USERSPACE

namespace zpp_lib {

#if CONFIG_USERSPACE
ZPP_LIB_BSS uint8_t Semaphore::_semaphoreInstanceCount = 0;
// we use busy semantics to avoid initialization
ZPP_LIB_BSS bool
    ZPP_SEMAPHORE_ARRAY_BUSY[CONFIG_ZPP_MUTEX_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE];
// the k_sem array must be initialized in global memory (not application domain)
static struct k_sem ZPP_SEMAPHORE_ARRAY[CONFIG_ZPP_SEMAPHORE_POOL_SIZE] = {};
#endif  // CONFIG_USERSPACE

Semaphore::Semaphore(uint32_t initial_count, uint32_t max_count) noexcept {
#if CONFIG_USERSPACE
  // kernel objects are allocated statically
  __ASSERT(_semaphoreInstanceCount < CONFIG_ZPP_SEMAPHORE_POOL_SIZE,
           "Too many semaphores created");

  // find a free semaphore
  uint8_t index = 0;
  for (; index < CONFIG_ZPP_SEMAPHORE_POOL_SIZE; index++) {
    if (!ZPP_SEMAPHORE_ARRAY_BUSY[index]) {
      break;
    }
  }
  __ASSERT(index < CONFIG_ZPP_SEMAPHORE_POOL_SIZE,
           "Internal error: free semaphore not found");

  // update the thread instance count
  __ASSERT_EVAL(
      k_sem_init(&ZPP_SEMAPHORE_ARRAY[index], initial_count, max_count),
      auto ret = k_sem_init(&ZPP_SEMAPHORE_ARRAY[index], initial_count, max_count),
      ret == 0,
      "Cannot create semaphore: %d",
      ret);
  _p_sem = &ZPP_SEMAPHORE_ARRAY[index];
  _semaphoreInstanceCount++;
  LOG_DBG("Semaphore %p allocated (instance index %d, total %d)",
          static_cast<void*>(_p_sem),
          index,
          _semaphoreInstanceCount);
#else   // CONFIG_USERSPACE
  __ASSERT_EVAL(k_sem_init(&_sem, initial_count, max_count),
                auto ret = k_sem_init(&_sem, initial_count, max_count),
                ret == 0,
                "Cannot create semaphore: %d",
                ret);
  _p_sem = &_sem;
  LOG_DBG("Semaphore %p created with count %d (vs %d) and max count %d",
          static_cast<void*>(_p_sem),
          k_sem_count_get(_p_sem),
          initial_count,
          max_count);
#endif  // CONFIG_USERSPACE
}

#if CONFIG_USERSPACE
Semaphore::~Semaphore() {
  bool found = false;
  for (uint8_t index = 0; index < CONFIG_ZPP_SEMAPHORE_POOL_SIZE; index++) {
    if (_p_sem == &ZPP_SEMAPHORE_ARRAY[index]) {
      // flag it as free
      ZPP_SEMAPHORE_ARRAY_BUSY[index] = false;
      _semaphoreInstanceCount--;
      LOG_DBG("Semaphore %p freed (instance index %d, total %d)",
              static_cast<void*>(_p_sem),
              index,
              _semaphoreInstanceCount);
      found = true;
      break;
    }
  }
  __ASSERT(found, "Semaphore %p not found", static_cast<void*>(_p_sem));
}
#endif  // CONFIG_USERSPACE

ZephyrResult Semaphore::acquire() {
  LOG_DBG("Acquiring semaphore %p with count %d", _p_sem, k_sem_count_get(_p_sem));
  ZephyrResult res;
  int ret = k_sem_take(_p_sem, K_FOREVER);
  if (ret != 0) {
    LOG_ERR("Cannot acquire semaphore: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

ZephyrBoolResult Semaphore::try_acquire() {
  ZephyrBoolResult res;
  int ret = k_sem_take(_p_sem, K_NO_WAIT);
  if (ret == -EBUSY) {
    // timeout -> return false without error
    res.assign_value(false);
  } else if (ret != 0) {
    // other failure -> return false with error
    LOG_ERR("Cannot acquire semaphore: %d", ret);
    res.assign_value(false);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

ZephyrResult Semaphore::release() {
  ZephyrResult res;
  LOG_DBG("Releasing semaphore %p with count %d", _p_sem, k_sem_count_get(_p_sem));
  k_sem_give(_p_sem);
  return res;
}

#if CONFIG_USERSPACE
void Semaphore::grant_access(k_tid_t tid) {
  LOG_DBG("Granting access to semaphore %p for thread %p",
          static_cast<void*>(_p_sem),
          static_cast<void*>(tid));
  k_object_access_grant(_p_sem, tid);
}
#endif  // CONFIG_USERSPACE

}  // namespace zpp_lib
