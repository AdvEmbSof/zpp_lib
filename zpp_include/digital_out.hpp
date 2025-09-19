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
 * @file digital_out.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration for wrapping zephyr os _gpio used as digital output pin
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

/** A digital output, used for setting the state of a pin
 *
 * @note Synchronization level: Interrupt safe
 *
 */

class DigitalOut : private NonCopyable<DigitalOut> {
 public:
  /**
   * @brief Enumeration to be used for instanciating a specific output pin
   *
   */
  enum class PinName { LED0, LED1 };

  /** Create a DigitalOut connected to the specified pin
   *
   *  @param pin DigitalOut pin to connect to
   */
  explicit DigitalOut(PinName pinName);

  /** Create a DigitalOut connected to the specified pin
   *
   *  @param pin DigitalOut pin to connect to
   *  @param value the initial pin value
   */
  explicit DigitalOut(PinName pinName, uint32_t value);

  /** Set the output, specified as 0 or 1 (int)
   *
   *  @param value An integer specifying the pin output value,
   *      0 for logical 0, 1 (or any other non-zero value) for logical 1
   */
  ZephyrResult write(int value);

  /** Return the output setting, represented as 0 or 1 (int)
   *
   *  @returns
   *    an integer representing the output setting of the pin,
   *    0 for logical 0, 1 for logical 1
   */
  int read();

  /** A shorthand for write()
   * \sa DigitalOut::write()
   * @code
   *      DigitalIn  button(BUTTON1);
   *      DigitalOut led(LED1);
   *      led = button;   // Equivalent to led.write(button.read())
   * @endcode
   */
  DigitalOut& operator=(int value) {
    // Underlying write is thread safe
    write(value);
    return *this;
  }

  /** A shorthand for read()
   * \sa DigitalOut::read()
   * @code
   *      DigitalIn  button(BUTTON1);
   *      DigitalOut led(LED1);
   *      led = button;   // Equivalent to led.write(button.read())
   * @endcode
   */
  operator int() {
    // Underlying call is thread safe
    return read();
  }

 protected:
  struct gpio_dt_spec _gpio;
};

/** @}*/

}  // namespace zpp_lib
