#pragma once

// zephyr
#include <zephyr/kernel.h>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"
#include "zpp_include/work.hpp"

// std
#include <string>

namespace zpp_lib {
  
class WorkQueue : private NonCopyable<WorkQueue>
{
public:
  WorkQueue(const char *name);

  void run();
  [[nodiscard]] ZephyrResult call(Work& work);

private:
  struct k_work_q _workQueue;
  std::string _name;
};

} // namespace zpp_lib