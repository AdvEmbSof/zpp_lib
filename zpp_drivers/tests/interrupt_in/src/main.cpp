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
 * @file test_thread.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Test program for zpp_lib Thread class
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

// zephyr

// std
#include <functional>

// zpp_rtos
#include "zpp_include/interrupt_in.hpp"
#include "zpp_include/this_thread.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_log.hpp"
#include "zpp_include/zpp_test.hpp"

ZPP_LOG_MODULE_REGISTER(test_interrupt_in, CONFIG_APP_LOG_LEVEL);

void callback() {
  ZPP_LOG_DBG("Button1 pressed");
}
void callback1() {
  ZPP_LOG_DBG("Button1_1 pressed");
}
void callback2() {
  ZPP_LOG_DBG("Button1_2 pressed");
}

ZPP_ZTEST_USER(zpp_interrupt_in, test_destructor) {
  using std::literals::chrono_literals::operator""s;

  {
    zpp_lib::InterruptIn button1(zpp_lib::InterruptIn::PinName::BUTTON1);
    zpp_lib::RegistrationToken token = button1.add_callback(callback);

    printk("Press button 1! -> a message should print\n");

    // if the user presses the button in the next 10s, the callback message should print
    zpp_lib::ThisThread::sleep_for(10s);
  }

  // the InterruptIn instance is destroyed here
  // if the user presses the button in the next 10s, the callback message should not print
  // and the system should behave correctly
  printk("Press button 1! -> nothing should print\n");
  zpp_lib::ThisThread::sleep_for(10s);
}

ZPP_ZTEST_USER(zpp_interrupt_in, test_multiple_instances) {
  using std::literals::chrono_literals::operator""s;
  {
    zpp_lib::InterruptIn button1_1(zpp_lib::InterruptIn::PinName::BUTTON1);
    zpp_lib::RegistrationToken token1_1 = button1_1.add_callback(callback1);

    {
      zpp_lib::InterruptIn button1_2(zpp_lib::InterruptIn::PinName::BUTTON1);
      zpp_lib::RegistrationToken token1_2 = button1_2.add_callback(callback2);
      printk("Press button 1! -> two messages should print\n");

      // if the user presses the button in the next 10s, the callback message should print
      // twice
      zpp_lib::ThisThread::sleep_for(10s);
    }

    printk("Press button 1! -> one message should print\n");

    // if the user presses the button in the next 10s, the callback message should print
    zpp_lib::ThisThread::sleep_for(10s);
  }

  // the InterruptIn instance is destroyed here
  // if the user presses the button in the next 10s, the callback message should not print
  // and the system should behave correctly
  printk("Press button 1! -> nothing should print\n");
  zpp_lib::ThisThread::sleep_for(10s);
}

ZPP_ZTEST_SUITE(zpp_interrupt_in, nullptr, nullptr, nullptr, nullptr, nullptr);
