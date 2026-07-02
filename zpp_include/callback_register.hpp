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
 * @file callback_register.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Callback register for managing callback registrations (used by InterruptIn)
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// std
#include <cstddef>
#include <functional>
#include <map>

// zpp_lib
#include "zpp_include/registration_token.hpp"

namespace zpp_lib {

// forward declaration
class InterruptIn;

class CallbackRegister {
public:
  using CallbackFunction = std::function<void()>;
  CallbackRegister(InterruptIn& owner);
  virtual ~CallbackRegister();

  // returns true if there are any registered callbacks
  [[nodiscard]] bool has_callbacks() const;

  // register a callback function and return a token for unregistering it
  [[nodiscard]] RegistrationToken register_callback(const CallbackFunction& cb);
  // unregister a callback function using its token
  void unregister_callback(size_t id);
  void unregister_all_callbacks();

  // called to execute all registered callbacks
  void execute_callbacks() const;

  /** Explicity prevent (move) copy and assignment 
      rather than inheriting from NonCopyable. This avoids 
      cppcoreguidelines-special-member-functions warning by clang-tidy.
  */
  CallbackRegister(const CallbackRegister&) = delete;
  CallbackRegister(CallbackRegister&&)      = delete;
  CallbackRegister& operator=(const CallbackRegister&) = delete;
  CallbackRegister& operator=(CallbackRegister&&)      = delete;

private:
  InterruptIn& _owner;
  size_t _unique_id = 0;
  using CallbackFunctionMap = std::map<size_t, CallbackFunction>;
  CallbackFunctionMap _callbacks;
  using RegistrationRecordMap = std::map<size_t, RegistrationToken::RegistrationRecord>;
  RegistrationRecordMap _registrations;
  //std::vector<RegistrationRecord> _registrations;
  // std::vector<Callback> _callbacks;
};

}  // namespace zpp_lib