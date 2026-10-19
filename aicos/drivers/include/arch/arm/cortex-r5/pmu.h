/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines performance monitor unit(PMU) of cortex-r5.
 */

#ifndef __PMU_H__
#define __PMU_H__

#include <aic_type.h>
#include <core_op.h>
#include <aic_bitmap.h>

/* Bits defines */
#define PMCR_COUNTERS_EN_ALL  BIT_0
/* PMCR_COUNTERS_RESET 0:no effect 1: clear other counters */
#define PMCR_COUNTERS_RESET   BIT_1
/* PMCR_CYCLECNT_RESET 0:no effect 1: clear cycle count */
#define PMCR_CYCLECNT_RESET   BIT_2
/* PMCR_CYCLECNT_PERIOID 0:count every cycle 1: count every 64 cycle */
#define PMCR_CYCLECNT_PERIOID BIT_3
#define PMCNTENSET_CYCLECNT_ON   BIT_31
#define PMCNTENCLR_CYCLECNT_OFF  BIT_31

/* Number of performance counters */
#define CTRCOUNT 0x3

/* The following constants define the Cortex-R5 Performance Monitor Events */

/*
 * Software increment. The register is incremented only on writes to the
 * Software Increment Register
 */
#define EVENT_SOFTINCR 0x00

/*
 * Instruction cache miss
 * Each Instruction fetch from normal cacheable memory that causes a refill from
 * the level 2 memory system generates this event.Accesses that do not cause a new
 * cache refill, but are satisfied from refilling data of a previous miss are not
 * counted. Where instruction fetches consists of multiple instructions, these
 * accesses count as single events.
 */
#define EVENT_INSTRCACHEMISS 0x01

/*
 * Data cache miss
 * Each data read from or write to normal cacheable memory that causes a refill
 * from the level 2 memory system generates this event.Accesses that do not cause
 * a new cache refill, but are satisfied from refilling data of a previous miss are
 * not counted. Each access to a cache line to normal cacheable memory that causes
 * a new linefill is counted,including the multiple transactions of an LDM and STM
 */

#define EVENT_DATACACHEMISS 0x03

/*
 * Each access to a cache line is counted including the multiple transactions of
 * an LDM, STM, or other operations.
 */

#define EVENT_DATACACHEACCESS 0x04

/*
 *Data read architecturally executed
 *This evevt occurs for every every instruction that explicitly reads data,
 *including SWP.
 */

#define EVENT_DATAREAD 0x06

/*
 *Data write architecturally executed
 *This evevt occurs for every every instruction that explicitly writes data,
 *including SWP.
 */

#define EVENT_DATAWRITE 0x07

/*
 *Instruction architecturally executed
 */

#define EVENT_INSTR 0x08
/*
 * Dual-issued pair of instructions architecturally executed
 */

#define EVENT_DUALINSTR 0x5E

/*
 * Exception taken
 * This event occurs on each exception taken
 */

#define EVENT_EXCEPTION 0x09

/*
 * Exception return architecturally executed.
 * This event occurs on every exception return
 * for example:
 * RFE,MOVS PC,LDM Rn, {..,PC}^
 */

#define EVENT_EXCEPTIONRET 0x0A

/*
 * Change to Context ID Executed
 */

#define EVENT_CHANGETOCONTEXID 0x0B

/*
 * Software Change of PC, except by an exception,
 * architecturally executed
 */

#define EVENT_SWCHANGE 0x0C

/*
 * B immediate, BL immediate or BLX immediate instruction
 * architecturally executed
 *
 */

#define EVENT_IMMEDIATEINSTR 0x0D

/*
 * Procedure return architecturally executed, other than exception returns
 * For example:
 * BZ Rm, "LDM Rn, {..,PC}"
 */

#define EVENT_PROCEDURERET 0x0E

/*
 * Unaligned access architecturally executed
 * This event occurs for each instruction that was to an unaligned
 * address that either triggered an alignment fault, or would have done
 * so if the SCTLR A-bit had been set.
 */

#define EVENT_UNALIGNACCESS 0x0F

/*
 * Branch mispredicted or not predicted
 * This event ocurs for every pipeline flush
 * caused by a branch
 */

#define EVENT_BRANCHMISPREDICT 0x10

/*
 * Counts clock cycles when the Cortex-r5 processor is not in WFE/WFI. This
 * event is not exported on the EVENT bus
 */

