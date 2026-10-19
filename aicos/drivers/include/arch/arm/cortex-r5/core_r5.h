/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines operation interface of cortex r5.
 */

#ifndef __CORE_R5_H__
#define __CORE_R5_H__

/* START of CORTEX-R Definitions */
/* cr5 cpsr fiq,irq control bits */
#define CPSR_IF_MASK                 0xC0

/*
 * Arm Compiler 4/5
 */
#if   defined ( __CC_ARM )

#ifndef   __ASM
    #define __ASM                                  __asm
#endif
#ifndef   __INLINE
    #define __INLINE                               __inline
#endif
#ifndef   __STATIC_INLINE
    #define __STATIC_INLINE                        static __inline
#endif
#ifndef   __STATIC_FORCEINLINE
    #define __STATIC_FORCEINLINE                   static __forceinline
#endif

__STATIC_INLINE uint32_t get_cpsr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrs val, cpsr");
    return val;
}

__STATIC_INLINE void set_cpsr(uint32_t v)
{
    __ASM __volatile__("msr cpsr_c, v");
}

__STATIC_INLINE uint32_t get_sctlr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrc p15, 0, val, c1, c0, 0");
    return val;
}

__STATIC_INLINE void set_sctlr(uint32_t v)
{
    __ASM __volatile__("mcr p15, 0, v, c1, c0, 0");
}

__STATIC_INLINE uint32_t get_spsr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrs val, spsr");
    return val;
}

__STATIC_INLINE void disable_ints(void)
{
    /* Disable IRQ and FIQ */
    __ASM __volatile__("cpsid if");
}

__STATIC_INLINE void enable_ints(void)
{
    /* Enable IRQ and FIQ */
    __ASM __volatile__("cpsie if");
}

/**
  \brief   Instruction Synchronization Barrier
  \details Instruction Synchronization Barrier flushes the pipeline in the processor,
           so that all instructions following the ISB are fetched from cache or memory,
           after the instruction has been completed.
 */
#define __ISB()                           __isb(0xF)

/**
  \brief   Data Synchronization Barrier
  \details Acts as a special kind of Data Memory Barrier.
           It completes when all explicit memory accesses before this instruction complete.
 */
#define __DSB()                           __dsb(0xF)

/**
  \brief   Data Memory Barrier
  \details Ensures the apparent order of the explicit memory operations before
           and after the instruction, without ensuring their completion.
 */
#define __DMB()                           __dmb(0xF)

/**
  \brief   Count leading zeros
  \details Counts the number of leading zeros of a data value.
  \param [in]  value  Value to count the leading zeros
  \return             number of leading zeros in value
 */
#define __CLZ                             __clz

/*
 * Arm Compiler above 6.10.1 (armclang)
 */
#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6100100)

#ifndef   __ASM
    #define __ASM                                  __asm
#endif
#ifndef   __INLINE
    #define __INLINE                               __inline
#endif
#ifndef   __STATIC_INLINE
    #define __STATIC_INLINE                        static __inline
#endif
#ifndef   __STATIC_FORCEINLINE
    #define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static __inline
#endif

__STATIC_INLINE uint32_t get_cpsr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrs %0, cpsr":"=r"(val));

    return val;
}

__STATIC_INLINE void set_cpsr(uint32_t v)
{
    __ASM __volatile__("msr cpsr_c, %0"::"r"(v):"cc");
}

__STATIC_INLINE uint32_t get_sctlr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrc p15, 0, %0, c1, c0, 0":"=r" (val));

    return val;
}

__STATIC_INLINE void set_sctlr(uint32_t v)
{
    __ASM __volatile__("mcr p15, 0, %0, c1, c0, 0"::"r" (v));
}

__STATIC_INLINE uint32_t get_spsr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrs %0, spsr":"=r"(val));

    return val;
}

__STATIC_INLINE void disable_ints(void)
{
    __ASM __volatile__("cpsid if":::"memory");
}

__STATIC_INLINE void enable_ints(void)
{
    __ASM __volatile__("cpsie if":::"memory");
}

/**
  \brief   Instruction Synchronization Barrier
  \details Instruction Synchronization Barrier flushes the pipeline in the processor,
           so that all instructions following the ISB are fetched from cache or memory,
           after the instruction has been completed.
 */
#define __ISB()        __builtin_arm_isb(0xF)

/**
  \brief   Data Synchronization Barrier
  \details Acts as a special kind of Data Memory Barrier.
           It completes when all explicit memory accesses before this instruction complete.
 */
#define __DSB()        __builtin_arm_dsb(0xF)


/**
  \brief   Data Memory Barrier
  \details Ensures the apparent order of the explicit memory operations before
           and after the instruction, without ensuring their completion.
 */
