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
 * @file zpp_log.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Redefinitions of zephyr logging macros for use in zpp_lib
 *        This prevents macro related warning when using clang-tidy and applying
 *        some cppcoreguidelines checks
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

#include <zephyr/logging/log.h>

#ifdef ZPP_CLANG_TIDY
#define ZPP_LOG_MODULE_REGISTER(name, level)
#define ZPP_LOG_MODULE_DECLARE(name, level)

// NOLINTBEGIN(readability-identifier-naming,readability-named-parameter)
template <typename... Args> constexpr void ZPP_LOG_INF(Args&&...) {}

template <typename... Args> constexpr void ZPP_LOG_DBG(Args&&...) {}

template <typename... Args> constexpr void ZPP_LOG_WRN(Args&&...) {}

template <typename... Args> constexpr void ZPP_LOG_ERR(Args&&...) {}
// NOLINTEND(readability-identifier-naming,readability-named-parameter)
#else  // !defined(ZPP_CLANG_TIDY)
#define ZPP_LOG_MODULE_REGISTER(name, level) LOG_MODULE_REGISTER(name, level)
#define ZPP_LOG_MODULE_DECLARE(name, level) LOG_MODULE_DECLARE(name, level)

#define ZPP_LOG_INF(...) LOG_INF(__VA_ARGS__)
#define ZPP_LOG_DBG(...) LOG_DBG(__VA_ARGS__)
#define ZPP_LOG_WRN(...) LOG_WRN(__VA_ARGS__)
#define ZPP_LOG_ERR(...) LOG_ERR(__VA_ARGS__)
#endif  // ZPP_CLANG_TIDY
