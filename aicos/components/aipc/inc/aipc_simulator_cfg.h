/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file contains all aipc module config data of simulator system.
 *    It contains bellow two parts:
 *      aipc_cfg: Contains all aipc configurations of this system communicating
 *                with other systems.
 *      ablk_cfg: Contains all ablk configurations of this system communicating
 *                with other systems.
 */
#ifndef __AIPC_SIMULATOR_CFG_H__
#define __AIPC_SIMULATOR_CFG_H__

/**
 * aipc configuration section.
 */

#if defined(CONFIG_SUBSYS_CP) || defined(WIN32)
/*
 * AIPC_SIMULATOR_AP - run system as cp, this file simulator ap.
 */
#define AIPC_SIMULATOR_AP
#endif

#ifdef CONFIG_SUBSYS_AP
/*
 * AIPC_SIMULATOR_CP - run system as ap, this file simulator cp.
 */
#define AIPC_SIMULATOR_CP
#endif

#define AMEM_NORMAL_SIZE    0x000400000
#define AMEM_INTERNAL_SIZE  0x000020000

static u32 amem_normal[AMEM_NORMAL_SIZE/sizeof(u32)];
static u32 amem_internal[AMEM_INTERNAL_SIZE/sizeof(u32)];

#define AMEM_NORMAL_BASE      (u32)&amem_normal[0]
#define AMEM_INTERNAL_BASE    (u32)&amem_internal[0]

#ifdef AIPC_SIMULATOR_AP
static AIPC_CFG_T sim_cfg_table[] = {
    /* ap <--> cp ipc */
    {
        AIC_SYS_CP,
        0,
        1, /* task pri */
        /* normal share memory */
        {AMEM_NORMAL_BASE, AMEM_NORMAL_SIZE, AMEM_256B_SHIFT},
        /* internal share memory */
        {AMEM_INTERNAL_BASE, AMEM_INTERNAL_SIZE, AMEM_16B_SHIFT},
        "ap_cp"
    }
};

/******************************************************************************
 *                    rmsg configuration section.
 ******************************************************************************/
/**
 * rmsg_cfg - all the rmsg config in current system.
 */
static const RMSG_CFG_T rmsg_cfg[] = {
    {
        AIC_SYS_CP, /* sys */
        AIPC_BRR_COMM, /* bearer id */
        RMSG_LINK, /* link id */
        0 /* rsv */
    }
};

/**
* all ablk of all sytem pool cfg,
* pool cnt and pool size config array.
*/
/* ap ablk pool cfg*/
/* 1. ap ==> cp ,AT commd pool */
#define APCP_AT_ABLK_NAME   "ap-cp-at"
#define APCP_AT_POOL_CNT    2
static const u16 apcp_at_pool_cnt[APCP_AT_POOL_CNT] = {24, 4,};
static const aipc_size_t apcp_at_pool_size[APCP_AT_POOL_CNT] = {32, 256};
static const ABLK_LINK_CFG_T apcp_at_link_cfg[] = {
    {AT_LINK0, LINK_NORMAL, 4},
    {AT_LINK1, LINK_NORMAL, 4},
    {AT_LINK2, LINK_NORMAL, 4},
    {AT_LINK3, LINK_NORMAL, 4},
    {AT_LINK4, LINK_NORMAL, 4},
    {AT_LINK5, LINK_NORMAL, 4}
};
#define APCP_AT_LINK_CNT    ARRAY_SIZE(apcp_at_link_cfg)

/* 2. ap ==> cp, ip ul pool */
#define APCP_IP_ABLK_NAME   "ap-cp-ip"
#define APCP_IP_POOL_CNT    2
#define APCP_IP_LINK_CNT    16
const u16 apcp_ip_pool_cnt[APCP_IP_POOL_CNT] = {32, 10};
const aipc_size_t apcp_ip_pool_size[APCP_IP_POOL_CNT] = {128, 1600};

/* 3. ap ==> cp, common, such as nv, cp status */
#define APCP_COMM_ABLK_NAME "ap-cp-comm"
#define APCP_COMM_POOL_CNT  2

