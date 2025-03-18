#pragma once

// zephyr
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// stl
#include <functional>

// zpp_lib
#include "zpp_include/result.hpp"
#include "zpp_include/non_copyable.hpp"

namespace zpp_lib {
  
/** A digital interrupt input, used to call a function on a rising or falling edge
 *
 * @note Synchronization level: Interrupt safe
 *
 * 
 */
class InterruptIn : private NonCopyable<InterruptIn> {

public:

    /** Create an InterruptIn connected to the specified pin
     *
     *  @param pin InterruptIn pin to connect to
     */
    InterruptIn(const gpio_dt_spec& gpio);

    /** Read the input, represented as 0 or 1 (int)
     *
     *  @returns
     *    An integer representing the state of the input pin,
     *    0 for logical 0, 1 for logical 1
     */
    int read();

    /** An operator shorthand for read()
     */
    operator int();

    /** Attach a function to call when a rising edge occurs on the input
     *  Interrupts are enabled for the pin
     *
     *  @param func A pointer to a void function, or 0 to set as none
     */
    void rise(std::function<void()> func);

    /** Attach a function to call when a falling edge occurs on the input
     *  Interrupts are enabled for the pin
     *
     *  @param func A pointer to a void function, or 0 to set as none
     */
    void fall(std::function<void()> func);

protected:
    void callback(const struct device *port,
	             struct gpio_callback *cb,
	             gpio_port_pins_t pins);
    struct gpio_dt_spec _gpio;    
    struct gpio_callback _cbData;
    std::function<void()> _rise_callback;
};

/** @}*/

} // namespace zpp_lib

