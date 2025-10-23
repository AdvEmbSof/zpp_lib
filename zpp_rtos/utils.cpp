
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
#include <zephyr/debug/cpu_load.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/sys_heap.h>

// zpp_lib
#include "zpp_include/utils.hpp"

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

extern "C" {
// Zephyr defines this symbol globally
// To access it you need to define CONFIG_HEAP_MEM_POOL_SIZE=...
extern struct sys_heap _system_heap;
}

namespace zpp_lib {

#ifdef CONFIG_THREAD_ANALYZER
static void logThreadStatistics(const struct k_thread* thread, void* user_data) {
  int* idx = static_cast<int*>(user_data);

  size_t total_stack  = 0;
  size_t unused_stack = 0;
  size_t used_stack   = 0;

  k_tid_t thread_id                        = (k_tid_t)thread;
  static constexpr uint8_t STATE_STR_LEN   = 32;
  char state_str[STATE_STR_LEN]            = {0};
  k_thread_runtime_stats_t rt_stats_thread = {0};

  k_thread_runtime_stats_get(thread_id, &rt_stats_thread);

  const char* name = k_thread_name_get(thread_id);
  int prio         = k_thread_priority_get(thread_id);
  k_thread_state_str(thread_id, state_str, sizeof(state_str));

  /* --- Detect idle thread --- */
  // By default, zephyr does not add metadata for idle thread.
  // it is always ready.
  bool is_idle = false;
  if (name && strncmp(name, "idle", 4) == 0) {
    is_idle = true;
  } else if (prio == K_IDLE_PRIO) {
    is_idle = true;
  }

  if (is_idle) {
    strncpy(state_str, "ready", STATE_STR_LEN);
  }

#if defined(CONFIG_THREAD_STACK_INFO)
  total_stack = thread->stack_info.size;
  k_thread_stack_space_get(thread_id, &unused_stack);
  used_stack = total_stack - unused_stack;
#endif

  LOG_INF("%2d | %14p | %-10s | %-9s | %4d | %4u / %-4u",
          (*idx)++,
          thread_id,
          name ? name : "Unnamed",
          state_str,
          prio,
          (unsigned int)used_stack,
          (unsigned int)total_stack);
}
#endif

void Utils::logThreadsSummary() {
#if defined(CONFIG_THREAD_ANALYZER)
  LOG_INF("=== Threads Summary ===");
  LOG_INF(" # |      Thread ID | Name       | State     | Prio | Stack (used/total)");
  LOG_INF("---+----------------+------------+-----------+------+-------------------");

  int idx = 0;
  k_thread_foreach(logThreadStatistics, &idx);

  LOG_INF("---+----------------+------------+-----------+------+-------------------\n");
#else
  LOG_WRN("Thread statistics not available (enable CONFIG_THREAD_ANALYZER)");
#endif
}

void Utils::logHeapSummary() {
#if defined(CONFIG_SYS_HEAP_RUNTIME_STATS)

  struct sys_memory_stats stats;
  sys_heap_runtime_stats_get(&_system_heap, &stats);

  LOG_INF("=== Heap Summary ===");
  LOG_INF("\tAllocated: %u bytes", stats.allocated_bytes);
  LOG_INF("\tFree:      %u bytes", stats.free_bytes);
  LOG_INF("\tMax Alloc: %u bytes\n", stats.max_allocated_bytes);
  // LOG_INF("\tBlocks:    %u", stats.blocks);
#else
  LOG_WRN("Heap statistics not available (enable CONFIG_SYS_HEAP_RUNTIME_STATS)");
#endif
}

void Utils::logCPULoad() {
#if defined(CONFIG_CPU_LOAD)
  int load         = cpu_load_get(true);
  uint32_t percent = load / 10;
  int32_t fraction = load % 10;
  LOG_INF("=== CPU Load Summary ===");
  LOG_INF("\tCPU Load: %d.%03d%%\n", percent, fraction);
#else
  LOG_WRN("CPU Load not available (enable CONFIG_CPU_LOAD)");
#endif
}

}  // namespace zpp_lib
