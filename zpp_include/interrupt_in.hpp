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

#include <zephyr/devicetree.h>

// Check each alias at compile time
#define HAS_SW0 DT_NODE_EXISTS(DT_ALIAS(sw0))
#define HAS_SW1 DT_NODE_EXISTS(DT_ALIAS(sw1))
#define HAS_SW2 DT_NODE_EXISTS(DT_ALIAS(sw2))
#define HAS_SW3 DT_NODE_EXISTS(DT_ALIAS(sw3))
#define NUM_BUTTONS (HAS_SW0 + HAS_SW1 + HAS_SW2 + HAS_SW3)

// zephyr
#if CONFIG_INTERRUPT_IN_EMUL != 1
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
static constexpr bool kPolarityPressed = true;

class InterruptIn : private NonCopyable<InterruptIn> {
public:
  enum class PinName {
#if HAS_SW0
    BUTTON1 = 1,
#if HAS_SW1
    BUTTON2 = 2,
#if HAS_SW2
    BUTTON3 = 3,
#if HAS_SW3
    BUTTON4 = 4,
#endif // HAS_SW3
#endif // HAS_SW2
#endif // HAS_SW1
#endif // HAS_SW0
    LAST_BUTTON = NUM_BUTTONS
  };

  /**
   * @brief Enumeration to be used for instanciating a specific input pin
   *
   */
  /** Create an InterruptIn connected to the specified pin
   *
   *  @param pin InterruptIn pin to connect to
   */
  explicit InterruptIn(PinName pin_name);

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
  bool read();

  /** An operator shorthand for read()
   */
  operator bool();

  /** Attach a function to call when a falling edge occurs on the input
   *  Interrupts are enabled for the pin
   *
   *  @param func A pointer to a void function, or 0 to set as none
   */
  void fall(const std::function<void()>& func);

#if CONFIG_INTERRUPT_IN_EMUL
  /** Used for testing purposes
   *  Sets the value of the input pin
   */
  void write(bool value);
#endif // CONFIG_INTERRUPT_IN_EMUL

protected:
#if CONFIG_INTERRUPT_IN_EMUL
  static inline bool _value{!kPolarityPressed}; // button not pressed by default
#else
  static void callback(const struct device* port, struct gpio_callback* cb, gpio_port_pins_t pins);
  struct gpio_dt_spec _gpio;
  struct CallbackData {
    struct gpio_callback _gpio_cb;
    InterruptIn* _instance;
  };
  struct CallbackData _cbData = {._gpio_cb = {}, ._instance = nullptr};
#endif // CONFIG_INTERRUPT_IN_EMUL
  PinName _pin_name;
  using CallbackFunction                = std::function<void()>;
  using CallbackFunctionMap             = std::map<void*, CallbackFunction>;
  static constexpr size_t kNbrOfButtons = static_cast<size_t>(PinName::LAST_BUTTON);
  static inline CallbackFunctionMap _fall_cb_map[kNbrOfButtons];
  static inline zpp_lib::Mutex _cb_mutex;
};

/** @}*/

} // namespace zpp_lib
