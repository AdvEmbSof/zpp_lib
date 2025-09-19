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
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

// std
#include <functional>

// zpp_rtos
#include "zpp_include/semaphore.hpp"
#include "zpp_include/this_thread.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/time.hpp"

LOG_MODULE_REGISTER(test_thread, CONFIG_APP_LOG_LEVEL);

ZTEST_USER(zpp_thread, test_priority) {
  zpp_lib::Thread otherThread(zpp_lib::PreemptableThreadPriority::PriorityNormal,
                              "Secondary thread");
}

void sleep_function() {
  using namespace std::literals;

  // TESTPOINT validate that busy wait works properly
  static constexpr uint8_t kNbrOfDurations = 5;
  std::chrono::microseconds waitDuration   = 10ms;
  for (uint8_t i = 0; i < kNbrOfDurations; i++) {
    std::chrono::microseconds beforeWaitTime = zpp_lib::Time::getUpTime();
    zpp_lib::ThisThread::busyWait(waitDuration);
    std::chrono::microseconds afterWaitTime = zpp_lib::Time::getUpTime();

    std::chrono::microseconds deltaTime = (afterWaitTime - beforeWaitTime) - waitDuration;
    static constexpr uint64_t allowedDeltaInUs = 500;

    zassert_true(abs(deltaTime.count()) < allowedDeltaInUs,
                 "(BUSY WAIT) iteration %d: Elapsed time is not within expected "
                 "range, delta %lld, allowed = %lld, before = %lld, after = %lld",
                 i,
                 deltaTime.count(),
                 allowedDeltaInUs,
                 beforeWaitTime.count(),
                 afterWaitTime.count());

    // double wait duration
    waitDuration *= 2;
  }
  LOG_DBG("Done with busyWait");

  // TESTPOINT validate that sleep works properly
  std::chrono::milliseconds sleepDuration = 1000ms;
  for (uint8_t i = 0; i < kNbrOfDurations; i++) {
    std::chrono::microseconds beforeSleepTime = zpp_lib::Time::getUpTime();
    zpp_lib::ThisThread::sleep_for(sleepDuration);
    std::chrono::microseconds afterSleepTime = zpp_lib::Time::getUpTime();

    std::chrono::microseconds deltaTime =
        (afterSleepTime - beforeSleepTime) - sleepDuration;
    static constexpr uint64_t allowedDeltaInUs = 500;

    zassert_true(abs(deltaTime.count()) < allowedDeltaInUs,
                 "(SLEEP) iteration %d: Elapsed time is not within expected range, "
                 "delta %lld, allowed = %lld, before = %lld, after = %lld",
                 i,
                 deltaTime.count(),
                 allowedDeltaInUs,
                 beforeSleepTime.count(),
                 afterSleepTime.count());

    // double sleep duration
    sleepDuration *= 2;
  }
  LOG_DBG("Done with sleep_for");
}

ZTEST_USER(zpp_thread, test_sleep_main) {
  // test sleep (main thread)
  sleep_function();
}

ZTEST_USER(zpp_thread, test_sleep_secondary_thread) {
  // test sleep (secondary thread)
  zpp_lib::Thread otherThread(zpp_lib::PreemptableThreadPriority::PriorityNormal,
                              "Secondary thread");
  auto res = otherThread.start(sleep_function);
  if (!res) {
    zassert_true(res, "Cannot start thread: %d", res.error());
  }

  // wait for thread to complete
  LOG_DBG("Main thread waiting for secondary thread");
  res = otherThread.join();
  if (!res) {
    zassert_true(res, "Cannot start thread: %d", res.error());
  }
}

void thread_fn(volatile uint64_t* counter, const volatile bool* stop) {
  while (!*stop) {
    (*counter)++;
  }
  LOG_DBG("Exiting thread function");
}

ZTEST_USER(zpp_thread, test_round_robin) {
  // Create two threads with below normal priority
  zpp_lib::Thread thread1(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal,
                          "Thread1");
  zpp_lib::Thread thread2(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal,
                          "Thread2");

  // Start the two threads
  static volatile uint64_t counter1 = 0;
  static volatile uint64_t counter2 = 0;
  static volatile bool stop         = false;

  auto res = thread1.start(std::bind(thread_fn, &counter1, &stop));
  if (!res) {
    zassert_true(res, "Cannot start thread1: %d", res.error());
  }
  res = thread2.start(std::bind(thread_fn, &counter2, &stop));
  if (!res) {
    zassert_true(res, "Cannot start thread2: %d", res.error());
  }

  // Make sure that it has a higher priority than the other threads
  zpp_lib::ThisThread::setPriority(
      zpp_lib::PreemptableThreadPriority::PriorityAboveNormal);
  LOG_DBG("Main thread priority is %d",
          zpp_lib::preemptable_thread_priority_to_zephyr_prio(
              zpp_lib::ThisThread::getPriority()));

  // Have the main thread for the duration of two slices
  using namespace std::literals;
  // determine the number of time slices to wait before checking for counters (must be
  // even)
  static constexpr uint8_t NbrOfTimeSlicesToWait = 100;
  static std::chrono::milliseconds sleepDuration(NbrOfTimeSlicesToWait *
                                                 CONFIG_TIMESLICE_SIZE);
  LOG_DBG("Main thread waiting for %lld msecs", sleepDuration.count());
  zpp_lib::ThisThread::sleep_for(sleepDuration);

  LOG_DBG("Main thread stops waiting");

  // tell the two threads to stop
  stop = true;

  // TESTPOINT: check that both thread1 and thread2 got approximately the same CPU
  // resources
  static constexpr uint64_t delta = 20000;
  uint64_t diff = counter1 > counter2 ? counter1 - counter2 : counter2 - counter1;

  LOG_DBG("Waiting for threads");
  res = thread1.join();
  if (!res) {
    zassert_true(res, "Cannot join thread1: %d", res.error());
  }
  res = thread2.join();
  if (!res) {
    zassert_true(res, "Cannot join thread2: %d", res.error());
  }
  zassert_true(diff < delta,
               "Time slicing looks uneven, diff = %lld, delta = %lld, counter1 %lld, "
               "counter2 %lld",
               diff,
               delta,
               counter1,
               counter2);
}

ZTEST_SUITE(zpp_thread, NULL, NULL, NULL, NULL, NULL);
