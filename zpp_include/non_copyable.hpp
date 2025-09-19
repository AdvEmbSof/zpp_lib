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
 * @file non_copyable.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration declaring a class as non copyable (by inheritance)
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

namespace zpp_lib {

template <typename T>
class NonCopyable {
 protected:
  /**
   * Disallow construction of NonCopyable objects from outside of its hierarchy.
   */
  NonCopyable() = default;
  /**
   * Disallow destruction of NonCopyable objects from outside of its hierarchy.
   */
  ~NonCopyable() = default;

 public:
  /**
   * Define copy constructor as deleted. Any attempt to copy construct
   * a NonCopyable will fail at compile time.
   */
  NonCopyable(const NonCopyable&) = delete;

  /**
   * Define copy assignment operator as deleted. Any attempt to copy assign
   * a NonCopyable will fail at compile time.
   */
  NonCopyable& operator=(const NonCopyable&) = delete;
};

}  // namespace zpp_lib
