
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
 * @file utils.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Helper implementation for using some zephyr RTOS utilities
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

 // zephyr
#include <zephyr/logging/log.h>
#include <zephyr/debug/cpu_load.h>

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

#ifdef CONFIG_CPU_LOAD
void printCPULoad() {
  int load = cpu_load_get(true);
  uint32_t percent = load / 10;
  int32_t fraction = load % 10;

	LOG_INF("CPU Load: %d.%03d%%", percent, fraction);
}
#endif

} // namespace zpp_lib