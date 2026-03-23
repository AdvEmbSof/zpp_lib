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

#if CONFIG_USERSPACE
extern uint8_t gMsgqInstanceCount;
extern struct k_msgq ZPP_MESSAGE_QUEUE_ARRAY[CONFIG_ZPP_MSGQ_POOL_SIZE];
#endif  // CONFIG_USERSPACE

template <typename T, uint32_t QueueSize>
class MessageQueue : private NonCopyable<MessageQueue<T, QueueSize> > {
 public:
#if CONFIG_USERSPACE
  explicit MessageQueue(char* msgqBuffer) {
    // kernel objects are allocated statically
    __ASSERT(gMsgqInstanceCount < CONFIG_ZPP_MSGQ_POOL_SIZE,
             "Too many message queues created");

    // update the thread instance count
    k_msgq_init(
        &ZPP_MESSAGE_QUEUE_ARRAY[gMsgqInstanceCount], msgqBuffer, sizeof(T), QueueSize);
    _p_msgq = &ZPP_MESSAGE_QUEUE_ARRAY[gMsgqInstanceCount];
    gMsgqInstanceCount++;
#else   // CONFIG_USERSPACE
  MessageQueue() {
    k_msgq_init(&_msgq, _msgq_buffer, sizeof(T), QueueSize);
    _p_msgq = &_msgq;
#endif  // // CONFIG_USERSPACE
  }

  [[nodiscard]] ZephyrBoolResult try_put_for(const std::chrono::milliseconds& timeout,
                                             const T& data) {
    auto k_timeout = milliseconds_to_ticks(timeout);
    auto ret       = k_msgq_put(_p_msgq, &data, k_timeout);
    ZephyrBoolResult res;
    if (ret == -EAGAIN) {
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
    k_timeout_t k_timeout = milliseconds_to_ticks(timeout);
    auto ret              = k_msgq_get(_p_msgq, &data, k_timeout);
    ZephyrBoolResult res;
    if (ret == -EAGAIN) {
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

  uint32_t get_nbr_of_queued_messages() { return k_msgq_num_used_get(_p_msgq); }

#if CONFIG_USERSPACE
  void grant_access(k_tid_t tid) { k_object_access_grant(_p_msgq, tid); }
#endif  // CONFIG_USERSPACE

 private:
#if CONFIG_USERSPACE
#else   // CONFIG_USERSPACE
  struct k_msgq _msgq;
  char _msgq_buffer[sizeof(T) * QueueSize];
#endif  // CONFIG_USERSPACE
  struct k_msgq* _p_msgq = nullptr;
};  // NOLINT(readability/braces)

}  // namespace zpp_lib