static const u16 apcp_comm_pool_cnt[APCP_COMM_POOL_CNT] = {4, 4};
static aipc_size_t apcp_comm_pool_size[APCP_COMM_POOL_CNT] = {256, 2048};
static const ABLK_LINK_CFG_T apcp_comm_link_cfg[] = {
    {ALTRACE_LINK, LINK_NORMAL, 0},
    {ALMSG_LINK, LINK_NORMAL, 0},
    {ALDATA_LINK, LINK_NORMAL, 0},
    {STATUS_LINK, LINK_NORMAL, 4},
    {ADIAG_LINK, LINK_NORMAL, 6},
    {RMSG_LINK, LINK_NORMAL, 10}
};
#define APCP_COMM_LINK_CNT    ARRAY_SIZE(apcp_comm_link_cfg)

/**
 *ablk_cfg - all the ablk config in current system.
 */
static const ABLK_CFG_T sim_ablk_cfg[] = {
    /* 1. ap-cp at blk */
    {
        APCP_AT_ABLK_NAME,
        AIC_SYS_CP, /* sys */
        AIPC_BRR_AT, /* bearer id */
        APCP_AT_POOL_CNT, /* pool cnt */
        APCP_AT_LINK_CNT, /* link cnt */
        ABLK_NORMAL, /* type */
        ABLK_DEFAULT_ALLOC_RANGE, /* alloc range */
        apcp_at_pool_cnt, /* pool cnt arrary */
        apcp_at_pool_size, /* pool size arrary */
        apcp_at_link_cfg, /* link cfg */
    },

    /* 2. ap-cp ip blk */
    {
        APCP_IP_ABLK_NAME,
        AIC_SYS_CP, /* sys */
        AIPC_BRR_IP, /* bearer id */
        APCP_IP_POOL_CNT, /* pool cnt */
        APCP_IP_LINK_CNT, /* link cnt */
        ABLK_NORMAL, /* type */
        ABLK_DEFAULT_ALLOC_RANGE, /* alloc range */
        apcp_ip_pool_cnt, /* pool cnt arrary */
        apcp_ip_pool_size, /* pool size arrary */
        NULL, /* no link config, will create by owner. */
    },

    /* 3. ap-cp common blk */
    {
        APCP_COMM_ABLK_NAME,
        AIC_SYS_CP, /* sys */
        AIPC_BRR_COMM, /* bearer id */
        APCP_COMM_POOL_CNT, /* pool cnt */
        APCP_COMM_LINK_CNT, /* link cnt */
        ABLK_NORMAL, /* type */
        ABLK_MININUM_ALLOC_RANGE, /* alloc range */
        apcp_comm_pool_cnt, /* pool cnt arrary */
        apcp_comm_pool_size, /* pool size arrary */
        apcp_comm_link_cfg, /* link cfg */
    },
};

/* create ap simulator outputer. */
#include <alog_outputer.h>

ALOP_T *ap_sim_log_outputer;
static const ALBRR_CFG_T ap_sim_brr_cfg = {AIC_SYS_CP, AIPC_BRR_DEBUG};

static void alog_create_ap_sim_outputer(void)
{
    ap_sim_log_outputer =
    alop_create(AIC_IODEV_UART, AIC_COM1);
}
#endif /* AIPC_SIMULATOR_AP */

#ifdef AIPC_SIMULATOR_CP
static AIPC_CFG_T sim_cfg_table[] = {
     /* cp <--> ap ipc */
    {
        AIC_SYS_AP,
        1,
        1, /* task pri */
        /* normal share memory */
        {AMEM_NORMAL_BASE, AMEM_NORMAL_SIZE, AMEM_256B_SHIFT},
        /* internal share memory */
        {AMEM_INTERNAL_BASE, AMEM_INTERNAL_SIZE, AMEM_16B_SHIFT},

        "cp_ap"
    }
};

/******************************************************************************
 *                    rmsg configuration section.
 ******************************************************************************/
/**
 * rmsg_cfg - all the rmsg config in current system.
 */
static const RMSG_CFG_T rmsg_cfg[] = {
    {
        AIC_SYS_AP, /* sys */
        AIPC_BRR_COMM, /* bearer id */
        RMSG_LINK, /* link id */
        0 /* rsv */
    }
};

