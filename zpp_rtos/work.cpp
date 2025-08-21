#include "zpp_include/work.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

//#ifndef CONFIG_USERSPACE
//#error This sample requires CONFIG_USERSPACE.
//#endif

#ifndef CONFIG_DYNAMIC_THREAD_ALLOC
//#error This sample requires CONFIG_DYNAMIC_THREAD_ALLOC.
#endif

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

Work::Work() {}

void Work::initialize(std::function<void()> workFunction) {
  _workInfo._workFunction = workFunction;
  // this ugly casting is the simplest way of getting the information
  // we need in the _thunk method
  k_work_init((struct k_work*) this, &Work::_thunk);
}

void Work::_thunk(struct k_work *item) {
  Work* pWork = (Work*) item;
  pWork->_workInfo._workFunction();
}

} // namespace zpp_lib