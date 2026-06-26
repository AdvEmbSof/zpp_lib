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
 * @file test_thread.cpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief Test program for zpp_lib Thread class
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

// zephyr

// std
#include <functional>

// zpp_rtos
#include "zpp_include/semaphore.hpp"
#include "zpp_include/this_thread.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/time.hpp"
#include "zpp_include/zpp_assert.hpp"
#include "zpp_include/zpp_log.hpp"
#include "zpp_include/zpp_test.hpp"

ZPP_LOG_MODULE_REGISTER(test_thread, CONFIG_APP_LOG_LEVEL);

void sleep_function() {
  using std::literals::chrono_literals::operator""ms;

  // TESTPOINT validate that busy wait works properly
  static constexpr uint64_t kInitialAllowedDeltaInUs = 600;
  uint64_t allowed_delta_in_us                       = kInitialAllowedDeltaInUs;
  static constexpr uint64_t kIncreaseDeltaInUs       = 100;
  static constexpr uint8_t kNbrOfDurations           = 5;
  std::chrono::microseconds wait_duration            = 10ms;
  for (uint8_t i = 0; i < kNbrOfDurations; i++) {
    std::chrono::microseconds before_wait_time = zpp_lib::Time::get_uptime();
    zpp_lib::ThisThread::busy_wait(wait_duration);
    std::chrono::microseconds after_wait_time = zpp_lib::Time::get_uptime();

    std::chrono::microseconds delta_time = (after_wait_time - before_wait_time) - wait_duration;

    zpp_zassert_true(abs(delta_time.count()) < allowed_delta_in_us,
                     "(BUSY WAIT) iteration %d: Elapsed time is not within expected "  // MISRA-suppress:
                                                                                       // 7.2.1 false
                                                                                       // positive,
                                                                                       // reviewed by
                                                                                       // Serge
                                                                                       // 2026-03-16
                     "range, delta %lld, allowed = %lld, before = %lld, after = %lld",
                     i,
                     delta_time.count(),
                     allowed_delta_in_us,
                     before_wait_time.count(),
                     after_wait_time.count());

    // double wait duration
    wait_duration *= 2;
    allowed_delta_in_us += kIncreaseDeltaInUs;
  }
  ZPP_LOG_DBG("Done with busy_wait");

  // TESTPOINT validate that sleep works properly
  std::chrono::milliseconds sleep_duration = 1000ms;
  for (uint8_t i = 0; i < kNbrOfDurations; i++) {
    std::chrono::microseconds before_sleep_time = zpp_lib::Time::get_uptime();
    zpp_lib::ThisThread::sleep_for(sleep_duration);
    std::chrono::microseconds after_sleep_time = zpp_lib::Time::get_uptime();

    std::chrono::microseconds delta_time        = (after_sleep_time - before_sleep_time) - sleep_duration;
    static constexpr uint64_t kAllowedDeltaInUs = 500;

    zpp_zassert_true(abs(delta_time.count()) < kAllowedDeltaInUs,
                     "(SLEEP) iteration %d: Elapsed time is not within expected range, "  // MISRA-suppress:
                                                                                          // 7.2.1
                                                                                          // false
                                                                                          // positive,
                                                                                          // reviewed
                                                                                          // by Serge
                                                                                          // 2026-03-16
                     "delta %lld, allowed = %lld, before = %lld, after = %lld",
                     i,
                     delta_time.count(),
                     kAllowedDeltaInUs,
                     before_sleep_time.count(),
                     after_sleep_time.count());

    // double sleep duration
    sleep_duration *= 2;
  }
  ZPP_LOG_DBG("Done with sleep_for");
}

ZPP_ZTEST_USER(zpp_thread, test_sleep_main) {
  // test sleep (main thread)
  sleep_function();
}

ZPP_ZTEST_USER(zpp_thread, test_sleep_secondary_thread) {
  // test sleep (secondary thread)
  static constexpr auto kThreadName = "secondary_thread";
#if CONFIG_USER_SPACE
  zpp_lib::Thread other_thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, kThreadName, true);
#else   // CONFIG_USER_SPACE
  zpp_lib::Thread other_thread(zpp_lib::PreemptableThreadPriority::PriorityNormal, kThreadName);
