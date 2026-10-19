/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines L1 cache operations interface for cortex-r5.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0391c5
#define LOG_MODULE    AMOD_DRV

#include <aic_type.h>
#include <aic_bitmap.h>
#include <aic_io.h>
#include <core_op.h>

#define L1C_LINE_SIZE                (0x20)
#define IC_CTRL_BIT                  (BIT_12)
#define DC_CTRL_BIT                  (BIT_2)

/*
 Note:Level 1 instruction and data cache line size is 32 bytes, say 8 words
 SCTLR
 bit2: l1 dcache enable bit12: l1 icache enable
 mcr p15, 0, r1, c1, c0, 0
 CSSELR
 mcr p15, 2, Rt, c0, c0, 0
 CCSIDR
 mrc p15, 1, Rt, c0, c0, 0

 mcr: write
 mcr p15, opc1, Rt, CRn, CRm, opc2
 mrc: read
 mrc p15, opc1, Rt, CRn, CRm, opc2

          CRn opc1 CRm opc2
 DCCIMVAC c7   0   c14  1
 DCCISW   c7   0   c14  2
 DCCMVAC  c7   0   c10  1
 DCCSW    c7   0   c10  2
 DCIMVAC  c7   0   c6   1
 DCISW    c7   0   c6   2
 ICIALLU  c7   0   c5   0
 ICIMVAU  c7   0   c5   1
 DCIALL   c15  0   c5   0
*/

#if defined(__CC_ARM) /* Arm compiler 5 */

#define dciall() __ASM __volatile__(\
        "mcr p15, 0, dciall, c15, c5, 0"\
        )
#define iciallu() __ASM __volatile__(\
        "mcr p15, 0, iciallu, c7, c5, 0"\
        )
#define icimvau(addr) __ASM __volatile__(\
        "mcr p15, 0, addr, c7, c5, 1"\
        )
#define dcisw(reg) __ASM __volatile__(\
        "mcr p15, 0, reg, c7, c6, 2"\
        )
#define dccsw(reg) __ASM __volatile__(\
        "mcr p15, 0, reg, c7, c10, 2"\
        )
#define dccisw(reg) __ASM __volatile__(\
        "mcr p15, 0, reg, c7, c14, 2"\
        )
#define dcimvac(addr) __ASM __volatile__(\
        "mcr p15, 0, addr, c7, c6, 1"\
        )
#define dccmvac(addr) __ASM __volatile__(\
        "mcr p15, 0, addr, c7, c10, 1"\
        )
#define dccimvac(addr) __ASM __volatile__(\
        "mcr p15, 0, addr, c7, c14, 1"\
        )

__STATIC_INLINE void set_csselr(u32 val)
{
    __ASM __volatile__("mcr p15, 2, val, c0, c0, 0");
}

__STATIC_INLINE u32 get_ccsidr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 1, val, c0, c0, 0");
    return val;
}

#elif defined(__GNUC__) /* Normal GCC and AC6 */

#define dciall() __ASM __volatile__(\
        "mcr p15, 0, %[input_dciall], c15, c5, 0"\
        :: [input_dciall] "r" (dciall)\
        )

#define iciallu() __ASM __volatile__(\
        "mcr p15, 0, %[input_iciallu], c7, c5, 0"\
        :: [input_iciallu] "r" (iciallu)\
        )
#define icimvau(addr) __ASM __volatile__(\
        "mcr p15, 0, %[input_addr], c7, c5, 1"\
        :: [input_addr] "r" (addr)\
        )
#define dcisw(sw) __ASM __volatile__(\
        "mcr p15, 0, %[input_sw], c7, c6, 2"\
        ::[input_sw] "r" (sw)\
        )
#define dccsw(sw) __ASM __volatile__(\
        "mcr p15, 0, %[input_sw], c7, c10, 2"\
        ::[input_sw] "r" (sw)\
        )
#define dccisw(sw) __ASM __volatile__(\
        "mcr p15, 0, %[input_sw], c7, c14, 2"\
        ::[input_sw] "r" (sw)\
        )
#define dcimvac(addr) __ASM __volatile__(\
        "mcr p15, 0, %[input_addr], c7, c6, 1"\
        :: [input_addr] "r" (addr)\
        )
#define dccmvac(addr) __ASM __volatile__(\
        "mcr p15, 0, %[input_addr], c7, c10, 1"\
        :: [input_addr] "r" (addr)\
        )
#define dccimvac(addr) __ASM __volatile__(\
        "mcr p15, 0, %[input_addr], c7, c14, 1"\
        :: [input_addr] "r" (addr)\
        )

__STATIC_INLINE void set_csselr(u32 val)
{
    __ASM __volatile__(\
        "mcr p15, 2, %0, c0, c0, 0"\
        ::"r" (val)\
        );
}

__STATIC_INLINE u32 get_ccsidr(void)
{
    u32 val;

    __ASM __volatile__(\
        "mrc p15, 1, %0, c0, c0, 0"\
        :"=r" (val)\
        );
    return val;
}

#else
 #error Unknown compiler
#endif

#define LOOP_SET_WAY  for(int s = 0; s <= num_set; s++)\
                          for(int w = 0; w <= num_way; w++) \
                          {\
                              reg = (s << set_shift) | \
                                    (w << way_shift);
#define LOOP_END      }
#define WHILE_FOR_RANGE  while(oper_addr < end_addr) {
#define ADDR_INC         oper_addr += L1C_LINE_SIZE;\
                         }

