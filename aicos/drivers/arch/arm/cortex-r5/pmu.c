/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines performance monitor unit(PMU) interface of cortex-r5.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f0500
#define LOG_MODULE    AMOD_DRV

#include <aic_errno.h>
#include <arch/arm/cortex-r5/pmu.h>
#include <irq_chip.h>
#include <irq.h>

typedef const u32 PmcrEventCfg32[CTRCOUNT];
u64 s_cnts_vals[CTRCOUNT] = {0};

void pmu_disable_cnts(void)
{
    u32 val;

    /* Enable interrupt */
    set_pmuintenclr(0x7);
    /* Disable the event counters */
    val = get_pmcntenclr();
    val |= EVENT_CNTRS_MASK;
    set_pmcntenclr(val);
}

void pmu_enable_cnts(void)
{
    u32 val;

    /* Enable the event counters */
    val = get_pmcntenset();
    val |= EVENT_CNTRS_MASK;
    set_pmcntenset(val);
    /* Clear overflow flag */
    set_pmovsr(0x7);
    /* Enable interrupt */
    set_pmuintenset(0x7);
    /* Enable ALL counter */
    val = get_pmcr();
    val |= PMCR_COUNTERS_EN_ALL;
    set_pmcr(val);
}

void pmu_reset_cnts(void)
{
    u32 val;

    s_cnts_vals[0] = s_cnts_vals[1] = s_cnts_vals[2] = 0;
    val = get_pmcr();
    val |= PMCR_COUNTERS_RESET; /* reset event counters */
    set_pmcr(val);
}

int pmu_disable_cnt(u32 cntid)
{
    u32 counters;
    u32 cntrmask = 0x1;

    if (cntid > MAX_EVENTHANDLER_ID) {
        aicx_info(0x1be0c5, "", "Invalid EventHandlerID\n");
        return AIC_EINVAL;
    } else {
        cntrmask = cntrmask << cntid;
        counters = get_pmcntenclr();
        counters |= cntrmask;
        set_pmcntenclr(counters);
        return AIC_SUCCESS;
    }
}

int pmu_setup_cnt(u32 eventid)
{
    u32 counters;
    u32 orig_counters;
    u32 index;

    orig_counters= get_pmcntenset();
    orig_counters &= EVENT_CNTRS_BIT_MASK;
    counters = orig_counters;
    if (counters == ALL_EVENT_CNTRS_IN_USE) {
        return AIC_EINVAL;
    } else {
        for(index = 0U; index < CTRCOUNT; index++) {
            if ((counters & 0x1U) == 0x0) {
                break;
            } else {
                counters = counters >> 0x1U;
                continue;
            }
        }
    }

    /* Select event counter */
    set_pmselr(index);
    /* Set the event */
    set_pmxevtype(eventid);
    /* Enable event counter */
    set_pmcntenset(orig_counters | (1U << index));

    return AIC_SUCCESS;
}

int pmu_get_cntval(u32 cntid, u32 *cntval)
{
    if (cntid > MAX_EVENTHANDLER_ID) {
        aicx_error(0x0f8b3e, "", "Invalid Event Handler ID\n");
        return AIC_EINVAL;
    } else {
        set_pmselr(cntid);
        *cntval = get_pmxevcntr();
        return AIC_SUCCESS;
    }
}

ISR_RET_E pmu_uisr(void *param)
{
    u32 int_en, over_sts;

    int_en = get_pmuintenset();
    over_sts = get_pmovsr();
    /* clear over flag */
    set_pmovsr(int_en & over_sts & 0x7);
    for (s32 i = 0; i < CTRCOUNT; i++) {
        if ((0x1 << i) & int_en & over_sts) s_cnts_vals[i] += 0xFFFFFFFFUL;
    }
    return NONE_WORKER;
}

