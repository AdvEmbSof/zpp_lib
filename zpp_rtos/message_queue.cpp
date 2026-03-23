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
 * @file message_queue.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class implementation wrapping zephyr OS message queue
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

 #include "zpp_include/message_queue.hpp"
 
// Zephyr sdk
#include <zephyr/logging/log.h>
#if CONFIG_USERSPACE
#include <zephyr/app_memory/app_memdomain.h>
#endif  // CONFIG_USERSPACE

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

#if CONFIG_USERSPACE
extern struct k_mem_partition zpp_lib_partition;
#define ZPP_LIB_DATA K_APP_DMEM(zpp_lib_partition)
#define ZPP_LIB_BSS K_APP_BMEM(zpp_lib_partition)
#else  // CONFIG_USERSPACE
#define ZPP_LIB_DATA
#define ZPP_LIB_BSS
#endif  // CONFIG_USERSPACE

namespace zpp_lib {

#if CONFIG_USERSPACE
ZPP_LIB_DATA uint8_t gMsgqInstanceCount = 0;
struct k_msgq ZPP_MESSAGE_QUEUE_ARRAY[CONFIG_ZPP_MSGQ_POOL_SIZE] = {};
#endif  // CONFIG_USERSPACE

} // namespace zpp_lib