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

namespace zpp_lib {

class WorkQueue : private NonCopyable<WorkQueue> {
 public:
  // constructor for running the work queue from an external thread calling run()
  explicit WorkQueue(const char* name) : _name(name) { k_work_queue_init(&_workQueue); }

  // constructor for running the work queue from an internal thread calling run()
  explicit WorkQueue(const char* name, zpp_lib::PreemptableThreadPriority threadPriority)
      : _name(name), _thread(threadPriority, name) {
    k_work_queue_init(&_workQueue);

    // start the _isrWorkQueueThread thread
    auto res = _thread.start(std::bind(&zpp_lib::WorkQueue::run, this));
    if (!res) {
      __ASSERT(false, "Could not start WorkQueue thread: %d", (int)res.error());
    }

    // wait for the thread to be started
    _thread.waitStarted();
  }

  void run() {
    if (_isStarted.load()) {
      return;
    }
    struct k_work_queue_config cfg = {
        .name     = _name.c_str(),
        .no_yield = true,
    };
    _isStarted.store(true);
    k_work_queue_run(&_workQueue, &cfg);
  }

  [[nodiscard]] ZephyrResult stop() {
    ZephyrResult res;
    if (!_isStarted.load()) {
      // not started or already stopped, return silently
      return res;
    }
    auto ret = k_work_queue_drain(&_workQueue, true);
    if (ret < 0) {
      __ASSERT(false, "Failed to drain work queue: %d", ret);
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }
    ret = k_work_queue_stop(&_workQueue, K_FOREVER);
    if (ret != 0) {
      __ASSERT(false, "Failed to stop work queue: %d", ret);
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }
    _isStarted.store(false);
    return res;
  }

  template <typename F>
  // Passing a parameter as a non-const reference is accepted
  // NOLINTNEXTLINE(runtime/references)
  [[nodiscard]] ZephyrResult call(Work<F>& work) {
    ZephyrResult res;
    if (!_isStarted.load()) {
      __ASSERT(false, "Workqueue should have started before calling call()");
      res.assign_error(ZephyrErrorCode::k_nodev);
      return res;
    }
    // Non error return values are documented as follows:
    // @retval 0 if work was already submitted to a queue
    // @retval 1 if work was not submitted and has been queued to @p queue
    // @retval 2 if work was running and has been queued to the queue that was running
    // it
    auto ret = k_work_submit_to_queue(&_workQueue, &work._workInfo._work);
    if (ret != 0 && ret != 1 && ret != 2) {
      __ASSERT(false, "Failed to submit work: %d", ret);
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }
    return res;
  }

 private:
  struct k_work_q _workQueue;
  std::string _name;
  zpp_lib::Thread _thread;
  std::atomic<bool> _isStarted = false;
};

}  // namespace zpp_lib
