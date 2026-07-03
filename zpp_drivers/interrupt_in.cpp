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
 * @file interrupt_in.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class implementation for wrapping zephyr os _gpio used as interrupt input
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#include "zpp_include/interrupt_in.hpp"

// Zephyr sdk

// stl
// for std::scoped_lock definition
#include <mutex>

// zpp_lib
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_log.hpp"

ZPP_LOG_MODULE_DECLARE(zpp_drivers, CONFIG_ZPP_DRIVERS_LOG_LEVEL);

namespace zpp_lib {

// _gpio is initialized with an error in default switch case,
// complexity is not an issue since we only call a zephyr macro in the switch cases
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,readability-function-cognitive-complexity)
InterruptIn::InterruptIn(PinName pin_name) : _pin_name(pin_name), _callback_register(*this) {
#if !CONFIG_INTERRUPT_IN_EMUL
  switch (pin_name) {
#if HAS_SW0
  case PinName::BUTTON1:  // NOLINT(bugprone-branch-clone) - this is a false positive,
                          // the code is not duplicated since the only thing that
                          // changes is the alias used in GPIO_DT_SPEC_GET
    _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
    break;
#endif  // HAS_SW0

#if HAS_SW1
  case PinName::BUTTON2:
    _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
    break;
#endif  // HAS_SW1

#if HAS_SW2
  case PinName::BUTTON3:
    _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
    break;
#endif  // HAS_SW2

#if HAS_SW3
  case PinName::BUTTON4:
    _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);
    break;
#endif  // HAS_SW3
  default:
    ZPP_ASSERT(false, "Invalid pinName %d", static_cast<int>(pin_name));
    ZPP_LOG_ERR("Invalid pinName %d", static_cast<int>(pin_name));
    break;
  }
  if (!gpio_is_ready_dt(&_gpio)) {
    ZPP_LOG_ERR("GPIO %s not existing on platform", _gpio.port->name);
    ZPP_ASSERT(false, "GPIO %s not existing on platform", _gpio.port->name);
    return;
  }

  int ret = gpio_pin_configure_dt(&_gpio, GPIO_INPUT);
  if (ret < 0) {
    ZPP_LOG_ERR("Cannot configure GPIO %s as input (%d)", _gpio.port->name, ret);
    ZPP_ASSERT(false, "Cannot configure GPIO %s as input (%d)", _gpio.port->name, ret);
    return;
  }

  ret = gpio_pin_interrupt_configure_dt(&_gpio, GPIO_INT_EDGE_FALLING);
  if (ret != 0) {
    ZPP_LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n", ret, _gpio.port->name, _gpio.pin);
    ZPP_ASSERT(false, "Cannot configure interrupt on GPIO %s (%d)", _gpio.port->name, ret);
    return;
  }
  ZPP_LOG_DBG("Pin %s initialized", _gpio.port->name);
#endif  // !CONFIG_INTERRUPT_IN_EMUL
}

// Complexity is increased by Zephyr Macros
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
InterruptIn::~InterruptIn() {
#if NUM_BUTTONS > 0
  _callback_register.unregister_all_callbacks();
#endif  // NUM_BUTTONS > 0
}

// False posititive
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
bool InterruptIn::read() {
#if CONFIG_INTERRUPT_IN_EMUL
  size_t button_index = static_cast<size_t>(_pin_name) - 1;
  // PinName is an enum class that starts at 1, so buttonIndex is in
  // the range [0, NUM_BUTTONS-1], which is the valid range for s_fall_cb_map
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return _value[button_index];
#else   // CONFIG_INTERRUPT_IN_EMUL
  return static_cast<bool>(gpio_pin_get_dt(&_gpio));
#endif  // CONFIG_INTERRUPT_IN_EMUL
}

#if CONFIG_INTERRUPT_IN_EMUL
void InterruptIn::write(bool value) {
  size_t button_index = static_cast<size_t>(_pin_name) - 1;
  // PinName is an enum class that starts at 1, so buttonIndex is in
  // the range [0, NUM_BUTTONS-1], which is the valid range for s_fall_cb_map
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  bool edge_falling = _value[button_index] == !kPolarityPressed && value == kPolarityPressed;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  _value[button_index] = value;
  if (edge_falling) {
    // printk("TEST mode: Button %d pressed at %" PRIu32 "\n", static_cast<int>(_pin_name), k_cycle_get_32());
    size_t button_index = static_cast<size_t>(_pin_name) - 1;
    // PinName is an enum class that starts at 1, so buttonIndex is in
    // the range [0, NUM_BUTTONS-1], which is the valid range for s_fall_cb_map
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    s_callback_register[button_index].execute_callbacks();
  }
}
#endif  // CONFIG_INTERRUPT_IN_EMUL

// Complexity is increased by Zephyr Macros
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
RegistrationToken InterruptIn::add_callback(const CallbackRegister::CallbackFunction& cb) {
  if (cb == nullptr) {
    ZPP_LOG_ERR("Cannot call fall with nullptr");
    return RegistrationToken(nullptr);
  }

  // We allow for multiple calls to fall() for setting multiple callbacks.
  // On first call, we configure the Zephyr driver to call the
  // InterruptIn<pinName>::callback() on button fall
  // On subsequent calls, we simply push the callback to the vector
#if CONFIG_ZPP_DRIVERS_LOG_LEVEL_DBG
  size_t button_index = static_cast<size_t>(_pin_name) - 1;
  ZPP_LOG_DBG("Setting up callback for button %d", button_index);
#endif  // CONFIG_ZPP_DRIVERS_LOG_LEVEL_DBG
#if !CONFIG_INTERRUPT_IN_EMUL
  if (!_callback_register.has_callbacks()) {
    _cb_data._instance = this;
    gpio_init_callback(&_cb_data._gpio_cb, &InterruptIn::callback, BIT(_gpio.pin));
    gpio_add_callback(_gpio.port, &_cb_data._gpio_cb);
    ZPP_LOG_DBG("Callback set for %s pin %d", _gpio.port->name, _gpio.pin);
  }
#endif  // !CONFIG_INTERRUPT_IN_EMUL
  ZPP_LOG_DBG("Registering callback in map for %p", this);

  return _callback_register.register_callback(cb);
}

void InterruptIn::remove_gpio_callback() {
  // size_t button_index = static_cast<size_t>(_pin_name) - 1;
  // ZPP_LOG_DBG("Trying to unregistering callback for %p (button %d)", this, button_index);
#if !CONFIG_INTERRUPT_IN_EMUL
  auto ret = gpio_remove_callback(_gpio.port, &_cb_data._gpio_cb);
  if (ret != 0) {
    ZPP_ASSERT(false, "Cannot remove callback on GPIO %s (%d)", _gpio.port->name, ret);
  }
  ZPP_LOG_DBG("Gpio callback removed");
#endif  // !CONFIG_INTERRUPT_IN_EMUL
}

#if !CONFIG_INTERRUPT_IN_EMUL
void InterruptIn::callback(const struct device* port, struct gpio_callback* cb, gpio_port_pins_t pins) {
  // printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
  // We need to cast cb for getting the instance on which the callback is running
  // static_cast<CallbackData*> is not accepted here, reinterpret_cast is not supported
  // NOLINTNEXTLINE(readability/casting,modernize-avoid-c-style-cast,cppcoreguidelines-pro-type-cstyle-cast)
  auto* p_callback_data   = (CallbackData*)cb;
  InterruptIn* p_instance = p_callback_data->_instance;
  p_instance->_callback_register.execute_callbacks();
}
#endif  // ! defined(CONFIG_INTERRUPT_IN_EMUL)

}  // namespace zpp_lib
