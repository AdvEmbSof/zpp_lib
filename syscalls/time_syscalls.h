#pragma once
#include <zephyr/kernel.h>

#include <stdint.h>

// Declare as a proper Zephyr syscall — generates the privilege elevation glue
__syscall uint32_t userspace_cycle_get_32(void);

#include <zephyr/syscalls/app_syscall.h>