/**
* all ablk of all sytem pool cfg,
* pool cnt and pool size config array.
*/
/* cp ablk pool cfg*/
/* 1. cp ==> ap ,AT commd pool */
#define CPAP_AT_ABLK_NAME   "cp-ap-at"
#define CPAP_AT_POOL_CNT    4
static const u16 cpap_at_pool_cnt[CPAP_AT_POOL_CNT] = {16, 8, 8, 4,};
static const aipc_size_t cpap_at_pool_size[CPAP_AT_POOL_CNT] = {12, 32, 64, 128};
static const ABLK_LINK_CFG_T cpap_at_link_cfg[] = {
    {AT_LINK0, LINK_NORMAL, 8},
    {AT_LINK1, LINK_NORMAL, 12},
    {AT_LINK2, LINK_NORMAL, 12},
    {AT_LINK3, LINK_NORMAL, 12},
    {AT_LINK4, LINK_NORMAL, 12},
    {AT_LINK5, LINK_NORMAL, 12}
};
#define CPAP_AT_LINK_CNT    ARRAY_SIZE(cpap_at_link_cfg)


/* 2. cp ==> ap, ip ul pool */
#define CPAP_IP_ABLK_NAME   "cp-ap-ip"
#define CPAP_IP_POOL_CNT    1
#define CPAP_IP_LINK_CNT   16

static const u16 cpap_ip_pool_cnt[CPAP_IP_POOL_CNT] = {32};
static const aipc_size_t cpap_ip_pool_size[CPAP_IP_POOL_CNT] = {1600};

/* 3. cp ==> ap, commn, such as nv, cp status */
#define CPAP_COMM_ABLK_NAME "cp-ap-comm"
#define CPAP_COMM_POOL_CNT  3

static const u16 cpap_comm_pool_cnt[CPAP_COMM_POOL_CNT] = {8, 4, 20};
static const aipc_size_t cpap_comm_pool_size[CPAP_COMM_POOL_CNT] = {256, 2048, 4096};
static const ABLK_LINK_CFG_T cpap_comm_link_cfg[] = {
    {ALTRACE_LINK, LINK_NORMAL, 10},
    {ALMSG_LINK, LINK_NORMAL, 10},
    {ALDATA_LINK, LINK_NORMAL, 8},
    {STATUS_LINK, LINK_NORMAL, 4},
    {ADIAG_LINK, LINK_NORMAL, 6},
    {RMSG_LINK, LINK_NORMAL, 10}
};
#define CPAP_COMM_LINK_CNT  ARRAY_SIZE(cpap_comm_link_cfg)

/**
 *ablk_cfg - all the ablk config in current system.
 */
static const ABLK_CFG_T sim_ablk_cfg[] = {
    /* 1. cp-ap at ablk */
    {
        CPAP_AT_ABLK_NAME,
        AIC_SYS_AP, /* sys */
        AIPC_BRR_AT, /* bearer id */
        CPAP_AT_POOL_CNT, /* pool cnt */
        CPAP_AT_LINK_CNT, /* link cnt */
        ABLK_NORMAL, /* type */
        ABLK_DEFAULT_ALLOC_RANGE, /* alloc range */
        cpap_at_pool_cnt, /* pool cnt arrary */
        cpap_at_pool_size, /* pool size arrary */
        cpap_at_link_cfg, /* link cfg */
    },

    /* 2. cp-ap ip ablk */
    {
        CPAP_IP_ABLK_NAME,
        AIC_SYS_AP, /* sys */
        AIPC_BRR_IP, /* bearer id */
        CPAP_IP_POOL_CNT, /* pool cnt */
        CPAP_IP_LINK_CNT, /* link cnt */
        ABLK_SNORMAL, /* type */
        ABLK_DEFAULT_ALLOC_RANGE, /* alloc range */
        cpap_ip_pool_cnt, /* pool cnt arrary */
        cpap_ip_pool_size, /* pool size arrary */
        NULL, /* no link config, will create by owner. */
    },

    /* 3. cp-ap common ablk */
    {
        CPAP_COMM_ABLK_NAME,
        AIC_SYS_AP, /* sys */
        AIPC_BRR_COMM, /* bearer id */
        CPAP_COMM_POOL_CNT, /* pool cnt */
        CPAP_COMM_LINK_CNT, /* link cnt */
        ABLK_NORMAL, /* type */
        ABLK_MININUM_ALLOC_RANGE, /* alloc range */
        cpap_comm_pool_cnt, /* pool cnt arrary */
        cpap_comm_pool_size, /* pool size arrary */
        cpap_comm_link_cfg, /* link cfg */
    },
};
#endif /* AIPC_SIMULATOR_CP */

#endif /* __AIPC_SIMULATOR_CFG_H__ */

