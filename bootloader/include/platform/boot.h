/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PLATFORM_BOOT_H__
#define __PLATFORM_BOOT_H__

#define __STATIC_FORCEINLINE \
    __attribute__((always_inline)) static inline

__STATIC_FORCEINLINE void __ISB(void)
{
    __asm volatile ("isb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __DSB(void)
{
    __asm volatile ("dsb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __DMB(void)
{
    __asm volatile ("dmb 0xF":::"memory");
}

__STATIC_FORCEINLINE void __set_MSP(uint32_t topOfMainStack)
{
    __asm volatile ("MSR msp, %0" : : "r" (topOfMainStack) : );
}

__STATIC_FORCEINLINE void __set_MSPLIM(uint32_t value)
{
    __asm volatile("MSR       MSPLIM,%0" :: "r" (value));
}

struct boot_arm_vector_table {
    uint32_t msp;
    uint32_t reset;
};

extern char __stack_start__[];
extern void boot_platform_quit(struct boot_arm_vector_table *vt);

#endif /* __PLATFORM_BOOT_H__ */
