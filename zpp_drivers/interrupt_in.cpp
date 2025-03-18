#include "zpp_include/interrupt_in.hpp"
#include "zpp_include/callback.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

// zpp lib
#include "zpp_include/func_ptr_helper.hpp"

LOG_MODULE_REGISTER(zpp_drivers, CONFIG_ZPP_DRIVERS_LOG_LEVEL);

namespace zpp_lib {

InterruptIn::InterruptIn(const gpio_dt_spec& gpio) : _gpio(gpio)
{
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
  
	ret = gpio_pin_interrupt_configure_dt(&_gpio, GPIO_INT_EDGE_RISING);
	if (ret != 0) {
		LOG_ERR("Error %d: failed to configure interrupt on %s pin %d\n",
          	ret, _gpio.port->name, _gpio.pin);
		__ASSERT(false, "Cannot configure interrupt on GPIO %s (%d)", _gpio.port->name, ret);
    return;
  }
  LOG_DBG("Pin %s initialized", _gpio.port->name);
}

void InterruptIn::rise(std::function<void()> func)
{    
  _rise_callback = func;

  typedef std::function<void(const struct device*, struct gpio_callback*, gpio_port_pins_t)> CallbackFunctionType;
  using namespace std::placeholders;
  CallbackFunctionType callbackFunction = std::bind(&InterruptIn::callback, this, _1, _2, _3);
  gpio_callback_handler_t callbackHandler = getFuncPtr<1, void, const struct device*, struct gpio_callback*, gpio_port_pins_t>(callbackFunction);
  gpio_init_callback(&_cbData, callbackHandler, BIT(_gpio.pin));
	gpio_add_callback(_gpio.port, &_cbData);

	LOG_DBG("Set up button at %s pin %d\n", _gpio.port->name, _gpio.pin);
}

void InterruptIn::callback(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
{
  	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());

    if (_rise_callback != nullptr) {
      _rise_callback();
    }
}

} // namespace zpp_lib