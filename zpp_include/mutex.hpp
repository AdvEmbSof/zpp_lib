#pragma once

// zephyr
#include <zephyr/kernel.h>

// stl
#include <chrono>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/result.hpp"

namespace zpp_lib {

/** The Mutex class is used to synchronize the execution of threads.
 This is, for example, used to protect access to a shared resource.

 @note You cannot use member functions of this class in ISR context. If you require Mutex functionality within
 ISR handler, consider using @a Semaphore.
*/
class Mutex : private NonCopyable<Mutex> {
public:
    /** Create and Initialize a Mutex object
     *
     * @note You cannot call this function from ISR context.
    */
    Mutex() noexcept;

    /** Create and Initialize a Mutex object

     @param name name to be used for this mutex. It has to stay allocated for the lifetime of the thread.
     @note You cannot call this function from ISR context.
    */
    Mutex(const char *name) noexcept;

    /**
      Wait until a Mutex becomes available.

      @note You cannot call this function from ISR context.
     */
    [[nodiscard]] ZephyrResult lock();

    /** Try to lock the mutex, and return immediately
      @return true if the mutex was acquired, false otherwise.
      @note equivalent to trylock_for(0)

      @note You cannot call this function from ISR context.
     */
    [[nodiscard]] ZephyrBoolResult try_lock() noexcept;

    /** Try to lock the mutex for a specified time
      @param   rel_time  timeout value.
      @return true if the mutex was acquired, false otherwise.
      @note the underlying RTOS may have a limit to the maximum wait time
            due to internal 32-bit computations, but this is guaranteed to work if the
            wait is <= 0x7fffffff milliseconds (~24 days). If the limit is exceeded,
            the lock attempt will time out earlier than specified.

      @note You cannot call this function from ISR context.
     */
    [[nodiscard]] ZephyrBoolResult try_lock_for(const std::chrono::milliseconds& timeout) noexcept;

    /**
      Unlock the mutex that has previously been locked by the same thread

      @note You cannot call this function from ISR context.
     */
    [[nodiscard]] ZephyrResult unlock();

    /** Mutex destructor
     *
     * @note You cannot call this function from ISR context.
     */
    ~Mutex();

private:
    struct k_mutex _mutex_obj;
};

} // namespace zpp_lib