#define EVENT_CLOCKCYCLES 0x11U

/*
 * Branches or other change in program flow that could have
 * been predicted by the branch prediction resources of the processor.
 */

#define EVENT_BRANCHPREDICT 0x12

/*
 * Stall because instruction buffer cannot deliver an instruction
 * This can indicate a cache miss. This event occurs every cycle where
 * the conditions is present.
 */

#define EVENT_INSTRSTALL 0x40

/*
 * Stall because of data dependency between instructions.
 * This event occurs every cycle where the condition is present.
 */

#define EVENT_DATASTALL 0x41

/*
 * Data cache write-back
 * This event occurs once for each line that is written back from the cache.
 */

#define EVENT_DATACACHEWRITE 0x42

/*
 * External memory request
 * Examples of this are cache refill, Non-cacheable accesses, write through
 * writes, cache line evictions(write-back)
 */

#define EVENT_EXTERNALMEMREQ 0x43U

/*
 * Stall because of LSU being busy
 * This event takes place each clock cycle where the condition is met.
 * A high incidence of this event indicates the pipeline is often waiting
 * for transactions to complete on the external bus.
 */

#define EVENT_LSUSTALL 0x44

/*
 * Store Buffer was forced to drain completely
 * Examples of this Fir cortex-R5 are DMB, Strongly ordered memory access,
 * or similar events.
 *
 */

#define EVENT_FORCEDRAINSTORE 0x45

/*
 * Instruction cache tag RAm parity or correctable ECC error
 */

#define EVENT_INSTRTAGPARITY 0x4A

/*
 * Instruction cache data RAm parity or correctable ECC error
 */

#define EVENT_INSTRDATAPARITY 0x4B

/*
 * Data cache tag or dirty RAM parity error or correctable ECC error,
 * from data-side or ACP
 */

#define EVENT_DATATAGPARITY 0x4C

/*
 * Data cache data RAM parity error or correctable ECC error
 */

#define EVENT_DATADATAPARITY 0x4D

/*
 * TCM fatal ECC error reported from the prefetch unit
 */

#define EVENT_TCMERRORPREFETCH 0x4E

/*
 * TCM fatal ECC error reported from the load/store unit
 */

#define EVENT_TCMERRORSTORE 0x4F

/*
 * Non-cacheable access on AXI master bus
 */

#define EVENT_NCONAXIM 0x57

/*
 * Instruction cache access
 */

#define EVENT_INSTRCACHEACCESS 0x58

/*
 * Dual issue case A(branch)
 */

#define EVENT_DUALISSUEA 0x5A

/*
 * Dual issue case B1,B2,F2(load/store), F2D.
 */

#define EVENT_DUALISSUEB 0x5B

/*
 * Dual issue other case
 */

#define EVENT_DUALISSUEOTHER 0x5C

/*
 * Double precision floating point arithmetic or
 * conversion instruction executed.
 */

#define EVENT_FPA 0x5D

/*
 * Data cache data RAM fatal ECC error
 */

#define EVENT_DATACACHEDATAERROR 0x60

/*
 * Data cache tag/dirty RAM fatal ECC error, from data-side or ACP
 */

#define EVENT_DATACACHETAGERROR 0x61

/*
 * Processor livelock because of hard errors or exception vector
 */

#define EVENT_PROCESSORLIVELOCK 0x62

/*
 * ATCM Multi-bit error
 */

#define EVENT_ATCMMULTIBITERROR 0x64

/*
 * B0TCM Multi-bit error
 */

#define EVENT_B0TCMMULTIBITERROR 0x65

/*
 * B1TCM Multi-bit error
 */

#define EVENT_B1TCMMULTIBITERROR 0x66

/*
 * ATCM Single-bit error
 */

#define EVENT_ATCMSINGLEBITERROR 0x67

/*
 * B0TCM Single-bit error
 */

#define EVENT_B0TCMSINGLEBITERROR 0x68

/*
 * B1TCM Single-bit error
 */

#define EVENT_B1TCMSINGLEBITERROR 0x69

/*
 * TCM correctable ECC error reported by load/store unit
 */

#define EVENT_TCMERRORLSU 0x6A

/*
 * TCM correctable ECC error reported by prefetch unit
 */

#define EVENT_TCMERRORPFU 0x6B

/*
 * TCM fatal ECC error reported by AXI slave unit
 */

#define EVENT_TCMFATALERRORAXI 0x6C

