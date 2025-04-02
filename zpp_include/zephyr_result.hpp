#pragma once

// zephyr sdk
#include <zephyr/sys/__assert.h>

// zpp rtos
#include "zephyr_error_code.hpp"

namespace zpp_lib {

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Result class for checking whether a method returned an error or not
///
////////////////////////////////////////////////////////////////////////////////
class ZephyrResult {
public:
  ///
  /// @brief default initialization to ok (no error) state
  ///
  ZephyrResult() noexcept = default;

  ///
  /// @brief initialization to error state
  ///
  /// @param rhs the error value to assign
  ///
  ZephyrResult(const ZephyrResult& res) noexcept
    : _is_ok(false), 
      _error_value(res.error())
  {
  }

  ///
  /// @brief initialization to error state
  ///
  /// @param rhs the error value to assign
  ///
  explicit ZephyrResult(ZephyrErrorCode error) noexcept
    : _is_ok(false), 
      _error_value(error)
  {
  }

  ///
  /// @brief assign error state
  ///
  /// @param rhs the error value to assign
  ///
  void assign_error(ZephyrErrorCode error) noexcept {
    _is_ok = false;
    _error_value = error;
  }
  
  ///
  /// @brief assign error state
  ///
  /// @param rhs the error value to assign
  ///
  void assign_error(const ZephyrResult& res) noexcept {
    _is_ok = false;
    _error_value = res.error();
  }

  ///
  /// @brief convert the result to a bool
  ///
  /// @return true if no error was assigned
  ///
  constexpr operator bool() const noexcept {
    return _is_ok;
  }

  ///
  /// @brief return a reference to the error value
  ///
  /// @return reference to error value
  ///
  /// @warning when the result is in error state the the thread will terminate
  ///
  ZephyrErrorCode error() const {
    __ASSERT(!_is_ok, "Result is in ok state");
    return _error_value;
  }

private:
  bool _is_ok{true};
  ZephyrErrorCode _error_value;
};

////////////////////////////////////////////////////////////////////////////////
///
/// @brief Result class for checking the boolean return value of a method
//         and for checking whether a method returned an error or not
///
////////////////////////////////////////////////////////////////////////////////
class ZephyrBoolResult {
public:
  ///
  /// @brief default initialization to ok (no error) state
  ///
  ZephyrBoolResult() noexcept = default;

  ///
  /// @brief initialization to error state
  ///
  /// @param rhs the error value to assign
  ///
  ZephyrBoolResult(const ZephyrBoolResult& res) noexcept
    : _is_ok(false),
      _bool_result(false),
      _error_value(res.error())
  {
  }
  
  ///
  /// @brief initialization to error state
  ///
  /// @param rhs the error value to assign
  ///
  explicit ZephyrBoolResult(ZephyrErrorCode error) noexcept
    : _is_ok(false),
      _bool_result(false),
      _error_value(error)
  {
  }

  ///
  /// @brief assign return value
  ///
  /// @param rhs the return value to assign
  ///
  void assign_value(const bool& v) noexcept {
    _is_ok = true;
    _bool_result = v;
  }
  
  ///
  /// @brief assign error state
  ///
  /// @param rhs the error value to assign
  ///
  void assign_error(ZephyrErrorCode error) noexcept {
    _is_ok = false;
    _bool_result = false;
    _error_value = error;
  }

  ///
  /// @brief assign error state
  ///
  /// @param rhs the error value to assign
  ///
  void assign_error(const ZephyrBoolResult& res) noexcept {
    _is_ok = false;
    _bool_result = false;
    _error_value = res.error();
  }
 
  ///
  /// @brief convert the result to a bool
  ///
  /// @return true if no error was assigned
  ///
  constexpr operator bool() const noexcept {
    __ASSERT(!has_error(), "Result is in error state");
    return _bool_result;
  }

  ///
  /// @brief returns true in error state, false otherwise
  ///
  /// @return true if the result is valid
  ///
  constexpr bool has_error() const noexcept {
    return ! _is_ok;
  }

  ///
  /// @brief return a reference to the error value
  ///
  /// @return reference to error value
  ///
  /// @warning when the result is in error state the the thread will terminate
  ///
  ZephyrErrorCode error() const noexcept {
    __ASSERT(has_error(), "Result is in ok state");
    return _error_value;
  }
  
private:
  bool _is_ok{true};
  bool _bool_result{true};
  ZephyrErrorCode _error_value;
};

} // namespace zpp_lib