#include "zpp_include/work_queue.hpp"

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

WorkQueue::WorkQueue(const char* name) {
  _name = name;
}

void WorkQueue::run() {
  struct k_work_queue_config cfg = {
		.name = _name.c_str(),
		.no_yield = true,
	};

 	k_work_queue_run(&_workQueue, &cfg);
}

ZephyrResult WorkQueue::call(Work& work) {
  ZephyrResult res;
  // Non error return values are documented as follows:
  // @retval 0 if work was already submitted to a queue
  // @retval 1 if work was not submitted and has been queued to @p queue
  // @retval 2 if work was running and has been queued to the queue that was running it
  auto ret = k_work_submit(&work._workInfo._work);
  if (ret != 0 && ret != 1 && ret != 2) {
    LOG_ERR("Failed to submit work: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
    return res;
  }
  return res;
}

} // namespace zpp_lib