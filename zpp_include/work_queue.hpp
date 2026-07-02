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
 * @file work_queue.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration/implementation wrapping zephyr OS work_queue
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr
#include <zephyr/kernel.h>

// std
#include <atomic>
#include <chrono>
#include <string>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/work.hpp"
#include "zpp_include/zephyr_result.hpp"
#include "zpp_include/zpp_assert.hpp"

namespace zpp_lib {

class WorkQueue final : private NonCopyable {
public:
  // constructor for running the work queue from an external thread calling run()
  explicit WorkQueue(const char* name) : _name(name), _thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, name) {
    k_work_queue_init(&_work_queue);
  }

  // constructor for running the work queue from an internal thread calling run()
#if CONFIG_USERSPACE
  explicit WorkQueue(const char* name, zpp_lib::PreemptableThreadPriority threadPriority, bool userMode)
      : _name(name), _thread(threadPriority, name, userMode) {
#else   // CONFIG_USERSPACE
  explicit WorkQueue(const char* name, zpp_lib::PreemptableThreadPriority threadPriority) : _name(name), _thread(threadPriority, name) {
#endif  // CONFIG_USERSPACE
    k_work_queue_init(&_work_queue);

    // start the _isrWorkQueueThread thread
    auto res = _thread.start([this]() {
      this->run();
    });
    if (!res) {
      ZPP_ASSERT(false, "Could not start WorkQueue thread: %d", (int)res.error());
    }

    // wait for the thread to be started
    _thread.wait_started();
  }

  void run() {
    if (_is_started.load()) {
      return;
    }
    struct k_work_queue_config cfg = {
        .name     = _name.c_str(),
        .no_yield = true,
    };
    // flag
    _is_started.store(true);

    // signal the event
    _event.set(kStartedEvent);

    k_work_queue_run(&_work_queue, &cfg);
  }

  [[nodiscard]] ZephyrResult stop() {
    ZephyrResult res;
    if (!_is_started.load()) {
      // not started or already stopped, return silently
      return res;
    }
    auto ret = k_work_queue_drain(&_work_queue, true);
    if (ret < 0) {
      ZPP_ASSERT(false, "Failed to drain work queue: %d", ret);
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }
    ret = k_work_queue_stop(&_work_queue, K_FOREVER);
    if (ret != 0) {
      ZPP_ASSERT(false, "Failed to stop work queue: %d", ret);
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }
    _is_started.store(false);
    return res;
  }

  void wait_started() noexcept {
    _event.wait_any(kStartedEvent);
  }

  //  Passing a parameter as a non-const reference is accepted
  //  NOLINTNEXTLINE(runtime/references)
  template <typename Obj, typename... Args> [[nodiscard]] ZephyrResult call(Work<Obj, Args...>& work) {
    ZephyrResult res;
    if (!_is_started.load()) {
      ZPP_ASSERT(false, "Workqueue should have started before calling call()");
      res.assign_error(ZephyrErrorCode::Nodev);
      return res;
    }
    // Non error return values are documented as follows:
    // @retval 0 if work was already submitted to a queue
    // @retval 1 if work was not submitted and has been queued to @p queue
    // @retval 2 if work was running and has been queued to the queue that was running
    // it
    auto ret = k_work_submit_to_queue(&_work_queue, work.native_handle());
    if (ret != 0 && ret != 1 && ret != 2) {
      ZPP_ASSERT(false, "Failed to submit work: %d", ret);
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }
    return res;
  }

private:
  struct k_work_q _work_queue = {};
  std::string _name;
  zpp_lib::Thread _thread;
  Event _event;
  static constexpr uint32_t kStartedEvent = 0x01;
  std::atomic<bool> _is_started            = false;
};  // NOLINT(readability/braces)

}  // namespace zpp_lib