void pmu_setup_cnts(u32 PmcrCfg)
{
    u32 counter;
    static PmcrEventCfg32 PmcrEvents[] = {
        {//0 , 11/08/44
            EVENT_CLOCKCYCLES,
            EVENT_INSTR,
            EVENT_LSUSTALL,
            //EVENT_INSTRSTALL
        },
        {//1 , 40/03/04
            EVENT_INSTRSTALL,
            EVENT_DATACACHEMISS,
            EVENT_DATACACHEACCESS
        },
        {//2,  01/58/42
            EVENT_INSTRCACHEMISS,
            EVENT_INSTRCACHEACCESS,
            EVENT_DATACACHEWRITE
        },
        {//3,  57
            EVENT_NCONAXIM,
            EVENT_DATACACHEMISS,
            EVENT_DATACACHEACCESS
        },
        {
            EVENT_IMMEDIATEINSTR,
            EVENT_PROCEDURERET,
            EVENT_UNALIGNACCESS
        },
        {
            EVENT_BRANCHMISPREDICT,
            EVENT_CLOCKCYCLES,
            EVENT_BRANCHPREDICT
        },
        {
            EVENT_INSTRSTALL,
            EVENT_DATASTALL,
            EVENT_DATACACHEWRITE
        },
        {
            EVENT_EXTERNALMEMREQ,
            EVENT_LSUSTALL,
            EVENT_FORCEDRAINSTORE
        },
        {
            EVENT_INSTRTAGPARITY,
            EVENT_INSTRDATAPARITY,
            EVENT_DATATAGPARITY
        },
        {
            EVENT_DATADATAPARITY,
            EVENT_TCMERRORPREFETCH,
            EVENT_TCMERRORSTORE
        },
        {
            EVENT_INSTRCACHEACCESS,
            EVENT_DUALISSUEA,
            EVENT_DUALISSUEB
        },
        {
            EVENT_DUALISSUEOTHER,
            EVENT_FPA,
            EVENT_DATACACHEDATAERROR
        },
        {
            EVENT_DATACACHETAGERROR,
            EVENT_PROCESSORLIVELOCK,
            EVENT_ATCMMULTIBITERROR
        },
        {
            EVENT_B0TCMMULTIBITERROR,
            EVENT_B1TCMMULTIBITERROR,
            EVENT_ATCMSINGLEBITERROR
        },
        {
            EVENT_B0TCMSINGLEBITERROR,
            EVENT_B1TCMSINGLEBITERROR,
            EVENT_TCMERRORLSU
        },
        {
            EVENT_TCMERRORPFU,
            EVENT_TCMFATALERRORAXI,
            EVENT_TCMERRORAXI
        },

    };
    const u32 *ptr = PmcrEvents[PmcrCfg];
    pmu_disable_cnts();

    for(counter = 0U; counter < CTRCOUNT; counter++) {

        /* Select event counter */
        set_pmselr(counter);

        /* Set the event */
        set_pmxevtype(ptr[counter]);
    }

    pmu_reset_cnts();
    pmu_enable_cnts();
#if 0
    register_irq(INT_IRQ0, pmu_uisr, AIC_NULL, AIC_NULL, AIC_NULL);
#endif
}

void pmu_get_cntsvals(u64 *PmCtrValue)
{
    u32 Counter;

#if 0
    pmu_disable_cnts();

    for(Counter = 0U; Counter < CTRCOUNT; Counter++) {
        set_pmselr(Counter);
        s_cnts_vals[Counter] += get_pmxevcntr();
        PmCtrValue[Counter] = s_cnts_vals[Counter];
    }
#endif


    for(Counter = 0U; Counter < CTRCOUNT; Counter++) {
        set_pmselr(Counter);
        PmCtrValue[Counter] = get_pmxevcntr();
    }
}

void pmu_enable_cyccnt(void)
{
    u32 val;

    val = get_pmcr();
    /* Count every 64 cycle */
    val |= PMCR_CYCLECNT_PERIOID;
    val |= PMCR_CYCLECNT_RESET;
    val |= PMCR_COUNTERS_EN_ALL;
    /* Enable all counter */
    set_pmcr(val);
    /* Enable cycle counter single */
    set_pmcntenset(PMCNTENSET_CYCLECNT_ON);
}

void pmu_disable_cyccnt(void)
{
    set_pmcntenclr(PMCNTENCLR_CYCLECNT_OFF);
}

u32 pmu_get_cyccnt(void)
{
    u32 val;

    val = get_pmccntr();
    return val;
}
