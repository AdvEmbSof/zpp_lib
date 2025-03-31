#include "zpp_include/digital_out.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

namespace zpp_lib {
  
LOG_MODULE_REGISTER(zpp_drivers, CONFIG_ZPP_DRIVERS_LOG_LEVEL);

#define LED0_NODE DT_ALIAS(led0)

DigitalOut::DigitalOut(PinName pinName) : DigitalOut(pinName, 0) { 
}

DigitalOut::DigitalOut(PinName pinName, uint32_t value) {
  switch (pinName) {
    case PinName::LED0:
      _gpio = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
      break;

    default:
      break;
  }

  if (!gpio_is_ready_dt(&_gpio)) {
    LOG_ERR("GPIO %s not existing on platform", _gpio.port->name);
    __ASSERT(false, "GPIO %s not existing on platform", _gpio.port->name);
    return;
  }

  int ret = gpio_pin_configure_dt(&_gpio, GPIO_OUTPUT_ACTIVE);
  if (ret < 0) {
    LOG_ERR("Cannot configure GPIO %s as output (%d)", _gpio.port->name, ret);
    __ASSERT(false, "Cannot configure GPIO %s as output (%d)", _gpio.port->name, ret);
    return;
  }
  LOG_DBG("Pin %s initialized", _gpio.port->name);
  
  ZephyrResult rc = write(value);
  if (! rc) {
    LOG_ERR("Cannot write value %d to output (%s)", value, _gpio.port->name);
    __ASSERT(false, "Cannot write value %d to output (%s)", value, _gpio.port->name);
    return;    
  }
}

ZephyrResult DigitalOut::write(int value) {
  ZephyrResult res;
  auto ret = gpio_pin_set_dt(&_gpio, value);
  if (ret != 0) {
    res.assign_error(zephyr_to_zpp_error_code(ret));
    LOG_ERR("Cannot set value %d to pin %s", value, _gpio.port->name);
  }
  return res;
}
 
int DigitalOut::read() {
  return gpio_pin_get_dt(&_gpio);
}
 
} // namespace zpp_lib