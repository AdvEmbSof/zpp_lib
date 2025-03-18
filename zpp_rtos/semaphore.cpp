#include "zpp_include/semaphore.hpp"

// Zephyr sdk
#include <zephyr/logging/log.h>

// zpp_lib
#include "zpp_include/clock.hpp"

LOG_MODULE_DECLARE(zpp_rtos, CONFIG_ZPP_RTOS_LOG_LEVEL);

namespace zpp_lib {

Semaphore::Semaphore(uint32_t initial_count, uint32_t max_count) noexcept {
  //__ASSERT(K_SEM_DEFINE(_sem_obj, 0U, 1U) == 0, "Cannot create semaphore");
  auto ret = k_sem_init(&_sem_obj, initial_count, max_count);
  __ASSERT(ret == 0, "Cannot create semaphore");
  LOG_DBG("Semaphore created with count %d (vs %d) and max count %d", k_sem_count_get(&_sem_obj), initial_count, max_count);
}

ZephyrResult Semaphore::acquire() {
  LOG_DBG("Acquiring semaphore with count %d", k_sem_count_get(&_sem_obj));
  ZephyrResult res;
  int ret = k_sem_take(&_sem_obj, K_FOREVER);
  if (ret != 0) {
    LOG_ERR("Cannot acquire semaphore: %d", ret);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;
}

ZephyrBoolResult Semaphore::try_acquire() {
  ZephyrBoolResult res;
  int ret = k_sem_take(&_sem_obj, K_NO_WAIT);
  if (ret == -EBUSY) {
    // timeout -> return false without error
    res.assign_value(false);
  }
  else if (ret != 0) {
    // other failure -> return false with error
    LOG_ERR("Cannot acquire semaphore: %d", ret);
    res.assign_value(false);
    res.assign_error(zephyr_to_zpp_error_code(ret));
  }
  return res;

}

ZephyrResult Semaphore::release() {
  ZephyrResult res;
  k_sem_give(&_sem_obj);
  return res;
}

} // namespace zpp_lib