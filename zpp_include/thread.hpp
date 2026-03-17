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
 * @file thread.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration wrapping zephyr OS thread
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/
#pragma once

// zephyr
#include <zephyr/kernel.h>

// std
#include <functional>
#include <string>

// zpp_lib
#include "zpp_include/event.hpp"
#include "zpp_include/mutex.hpp"
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/types.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

class Thread : private NonCopyable<Thread> {
 public:
  /** Allocate a new thread without starting execution
    @param   priority       initial priority of the thread function. (default:
    osPriorityNormal).
    @param   name           name to be used for this thread. It has to stay allocated
    for the lifetime of the thread (default: nullptr)
#if CONFIG_USERSPACE
    @param   userMode       flag stating whether the thread must be created in user mode
#endif // CONFIG_USERSPACE

    @note You cannot call this function from ISR context.
  */
#if CONFIG_USERSPACE
  explicit Thread(
      PreemptableThreadPriority priority = PreemptableThreadPriority::PriorityNormal,
      const char* name                   = nullptr,
      bool userMode                      = false);
#else   // CONFIG_USERSPACE
  explicit Thread(
      PreemptableThreadPriority priority = PreemptableThreadPriority::PriorityNormal,
      const char* name                   = nullptr);
#endif  // CONFIG_USERSPACE

  /** Performs sanity checks
   */
  virtual ~Thread();

  /** Starts a thread executing the specified function.
    @param   task           function to be executed by this thread.
    @return  status code that indicates the execution status of the function,
             or osErrorNoMemory if stack allocation failed.
    @note a thread can only be started once

    @note You cannot call this function ISR context.
  */
  using task_function_t = std::function<void()>;
  [[nodiscard]] ZephyrResult start(task_function_t task) noexcept;

  /** Wait for thread to have started
    @return  status code that indicates the execution status of the function.

    @note You cannot call this function from ISR context.
  */
  void waitStarted() noexcept;

  /** Wait for thread to terminate
    @return  status code that indicates the execution status of the function.

    @note You cannot call this function from ISR context.
  */
  [[nodiscard]] ZephyrResult join() noexcept;

 private:
  // Required to share definitions without
  // delegated constructors
  static void _thunk(void* p1, void* p2, void* p3);

 private:
  Mutex _mutex;
  Event _event;
#if CONFIG_USERSPACE
  bool _userMode = false;
#else   // CONFIG_USERSPACE
  Thread::task_function_t _task;
#endif  // CONFIG_USERSPACE

  PreemptableThreadPriority _priority;
  static constexpr uint32_t kStartedEvent = 0x01;
  std::string _name;
  k_tid_t _tid = nullptr;

  // used for accessing the corresponding statically allocated stack
  static uint8_t _threadInstanceCount;
};

}  // namespace zpp_lib
