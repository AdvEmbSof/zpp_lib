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
 * @file test_mutex.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Test program for zpp_lib Mutex class
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#include <zephyr/ztest.h>

// zpp_rtos
#include "zpp_include/mutex.hpp"
#include "zpp_include/thread.hpp"

// test cases
ZTEST_USER(zpp_mutex, test_mutex_lock_unlock) {
  zpp_lib::Mutex mutex;

  // TESTPOINT: try to lock and unlock mutex several times
  auto boolRet = mutex.try_lock();
  zassert_true(!boolRet.has_error());
  zassert_true(boolRet);
  zassert_true(mutex.unlock());
  boolRet = mutex.try_lock();
  zassert_true(!boolRet.has_error());
  zassert_true(boolRet);
  zassert_true(mutex.unlock());

  // TESTPOINT: try to lock and unlock mutex in another thread
  zpp_lib::Thread thread;
  auto ret = thread.start([&mutex]() {
    auto boolRet = mutex.try_lock();
    zassert_true(!boolRet.has_error());
    zassert_true(boolRet);
    zassert_true(mutex.unlock());
  });
  zassert_true(ret);
  zassert_true(thread.join());
}

ZTEST_SUITE(zpp_mutex, NULL, NULL, NULL, NULL, NULL);
