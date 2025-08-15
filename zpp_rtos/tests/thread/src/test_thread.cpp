#include <zephyr/ztest.h>

// zpp_rtos
#include "zpp_include/semaphore.hpp"
#include "zpp_include/thread.hpp"
#include "zpp_include/time.hpp"
#include "zpp_include/this_thread.hpp"

#include <functional>

ZTEST(zpp_thread, test_native_sleep)
{
    printk("%lld %lld\n", k_uptime_get(), k_uptime_ticks());
    printk("%d\n", k_msleep(10 * 1000));
    printk("%lld %lld\n", k_uptime_get(), k_uptime_ticks());
}

void thread_fn(volatile uint64_t* counter, volatile bool* stop)
{
  while (! *stop) {
    (*counter)++;
  }
}

ZTEST_USER(zpp_thread, test_sleep)
{
  using namespace std::literals;
  static std::chrono::milliseconds waitDuration = 1ms;
  
  // TESTPOINT validate that busy wait works properly
  static constexpr uint8_t kNbrOfDurations = 10;
  for (uint8_t i = 0; i < kNbrOfDurations; i++) {
    std::chrono::microseconds currentTime = zpp_lib::Time::getUpTime();
    zpp_lib::ThisThread::busyWait(waitDuration);
    std::chrono::microseconds afterWaitTime = zpp_lib::Time::getUpTime();

    std::chrono::microseconds deltaTime = (afterWaitTime - currentTime) - waitDuration;
    static constexpr uint64_t allowedDeltaInUs = 1000;
  
    zassert_true(abs(deltaTime.count()) < allowedDeltaInUs, 
                 "(BUSY WAIT) iteration %d: Elapsed time is not within expected range, delta %lld, allowed = %lld", 
                 i, deltaTime.count(), allowedDeltaInUs);

    // double wait duration
    // waitDuration *= 2;
  }

  // TESTPOINT validate that sleep works properly
  static std::chrono::milliseconds sleepDuration = 1000ms;
  for (uint8_t i = 0; i < kNbrOfDurations; i++) {
    std::chrono::microseconds currentTime = zpp_lib::Time::getUpTime();
    zpp_lib::ThisThread::sleep_for(sleepDuration);
    std::chrono::microseconds afterSleepTime = zpp_lib::Time::getUpTime();

    std::chrono::microseconds deltaTime = (afterSleepTime - currentTime) - sleepDuration;
    static constexpr uint64_t allowedDeltaInUs = 200;
  
    zassert_true(abs(deltaTime.count()) < allowedDeltaInUs, 
                 "(SLEEP) iteration %d: Elapsed time is not within expected range, delta %lld, allowed = %lld", 
                 i, deltaTime.count(), allowedDeltaInUs);

    // double sleep duration
    // sleepDuration *= 2;
  }
}

/*ZTEST_USER(zpp_thread, test_round_robin)
{
  // Create two threads with below normal priority
  zpp_lib::Thread thread1(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal, "Thread1");
  zpp_lib::Thread thread2(zpp_lib::PreemptableThreadPriority::PriorityBelowNormal, "Thread2");
 
  // Start the two threads
  static volatile uint64_t counter1 = 0;
  static volatile uint64_t counter2 = 0;  
  static volatile bool stop = false;

  auto ret = thread1.start(std::bind(thread_fn, &counter1, &stop));
	zassert_true(ret);
  ret = thread2.start(std::bind(thread_fn, &counter2, &stop));
	zassert_true(ret);

  // Make sure that it has a higher priority than the other threads
  zpp_lib::ThisThread::setPriority(zpp_lib::PreemptableThreadPriority::PriorityAboveNormal);
  printk("Main thread priority is %d\n", zpp_lib::preemptable_thread_priority_to_zephyr_prio(zpp_lib::ThisThread::getPriority()));
 
  // Have the main thread for the duration of two slices
  using namespace std::literals;
  // determine the number of time slices to wait before checking for counters (must be even)
  static constexpr uint8_t NbrOfTimeSlicesToWait = 100;
  static std::chrono::milliseconds sleepDuration(NbrOfTimeSlicesToWait * CONFIG_TIMESLICE_SIZE); 
  printk("Main thread waiting for %lld msecs\n", sleepDuration.count());
  zpp_lib::ThisThread::sleep_for(sleepDuration);

  printk("Main thread stops waiting\n");
  
  // tell the two threads to stop
  stop = true;
  
	// TESTPOINT: check that both thread1 and thread2 got approximately the same CPU resources
  static constexpr uint64_t delta = 4000;
  uint64_t diff = counter1 > counter2 ? counter1 - counter2 : counter2 - counter1;
  zassert_true(diff < delta, "Time slicing looks uneven, diff = %lld, delta = %lld", diff, delta);

  printk("Waiting for threads\n");
  ret = thread1.join();
	zassert_true(ret);
  ret = thread2.join();
	zassert_true(ret);
}
*/
static void *zpp_thread_tests_setup(void)
{
#ifdef CONFIG_USERSPACE
	//k_thread_access_grant(k_current_get(), &tdata, &tstack, &tdata2,
	//			&tstack2, &tdata3, &tstack3, &kmutex,
	//			&tmutex);
#endif
	return NULL;
}

ZTEST_SUITE(zpp_thread, NULL, zpp_thread_tests_setup, NULL, NULL, NULL);

