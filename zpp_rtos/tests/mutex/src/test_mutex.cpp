#include <zephyr/ztest.h>

// zpp_rtos
#include "zpp_include/mutex.hpp"
#include "zpp_include/thread.hpp"

// test cases
ZTEST_USER(zpp_mutex, test_mutex_lock_unlock)
{
  zpp_lib::Mutex mutex;

	// TESTPOINT: try to lock and unlock mutex several times
	auto boolRet = mutex.try_lock();
	zassert_true(! boolRet.has_error());
	zassert_true(boolRet);
	zassert_true(mutex.unlock());
	boolRet = mutex.try_lock();
	zassert_true(! boolRet.has_error());
	zassert_true(boolRet);
	zassert_true(mutex.unlock());

  // TESTPOINT: try to lock and unlock mutex in another thread
  zpp_lib::Thread thread;	
  auto ret = thread.start([&mutex] () { 
		auto boolRet = mutex.try_lock();
		zassert_true(! boolRet.has_error());
		zassert_true(boolRet);
		zassert_true(mutex.unlock());
	});
	zassert_true(ret);
	zassert_true(thread.join());
}

ZTEST_SUITE(zpp_mutex, NULL, NULL, NULL, NULL, NULL);