/*
 * TCM correctable ECC error reported by AXI slave unit
 */

#define EVENT_TCMERRORAXI 0x6D

/*
 * ACP D-cache access, lookup or invalidate
 */


#define EVENT_DCACHEACCESS 0x72

/*
 * ACP D-cache invalidate
 */

#define EVENT_DCACHEINVALIDATE 0x73


/*
 * The following constants define the configurations for Cortex-R5 Performance
 * Monitor Events. Each configuration configures the event counters for a set
 * of events.
 * -----------------------------------------------
 * Config        PmCtr0... PmCtr5
 * -----------------------------------------------
 * CNTRCFG1        { EVENT_SOFTINCR,
 *              EVENT_INSTRCACHEMISS,
 *              EVENT_DATACACHEMISS }
 *
 * CNTRCFG2        { EVENT_DATACACHEACCESS,
 *              EVENT_DATAREAD,
 *              EVENT_DATAWRITE }
 *
 * CNTRCFG3        { EVENT_INSTR,
 *              EVENT_DUALINSTR,
 *              EVENT_EXCEPTION }
 *
 * CNTRCFG4        { EVENT_EXCEPTIONRET,
 *              EVENT_CHANGETOCONTEXID,
 *              EVENT_SWCHANGE }
 *
 * CNTRCFG5        { EVENT_IMMEDIATEINSTR,
 *              EVENT_PROCEDURERET,
 *              EVENT_UNALIGNACCESS }
 *
 * CNTRCFG6        { EVENT_BRANCHMISPREDICT,
 *              EVENT_CLOCKCYCLES,
 *              EVENT_BRANCHPREDICT }
 *
 * CNTRCFG7        { EVENT_INSTRSTALL,
 *              EVENT_DATASTALL,
 *              EVENT_DATACACHEWRITE }
 *
 * CNTRCFG8        { EVENT_EXTERNALMEMREQ,
 *                        EVENT_LSUSTALL,
 *              EVENT_FORCEDRAINSTORE }
 *
 * CNTRCFG9        { EVENT_INSTRTAGPARITY,
 *                        EVENT_INSTRDATAPARITY,
 *              EVENT_DATATAGPARITY }
 *
 * CNTRCFG10            { EVENT_DATADATAPARITY,
 *                        EVENT_TCMERRORPREFETCH,
 *              EVENT_TCMERRORSTORE }
 *
 * CNTRCFG11        { EVENT_INSTRCACHEACCESS,
 *                        EVENT_DUALISSUEA,
 *              EVENT_DUALISSUEB }
 *
 * CNTRCFG12            { EVENT_DUALISSUEOTHER,
 *                        EVENT_FPA,
 *              EVENT_DATACACHEDATAERROR }
 *
 * CNTRCFG13            { EVENT_DATACACHETAGERROR,
 *                        EVENT_PROCESSORLIVELOCK,
 *              EVENT_ATCMMULTIBITERROR }
 *
 * CNTRCFG14            { EVENT_B0TCMMULTIBITERROR,
 *                        EVENT_B1TCMMULTIBITERROR,
 *              EVENT_ATCMSINGLEBITERROR }
 *
 * CNTRCFG15            { EVENT_B0TCMSINGLEBITERROR,
 *                        EVENT_B1TCMSINGLEBITERROR,
 *              EVENT_TCMERRORLSU }
 *
 * CNTRCFG16            { EVENT_TCMERRORPFU,
 *                        EVENT_TCMFATALERRORAXI,
 *              EVENT_TCMERRORAXI }
 */
#define CNTRCFG1        0
#define CNTRCFG2        1
#define CNTRCFG3        2
#define CNTRCFG4        3
#define CNTRCFG5        4
#define CNTRCFG6        5
#define CNTRCFG7        6
#define CNTRCFG8        7
#define CNTRCFG9        8
#define CNTRCFG10       9
#define CNTRCFG11       10
#define CNTRCFG12       11
#define CNTRCFG13       12
#define CNTRCFG14       13
#define CNTRCFG15       14
#define CNTRCFG16       15

#define NO_COUNTERS_AVAILABLE     0xFFU
#define MAX_EVENTHANDLER_ID       0x2U
#define EVENT_CNTRS_BIT_MASK      0x7U
#define ALL_EVENT_CNTRS_IN_USE    0x7U
#define EVENT_CNTRS_MASK          0x7U

