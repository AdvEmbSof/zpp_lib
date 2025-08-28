#include <zephyr/ztest.h>

// zpp_rtos
#include "zpp_include/semaphore.hpp"
#include "zpp_include/thread.hpp"

// stl
#include <memory>

// test cases
ZTEST_USER(zpp_semaphore, test_semaphore_release_acquire)
{	
	static constexpr uint32_t initial_count = 0U;
	static constexpr uint32_t max_count = 1U;
  zpp_lib::Semaphore sem(initial_count, max_count);

	// TESTPOINT: try to release and acquire semaphore several times
  zassert_true(sem.release());
	zassert_true(sem.acquire());
  zassert_true(sem.release());
	zassert_true(sem.acquire());

	// TESTPOINT: try to acquire one more time (ret should evaluate to false without error)
	auto boolRet = sem.try_acquire();
	zassert_true(! boolRet.has_error());
	zassert_true(! boolRet);

	// TESTPOINT: try to acquire semaphore that is released in another thread
  zpp_lib::Thread thread;
	auto ret = thread.start([&sem]() { auto ret = sem.release(); zassert_true(ret); });
	zassert_true(ret);
	zassert_true(thread.join());
	zassert_true(sem.acquire());  
}

ZTEST_SUITE(zpp_semaphore, NULL, NULL, NULL, NULL, NULL);