#define __DMB()        __builtin_arm_dmb(0xF)

/**
  \brief   Count leading zeros
  \details Counts the number of leading zeros of a data value.
  \param [in]  value  Value to count the leading zeros
  \return             number of leading zeros in value
 */
__STATIC_FORCEINLINE uint8_t __CLZ(uint32_t value)
{
  /* Even though __builtin_clz produces a CLZ instruction on ARM, formally
     __builtin_clz(0) is undefined behaviour, so handle this case specially.
     This guarantees ARM-compatible results if happening to compile on a non-ARM
     target, and ensures the compiler doesn't decide to activate any
     optimisations using the logic "value was passed to __builtin_clz, so it
     is non-zero".
     ARM GCC 7.3 and possibly earlier will optimise this test away, leaving a
     single CLZ instruction.
   */
  if (value == 0U)
  {
    return 32U;
  }
  return __builtin_clz(value);
}

/*
 * GNU Compiler
 */
#elif defined ( __GNUC__ )

#ifndef   __ASM
    #define __ASM                                  __asm
#endif
#ifndef   __INLINE
    #define __INLINE                               inline
#endif
#ifndef   __STATIC_INLINE
    #define __STATIC_INLINE                        static inline
#endif
#ifndef   __STATIC_FORCEINLINE
    #define __STATIC_FORCEINLINE                   __attribute__((always_inline)) static inline
#endif
__STATIC_INLINE uint32_t get_cpsr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrs %0, cpsr":"=r"(val));

    return val;
}

__STATIC_INLINE void set_cpsr(uint32_t v)
{
    __ASM __volatile__("msr cpsr_c, %0"::"r"(v):"cc");
}

__STATIC_INLINE uint32_t get_sctlr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrc p15, 0, %0, c1, c0, 0":"=r" (val));

    return val;
}

__STATIC_INLINE void set_sctlr(uint32_t v)
{
    __ASM __volatile__("mcr p15, 0, %0, c1, c0, 0"::"r" (v));
}

__STATIC_INLINE uint32_t get_spsr(void)
{
    uint32_t val = 0;

    __ASM __volatile__("mrs %0, spsr":"=r"(val));

    return val;
}

__STATIC_INLINE void disable_ints(void)
{
    __ASM __volatile__("cpsid i":::"memory");
}

__STATIC_INLINE void enable_ints(void)
{
    __ASM __volatile__("cpsie i":::"memory");
}

/**
  \brief   Instruction Synchronization Barrier
  \details Instruction Synchronization Barrier flushes the pipeline in the processor,
           so that all instructions following the ISB are fetched from cache or memory,
           after the instruction has been completed.
 */
__STATIC_FORCEINLINE void __ISB(void)
{
  __ASM volatile ("isb 0xF":::"memory");
}


/**
  \brief   Data Synchronization Barrier
  \details Acts as a special kind of Data Memory Barrier.
           It completes when all explicit memory accesses before this instruction complete.
 */
__STATIC_FORCEINLINE void __DSB(void)
{
  __ASM volatile ("dsb 0xF":::"memory");
}


/**
  \brief   Data Memory Barrier
  \details Ensures the apparent order of the explicit memory operations before
           and after the instruction, without ensuring their completion.
 */
__STATIC_FORCEINLINE void __DMB(void)
{
  __ASM volatile ("dmb 0xF":::"memory");
}

/**
  \brief   Count leading zeros
  \details Counts the number of leading zeros of a data value.
  \param [in]  value  Value to count the leading zeros
  \return             number of leading zeros in value
 */
__STATIC_FORCEINLINE uint8_t __CLZ(uint32_t value)
{
  /* Even though __builtin_clz produces a CLZ instruction on ARM, formally
     __builtin_clz(0) is undefined behaviour, so handle this case specially.
     This guarantees ARM-compatible results if happening to compile on a non-ARM
     target, and ensures the compiler doesn't decide to activate any
     optimisations using the logic "value was passed to __builtin_clz, so it
     is non-zero".
     ARM Compiler 6.10 and possibly earlier will optimise this test away, leaving a
     single CLZ instruction.
   */
  if (value == 0U)
  {
    return 32U;
  }
  return __builtin_clz(value);
}

#else
  #error Unknown compiler.
#endif

__STATIC_FORCEINLINE unsigned int disable_interrupts(void)
{
    unsigned int int_posture;

    int_posture = get_cpsr();

    disable_ints();

    return(int_posture);
}

__STATIC_FORCEINLINE void restore_interrupts(unsigned int int_posture)
{
    set_cpsr(int_posture);
}

#endif /* __CORE_R5_H__ */