/* c9 operations
 * PMCR
 * p15, 0, Rd, c9, c12, 0
 * PMCNTENSET
 * p15, 0, Rd, c9, c12, 1 
 * PMCNTENCLR
 * p15, 0, Rd, c9, c12, 2
 * PMOVSR
 * p15, 0, Rd, c9, c12, 3
 * PMSWINC
 * p15, 0, Rd, c9, c12, 4
 * PMSELR
 * p15, 0, Rd, c9, c12, 5
 * PMCCNTR
 * p15, 0, Rd, c9, c13, 0 
 * PMXEVTYPER0
 * p15, 0, Rd, c9, c13, 1
 * PMXEVCNTR0
 * p15, 0, Rd, c9, c13, 2
 * PMUSERENR
 * p15, 0, Rd, c9, c14, 0 
 * PMINTENSET
 * p15, 0, Rd, c9, c14, 1
 * PMINTENCLR
 * p15, 0, Rd, c9, c14, 2 
 *
 */
#if defined(__CC_ARM) /* Arm compiler 5 */

__STATIC_INLINE u32 get_pmcr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c12, 0");
    return val;
}

__STATIC_INLINE void set_pmcr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c12, 0");
}

__STATIC_INLINE u32 get_pmcntenset(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c12, 1");
    return val;
}

__STATIC_INLINE void set_pmcntenset(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c12, 1");
}

__STATIC_INLINE u32 get_pmcntenclr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c12, 2");
    return val;
}

__STATIC_INLINE void set_pmcntenclr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c12, 2");
}

__STATIC_INLINE u32 get_pmovsr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c12, 3");
    return val;
}

__STATIC_INLINE void set_pmovsr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c12, 3");
}

__STATIC_INLINE u32 get_pmswinc(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c12, 4");
    return val;
}

__STATIC_INLINE void set_pmswinc(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c12, 4");
}

__STATIC_INLINE u32 get_pmselr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c12, 5");
    return val;
}

__STATIC_INLINE void set_pmselr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c12, 5");
}

__STATIC_INLINE u32 get_pmccntr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c13, 0");
    return val;
}

__STATIC_INLINE void set_pmccntr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c13, 0");
}

__STATIC_INLINE u32 get_pmxevtype(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c13, 1");
    return val;
}

__STATIC_INLINE void set_pmxevtype(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c13, 1");
}

__STATIC_INLINE u32 get_pmxevcntr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c13, 2");
    return val;
}

__STATIC_INLINE void set_pmxevcntr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c13, 2");
}

__STATIC_INLINE u32 get_pmuintenset(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c14, 1");
    return val;
}

__STATIC_INLINE void set_pmuintenset(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c14, 1");
}

__STATIC_INLINE u32 get_pmuintenclr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c9, c14, 2");
    return val;
}

__STATIC_INLINE void set_pmuintenclr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c9, c14, 2");
}

#elif defined(__GNUC__) /* Normal GCC and AC6 */

__STATIC_INLINE u32 get_pmcr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c12, 0"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmcr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c12, 0"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmcntenset(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c12, 1"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmcntenset(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c12, 1"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmcntenclr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c12, 2"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmcntenclr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c12, 2"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmovsr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c12, 3"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmovsr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c12, 3"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmswinc(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c12, 4"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmswinc(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c12, 4"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmselr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c12, 5"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmselr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c12, 5"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmccntr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c13, 0"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmccntr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c13, 0"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmxevtype(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c13, 1"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmxevtype(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c13, 1"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmxevcntr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c13, 2"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmxevcntr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c13, 2"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmuintenset(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c14, 1"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmuintenset(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c14, 1"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_pmuintenclr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c9, c14, 2"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_pmuintenclr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c9, c14, 2"\
    ::"r"(val)\
    );
}

#else
 #error Unknown compiler
#endif

void pmu_enable_cyccnt(void);
void pmu_disable_cyccnt(void);
u32 pmu_get_cyccnt(void);
void pmu_setup_cnts(u32 PmcrCfg);
void pmu_get_cntsvals(u64 *PmCtrValue);
int pmu_disable_cnt(u32 EventHandlerId);
int pmu_setup_cnt(u32 EventID);
int pmu_get_cntval(u32 EventHandlerId, u32 *CntVal);
void pmu_disable_cnts(void);
void pmu_enable_cnts(void);
void pmu_reset_cnts(void);

#endif /* __PMU_H__ */