typedef enum {
    DCCSW = 0,
    DCISW,
    DCCISW,
    DCCMVAC,
    DCIMVAC,
    DCCIMVAC,
    ICIMVAU
} L1IDC_OPER;

static void l1dcache_sw_operation(L1IDC_OPER oper)
{
    u32 num_set, num_way, set_shift, way_shift;
    u32 csselr, ccsidr, reg;

    csselr = 1; /* select d cache */
    set_csselr(csselr);
    ccsidr = get_ccsidr();
    num_set = (ccsidr & 0xFFFE000) >> 13;
    num_way = (ccsidr & 0x1FFF) >> 3;
    set_shift = (ccsidr & 0x3) + 4;
    way_shift = __CLZ(num_way); /* count leading 0 */
    switch(oper) {
    case DCCSW:
        LOOP_SET_WAY
        dccsw(reg);
        LOOP_END
        break;
    case DCISW:
        LOOP_SET_WAY
        dcisw(reg);
        LOOP_END
        break;
    case DCCISW:
        LOOP_SET_WAY
        dccisw(reg);
        LOOP_END
        break;
    default:
        break;
    }
    __DSB();
    __ISB();
}

static void l1cache_range_oper(u32 addr, size_t length, L1IDC_OPER oper)
{
    u32 oper_addr, end_addr;

    end_addr = addr + length;
    oper_addr = addr & ~(L1C_LINE_SIZE - 1);
    switch(oper){
    case DCCMVAC:
        WHILE_FOR_RANGE
        dccmvac(oper_addr);
        ADDR_INC
        break;
    case DCIMVAC:
        WHILE_FOR_RANGE
        dcimvac(oper_addr);
        ADDR_INC
        break;
    case DCCIMVAC:
        WHILE_FOR_RANGE
        dccimvac(oper_addr);
        ADDR_INC
        break;
    case ICIMVAU:
        WHILE_FOR_RANGE
        icimvau(oper_addr);
        ADDR_INC
    default:
        break;
    }
    __DSB();
    __ISB();
}

void l1dcache_inv_all(void)
{
    u32 dciall = 0;
    dciall();
    __DSB();
    __ISB();

}

void l1dcache_clean_all(void)
{
    l1dcache_sw_operation(DCCSW);
}

void l1dcache_invcc_all(void)
{
    l1dcache_sw_operation(DCCISW);
}

void l1dcache_inv_by_range(u32 addr, size_t length)
{
    l1cache_range_oper(addr, length, DCIMVAC);
}

void l1dcache_clean_by_range(u32 addr, size_t length)
{
    l1cache_range_oper(addr, length, DCCMVAC);
}

void l1dcache_invcc_by_range(u32 addr, size_t length)
{
    l1cache_range_oper(addr, length, DCCIMVAC);
}

void l1dcache_enable(void)
{
    u32 reg;

    reg = get_sctlr();
    if (!(reg & DC_CTRL_BIT)) {
        l1dcache_inv_all();
        reg |= DC_CTRL_BIT;
        set_sctlr(reg);
        __DSB();
        __ISB();
    }
}

void l1dcache_disable(void)
{
    u32 reg;

    reg = get_sctlr();
    if ((reg & DC_CTRL_BIT)) {
        l1dcache_invcc_all();
        reg &= ~DC_CTRL_BIT;
        set_sctlr(reg);
        __DSB();
        __ISB();
    }
}

void l1icache_inv_all(void)
{
    u32 iciallu = 0;
    iciallu();
    __DSB();
    __ISB();
}

void l1icache_inv_by_range(u32 addr, size_t length)
{
    l1cache_range_oper(addr, length, ICIMVAU);
}

void l1icache_enable(void)
{
    u32 reg;

    reg = get_sctlr();
    if (!(reg & IC_CTRL_BIT)) {
        l1icache_inv_all();
        reg |= IC_CTRL_BIT;
        set_sctlr(reg);
        __DSB();
        __ISB();
    }
}

void l1icache_disable(void)
{
    u32 reg;

    reg = get_sctlr();
    if ((reg & IC_CTRL_BIT)) {
        reg &= ~IC_CTRL_BIT;
        set_sctlr(reg);
        __DSB();
        __ISB();
    }
}

__attribute__ ((section(".idle_text")))
void slp_l1dc_enable(void)
{
    u32 reg;
    u32 dciall = 0;

    reg = get_sctlr();
    if (!(reg & DC_CTRL_BIT)) {
        dciall();
        __DSB();
        __ISB();
        reg |= DC_CTRL_BIT;
        set_sctlr(reg);
        __DSB();
        __ISB();
    }
}

__attribute__ ((section(".idle_text")))
void slp_l1ic_enable(void)
{
    u32 reg;
    u32 iciallu = 0;

    reg = get_sctlr();
    if (!(reg & IC_CTRL_BIT)) {
        iciallu();
        __DSB();
        __ISB();
        reg |= IC_CTRL_BIT;
        set_sctlr(reg);
        __DSB();
        __ISB();
    }
}

__attribute__ ((section(".idle_text")))
void slp_l1c_enable(void)
{
    slp_l1dc_enable();
    slp_l1ic_enable();
}
