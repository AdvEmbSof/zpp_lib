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
 * @file thread.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class implementation wrapping zephyr OS thread
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#include "zpp_include/thread.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>
#if CONFIG_USERSPACE
#include <zephyr/app_memory/app_memdomain.h>
#endif  // CONFIG_USERSPACE

// stl
// for std::scoped_lock definition
#include <mutex>

#if CONFIG_USERSPACE
extern struct k_mem_partition zpp_lib_partition;
#define ZPP_LIB_DATA K_APP_DMEM(zpp_lib_partition)
#else
#define ZPP_LIB_DATA
#endif  // CONFIG_USERSPACE

LOG_MODULE_REGISTER(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

// DECLARE STATIC GLOBAL VARIABLES
// Allocate stacks for threads created with zpp_lib
static K_THREAD_STACK_ARRAY_DEFINE(ZPP_THREADS_STACKS,
                                   CONFIG_ZPP_THREAD_POOL_SIZE,
                                   CONFIG_ZPP_THREAD_STACK_SIZE);
// Allocate a static k_thread array for preventing crashes in the SystemView tracing
// library and more generally for preventing stack overflow that may happen if we allocate
// too large objects on the stack
static struct k_thread _thread_data[CONFIG_ZPP_THREAD_POOL_SIZE] = {0};

#if CONFIG_USERSPACE
// initialize static data members
// _threadInstanceCount must be located in app domain since
// it is accessed by the thread
ZPP_LIB_DATA uint8_t Thread::_threadInstanceCount                           = 0;
ZPP_LIB_DATA Thread::task_function_t ZPP_TASKS[CONFIG_ZPP_THREAD_POOL_SIZE] = {nullptr};
#else   // CONFIG_USERSPACE
uint8_t Thread::_threadInstanceCount = 0;
#endif  // CONFIG_USERSPACE

#if CONFIG_USERSPACE
Thread::Thread(PreemptableThreadPriority priority, const char* name, bool userMode) {
#else   // CONFIG_USERSPACE
Thread::Thread(PreemptableThreadPriority priority, const char* name) {
#endif  // CONFIG_USERSPACE
  _priority = priority;
  _name     = name ? name : "application_unnamed_thread";
#if CONFIG_USERSPACE
  _userMode = userMode;
#endif  // CONFIG_USERSPACE
}

Thread::~Thread() {
  if (_tid != nullptr) {
    auto ret = k_thread_join(_tid, K_FOREVER);
    if (ret != 0) {
      LOG_DBG("Failed to join: %d", ret);
    }
  }
}

ZephyrResult Thread::start(std::function<void()> task) noexcept {
  std::scoped_lock<Mutex> guard(_mutex);

  // check that the thread was not already started
  ZephyrResult res;
  if (_tid != nullptr) {
    res.assign_error(ZephyrErrorCode::k_already);
    return res;
  }

  // the thread stacks are allocated statically
  __ASSERT(_threadInstanceCount < CONFIG_ZPP_THREAD_POOL_SIZE,
           "Too many threads created");

  // create the thread
  k_timeout_t delay = K_FOREVER;
#if CONFIG_USERSPACE
  // initialize callback used in Thread::_thunk
  ZPP_TASKS[_threadInstanceCount] = task;

  /* In user mode, initialize this thread with K_FOREVER timeout so we can
   * modify its permissions and then start it.
   */
  uint32_t options = _userMode ? (K_USER | K_INHERIT_PERMS) : K_INHERIT_PERMS;
#else   // CONFIG_USERSPACE
  // initialize callback used in Thread::_thunk
  _task = task;

  uint32_t options = 0;
#endif  // CONFIG_USERSPACE
  int zephyr_priority = preemptable_thread_priority_to_zephyr_prio(_priority);
  LOG_DBG("Creating thread with stack at %p of size %d, priority %d and name %s",
          ZPP_THREADS_STACKS[_threadInstanceCount],
          K_THREAD_STACK_SIZEOF(ZPP_THREADS_STACKS[_threadInstanceCount]),
          zephyr_priority,
          _name.c_str());
  // k_thread_create returns k_tid_t that is in fact typedef struct k_thread *k_tid_t;
  // so the return value of k_thread_create is in fact _thread_data initialized
#if CONFIG_USERSPACE
  _tid = k_thread_create(&_thread_data[_threadInstanceCount],
                         ZPP_THREADS_STACKS[_threadInstanceCount],
                         K_THREAD_STACK_SIZEOF(ZPP_THREADS_STACKS[_threadInstanceCount]),
                         Thread::_thunk,
                         // cppcheck-suppress cstyleCast
                         // NOLINTNEXTLINE(readability/casting)
                         (void*)static_cast<uint32_t>(
                             _threadInstanceCount),  // MISRA-suppress: 7.2.1  legacy API,
                                                     // reviewed by Serge 2026-03-11
                         _event._p_event,
                         _mutex._p_mutex,
                         zephyr_priority,
                         options,
                         delay);
#else   // CONFIG_USERSPACE
  _tid = k_thread_create(
      &_thread_data[_threadInstanceCount],
      ZPP_THREADS_STACKS[_threadInstanceCount],
      K_THREAD_STACK_SIZEOF(ZPP_THREADS_STACKS[_threadInstanceCount]),
      Thread::_thunk,
      // cppcheck-suppress cstyleCast
      // NOLINTNEXTLINE(readability/casting)
      (void*)this,  // MISRA-suppress: 7.2.1  legacy API, reviewed by Serge 2026-03-11
      nullptr,
      nullptr,
      zephyr_priority,
      options,
      delay);
#endif  // CONFIG_USERSPACE
  if (_tid == nullptr) {
    __ASSERT(false, "_tid is null");
    res.assign_error(ZephyrErrorCode::k_nomem);
    return res;
  }
  auto ret = k_thread_name_set(_tid, _name.c_str());
  if (ret != 0) {
    __ASSERT(false, "Cannot set name: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
    return res;
  }

#if CONFIG_USERSPACE
  // Grant access to the internal _event and _mutex attributes
  _event.grant_access(_tid);
  _mutex.grant_access(_tid);
#endif  // CONFIG_USERSPACE

  // Wake up the thread (after setting name and granting access)
  k_thread_start(_tid);

  // update the thread instance count
  LOG_DBG("Thread (instance count %d) started", _threadInstanceCount);
  _threadInstanceCount++;

  return res;
}

void Thread::waitStarted() noexcept { _event.wait_any(kStartedEvent); }

ZephyrResult Thread::join() noexcept {
  ZephyrResult res;

  res = _mutex.lock();
  __ASSERT(res, "Cannot lock mutex in join: %d", static_cast<int>(res.error()));

  if (_tid != nullptr) {
    // we need to unlock the mutex before calling k_thread_join
    res = _mutex.unlock();
    __ASSERT(res, "Cannot unlock mutex in join: %d", static_cast<int>(res.error()));

    auto ret = k_thread_join(_tid, K_FOREVER);
    if (ret != 0) {
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }

    res = _mutex.lock();
    __ASSERT(res, "Cannot lock mutex in join: %d", static_cast<int>(res.error()));

    // reset tid
    _tid = nullptr;
  }

  res = _mutex.unlock();
  __ASSERT(res, "Cannot unlock mutex in join: %d", static_cast<int>(res.error()));

  return res;
}

#if CONFIG_USERSPACE
k_tid_t Thread::get_tid() const noexcept { return _tid; }
#endif  // CONFIG_USERSPACE

void Thread::_thunk(void* p1, void* p2, void* p3) {
#if CONFIG_USERSPACE
  // cppcheck-suppress cstyleCast
  uint32_t threadInstanceIndex = (uint32_t)p1;  // NOLINT(readability/casting)
  LOG_DBG("Thread _thunk called for thread index %d", threadInstanceIndex);
  Event event(static_cast<k_event*>(p2));
  Mutex mutex(static_cast<k_mutex*>(p3));
#else   // CONFIG_USERSPACE
  Thread* t    = static_cast<Thread*>(p1);
  Event& event = t->_event;
  Mutex& mutex = t->_mutex;
#endif  // CONFIG_USERSPACE

  // signal that the start was effectively started
  event.set(kStartedEvent);

  // invoke the task
#if CONFIG_USERSPACE
  ZPP_TASKS[threadInstanceIndex]();
#else   // CONFIG_USERSPACE
  t->_task();
#endif  // CONFIG_USERSPACE

  LOG_DBG("Task done: exiting the thread (locking mutex)");
  {
    std::scoped_lock<Mutex> guard(mutex);
    // remove this thread from the existing threads
    _threadInstanceCount--;
    LOG_DBG("Job is marked finished, unlocking mutex");
  }
  LOG_DBG("Exiting _thunk");
}

}  // namespace zpp_lib
