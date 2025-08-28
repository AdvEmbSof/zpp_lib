#include "zpp_include/mutex.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

// zpp_lib
#include "zpp_include/clock.hpp"

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

Mutex::Mutex() noexcept {
  k_mutex_init(&_mutex_obj);
}

Mutex::~Mutex() {

}

ZephyrResult Mutex::lock() {
  ZephyrResult res;
  int ret = k_mutex_lock(&_mutex_obj, K_FOREVER);
  if (ret != 0) {
    LOG_ERR("Cannot lock mutex: %d", ret);
    __ASSERT(false, "Cannot lock mutex: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

ZephyrBoolResult Mutex::try_lock() noexcept {
  return try_lock_for(std::chrono::milliseconds::zero());
}

ZephyrBoolResult Mutex::try_lock_for(const std::chrono::milliseconds& timeout) {
  LOG_DBG("Trying to lock mutex with timeout %lld ms (ticks %lld)", timeout.count(), milliseconds_to_ticks(timeout).ticks);
  auto ret = k_mutex_lock(&_mutex_obj, milliseconds_to_ticks(timeout));
  ZephyrBoolResult res;
  if (ret == -EAGAIN) {
    // timeout -> return false without error
    res.assign_value(false);
  }
  else if (ret != 0) {
    // other failure -> return false with error
    LOG_ERR("Cannot lock mutex: %d", ret);
    __ASSERT(false, "Cannot lock mutex: %d", ret);
    res.assign_value(false);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

ZephyrResult Mutex::unlock() {
  ZephyrResult res;
  int ret = k_mutex_unlock(&_mutex_obj);
  if (ret != 0) {
    LOG_ERR("Cannot unlock mutex: %d", ret);
    __ASSERT(false, "Cannot unlock mutex: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

}  // namespace zpp_lib