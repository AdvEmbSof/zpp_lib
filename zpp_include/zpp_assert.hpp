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
 * @file zpp_assert.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Redefinitions of zephyr assertion macros for use in zpp_lib
 *        This prevents macro related warning when using clang-tidy and applying
 *        some cppcoreguidelines checks
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// NOLINTBEGIN(cppcoreguidelines-avoid-do-while)
#include <zephyr/sys/__assert.h>
#if CONFIG_TEST
#include <zephyr/ztest.h>
#endif  // CONFIG_TEST

#define ZPP_ASSERT(...) __ASSERT(__VA_ARGS__)            // NOLINT
#define ZPP_ASSERT_EVAL(...) __ASSERT_EVAL(__VA_ARGS__)  // NOLINT
#if CONFIG_TEST
#define zpp_zassert_true(...) zassert_true(__VA_ARGS__)      // NOLINT
#define zpp_zassert_equal(...) zassert_equal(__VA_ARGS__)    // NOLINT
#define zpp_zassert_within(...) zassert_within(__VA_ARGS__)  // NOLINT
#endif                                                       // CONFIG_TEST
// NOLINTNEXTLINE(readability/nolint)
// NOLINTEND(cppcoreguidelines-avoid-do-while)
