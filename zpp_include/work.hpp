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
#include <type_traits>
#include <utility>

// zpp_lib
#include "zpp_include/non_copyable.hpp"
#include "zpp_include/zephyr_result.hpp"

namespace zpp_lib {

template <typename Obj, typename... Args> class Work final {
public:
  using Method = void (Obj::*)(Args...);
  explicit Work(Obj* obj, Method f, Args... args) noexcept
      : _work(), _obj(obj), _work_method(f), _args(std::make_tuple(std::forward<Args>(args)...)) {
    k_work_init(&_work, &Work::s_thunk);
  }

  ~Work() = default;

  // allow to modify the params
  void set_params(Args... args) {
    // params should not be modified when the work is pending
    // we silently reject the new args, as if the UI (button) would be greyed out
    if (k_work_is_pending(&_work)) {
      return;
    }
    _args = std::make_tuple(std::forward<Args>(args)...);
  }

  [[nodiscard]] struct k_work* native_handle() noexcept {
    return &_work;
  }

  // a Work instance is not copyable, neither movable
  Work& operator=(Work&& other) = delete;
  Work(const Work&)             = delete;
  Work& operator=(const Work&)  = delete;
  Work(Work&& other)            = delete;

private:
  static void s_thunk(struct k_work* item) {
    // this ugly casting is the simplest way of getting the information
    // we need in the _thunk method
    // CASTING IS POSSIBLE ONLY WHEN k_work IS THE FIRST ATTRIBUTE
    // IN THE CLASS
    // static_cast<uint32_t*> is not accepted here, reinterpret_cast is not supported
    // NOLINTNEXTLINE(modernize-avoid-c-style-cast)
    Work* p_work = (Work*)item;  // NOLINT(readability/casting)
    std::apply([&](auto&&... params) { std::invoke(p_work->_work_method, p_work->_obj, std::forward<decltype(params)>(params)...); },
               p_work->_args);
  }

  // _work must stay first so the Zephyr callback can recover the enclosing Work.
  struct k_work _work;
  Obj* _obj;
  Method _work_method;
  std::tuple<Args...> _args;
};

}  // namespace zpp_lib
