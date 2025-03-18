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

} // namespace ThisThread

} // namespace zpp_lin