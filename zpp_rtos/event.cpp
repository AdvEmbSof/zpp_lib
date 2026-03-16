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

#if CONFIG_EVENTS == 1
#include "zpp_include/event.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>
#if CONFIG_USERSPACE == 1
#include <zephyr/app_memory/app_memdomain.h>
#endif

// zpp_lib
#include "zpp_include/clock.hpp"
#include "zpp_include/zephyr_result.hpp"

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
ZPP_LIB_BSS uint8_t Event::_eventInstanceCount = 0;

#define X(name) K_EVENT_DEFINE(name)
#include "events.def"
#undef X
#define X(name) &name,
ZPP_LIB_DATA
static struct k_event* const ZPP_EVENT_ARRAY[] = {
#include "events.def"  // NOLINT(build/include)
};
BUILD_ASSERT(ARRAY_SIZE(ZPP_EVENT_ARRAY) >=
             CONFIG_ZPP_EVENT_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE);
#undef X
#endif

Event::Event() noexcept {
#if CONFIG_USERSPACE == 1
  // kernel objects are allocated statically
  __ASSERT(_eventInstanceCount < CONFIG_ZPP_EVENT_POOL_SIZE + CONFIG_ZPP_THREAD_POOL_SIZE,
           "Too many events created");

  // update the thread instance count
  LOG_DBG("Event (instance index %d) created", _eventInstanceCount);
  _p_event = ZPP_EVENT_ARRAY[_eventInstanceCount];
  _eventInstanceCount++;
#else
  k_event_init(&_event);
  _p_event = &_event;
#endif
}

Event::~Event() {}

#if CONFIG_USERSPACE == 1
Event::Event(k_event* pEvent) noexcept {
  LOG_DBG("Copy event with address %p", static_cast<void*>(pEvent));
  _p_event = pEvent;
}
#endif

void Event::set(uint32_t event_flag) {
  LOG_DBG("Set event at address %p", static_cast<void*>(_p_event));
  if (k_is_in_isr()) {
    k_event_post(_p_event, event_flag);
  } else {
    k_event_set(_p_event, event_flag);
  }
}

void Event::wait_any(uint32_t events_flags) noexcept {
  // do not clear the set of events before calling k_event_wait
  uint32_t ret = k_event_wait(_p_event, events_flags, false, K_FOREVER);
  if (ret == 0) {
    // timeout -> return false without error
    LOG_DBG("Timemout! unblock without event...");
  }
  // clear the event
  k_event_clear(_p_event, events_flags);
}

ZephyrBoolResult Event::try_wait_any_for(const std::chrono::milliseconds& timeout,
                                         uint32_t events_flags) noexcept {
  LOG_DBG("Trying to wait on event %p with timeout %lld ms (ticks %lld)",
          _p_event,
          timeout.count(),
          milliseconds_to_ticks(timeout).ticks);
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
    LOG_ERR("Cannot wait on events: %d", ret);
    __ASSERT(false, "Cannot wait on event: %d", ret);
    res.assign_value(false);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  // clear the event
  k_event_clear(_p_event, events_flags);

  return res;
}

#if CONFIG_USERSPACE == 1
void Event::grant_access(k_tid_t tid) {
  LOG_DBG("Granting access to event %p for thread %p",
          static_cast<void*>(_p_event),
          static_cast<void*>(tid));
  k_object_access_grant(_p_event, tid);
}
#endif

}  // namespace zpp_lib

#endif  // CONFIG_EVENTS == 1