#endif  // CONFIG_USER_SPACE
  auto res = other_thread.start(sleep_function);
  if (!res) {
    zpp_zassert_true(res, "Cannot start thread: %d", static_cast<int>(res.error()));
  }

  // wait for thread to complete
  ZPP_LOG_DBG("Main thread waiting for secondary thread");
  res = other_thread.join();
  if (!res) {
    zpp_zassert_true(res, "Cannot start thread: %d", static_cast<int>(res.error()));
  }
}

void thread_fn(volatile uint64_t* counter,   // MISRA-suppress: 6.2.1
               const volatile bool* stop) {  // MISRA-suppress: 6.2.1  use of volatile for preventing
  // compiler optimization, reviewed by Serge 2026-03-16
  while (!*stop) {
    (*counter)++;
  }
  ZPP_LOG_DBG("Exiting thread function");
}

ZPP_ZTEST_USER(zpp_thread, test_round_robin) {
  // Create two threads with below normal priority
  static constexpr auto kThreadName1 = "Thread1";
  static constexpr auto kThreadName2 = "Thread2";
#if CONFIG_USER_SPACE
  zpp_lib::Thread thread1(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal, kThreadName1, true);
  zpp_lib::Thread thread2(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal, kThreadName2, true);
#else   // CONFIG_USER_SPACE
  zpp_lib::Thread thread1(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal, kThreadName1);
  zpp_lib::Thread thread2(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal, kThreadName2);
#endif  // CONFIG_USER_SPACE

  // Start the two threads
  static volatile uint64_t s_counter1 = 0;  // MISRA-suppress: 6.2.1
                                            // use of volatile for preventing compiler
                                            // optimization, reviewed by Serge 2026-03-16
  static volatile uint64_t s_counter2 = 0;  // MISRA-suppress: 6.2.1
                                            // use of volatile for preventing compiler
                                            // optimization, reviewed by Serge 2026-03-16
  static volatile bool s_stop = false;      // MISRA-suppress: 6.2.1
                                            // use of volatile for preventing compiler
                                            // optimization, reviewed by Serge 2026-03-16

  auto res = thread1.start([=]() { thread_fn(&s_counter1, &s_stop); });
  if (!res) {
    zpp_zassert_true(res, "Cannot start thread1: %d", static_cast<int>(res.error()));
  }
  res = thread2.start([=]() { thread_fn(&s_counter2, &s_stop); });
  if (!res) {
    zpp_zassert_true(res, "Cannot start thread2: %d", static_cast<int>(res.error()));
  }

  // Make sure that it has a higher priority than the other threads
  zpp_lib::ThisThread::set_priority(zpp_lib::PreemptableThreadPriority::PriorityAboveNormal);
  ZPP_LOG_DBG("Main thread priority is %d", zpp_lib::preemptable_thread_priority_to_zephyr_prio(zpp_lib::ThisThread::get_priority()));

  // Have the main thread for the duration of two slices
  // determine the number of time slices to wait before checking for counters (must be
  // even)
  static constexpr uint8_t kNbrOfTimeSlicesToWait = 100;
  static constexpr std::chrono::milliseconds kSleepDuration(kNbrOfTimeSlicesToWait * CONFIG_TIMESLICE_SIZE);
  ZPP_LOG_DBG("Main thread waiting for %lld msecs", kSleepDuration.count());
  zpp_lib::ThisThread::sleep_for(kSleepDuration);

  ZPP_LOG_DBG("Main thread stops waiting");

  // tell the two threads to stop
  s_stop = true;

  // TESTPOINT: check that both thread1 and thread2 got approximately the same CPU
  // resources
  static constexpr uint64_t kDelta = 30000;
  uint64_t diff                    = s_counter1 > s_counter2 ? s_counter1 - s_counter2 : s_counter2 - s_counter1;

  ZPP_LOG_DBG("Waiting for threads");
  res = thread1.join();
  if (!res) {
    zpp_zassert_true(res, "Cannot join thread1: %d", static_cast<int>(res.error()));
  }
  res = thread2.join();
  if (!res) {
    zpp_zassert_true(res, "Cannot join thread2: %d", static_cast<int>(res.error()));
  }
  zpp_zassert_true(diff < kDelta,
                   "Time slicing looks uneven, diff = %lld, delta = %lld, counter1 %lld, "
                   "counter2 %lld",
                   diff,
                   kDelta,
                   s_counter1,
                   s_counter2);
}

ZPP_ZTEST_SUITE(zpp_thread, nullptr, nullptr, nullptr, nullptr, nullptr);
