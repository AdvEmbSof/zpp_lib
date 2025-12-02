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
 * @file interrupt_in.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration for wrapping zephyr os _gpio used as interrupt input
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr
#if CONFIG_TEST != 1
#include <zephyr/drivers/gpio.h>
#endif
#include <zephyr/kernel.h>

// stl
#include <functional>
#include <map>

// zpp_lib
#include "zpp_include/mutex.hpp"
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

/** A digital interrupt input, used to call a function on a rising or falling edge
 *
 * @note Synchronization level: Interrupt safe
 *
 *
 */
/**
 * @brief Constant defining pressed polarity
 *
 */
static constexpr uint8_t kPolarityPressed = 1;

enum class PinName {
  BUTTON1     = 1,
  BUTTON2     = 2,
  BUTTON3     = 3,
  BUTTON4     = 4,
  LAST_BUTTON = 4
};
template <PinName pinName>
class InterruptIn : private NonCopyable<InterruptIn<pinName> > {
 public:
  /**
   * @brief Enumeration to be used for instanciating a specific input pin
   *
   */
  /** Create an InterruptIn connected to the specified pin
   *
   *  @param pin InterruptIn pin to connect to
   */
  InterruptIn();

  /** Remove callbacks added to gpio device
   *
   */
  virtual ~InterruptIn();

  /** Read the input, represented as 0 or 1 (int)
   *
   *  @returns
   *    An integer representing the state of the input pin,
   *    0 for logical 0, 1 for logical 1
   */
  uint8_t read();

  /** An operator shorthand for read()
   */
  operator uint8_t();

  /** Attach a function to call when a falling edge occurs on the input
   *  Interrupts are enabled for the pin
   *
   *  @param func A pointer to a void function, or 0 to set as none
   */
  void fall(std::function<void()> func);

#if CONFIG_TEST == 1
  /** Used for testing purposes
   *  Sets the value of the input pin
   */
  void write(uint8_t value);
#endif

 protected:
#if CONFIG_TEST == 1
  static inline uint8_t _value{!kPolarityPressed};  // button not pressed by default
#else
  void callback(const struct device* port,
                struct gpio_callback* cb,
                gpio_port_pins_t pins);
  struct gpio_dt_spec _gpio;
  struct gpio_callback _cbData;
#endif
  static constexpr size_t NBR_OF_BUTTONS = static_cast<size_t>(PinName::LAST_BUTTON);
  static constexpr size_t BUTTON_INDEX   = static_cast<size_t>(pinName) - 1;
  using CallbackFunction                 = std::function<void()>;
  using CallbackFunctionMap              = std::map<void*, CallbackFunction>;
  static inline CallbackFunctionMap _fall_cb_map[NBR_OF_BUTTONS];
  static inline zpp_lib::Mutex _cbMutex;
};

/** @}*/

}  // namespace zpp_lib
