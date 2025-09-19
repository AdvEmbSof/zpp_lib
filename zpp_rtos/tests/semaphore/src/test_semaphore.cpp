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

#include <zephyr/ztest.h>

// stl
#include <memory>

// zpp_rtos
#include "zpp_include/semaphore.hpp"
#include "zpp_include/thread.hpp"

// test cases
ZTEST_USER(zpp_semaphore, test_semaphore_release_acquire) {
  static constexpr uint32_t initial_count = 0U;
  static constexpr uint32_t max_count     = 1U;
  zpp_lib::Semaphore sem(initial_count, max_count);

  // TESTPOINT: try to release and acquire semaphore several times
  zassert_true(sem.release());
  zassert_true(sem.acquire());
  zassert_true(sem.release());
  zassert_true(sem.acquire());

  // TESTPOINT: try to acquire one more time (ret should evaluate to false without
  // error)
  auto boolRet = sem.try_acquire();
  zassert_true(!boolRet.has_error());
  zassert_true(!boolRet);

  // TESTPOINT: try to acquire semaphore that is released in another thread
  zpp_lib::Thread thread;
  auto ret = thread.start([&sem]() {
    auto ret = sem.release();
    zassert_true(ret);
  });
  zassert_true(ret);
  zassert_true(thread.join());
  zassert_true(sem.acquire());
}

ZTEST_SUITE(zpp_semaphore, NULL, NULL, NULL, NULL, NULL);
