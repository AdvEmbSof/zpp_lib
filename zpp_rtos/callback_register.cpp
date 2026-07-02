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

#include "zpp_include/callback_register.hpp"

// zpp_lib
#include "zpp_include/interrupt_in.hpp"

namespace zpp_lib {

CallbackRegister::CallbackRegister(InterruptIn& owner) : _owner(owner) {}

CallbackRegister::~CallbackRegister() {
  // Invalidate all outstanding tokens.
  for (auto& record : _registrations) {
    record.second._owner = nullptr;
  }
}

bool CallbackRegister::has_callbacks() const {
  return !_callbacks.empty();
}

void CallbackRegister::execute_callbacks() const {
  for (const auto& elem : _callbacks) {
    elem.second();
  }
}

RegistrationToken CallbackRegister::register_callback(const CallbackFunction& cb) {
  auto id = _unique_id++;
  _callbacks.insert({id, cb});
  RegistrationToken::RegistrationRecord record{._owner = this, ._id = id};
  _registrations.insert({id, record});

  return RegistrationToken(&_registrations.at(id));
}

void CallbackRegister::unregister_callback(size_t id) {
  _callbacks.erase(id);
  if (_callbacks.empty()) {
    _owner.remove_gpio_callback();
  }
  _registrations.erase(id);
}

void CallbackRegister::unregister_all_callbacks() {
  bool had_callbacks = !_callbacks.empty();
  _callbacks.clear();
  _registrations.clear();
  if (had_callbacks) {
    _owner.remove_gpio_callback();
  }
}

}  // namespace zpp_lib
