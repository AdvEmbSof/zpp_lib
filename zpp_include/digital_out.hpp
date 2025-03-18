#pragma once

// zephyr
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/result.hpp"

namespace zpp_lib {
  
/** A digital output, used for setting the state of a pin
 *
 * @note Synchronization level: Interrupt safe
 *
 */

class DigitalOut : private NonCopyable<DigitalOut>
{
public:
    /** Create a DigitalOut connected to the specified pin
     *
     *  @param pin DigitalOut pin to connect to
     */
    DigitalOut(const gpio_dt_spec& gpio);

    /** Create a DigitalOut connected to the specified pin
     *
     *  @param pin DigitalOut pin to connect to
     *  @param value the initial pin value
     */
    DigitalOut(uint32_t node_id, uint32_t value);

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
    DigitalOut &operator= (int value)
    {
        // Underlying write is thread safe
        write(value);
        return *this;
    }

    /** A shorthand for write() using the assignment operator which copies the
     * state from the DigitalOut argument.
     * \sa DigitalOut::write()
     */
    DigitalOut &operator= (DigitalOut &rhs);

    /** A shorthand for read()
     * \sa DigitalOut::read()
     * @code
     *      DigitalIn  button(BUTTON1);
     *      DigitalOut led(LED1);
     *      led = button;   // Equivalent to led.write(button.read())
     * @endcode
     */
    operator int()
    {
        // Underlying call is thread safe
        return read();
    }

protected:
    struct gpio_dt_spec _gpio;
};

/** @}*/

} // namespace zpp_lib

