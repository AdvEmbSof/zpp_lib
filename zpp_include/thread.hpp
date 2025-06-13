#pragma once

// zephyr
#include <zephyr/kernel.h>

// std
#include <string>
#include <functional>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/types.hpp"
#include "zpp_include/zephyr_result.hpp"
#include "zpp_include/mutex.hpp"

namespace zpp_lib {

class Thread : private NonCopyable<Thread> {
public:
    /** Allocate a new thread without starting execution
      @param   priority       initial priority of the thread function. (default: osPriorityNormal).
      @param   name           name to be used for this thread. It has to stay allocated for the lifetime of the thread (default: nullptr)

      @note Default value of tz_module will be MBED_TZ_DEFAULT_ACCESS
      @note You cannot call this function from ISR context.
    */
    Thread(PreemptableThreadPriority priority = PreemptableThreadPriority::PriorityNormal,
           const char *name = nullptr);

    /** Starts a thread executing the specified function.
      @param   task           function to be executed by this thread.
      @return  status code that indicates the execution status of the function,
               or osErrorNoMemory if stack allocation failed.
      @note a thread can only be started once

      @note You cannot call this function ISR context.
    */
    [[nodiscard]] ZephyrResult start(std::function<void()> task) noexcept;

    /** Wait for thread to terminate
      @return  status code that indicates the execution status of the function.

      @note You cannot call this function from ISR context.
    */
    [[nodiscard]] ZephyrResult join() noexcept;

private:
    // Required to share definitions without
    // delegated constructors
    void constructor(PreemptableThreadPriority priority,
                     const char *name);
    static void _thunk(void *thread_ptr, void* a2, void* a3);

private:
    std::function<void()> _task;
    bool _finished;
    PreemptableThreadPriority _priority;
    std::string _name;
    k_tid_t _tid = nullptr;
    struct k_thread _thread_data;
    
    Mutex _mutex;
    // used for accessing the corresponding statically allocated stack
    static uint8_t _threadInstanceCount;    
};

} // namespace zpp_lib