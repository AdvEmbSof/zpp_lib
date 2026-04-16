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
 * @brief Syscall declaration for k_cycle_get_32() function
 *
 *
 * @date 2026-04-01
 * @version 1.0.0
 ***************************************************************************/

#pragma once

#include <stdint.h>
#include <zephyr/kernel.h>

// Declare as a proper Zephyr syscall — generates the privilege elevation glue
__syscall uint32_t userspace_cycle_get_32(void);

#include <zephyr/syscalls/time_syscalls.h>
