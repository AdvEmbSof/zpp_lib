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

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

// forward declaration for friendship
class WorkQueue;

template <typename Obj, typename... Args>
class Work /*: private NonCopyable<Work<Obj, Args...>> */{
 public:
  using Method = void(Obj::*)(Args...);//std::function<void()>;
  explicit Work(Obj* obj, Method f, Args... args) noexcept {
    _workInfo._obj = obj;
    _workInfo._workMethod = f;
    _workInfo._args = std::make_tuple(std::forward<Args>(args)...);
    k_work_init(&_workInfo._work, &Work::_thunk);
  }
  
  // do NOT allow copy/assignment/move assignment
  Work(const Work&) = delete;
  Work& operator=(Work&& other) {
    // the work should NOT be submitted/running
    __ASSERT((_workInfo._work.flags & BIT(K_WORK_QUEUED_BIT | K_WORK_RUNNING_BIT)) == 0, "Work cannot be moved when queued or running");
    _workInfo._obj = other._workInfo._obj;
    _workInfo._workMethod = other._workInfo._workMethod;
    _workInfo._args = other._workInfo._args;//std::make_tuple(std::forward_as_tuple<Args>(other._workInfo._args)...);
    // Re-init the k_work for this instance (not strictly necessary?)
    k_work_init(&_workInfo._work, &Work::_thunk);

    return *this;
  }

  Work& operator=(const Work&) = delete;
  Work(Work&& other) = delete;
  // allow move copy
  // Work(Work&& other) noexcept {
  //   // the work should NOT be submitted/running
  //   __ASSERT(!flag_test(&_workInfo._work->flags, K_WORK_QUEUED_BIT | K_WORK_RUNNING_BIT), "Work cannot be moved when queued or running");
  //   _workInfo._obj = other._workInfo._obj;
  //   _workInfo._workMethod = other._workInfo._workMethod;
  //   _workInfo._args = std::make_tuple(std::forward<Args>(other._workInfo._args)...);
  //   // Re-init the k_work for this instance (not strictly necessary?)
  //   k_work_init(&_workInfo._work, &Work::_thunk);
  // }


 private:
  static void _thunk(struct k_work* item) {
    // this ugly casting is the simplest way of getting the information
    // we need in the _thunk method
    // CASTING IS POSSIBLE ONLY WHEN k_work IS THE FIRST ATTRIBUTE
    // IN THE CLASS (here first attribute of WorkInfo that is the unique attribute)
    // static_cast<uint32_t*> is not accepted here, reinterpret_cast is not supported
    // cppcheck-suppress dangerousTypeCast
    Work* pWork = (Work*)(item);  // NOLINT(readability/casting)
    WorkInfo& workInfo = pWork->_workInfo;
    std::apply([&](auto&&... params) {
      (workInfo._obj->*workInfo._workMethod)(params...);}, workInfo._args);
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
