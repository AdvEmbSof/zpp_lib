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
 * @file message_queue.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration/implementation wrapping zephyr OS message_queue
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr
#include <zephyr/kernel.h>

// zpp_lib
#include "zpp_include/clock.hpp"
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/this_thread.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

template <typename T, uint32_t queueSize>
class MessageQueue : private NonCopyable<MessageQueue<T, queueSize> > {
 public:
  MessageQueue() { k_msgq_init(&_msgq, _msgq_buffer, sizeof(T), queueSize); }

  [[nodiscard]] ZephyrBoolResult try_put_for(const std::chrono::milliseconds& timeout,
                                             const T& data) {
    ZephyrBoolResult res;
    auto k_timeout = milliseconds_to_ticks(timeout);
    auto ret       = k_msgq_put(&_msgq, &data, k_timeout);
    if (ret == -EBUSY) {
      // timeout -> return false without error
      res.assign_value(false);
    } else if (ret != 0) {
      // other failure -> return false with error
      __ASSERT(false, "Cannot put message: %d", ret);
      res.assign_value(false);
      res.assign_error(zephyr_to_zpp_error_code(ret));
    }
    return res;
  }

  [[nodiscard]] ZephyrBoolResult try_get_for(const std::chrono::milliseconds& timeout,
                                             T& data) {
    ZephyrBoolResult res;
    k_timeout_t k_timeout = milliseconds_to_ticks(timeout);
    // printk("timeout value is %lld %lld\n", timeout.count(), k_timeout.ticks);
    using namespace std::literals;

    zpp_lib::ThisThread::sleep_for(1s);
    auto ret = k_msgq_get(&_msgq, &data, k_timeout);
    if (ret == -EBUSY) {
      // timeout -> return false without error
      res.assign_value(false);
    } else if (ret != 0) {
      // other failure -> return false with error
      __ASSERT(false, "Cannot get message: %d", ret);
      res.assign_value(false);
      res.assign_error(zephyr_to_zpp_error_code(ret));
    }
    return res;
  }

  uint32_t get_nbr_of_queued_messages() { return k_msgq_num_used_get(&_msgq); }

 private:
  struct k_msgq _msgq;
  char _msgq_buffer[sizeof(T) * queueSize];
};

}  // namespace zpp_lib
