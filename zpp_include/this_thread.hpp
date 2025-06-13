#pragma once

// stl
#include <chrono>

// zpp_lib
#include "zpp_include/clock.hpp"

namespace zpp_lib {

namespace ThisThread {

///
/// @brief Busy wait for a specified time duration
///
/// @param wait_duration The time to busy wait
///
template<class T_Rep, class T_Period>
inline void
busy_wait_for(const std::chrono::duration<T_Rep, T_Period>& wait_duration)
{
  using namespace std::chrono;

  microseconds us = duration_cast<microseconds>(wait_duration);
  k_busy_wait(us.count());
}

///
/// @brief Suspend the current thread for a specified time duration
///
/// @param wait_duration The time to sleep
///
template<class T_Rep, class T_Period >
inline auto
sleep_for(const std::chrono::duration<T_Rep, T_Period>& sleep_duration)
{
  auto res = k_sleep(milliseconds_to_ticks(sleep_duration));

  return std::chrono::milliseconds(res);
}

///
/// @brief Set the current thread priority
///
/// @param thread priority
///
void setPriority(PreemptableThreadPriority priority) {
  k_tid_t tid =	k_current_get();
#if ASSERT
  __ASSERT(tid != nullptr, "Current thread has no tid");
#endif 
  k_thread_priority_set(tid, preemptable_thread_priority_to_zephyr_prio(priority));
}

///
/// @brief Get the current thread priority
///
///
PreemptableThreadPriority getPriority() {
  k_tid_t tid =	k_current_get();
#if ASSERT
  __ASSERT(tid != nullptr, "Current thread has no tid");
#endif 
  int prio = k_thread_priority_get(tid);
  return prio_to_preemptable_thread_priority(prio);
}

///
///
/// @brief Perform a busy wait on the current thread
///
///
void busyWait(std::chrono::microseconds waitTime) {
  // k_busy_wait takes usecs
  k_busy_wait(waitTime.count());
}

} // namespace ThisThread

} // namespace zpp_lin