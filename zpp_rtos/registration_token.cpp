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
 * @file registration_token.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Registration token implementation for managing callback registrations
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#include "zpp_include/registration_token.hpp"

// std
#include <utility>

// zpp_lib
#include "zpp_include/callback_register.hpp"

namespace zpp_lib {

RegistrationToken::RegistrationToken(RegistrationRecord* p_record) : _p_record(p_record) {
}
 
RegistrationToken::~RegistrationToken() {
  reset();
}

RegistrationToken::RegistrationToken(RegistrationToken&& other) noexcept : _p_record(std::exchange(other._p_record, nullptr)) {    
}

RegistrationToken& RegistrationToken::operator=(RegistrationToken&& other) noexcept {
  if (this != &other) {
    reset();
    _p_record = std::exchange(other._p_record, nullptr);
  }
  return *this;
}

void RegistrationToken::reset() {
  if (_p_record != nullptr && _p_record->_owner != nullptr) {
    _p_record->_owner->unregister_callback(_p_record->_id);    
  }
  _p_record = nullptr;
}

 }  // namespace zpp_lib