#pragma once

// zephyr
#include <zephyr/kernel.h>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"
#include "zpp_include/work.hpp"

// std
#include <string>
#include <chrono>

namespace zpp_lib {
  
class WorkQueue : private NonCopyable<WorkQueue>
{
public:
  WorkQueue(const char *name) {
    _name = name;
    k_work_queue_init(&_workQueue);
  }

  void run() {
    struct k_work_queue_config cfg = {
		  .name = _name.c_str(),
		  .no_yield = true,
	  };

 	  k_work_queue_run(&_workQueue, &cfg);
  }
  
  template <typename F>
  [[nodiscard]] ZephyrResult call(Work<F>& work) {
    ZephyrResult res;
    // Non error return values are documented as follows:
    // @retval 0 if work was already submitted to a queue
    // @retval 1 if work was not submitted and has been queued to @p queue
    // @retval 2 if work was running and has been queued to the queue that was running it
    auto ret = k_work_submit_to_queue(&_workQueue, &work._workInfo._work);
    if (ret != 0 && ret != 1 && ret != 2) {
      __ASSERT(false, "Failed to submit work: %d", ret);
      res.assign_error(zephyr_to_zpp_error_code(ret));
      return res;
    }
    return res;
  }

private:
  struct k_work_q _workQueue;
  std::string _name;
};

} // namespace zpp_lib