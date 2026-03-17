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
#include <zephyr/logging/log.h>

// stl
// for std::scoped_lock definition
#include <mutex>

LOG_MODULE_DECLARE(zpp_drivers, CONFIG_ZPP_DRIVERS_LOG_LEVEL);

namespace zpp_lib {

InterruptIn::InterruptIn(PinName pinName) {
#if !defined(CONFIG_INTERRUPT_IN_EMUL)
  switch (pinName) {
    case PinName::BUTTON1:
      _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
      break;

    case PinName::BUTTON2:
      _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);
      break;

    case PinName::BUTTON3:
      _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios);
      break;

    case PinName::BUTTON4:
      _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios);
      break;

    default:
      break;
  }
  if (!gpio_is_ready_dt(&_gpio)) {
    LOG_ERR("GPIO %s not existing on platform", _gpio.port->name);
    __ASSERT(false, "GPIO %s not existing on platform", _gpio.port->name);
    return;
  }

  int ret = gpio_pin_configure_dt(&_gpio, GPIO_INPUT);
  if (ret < 0) {
    LOG_ERR("Cannot configure GPIO %s as input (%d)", _gpio.port->name, ret);
    __ASSERT(false, "Cannot configure GPIO %s as input (%d)", _gpio.port->name, ret);
    return;
  }

  ret = gpio_pin_interrupt_configure_dt(&_gpio, GPIO_INT_EDGE_FALLING);
  if (ret != 0) {
    LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
            ret,
            _gpio.port->name,
            _gpio.pin);
    __ASSERT(false, "Cannot configure interrupt on GPIO %s (%d)", _gpio.port->name, ret);
    return;
  }
  LOG_DBG("Pin %s initialized", _gpio.port->name);
#endif  // ! defined(CONFIG_INTERRUPT_IN_EMUL)
  _pinName = pinName;
}

InterruptIn::~InterruptIn() {
  std::scoped_lock<Mutex> guard(_cbMutex);

  size_t buttonIndex                 = static_cast<size_t>(_pinName) - 1;
  CallbackFunctionMap& cbFunctionMap = _fall_cb_map[buttonIndex];
  LOG_DBG("Trying to unregistering callback for %p (button %d)", this, buttonIndex);
  if (cbFunctionMap.find(this) != cbFunctionMap.end()) {
#if !defined(CONFIG_INTERRUPT_IN_EMUL)
    if (cbFunctionMap.size() == 1) {
      auto ret = gpio_remove_callback(_gpio.port, &_cbData._gpio_cb);
      if (ret != 0) {
        __ASSERT(false, "Cannot remove callback on GPIO %s (%d)", _gpio.port->name, ret);
      }
      LOG_DBG("Gpio callback removed");
    }
#endif  // ! defined(CONFIG_INTERRUPT_IN_EMUL)
    LOG_DBG("Removing callback for button %d", buttonIndex);
    cbFunctionMap.erase(this);
  }
}

uint8_t InterruptIn::read() {
#if CONFIG_INTERRUPT_IN_EMUL
  return _value;
#else   // CONFIG_INTERRUPT_IN_EMUL
  return gpio_pin_get_dt(&_gpio);
#endif  // CONFIG_INTERRUPT_IN_EMUL
}

#if CONFIG_INTERRUPT_IN_EMUL
void InterruptIn::write(uint8_t value) {
  bool edgeFalling = _value == !kPolarityPressed && value == kPolarityPressed;
  _value           = value;
  if (edgeFalling) {
    // printk("TEST mode: Button %d pressed at %" PRIu32 "\n", static_cast<int>(pinName),
    // k_cycle_get_32());
    size_t buttonIndex                 = static_cast<size_t>(_pinName) - 1;
    CallbackFunctionMap& cbFunctionMap = _fall_cb_map[buttonIndex];
    for (CallbackFunctionMap::iterator iter = cbFunctionMap.begin();
         iter != cbFunctionMap.end();
         ++iter) {
      iter->second();
    }
  }
}
#endif  // CONFIG_INTERRUPT_IN_EMUL

void InterruptIn::fall(std::function<void()> func) {
  if (func == nullptr) {
    LOG_ERR("Cannot call fall with nullptr");
    return;
  }

  // We allow for multiple calls to fall() for setting multiple callbacks.
  // On first call, we configure the Zephyr driver to call the
  // InterruptIn<pinName>::callback() on button fall
  // On subsequent calls, we simply push the callback to the vector
  std::scoped_lock<Mutex> guard(_cbMutex);

  size_t buttonIndex                 = static_cast<size_t>(_pinName) - 1;
  CallbackFunctionMap& cbFunctionMap = _fall_cb_map[buttonIndex];
  LOG_DBG("Setting up callback for button %d", buttonIndex);
#if CONFIG_INTERRUPT_IN_EMUL != 1
  if (cbFunctionMap.empty()) {
    _cbData._instance = this;
    gpio_init_callback(&_cbData._gpio_cb, &InterruptIn::callback, BIT(_gpio.pin));
    gpio_add_callback(_gpio.port, &_cbData._gpio_cb);
    LOG_DBG("Callback set for %s pin %d", _gpio.port->name, _gpio.pin);
  }
#endif
  LOG_DBG("Registering callback in map for %p", this);
  cbFunctionMap[this] = func;
}

#if !defined(CONFIG_INTERRUPT_IN_EMUL)
void InterruptIn::callback(const struct device* port,
                           struct gpio_callback* cb,
                           gpio_port_pins_t pins) {
  // printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
  // We need to cast cb for getting the instance on which the callback is running
  // static_cast<CallbackData*> is not accepted here, reinterpret_cast is not supported
  // cppcheck-suppress cstyleCast
  // NOLINTNEXTLINE(readability/casting)
  CallbackData* pCallbackData = (CallbackData*)cb;  // MISRA-suppress: 7.2.1  legacy API
                                                    // reviewed by Serge 2026-03-11
  InterruptIn* pInstance             = pCallbackData->_instance;
  size_t buttonIndex                 = static_cast<size_t>(pInstance->_pinName) - 1;
  CallbackFunctionMap& cbFunctionMap = _fall_cb_map[buttonIndex];
  for (CallbackFunctionMap::iterator iter = cbFunctionMap.begin();
       iter != cbFunctionMap.end();
       ++iter) {
    iter->second();
  }
}
#endif  // ! defined(CONFIG_INTERRUPT_IN_EMUL)

}  // namespace zpp_lib
