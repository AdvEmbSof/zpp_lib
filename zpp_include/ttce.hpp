#pragma once

// zephyr
#include <zephyr/kernel.h>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"
#include "zpp_include/clock.hpp"

// std
#include <string>
#include <chrono>

namespace zpp_lib {

template <typename F, uint16_t NbrOfMinorCycles, uint16_t MaxMinorCycleSize>
class TTCE : private NonCopyable<TTCE<F, NbrOfMinorCycles, MaxMinorCycleSize>>
{
public:
  TTCE(std::chrono::milliseconds minorCycle) : _minorCycle(minorCycle) {
    k_timer_init(&_timer, &TTCE::_thunk, nullptr);
    // specify this instance as user data
    _timer.user_data = (void*) this;
    k_work_init(&_work, &TTCE::_workHandler);
    // initialize the work queue
    k_work_queue_init(&_workQueue);
  }
  
  void start() {
    // first start the timer
    k_timeout_t period = milliseconds_to_ticks(_minorCycle);
    k_timer_start(&_timer, period, period);

    // then run the work queue
    struct k_work_queue_config cfg = {
		  .name = "TTCE Work Queue",
		  .no_yield = true,
	  };
 	  k_work_queue_run(&_workQueue, &cfg);
  }

  [[nodiscard]] ZephyrResult addTask(uint16_t minorCycleIndex, F f) {
    ZephyrResult res;    
    if (minorCycleIndex >= NbrOfMinorCycles) {
      __ASSERT(false, "Invalid minor cycle index %d", minorCycleIndex);
      res.assign_error(ZephyrErrorCode::k_inval);
      return res;
    }
    if (_nbrOfTasksInMinorCycle[minorCycleIndex] >= MaxMinorCycleSize) {
      __ASSERT(false, "Too many tasks in minor cycle %d: %d", minorCycleIndex, _nbrOfTasksInMinorCycle[minorCycleIndex] + 1);
      res.assign_error(ZephyrErrorCode::k_inval);
      return res;
    }

    _tasks[minorCycleIndex][_nbrOfTasksInMinorCycle[minorCycleIndex]++] = f;

    return res;
  }

private:
  static void _thunk(struct k_timer *timer_id) {    
    // submit the periodic TTCE task
    if (timer_id != nullptr) {
      // get instance from user data
      TTCE* pTTCE = (TTCE*) timer_id->user_data;
      auto ret = k_work_submit_to_queue(&pTTCE->_workQueue, &pTTCE->_work);
      if (ret != 0 && ret != 1 && ret != 2) {
        __ASSERT(false, "Failed to submit work: %d", ret);
        return;
      }      
    }
  }

  static void _workHandler(struct k_work *item) {
    // this ugly casting is the simplest way of getting the information
    // we need in the _workHandler method
    // CASTING IS POSSIBLE ONLY WHEN k_work IS THE FIRST ATTRIBUTE IN THE CLASS
    TTCE* pTTCE = (TTCE*) item;

    // execute tasks based on schedule table
    for (uint16_t taskIndex = 0; taskIndex < MaxMinorCycleSize; taskIndex++) {
      if (pTTCE->_tasks[pTTCE->_minorCycleIndex][taskIndex] != nullptr) {
        pTTCE->_tasks[pTTCE->_minorCycleIndex][taskIndex]();
      }
    }
    pTTCE->_minorCycleIndex = (pTTCE->_minorCycleIndex + 1) % NbrOfMinorCycles;
  }

  // _work MUST be the first attribute
  struct k_work _work;
  struct k_work_q _workQueue;
  struct k_timer _timer;
  std::chrono::milliseconds _minorCycle;
  uint16_t _minorCycleIndex = 0;
  F _tasks[NbrOfMinorCycles][MaxMinorCycleSize] = {nullptr};
  uint16_t _nbrOfTasksInMinorCycle[NbrOfMinorCycles] = {0};
};

} // namespace zpp_lib