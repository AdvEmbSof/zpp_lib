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
 * @file utils.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Helper declaration for using some zephyr RTOS utilities
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

namespace zpp_lib {

class Utils {
 public:
  Utils() = default;

#if defined(CONFIG_THREAD_STACK_INFO)
  static void logThreadsStackInfo();
#endif
  static void logThreadsSummary();
#if defined(CONFIG_SYS_HEAP_RUNTIME_STATS)
  static void logHeapSummary();
#endif
  static void logCPULoad();
};

}  // namespace zpp_lib
