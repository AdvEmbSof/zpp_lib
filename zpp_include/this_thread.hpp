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
 * @file this_thread.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Declaration of various helper functions for accessing the running thread
 *        within ThisThread namespace
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

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
void set_priority(PreemptableThreadPriority priority);

///
/// @brief Get the current thread priority
///
///
PreemptableThreadPriority get_priority();

///
///
/// @brief Perform a busy wait on the current thread
///
///
void busy_wait(const std::chrono::microseconds& waitTime);

///
/// @brief Suspend the current thread for a specified time duration
///
/// @param waitTime The time to sleep
///
std::chrono::milliseconds sleep_for(const std::chrono::microseconds& waitTime);
std::chrono::milliseconds sleep_for(const std::chrono::milliseconds& waitTime);
std::chrono::milliseconds sleep_for(const std::chrono::seconds& waitTime);

///
/// @brief Suspend the current thread until a specified absolute time
///
/// @param absoluteTime The absolute time
///
std::chrono::milliseconds sleep_until(const std::chrono::microseconds& absoluteTime);
std::chrono::milliseconds sleep_until(const std::chrono::milliseconds& absoluteTime);
std::chrono::milliseconds sleep_until(const std::chrono::seconds& absoluteTime);

}  // namespace ThisThread

}  // namespace zpp_lib
