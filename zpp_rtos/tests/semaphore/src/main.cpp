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
 * @file test_semaphore.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Test program for zpp_lib Semaphore class
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

// stl
#include <memory>

// zpp_rtos
#include "zpp_include/semaphore.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_test.hpp"

// test cases
ZPP_ZTEST_USER(zpp_semaphore, test_semaphore_release_acquire) {
  static constexpr uint32_t kInitialCount = 0U;
  static constexpr uint32_t kMaxCount     = 1U;
  zpp_lib::Semaphore sem(kInitialCount, kMaxCount);

  // TESTPOINT: try to release and acquire semaphore several times
  zpp_zassert_true(sem.release());
  zpp_zassert_true(sem.acquire());
  zpp_zassert_true(sem.release());
  zpp_zassert_true(sem.acquire());

  // TESTPOINT: try to acquire one more time (ret should evaluate to false without
  // error)
  auto bool_ret = sem.try_acquire();
  zpp_zassert_true(!bool_ret.has_error());
  zpp_zassert_true(!bool_ret);

  // TESTPOINT: try to acquire semaphore that is released in another thread
  static constexpr auto kThreadName = "test_semaphore_thread";
#if CONFIG_USER_SPACE
  zpp_lib::Thread thread(zpp_lib::PreemptableThreadPriority::Normal, kThreadName, true);
#else   // CONFIG_USER_SPACE
  zpp_lib::Thread thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, kThreadName);
#endif  // CONFIG_USER_SPACE
  auto ret = thread.start([&sem]() {
    auto ret = sem.release();
    zpp_zassert_true(ret);
  });
  zpp_zassert_true(ret);
  zpp_zassert_true(thread.join());
  zpp_zassert_true(sem.acquire());
}

ZPP_ZTEST_SUITE(zpp_semaphore, nullptr, nullptr, nullptr, nullptr, nullptr);
