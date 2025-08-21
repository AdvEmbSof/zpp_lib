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

class Work : private NonCopyable<Work>
{
public:
  Work();

  void initialize(std::function<void()> workFunction);
  
private:
  static void _thunk(struct k_work *item);
  friend WorkQueue;  
  struct WorkInfo {
    struct k_work _work;
    std::function<void()> _workFunction;
  } _workInfo;  
};

} // namespace zpp_lib