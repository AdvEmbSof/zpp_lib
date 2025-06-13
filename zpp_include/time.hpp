#pragma once

// zephyr
#include <zephyr/kernel.h>

// std
#include <chrono>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

class Time : private NonCopyable<Time> {
public:
   static std::chrono::microseconds getUpTime();
};

} // namespace zpp_lib