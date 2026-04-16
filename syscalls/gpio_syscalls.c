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
 * @file time_syscalls.h
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Syscall implementation for gpio_pin_get()/gpio_pin_set() function
 *
 *
 * @date 2026-04-01
 * @version 1.0.0
 ***************************************************************************/

#include "gpio_syscalls.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/kernel.h>

/* Kernel implementation */
int z_impl_gpio_syscall_set(const struct device* dev,
                            struct gpio_dt_spec* gpio,
                            int value) {
  const struct gpio_syscall_driver_api* api =
      (const struct gpio_syscall_driver_api*)dev->api;

  return api->set(dev, gpio, value);
}

int z_vrfy_gpio_syscall_set(const struct device* dev,
                            struct gpio_dt_spec* gpio,
                            int value) {
  if (K_SYSCALL_DRIVER_GPIO_SYSCALL(dev, set)) {
    return -EINVAL;
  }
  return z_impl_gpio_syscall_set(dev, gpio, value);
}

#include <zephyr/syscalls/gpio_syscall_set_mrsh.c>  // NOLINT(build/include)

/* Kernel implementation */
int z_impl_gpio_syscall_get(const struct device* dev, struct gpio_dt_spec* gpio) {
  const struct gpio_syscall_driver_api* api =
      (const struct gpio_syscall_driver_api*)dev->api;

  return api->get(dev, gpio);
}

int z_vrfy_gpio_syscall_get(const struct device* dev, struct gpio_dt_spec* gpio) {
  if (K_SYSCALL_DRIVER_GPIO_SYSCALL(dev, get)) {
    return -EINVAL;
  }
  return z_impl_gpio_syscall_get(dev, gpio);
}

#include <zephyr/syscalls/gpio_syscall_get_mrsh.c>  // NOLINT(build/include)
