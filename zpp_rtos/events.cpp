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

// zpp_lib
#include "zpp_include/events.hpp"
#include "zpp_include/clock.hpp"
#include "zpp_include/zephyr_result.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

Events::Events() noexcept { k_event_init(&_event_obj);}

Events::~Events() {}

void Events::set(uint32_t event_flag) {
    if (k_is_in_isr()) {
        k_event_post(&_event_obj, event_flag);
    } else {
        k_event_set(&_event_obj, event_flag);
    }
}

void Events::wait_any(uint32_t events_flags) noexcept {
    uint32_t ret = k_event_wait(&_event_obj, events_flags, true, K_FOREVER);
    if (ret == 0){
        //timeout -> return false without error
        LOG_DBG("Timemout! unblock wihtout event...");
    }
}

ZephyrBoolResult Events::try_wait_any_for(const std::chrono::milliseconds& timeout, uint32_t events_flags) noexcept {
    LOG_DBG("Trying to wait on event with timeout %lld ms (ticks %lld)",
          timeout.count(),
          milliseconds_to_ticks(timeout).ticks);
    auto ret = k_event_wait(&_event_obj, events_flags, true, milliseconds_to_ticks(timeout));

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
    return res;

}

}  // namespace zpp_lib


#endif  // CONFIG_EVENTS == 1