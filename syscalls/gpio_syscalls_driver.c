
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
 * @brief Syscall driver implementation for gpio_pin_get()/gpio_pin_set() function
 *
 *
 * @date 2026-04-01
 * @version 1.0.0
 ***************************************************************************/

#include "gpio_syscalls_driver.h"

#include <string.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "gpio_syscalls.h"

LOG_MODULE_REGISTER(gpio_syscalls);

/*
 * Fake sample driver for demonstration purposes
 *
 * This is fake driver for demonstration purposes, showing how an application
 * can make system calls to interact with it.
 *
 * The driver sets up a timer which is used to fake interrupts.
 */

struct gpio_syscall_dev_data {
  const struct device* dev;
};

static int gpio_syscall_set_impl(const struct device* dev,
                                 struct gpio_dt_spec* gpio,
                                 int value) {
  LOG_DBG("%s(%p, %d)", __func__, dev, value);

  // we expect that gpio has already been configured correctly
  return gpio_pin_set(gpio->port, gpio->pin, value);
}

static int gpio_syscall_get_impl(const struct device* dev, struct gpio_dt_spec* gpio) {
  LOG_DBG("%s(%p)", __func__, dev);

  // we expect that gpio has already been configured correctly
  return gpio_pin_get(gpio->port, gpio->pin);
}

static DEVICE_API(gpio_syscall, gpio_syscall_api) = {.set = gpio_syscall_set_impl,
                                                     .get = gpio_syscall_get_impl};

static int gpio_syscall_init(const struct device* dev) { return 0; }

static struct gpio_syscall_dev_data gpio_syscall_dev_data;

DEVICE_DEFINE(gpio_syscall_driver,
              GPIO_SYSCALL_DRIVER_NAME,
              &gpio_syscall_init,
              NULL,
              &gpio_syscall_dev_data,
              NULL,
              POST_KERNEL,
              CONFIG_KERNEL_INIT_PRIORITY_DEVICE,
              &gpio_syscall_api);
