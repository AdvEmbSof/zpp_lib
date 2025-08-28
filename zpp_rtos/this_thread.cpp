// zpp_lib
#include "zpp_include/this_thread.hpp"

// zephyr
#include <zephyr/kernel.h>

namespace zpp_lib {
  
namespace ThisThread {

PreemptableThreadPriority getPriority() {
  k_tid_t tid =	k_current_get();
#if ASSERT
  __ASSERT(tid != nullptr, "Current thread has no tid");
#endif 
  int prio = k_thread_priority_get(tid);
  return prio_to_preemptable_thread_priority(prio);
}

void setPriority(PreemptableThreadPriority priority) {
  k_tid_t tid =	k_current_get();
#if ASSERT
  __ASSERT(tid != nullptr, "Current thread has no tid");
#endif 
  k_thread_priority_set(tid, preemptable_thread_priority_to_zephyr_prio(priority));
}

void busyWait(const std::chrono::microseconds& waitTime) {
  // k_busy_wait takes usecs
  k_busy_wait(waitTime.count());
}

std::chrono::microseconds sleep_for(const std::chrono::microseconds& sleep_duration)
{
  //auto res = k_sleep(milliseconds_to_ticks(sleep_duration));
  const auto sleep_duration_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(sleep_duration);
  auto res = k_msleep(sleep_duration_ms.count());

  return std::chrono::microseconds(res);
}

} // namespace ThisThread

} // namespace zpp_lib
