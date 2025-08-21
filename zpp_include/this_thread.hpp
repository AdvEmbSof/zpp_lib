#pragma once

// stl
#include <chrono>

// zpp_lib
#include "zpp_include/types.hpp"

namespace zpp_lib {

namespace ThisThread {

///
/// @brief Set the current thread priority
///
/// @param thread priority
///
void setPriority(PreemptableThreadPriority priority); 

///
/// @brief Get the current thread priority
///
///
PreemptableThreadPriority getPriority(); 

///
///
/// @brief Perform a busy wait on the current thread
///
///
void busyWait(const std::chrono::microseconds& waitTime); 


///
/// @brief Suspend the current thread for a specified time duration
///
/// @param wait_duration The time to sleep
///
std::chrono::microseconds sleep_for(const std::chrono::microseconds& waitTime);

} // namespace ThisThread

} // namespace zpp_lib