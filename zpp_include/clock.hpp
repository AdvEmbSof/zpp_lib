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
 * @file clock.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief function for converting std::chrono::milliseconds to k_timeout_t values
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

#include <zephyr/kernel.h>
#include <zephyr/sys/time_units.h>

// stl
#include <chrono>

namespace zpp_lib {

constexpr k_timeout_t milliseconds_to_ticks(const std::chrono::milliseconds& d) noexcept {
  return {K_MSEC(d.count())};
}

}  // namespace zpp_lib
