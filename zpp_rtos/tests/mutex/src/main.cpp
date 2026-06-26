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

// zpp_rtos
#include "zpp_include/mutex.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_test.hpp"

// test cases
ZPP_ZTEST_USER(zpp_mutex, test_mutex_lock_unlock) {
  zpp_lib::Mutex mutex;

  // TESTPOINT: try to lock and unlock mutex several times
  auto bool_ret = mutex.try_lock();
  zpp_zassert_true(!bool_ret.has_error());
  zpp_zassert_true(bool_ret);
  zpp_zassert_true(mutex.unlock());
  bool_ret = mutex.try_lock();
  zpp_zassert_true(!bool_ret.has_error());
  zpp_zassert_true(bool_ret);
  zpp_zassert_true(mutex.unlock());

  // TESTPOINT: try to lock and unlock mutex in another thread
  static constexpr auto kThreadName = "test_mutex_thread";
#if CONFIG_USER_SPACE
  zpp_lib::Thread thread(zpp_lib::PreemptableThreadPriority::Normal, kThreadName, true);
#else   // CONFIG_USER_SPACE
  zpp_lib::Thread thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, kThreadName);
#endif  // CONFIG_USER_SPACE
  auto ret = thread.start([&mutex]() {
    auto bool_ret = mutex.try_lock();
    zpp_zassert_true(!bool_ret.has_error());
    zpp_zassert_true(bool_ret);
    zpp_zassert_true(mutex.unlock());
  });
  zpp_zassert_true(ret);
  zpp_zassert_true(thread.join());
}

ZPP_ZTEST_SUITE(zpp_mutex, nullptr, nullptr, nullptr, nullptr, nullptr);
