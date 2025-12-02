
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

#include "zpp_include/utils.hpp"

// zephyr
#include <zephyr/debug/cpu_load.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/sys_heap.h>

// std
#include <cstdlib>

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

extern "C" {
// Zephyr defines this symbol globally
// To access it you need to define CONFIG_HEAP_MEM_POOL_SIZE=...
extern struct sys_heap _system_heap;
}

#if defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
void* operator new(size_t size) { return k_malloc(size); }

void operator delete(void* ptr) noexcept { k_free(ptr); }
#endif

namespace zpp_lib {

#if defined(CONFIG_THREAD_STACK_INFO)
static void logThreadStackInfo(const struct k_thread* thread, void* idx) {
  size_t stack_size    = thread->stack_info.size;
  size_t start_address = thread->stack_info.start;

  k_tid_t thread_id        = (k_tid_t)thread;
  size_t unused_stack_size = 0;
  auto rc                  = k_thread_stack_space_get(thread_id, &unused_stack_size);
  __ASSERT(rc == 0, "k_thread_stack_space_get failed: %d", rc);
  size_t used_stack_size = stack_size - unused_stack_size;

  const char* name    = k_thread_name_get(thread_id);
  uint32_t* threadIdx = static_cast<uint32_t*>(idx);
  LOG_INF("%2d | %10p | %-15s | 0x%08x | %4u / %-4u",
          (*threadIdx)++,
          thread_id,
          name ? name : "Unnamed",
          start_address,
          used_stack_size,
          stack_size);
}

void Utils::logThreadsStackInfo() {
  LOG_INF("=== Threads Summary ===");
  LOG_INF(" # |  Thread ID | Name           | Start address |Stack (used/total)");
  LOG_INF("---+------------+----------------+---------------+------------------");

  int idx = 0;
  k_thread_foreach(logThreadStackInfo, &idx);
  LOG_INF("---+------------+----------------+---------------+------------------\n");
}
#endif

#ifdef CONFIG_THREAD_ANALYZER
struct ThreadStatistics {
  k_tid_t thread_id                      = nullptr;
  const char* name                       = nullptr;
  int prio                               = 0;
  static constexpr uint8_t STATE_STR_LEN = 32;
  char state_str[STATE_STR_LEN]          = {0};
  size_t stack_size                      = 0;
  size_t used_stack_size                 = 0;
};

static const ThreadStatistics getThreadStatistics(const struct k_thread* thread) {
  ThreadStatistics threadStatistics;
  threadStatistics.thread_id = (k_tid_t)thread;

  k_thread_runtime_stats_t rt_stats_thread = {0};
  auto rc = k_thread_runtime_stats_get(threadStatistics.thread_id, &rt_stats_thread);
  __ASSERT(rc == 0, "k_thread_runtime_stats_get failed: %d", rc);

  threadStatistics.name = k_thread_name_get(threadStatistics.thread_id);
  threadStatistics.prio = k_thread_priority_get(threadStatistics.thread_id);
  k_thread_state_str(threadStatistics.thread_id,
                     threadStatistics.state_str,
                     sizeof(threadStatistics.state_str));

  /* --- Detect idle thread --- */
  // By default, zephyr does not add metadata for idle thread.
  // it is always ready.
  bool is_idle = false;
  if (threadStatistics.name && strncmp(threadStatistics.name, "idle", 4) == 0) {
    is_idle = true;
  } else if (threadStatistics.prio == K_IDLE_PRIO) {
    is_idle = true;
  }

  if (is_idle) {
    strncpy(threadStatistics.state_str, "ready", ThreadStatistics::STATE_STR_LEN);
  }

#if defined(CONFIG_THREAD_STACK_INFO)
  threadStatistics.stack_size = thread->stack_info.size;
  size_t unused_stack_size    = 0;
  rc = k_thread_stack_space_get(threadStatistics.thread_id, &unused_stack_size);
  __ASSERT(rc == 0, "k_thread_stack_space_get failed: %d", rc);
  threadStatistics.used_stack_size = threadStatistics.stack_size - unused_stack_size;
#endif

  return threadStatistics;
}

static void logThreadStatistics(const struct k_thread* thread, void* idx) {
  ThreadStatistics threadStatistics = getThreadStatistics(thread);

  uint32_t* threadIdx = static_cast<uint32_t*>(idx);
  LOG_INF("%2d | %14p | %-12s | %-9s | %4d | %4u / %-4u",
          (*threadIdx)++,
          threadStatistics.thread_id,
          threadStatistics.name ? threadStatistics.name : "Unnamed",
          threadStatistics.state_str,
          threadStatistics.prio,
          threadStatistics.used_stack_size,
          threadStatistics.stack_size);
}
#endif

void Utils::logThreadsSummary() {
#if defined(CONFIG_THREAD_ANALYZER)
  LOG_INF("=== Threads Summary ===");
  LOG_INF(" # |      Thread ID | Name         | State     | Prio | Stack (used/total)");
  LOG_INF("---+----------------+--------------+-----------+------+-------------------");

  int idx = 0;
  k_thread_foreach(logThreadStatistics, &idx);

  LOG_INF("---+----------------+------------+-----------+------+-------------------\n");
#else
  LOG_WRN("Thread statistics not available (enable CONFIG_THREAD_ANALYZER)");
#endif
}

#if defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
void Utils::logHeapSummary() {

uint8_t* leak = new uint8_t[256];
for (int i = 0; i < 256; i++) {
  leak[i] = i;
}
printk("leak: %d", leak[100]);

  struct sys_memory_stats stats;
  sys_heap_runtime_stats_get(&_system_heap, &stats);
  LOG_INF("=== Heap Summary ===");
  LOG_INF("\tAllocated: %u bytes", stats.allocated_bytes);
  LOG_INF("\tFree:      %u bytes", stats.free_bytes);
  LOG_INF("\tMax Alloc: %u bytes\n", stats.max_allocated_bytes);
  // LOG_INF("\tBlocks:    %u", stats.blocks);
}
#endif

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
