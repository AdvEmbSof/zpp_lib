#pragma once

// zephyr
#include <zephyr/kernel.h>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

// stl 
#include <functional>

namespace zpp_lib {

// forward declaration for friendship
class WorkQueue;

template <typename F>
class Work : private NonCopyable<Work<F>>
{
public:
  Work(F f) {
    _workInfo._workFunction = f;
    k_work_init(&_workInfo._work, &Work::_thunk);
  }

private:
  static void _thunk(struct k_work *item) {
    // this ugly casting is the simplest way of getting the information
    // we need in the _thunk method
    // CASTING IS POSSIBLE ONLY WHEN k_work IS THE FIRST ATTRIBUTE
    // IN THE CLASS (here first attribute of WorkInfo that is the unique attribute)    
    Work* pWork = (Work*) item;
    pWork->_workInfo._workFunction();
  }
  friend WorkQueue;
  struct WorkInfo {
    // _work must be the first attribute of the unique class attribute
    struct k_work _work;
    F _workFunction;
  };
  WorkInfo _workInfo;  
};

} // namespace zpp_lib