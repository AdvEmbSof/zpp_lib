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
 * @file func_ptr_helper.hpp
 * @author Serge Ayer <serge.ayer@hefr.ch>
 *
 * @brief CPP wrapper for getting a function pointer from a CPP instance method
 *        to pass to zephyr os as a callback
 *
 * @date 2025-08-31
 * @version 1.0.0
 ***************************************************************************/
#pragma once

namespace zpp_lib {

template <const size_t UniqueId, typename Res, typename... ArgTypes>
class FuncPtrHelper {
 public:
  typedef std::function<Res(ArgTypes...)> FunctionType;

  static void bind(FunctionType&& f) { instance()._fn.swap(f); }

  static void bind(const FunctionType& f) { instance()._fn = f; }

  static Res invoke(ArgTypes... args) { return instance()._fn(args...); }

  typedef decltype(&FuncPtrHelper::invoke) PointerType;
  static PointerType ptr() { return &invoke; }

 private:
  static FuncPtrHelper& instance() {
    static FuncPtrHelper instance;
    return instance;
  }

  FunctionType _fn;
};

template <const size_t UniqueId, typename Res, typename... ArgTypes>
typename FuncPtrHelper<UniqueId, Res, ArgTypes...>::PointerType getFuncPtr(
    const typename FuncPtrHelper<UniqueId, Res, ArgTypes...>::FunctionType& f) {
  FuncPtrHelper<UniqueId, Res, ArgTypes...>::bind(f);
  return FuncPtrHelper<UniqueId, Res, ArgTypes...>::ptr();
}

}  // namespace zpp_lib
