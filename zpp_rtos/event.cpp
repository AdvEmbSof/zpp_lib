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
 * @file events.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration for wrapping zephyr os Events
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#if CONFIG_EVENTS
#include "zpp_include/event.hpp"

// zephyr
#if CONFIG_USERSPACE
#include <zephyr/app_memory/app_memdomain.h>
#endif  // CONFIG_USERSPACE

// zpp_lib
#include "zpp_include/clock.hpp"
#include "zpp_include/zephyr_result.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_log.hpp"

#if CONFIG_USERSPACE
extern struct k_mem_partition zpp_lib_partition;
#define ZPP_LIB_DATA K_APP_DMEM(zpp_lib_partition)
#define ZPP_LIB_BSS K_APP_BMEM(zpp_lib_partition)
#else
#define ZPP_LIB_DATA
#define ZPP_LIB_BSS
#endif  // CONFIG_USERSPACE

ZPP_LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

#if CONFIG_USERSPACE
ZPP_LIB_BSS uint8_t Event::ZPP_ASSERT = 0;
// we use busy semantics to avoid initialization
ZPP_LIB_BSS bool ZPP_EVENT_ARRAY_BUSY[CONFIG_ZPP_EVENT_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE];

#define X(name) K_EVENT_DEFINE(name)
#include "events.def"
#undef X
#define X(name) &name,
ZPP_LIB_DATA
static struct k_event* const ZPP_EVENT_ARRAY[] = {
#include "events.def"  // NOLINT(build/include)
};
BUILD_ASSERT(ARRAY_SIZE(ZPP_EVENT_ARRAY) >= CONFIG_ZPP_EVENT_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE);
#undef X
#endif  // CONFIG_USERSPACE

// False positive, _event is initialized with k_event_init
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Event::Event() noexcept
    :
#if !CONFIG_USERSPACE
      _p_event(&_event)
#endif  //! CONFIG_USERSPACE
{
#if CONFIG_USERSPACE
  // kernel objects are allocated statically
  static constexpr uint8_t totalNbrOfEvents = CONFIG_ZPP_EVENT_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE;
  ZPP_ASSERT(_eventInstanceCount < totalNbrOfEvents,
             "Too many events created (pool size is %d)",
             CONFIG_ZPP_EVENT_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE);

  // find a free mutex
  uint8_t index = 0;
  for (; index < totalNbrOfEvents; index++) {
    if (!ZPP_EVENT_ARRAY_BUSY[index]) {
      break;
    }
  }
  ZPP_ASSERT(index < totalNbrOfEvents, "Internal error: free event not found");

  ZPP_EVENT_ARRAY_BUSY[index] = true;
  _p_event                    = ZPP_EVENT_ARRAY[index];
  ZPP_LOG_DBG("Event %p allocated (instance index %d, total %d)", static_cast<void*>(_p_event), index, ZPP_ASSERT);
#else   // CONFIG_USERSPACE
  k_event_init(&_event);
#endif  // CONFIG_USERSPACE
}

#if CONFIG_USERSPACE
Event::~Event() {
  bool found                                = false;
  static constexpr uint8_t totalNbrOfEvents = CONFIG_ZPP_EVENT_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE;
  for (uint8_t index = 0; index < totalNbrOfEvents; index++) {
    if (_p_event == ZPP_EVENT_ARRAY[index]) {
      // clear all events
      static constexpr uint32_t kAllEvents = 0xFFFFFFFF;
      k_event_clear(_p_event, kAllEvents);
      // flag it as free
      ZPP_EVENT_ARRAY_BUSY[index] = false;
      ZPP_ASSERT--;
      LOG_DBG("Event %p freed (instance index %d, total %d)", static_cast<void*>(_p_event), index, ZPP_ASSERT);
      found = true;
      break;
    }
  }
  ZPP_ASSERTSERT(found, "Event %p not found", static_cast<void*>(_p_event));
}
#endif  // CONFIG_USERSPACE

#if CONFIG_USERSPACE
Event::Event(k_event* pEvent) noexcept {
  LOG_DBG("Copy event with address %p", static_cast<void*>(pEvent));
  _p_event = pEvent;
}
#endif  // CONFIG_USERSPACE

void Event::set(uint32_t event_flag) {
  ZPP_LOG_DBG("Set event at address %p", static_cast<void*>(_p_event));
  // Cannot access k_is_in_isr() in user mode on qemu
#if CONFIG_QEMU_TARGET && CONFIG_USERSPACE
  k_event_post(_p_event, event_flag);
#else   // CONFIG_QEMU_TARGET && CONFIG_USERSPACE
  if (k_is_in_isr()) {
    k_event_post(_p_event, event_flag);
  } else {
    k_event_set(_p_event, event_flag);
  }
#endif  // CONFIG_QEMU_TARGET && CONFIG_USERSPACE
}

void Event::wait_any(uint32_t events_flags) noexcept {
  // do not clear the set of events before calling k_event_wait
  uint32_t ret = k_event_wait(_p_event, events_flags, false, K_FOREVER);
  if (ret == 0) {
    // timeout -> return false without error
    ZPP_LOG_DBG("Timemout! unblock without event...");
  }
  // clear the event
  k_event_clear(_p_event, events_flags);
}

ZephyrBoolResult Event::try_wait_any_for(const std::chrono::milliseconds& timeout, uint32_t events_flags) noexcept {
  ZPP_LOG_DBG(
      "Trying to wait on event %p with timeout %lld ms (ticks %lld)", _p_event, timeout.count(), milliseconds_to_ticks(timeout).ticks);
  // do not clear the set of events before calling k_event_wait
  auto ret = k_event_wait(_p_event, events_flags, false, milliseconds_to_ticks(timeout));

  ZephyrBoolResult res;
  if (ret == 0) {
    // timeout -> return false without error
    res.assign_value(false);
  } else if (ret > 0) {
    // handle event
    res.assign_value(true);
  } else {
    ZPP_LOG_ERR("Cannot wait on events: %d", ret);
    ZPP_ASSERT(false, "Cannot wait on event: %d", ret);
    res.assign_value(false);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  // clear the event
  k_event_clear(_p_event, events_flags);

  return res;
}

#if CONFIG_USERSPACE
void Event::grant_access(k_tid_t tid) {
  ZPP_LOG_DBG("Granting access to event %p for thread %p", static_cast<void*>(_p_event), static_cast<void*>(tid));
  k_object_access_grant(_p_event, tid);
}
#endif  // CONFIG_USERSPACE

}  // namespace zpp_lib

#endif  // CONFIG_EVENTS
