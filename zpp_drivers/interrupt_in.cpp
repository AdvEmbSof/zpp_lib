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

// zpp_lib
#include "zpp_include/func_ptr_helper.hpp"

LOG_MODULE_DECLARE(zpp_drivers, CONFIG_ZPP_DRIVERS_LOG_LEVEL);

namespace zpp_lib {

template <PinName pinName>
InterruptIn<pinName>::InterruptIn() {
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
    LOG_ERR("Cannot configure GPIO %s as output (%d)", _gpio.port->name, ret);
    __ASSERT(false, "Cannot configure GPIO %s as output (%d)", _gpio.port->name, ret);
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
}

template <PinName pinName>
int InterruptIn<pinName>::InterruptIn::read() {
  return gpio_pin_get_dt(&_gpio);
}

template <PinName pinName>
void InterruptIn<pinName>::InterruptIn::fall(std::function<void()> func) {
  _fall_callback = func;

  typedef std::function<void(
      const struct device*, struct gpio_callback*, gpio_port_pins_t)>
      CallbackFunctionType;
  using namespace std::placeholders;
  CallbackFunctionType callbackFunction =
      std::bind(&InterruptIn::callback, this, _1, _2, _3);
  gpio_callback_handler_t callbackHandler =
      getFuncPtr<static_cast<size_t>(pinName),
                 void,
                 const struct device*,
                 struct gpio_callback*,
                 gpio_port_pins_t>(callbackFunction);
  gpio_init_callback(&_cbData, callbackHandler, BIT(_gpio.pin));
  gpio_add_callback(_gpio.port, &_cbData);

  LOG_DBG("Set up button at %s pin %d\n", _gpio.port->name, _gpio.pin);
}

template <PinName pinName>
void InterruptIn<pinName>::callback(const struct device* port,
                                    struct gpio_callback* cb,
                                    gpio_port_pins_t pins) {
  printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());

  if (_fall_callback != nullptr) {
    _fall_callback();
  }
}

// template instantiation
template class InterruptIn<PinName::BUTTON1>;
template class InterruptIn<PinName::BUTTON2>;
template class InterruptIn<PinName::BUTTON3>;
template class InterruptIn<PinName::BUTTON4>;

}  // namespace zpp_lib
