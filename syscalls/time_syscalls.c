#include <zephyr/internal/syscall_handler.h>

// Implementation runs in supervisor mode — safe to read hardware registers
uint32_t z_impl_userspace_cycle_get_32(void)
{
    return k_cycle_get_32();
}

// Verification function — no parameters to validate
static inline uint32_t z_vrfy_userspace_cycle_get_32(void)
{
    return z_impl_userspace_cycle_get_32();
}

#include <zephyr/syscalls/userspace_cycle_get_32_mrsh.c>

