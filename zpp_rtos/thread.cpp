#include "zpp_include/thread.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

// stl
#include <mutex>

//#ifndef CONFIG_USERSPACE
//#error This sample requires CONFIG_USERSPACE.
//#endif

#ifndef CONFIG_DYNAMIC_THREAD_ALLOC
#error This sample requires CONFIG_DYNAMIC_THREAD_ALLOC.
#endif

LOG_MODULE_REGISTER(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

constexpr auto align_up(const uintptr_t pos, uint8_t align) { 
  return (pos) % (align) ? (pos) +  ((align) - (pos) % (align)) : (pos); 
}
static_assert(align_up(0, 8) == 0, "align_up error");
static_assert(align_up(1, 8) == 8, "align_up error");

constexpr auto align_down(const uintptr_t pos, uint8_t align) { 
  return (pos) - ((pos) % (align)); 
}
static_assert(align_down(7, 8) == 0, "align_down error");
static_assert(align_down(8, 8) == 8, "align_down error");

Thread::Thread(PreemptableThreadPriority priority,
               uint32_t stack_size,
               const char *name)
{
  constructor(priority, stack_size, name);
}

void Thread::constructor(PreemptableThreadPriority priority,
                         uint32_t stack_size, 
                         const char *name)
{
  LOG_DBG("Thread constructed with stack size %d", stack_size);
  const uint32_t aligned_size = align_down(stack_size, 8);
  _stack_size = aligned_size;    
  _priority = priority;
  _name = name ? name : "application_unnamed_thread";
  _finished = false;    
}

ZephyrResult Thread::start(std::function<void()> task) noexcept {
  std::scoped_lock<Mutex> guard(_mutex);

  // allocate the stack 
  ZephyrResult res;
  _thread_stack = k_thread_stack_alloc(_stack_size, IS_ENABLED(CONFIG_USERSPACE) ? K_USER : 0);
  if (_thread_stack == nullptr) {
    LOG_ERR("Cannot allocate stack memory");
    res.assign_error(ZephyrErrorCode::k_nomem);
    return res;
  }

  // initialize callback used in Thread::_thunk
  _task = task;

  // create the thread
  uint32_t options = 0;
  k_timeout_t delay = K_NO_WAIT;
  int zephyr_priority = static_cast<int>(_priority);
  LOG_DBG("Creating thread with stack size %d and priority %d", _stack_size, zephyr_priority);
  // k_thread_create returns k_tid_t that is in fact typedef struct k_thread *k_tid_t;
  // so the return value of k_thread_create is in fact _thread_data initialized
  _tid = k_thread_create(&_thread_data, _thread_stack, _stack_size, Thread::_thunk, this, nullptr, nullptr, zephyr_priority, options, delay);
  if (_tid == nullptr) {
    int rc = k_thread_stack_free(_thread_stack);
    if (rc != 0) {
      LOG_ERR("Failed to deallocate thread stack upon thread creation error (%d)", rc);
    }
    _thread_stack = nullptr;
        
    res.assign_error(ZephyrErrorCode::k_nomem);
    return res;
  }

  LOG_DBG("Thread created");
  return res;
}

ZephyrResult Thread::join() noexcept {
  ZephyrResult res;
  
  auto ret = k_thread_join(_tid, K_FOREVER);
  if (ret != 0) {
    res.assign_error(zephyr_to_zpp_error_code(ret));
    return res;
  }

  return res;
}

void Thread::_thunk(void *thread_ptr, void* a2, void* a3) {
  LOG_DBG("Thread _thunk called");
  Thread *t = (Thread *) thread_ptr;
  t->_task();
  LOG_DBG("Task done: exiting the thread (locking mutex)");
  auto ret = t->_mutex.lock();
  __ASSERT(ret, "Cannot lock mutex after task done");
  t->_tid = nullptr;
  t->_finished = true;
  LOG_DBG("Job is marked finished, unlocking mutex");
  ret = t->_mutex.unlock();
  __ASSERT(ret, "Cannot unlock mutex after task done"); 
  LOG_DBG("Exiting _thunk");
}

} // namespace zpp_lib