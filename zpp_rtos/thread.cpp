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

// stl
// for std::scoped_lock definition
#include <mutex>

LOG_MODULE_REGISTER(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

// DECLARE STATIC GLOBAL VARIABLES
K_THREAD_STACK_ARRAY_DEFINE(ZPP_THREADS,
                            CONFIG_ZPP_THREAD_POOL_SIZE,
                            CONFIG_ZPP_THREAD_STACK_SIZE);
// Allocate a static k_thread array for preventing crashes in the SystemView tracing
// library and more generally for preventing stack overflow that may happen if we allocate
// too large objects on the stack
static struct k_thread _thread_data[CONFIG_ZPP_THREAD_POOL_SIZE] = {0};

// initialize static data members
uint8_t Thread::_threadInstanceCount = 0;

Thread::Thread(PreemptableThreadPriority priority, const char* name) {
  constructor(priority, name);
}

Thread::~Thread() {
  if (_tid != nullptr) {
    auto ret = k_thread_join(_tid, K_FOREVER);
    if (ret != 0) {
      LOG_DBG("Failed to join: %d", ret);
    }
  }
}

void Thread::constructor(PreemptableThreadPriority priority, const char* name) {
  _priority = priority;
  _name     = name ? name : "application_unnamed_thread";
  _finished = false;
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

  // initialize callback used in Thread::_thunk
  _task = task;

  // create the thread
  uint32_t options    = 0;
  k_timeout_t delay   = K_NO_WAIT;
  int zephyr_priority = preemptable_thread_priority_to_zephyr_prio(_priority);
  LOG_DBG("Creating thread with stack size %d and priority %d",
          K_THREAD_STACK_SIZEOF(ZPP_THREADS[_threadInstanceCount]),
          zephyr_priority);
  // k_thread_create returns k_tid_t that is in fact typedef struct k_thread *k_tid_t;
  // so the return value of k_thread_create is in fact _thread_data initialized
  _tid = k_thread_create(&_thread_data[_threadInstanceCount],
                         ZPP_THREADS[_threadInstanceCount],
                         K_THREAD_STACK_SIZEOF(ZPP_THREADS[_threadInstanceCount]),
                         Thread::_thunk,
                         this,
                         nullptr,
                         nullptr,
                         zephyr_priority,
                         options,
                         delay);

  if (_tid == nullptr) {
    __ASSERT(false, "_tid is null");
    res.assign_error(ZephyrErrorCode::k_nomem);
    return res;
  }
  auto ret = k_thread_name_set(&_thread_data[_threadInstanceCount], _name.c_str());
  if (ret != 0) {
    __ASSERT(false, "Cannot set name: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
    return res;
  }

  // update the thread instance count
  _threadInstanceCount++;
  LOG_DBG("Thread (instance count %d) started", _threadInstanceCount);

  return res;
}

ZephyrResult Thread::join() noexcept {
  ZephyrResult res;

  auto ret = k_thread_join(_tid, K_FOREVER);
  if (ret != 0) {
    res.assign_error(zephyr_to_zpp_error_code(ret));
    return res;
  }

  // reset tid
  _tid = nullptr;

  return res;
}

void Thread::_thunk(void* thread_ptr, void* a2, void* a3) {
  LOG_DBG("Thread _thunk called");
  Thread* t = static_cast<Thread*>(thread_ptr);
  t->_task();
  LOG_DBG("Task done: exiting the thread (locking mutex)");
  auto ret = t->_mutex.lock();
  __ASSERT(ret, "Cannot lock mutex after task done");
  t->_tid      = nullptr;
  t->_finished = true;
  // remove this thread from the existing threads
  _threadInstanceCount--;
  LOG_DBG("Job is marked finished, unlocking mutex");
  ret = t->_mutex.unlock();
  __ASSERT(ret, "Cannot unlock mutex after task done");
  LOG_DBG("Exiting _thunk");
}

}  // namespace zpp_lib
