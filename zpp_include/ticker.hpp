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
 * @file thread.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration for the ticker class
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr
#include <zephyr/kernel.h>

// zpp_lib
#include "zpp_include/clock.hpp"
#include "zpp_include/non_copyable.hpp"

namespace zpp_lib {

template <typename F>
class Ticker : private NonCopyable<Ticker<F> > {
 public:
  Ticker() = default;

  [[nodiscard]] ZephyrResult attach(const F& f, const std::chrono::milliseconds& period) {
    ZephyrResult res;
    // reject the call if already attached
    if (_isAttached) {
      res.assign_error(ZephyrErrorCode::k_already);
      return res;
    }

    // initialize our timer
    k_timer_init(&_timer, &Ticker::_thunk, nullptr);

    // specify this instance as user data
    // this cast is ugly but the only way to pass a reference to this instance to the
    // timer
    // cppcheck-suppress cstyleCast
    _timer.user_data = (void*)this;  // NOLINT(readability/casting)

    // store the task
    _task = f;

    // start the timer
    k_timeout_t timeout_period = zpp_lib::milliseconds_to_ticks(period);
    k_timer_start(&_timer, timeout_period, timeout_period);

    // set the status
    _isAttached = true;

    return res;
  }

 private:
  static void _thunk(struct k_timer* timer_id) {
    // submit the periodic task
    if (timer_id != nullptr) {
      // get instance from user data
      // this cast is ugly but the only way to pass a reference to this instance to the
      // timer
      // cppcheck-suppress cstyleCast
      Ticker* pTicker = (Ticker*)timer_id->user_data;  // NOLINT(readability/casting)
      // will run in ISR context (should be dispatched to a work queue)
      pTicker->_task();
    }
  }

  // data members
  struct k_timer _timer;
  bool _isAttached = false;
  F _task;
};

}  // namespace zpp_lib
