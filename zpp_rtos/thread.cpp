#include "zpp_include/thread.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

// stl
#include <mutex>

//#ifndef CONFIG_USERSPACE
//#error This sample requires CONFIG_USERSPACE.
//#endif

#ifndef CONFIG_DYNAMIC_THREAD_ALLOC
//#error This sample requires CONFIG_DYNAMIC_THREAD_ALLOC.
#endif

LOG_MODULE_REGISTER(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {
  
K_THREAD_STACK_ARRAY_DEFINE(ZPP_THREADS, CONFIG_ZPP_THREAD_POOL_SIZE, CONFIG_ZPP_THREAD_STACK_SIZE);
uint8_t Thread::_threadInstanceCount = 0;

Thread::Thread(PreemptableThreadPriority priority,
               const char *name)
{
  constructor(priority, name);
}

void Thread::constructor(PreemptableThreadPriority priority,
                         const char *name)
{
  LOG_DBG("Thread constructed with stack size %d", K_THREAD_STACK_SIZEOF(ZPP_THREADS[_threadInstanceCount]));
  _priority = priority;
  _name = name ? name : "application_unnamed_thread";
  _finished = false;    
}

ZephyrResult Thread::start(std::function<void()> task) noexcept {
  std::scoped_lock<Mutex> guard(_mutex);

  // the thread stacks are allocated statically 
#if ASSERT
  __ASSERT(_threadInstanceCount < CONFIG_ZPP_THREAD_POOL_SIZE, "Too many threads created");
#endif // ASSERT

  // initialize callback used in Thread::_thunk
  _task = task;
  
  // create the thread
  uint32_t options = 0;
  k_timeout_t delay = K_NO_WAIT;
  int zephyr_priority = static_cast<int>(_priority);
  LOG_DBG("Creating thread with stack size %d and priority %d", 
          K_THREAD_STACK_SIZEOF(ZPP_THREADS[_threadInstanceCount]), 
          zephyr_priority);
  // k_thread_create returns k_tid_t that is in fact typedef struct k_thread *k_tid_t;
  // so the return value of k_thread_create is in fact _thread_data initialized
  _tid = k_thread_create(&_thread_data, 
                         ZPP_THREADS[_threadInstanceCount],
                         K_THREAD_STACK_SIZEOF(ZPP_THREADS[_threadInstanceCount]), 
                         Thread::_thunk, 
                         this, 
                         nullptr, 
                         nullptr, 
                         zephyr_priority, 
                         options, 
                         delay);
  ZephyrResult res;
  if (_tid == nullptr) {        
    res.assign_error(ZephyrErrorCode::k_nomem);
    return res;
  }
  
  // update the thread instance count
  _threadInstanceCount++;

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