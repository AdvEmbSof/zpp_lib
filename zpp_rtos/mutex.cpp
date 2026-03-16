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
 * @brief CPP class implemenation wrapping zephyr OS mutex
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#include "zpp_include/mutex.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>
#if CONFIG_USERSPACE == 1
#include <zephyr/app_memory/app_memdomain.h>
#endif

// zpp_lib
#include "zpp_include/clock.hpp"

#if CONFIG_USERSPACE == 1
extern struct k_mem_partition zpp_lib_partition;
#define ZPP_LIB_DATA K_APP_DMEM(zpp_lib_partition)
#define ZPP_LIB_BSS K_APP_BMEM(zpp_lib_partition)
#else
#define ZPP_LIB_DATA
#define ZPP_LIB_BSS
#endif

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

#if CONFIG_USERSPACE == 1
ZPP_LIB_BSS uint8_t Mutex::_mutexInstanceCount = 0;

#define X(name) K_MUTEX_DEFINE(name);
#include "mutexes.def"
#undef X
#define X(name) &name,
ZPP_LIB_DATA
static struct k_mutex* const ZPP_MUTEX_ARRAY[] = {
#include "mutexes.def"  // NOLINT(build/include)
};
BUILD_ASSERT(ARRAY_SIZE(ZPP_MUTEX_ARRAY) >=
             CONFIG_ZPP_MUTEX_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE);
#undef X
#endif

Mutex::Mutex() noexcept {
#if CONFIG_USERSPACE == 1
  // kernel objects are allocated statically
  __ASSERT(_mutexInstanceCount < CONFIG_ZPP_MUTEX_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE,
           "Too many mutexes created");

  // update the thread instance count
  LOG_DBG("Mutex (instance index %d) created", _mutexInstanceCount);
  _p_mutex = ZPP_MUTEX_ARRAY[_mutexInstanceCount];
  _mutexInstanceCount++;
#else
  k_mutex_init(&_mutex);
  _p_mutex = &_mutex;
#endif
}

Mutex::~Mutex() {}

#if CONFIG_USERSPACE == 1
Mutex::Mutex(k_mutex* pMutex) noexcept {
  LOG_DBG("Copy mutex with address %p", static_cast<void*>(pMutex));
  _p_mutex = pMutex;
}
#endif

ZephyrResult Mutex::lock() {
  ZephyrResult res;
  int ret = k_mutex_lock(_p_mutex, K_FOREVER);
  if (ret != 0) {
    LOG_ERR("Cannot lock mutex: %d", ret);
    __ASSERT(false, "Cannot lock mutex: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

ZephyrBoolResult Mutex::try_lock() noexcept {
  return try_lock_for(std::chrono::milliseconds::zero());
}

ZephyrBoolResult Mutex::try_lock_for(const std::chrono::milliseconds& timeout) {
  LOG_DBG("Trying to lock mutex with timeout %lld ms (ticks %lld)",
          timeout.count(),
          milliseconds_to_ticks(timeout).ticks);
  auto ret = k_mutex_lock(_p_mutex, milliseconds_to_ticks(timeout));
  ZephyrBoolResult res;
  if (ret == -EAGAIN) {
    // timeout -> return false without error
    res.assign_value(false);
  } else if (ret != 0) {
    // other failure -> return false with error
    LOG_ERR("Cannot lock mutex: %d", ret);
    __ASSERT(false, "Cannot lock mutex: %d", ret);
    res.assign_value(false);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

ZephyrResult Mutex::unlock() {
  ZephyrResult res;
  int ret = k_mutex_unlock(_p_mutex);
  if (ret != 0) {
    LOG_ERR("Cannot unlock mutex: %d", ret);
    __ASSERT(false, "Cannot unlock mutex: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

#if CONFIG_USERSPACE == 1
void Mutex::grant_access(k_tid_t tid) {
  LOG_DBG("Granting access to mutex %p for thread %p",
          static_cast<void*>(_p_mutex),
          static_cast<void*>(tid));
  k_object_access_grant(_p_mutex, tid);
}
#endif

}  // namespace zpp_lib
