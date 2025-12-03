// Copyright 2025 Haute école d'ingénierie et d'architecture de Fribourg
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
 * @file work.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP class declaration/implementation wrapping zephyr OS work
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/

#pragma once

// zephyr
#include <zephyr/kernel.h>

// stl
#include <functional>
#include <tuple>
#include <utility>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

// forward declaration for friendship
class WorkQueue;

template <typename Obj, typename... Args>
class Work {
 public:
  using Method = void (Obj::*)(Args...);
  explicit Work(Obj* obj, Method f, Args... args) noexcept {
    _workInfo._obj        = obj;
    _workInfo._workMethod = f;
    _workInfo._args       = std::make_tuple(std::forward<Args>(args)...);
    k_work_init(&_workInfo._work, &Work::_thunk);
  }

  // allow to modify the params
  void setParams(Args... args) {
    // params should not be modified when the work is pending
    // we silently reject the new args, as if the UI (button) would be greyed out
    if (k_work_is_pending(&_workInfo._work)) {
      return;
    }
    _workInfo._args = std::make_tuple(std::forward<Args>(args)...);
  }

  // a Work instance is not copyable, neither movable
  Work& operator=(Work&& other) = delete;
  Work(const Work&)             = delete;
  Work& operator=(const Work&)  = delete;
  Work(Work&& other)            = delete;

 private:
  static void _thunk(struct k_work* item) {
    // this ugly casting is the simplest way of getting the information
    // we need in the _thunk method
    // CASTING IS POSSIBLE ONLY WHEN k_work IS THE FIRST ATTRIBUTE
    // IN THE CLASS (here first attribute of WorkInfo that is the unique attribute)
    // static_cast<uint32_t*> is not accepted here, reinterpret_cast is not supported
    // cppcheck-suppress dangerousTypeCast
    Work* pWork        = (Work*)(item);  // NOLINT(readability/casting)
    WorkInfo& workInfo = pWork->_workInfo;
    std::apply(
        [&](auto&&... params) { (workInfo._obj->*workInfo._workMethod)(params...); },
        workInfo._args);
  }
  friend WorkQueue;
  struct WorkInfo {
    // _work must be the first attribute of the unique class attribute
    struct k_work _work;
    Obj* _obj;
    Method _workMethod;
    std::tuple<Args...> _args;
  };
  WorkInfo _workInfo;
};

}  // namespace zpp_lib
