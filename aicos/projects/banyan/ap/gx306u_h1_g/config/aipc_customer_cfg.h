
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file contains all aipc module config data of ap system.
 *    It contains bellow two parts:
 *      aipc_cfg: Contains all aipc configurations of this system communicating
 *                with other systems.
 *      ablk_cfg: Contains all ablk configurations of this system communicating
 *                with other systems.
 */
#ifndef __AIPC_CUSTOMER_CFG_H__
#define __AIPC_CUSTOMER_CFG_H__
#include <aic_type.h>
#include <aic_size.h>
#include <sys_mem_config.h>

/******************************************************************************
 *                    aipc configuration section.
 ******************************************************************************/

 /* share meory base and size. */
#define AMEM_NORMAL_BASE      SYS_2_CP_SHM_BASE
#define AMEM_NORMAL_SIZE      SYS_2_CP_SHM_SIZE
#define AMEM_INTERNAL_BASE    AON_AP2CP_BASE
#define AMEM_INTERNAL_SIZE    AON_AP2CP_SIZE

/**
 *aipc_cfg - used the table to create aipc instacnce.
 */
static const AIPC_CFG_T aipc_cfg[] = {
    /* ap <--> cp ipc */
    {
        AIC_SYS_CP, /* remote sys is cp */
        FALSE, /* is not client sys */
        1, /* task pri */
        /* normal share memory */
        {AMEM_NORMAL_BASE, AMEM_NORMAL_SIZE, AMEM_256B_SHIFT},
        /* internal share memory */
        {AMEM_INTERNAL_BASE, AMEM_INTERNAL_SIZE, AMEM_16B_SHIFT},
        "ap_cp" /* name */
    },
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

/******************************************************************************
 *                    ablk configuration section.
 ******************************************************************************/
/**
* all ablk of all sytem pool cfg,
* pool cnt and pool size config array.
* every ablk config like below(rx only just config the ablk name.) :
* ABLK name
* ABLK pool cnt
* number of blocks per pool
* the size of each pool
* link config table: each item contains, link id, link type, link fifo.
* ABLK link cnt
*/

/*******************************************************************************
 *                       ap-cp ablk pool and link cfg
 ******************************************************************************/

/*----------------------------------------------------------------------
 * 1. ap ==> cp, AT command pool config
 *----------------------------------------------------------------------*/
#define APCP_AT_ABLK_NAME   "at"
#define APCP_AT_POOL_CNT    2
static const u16 apcp_at_pool_cnt[APCP_AT_POOL_CNT] = {8};
static const aipc_size_t apcp_at_pool_size[APCP_AT_POOL_CNT] = {256};
static const ABLK_LINK_CFG_T apcp_at_link_cfg[] = {
    {AT_LINK0, LINK_NORMAL, 8},
    {AT_LINK1, LINK_NORMAL, 8},
    {AT_LINK2, LINK_NORMAL, 8},
    {AT_LINK3, LINK_NORMAL, 8},
    {AT_LINK4, LINK_NORMAL, 8},
    {AT_LINK5, LINK_NORMAL, 8}
};
#define APCP_AT_LINK_CNT    ARRAY_SIZE(apcp_at_link_cfg)

/*----------------------------------------------------------------------
 * 2. ap ==> cp, ip ul pool config, dynamic link
 *----------------------------------------------------------------------*/
#define APCP_IP_ABLK_NAME   "ip"
#define APCP_IP_POOL_CNT    2
#define APCP_IP_LINK_CNT    16

const u16 apcp_ip_pool_cnt[APCP_IP_POOL_CNT] = {20, 10};
const aipc_size_t apcp_ip_pool_size[APCP_IP_POOL_CNT] = {128, 1600};

/*----------------------------------------------------------------------
 * 3. ap ==> cp, common pool config, such as nv, cp status
 *----------------------------------------------------------------------*/
#define APCP_COMM_ABLK_NAME "comm"
#define APCP_COMM_POOL_CNT  2

static const u16 apcp_comm_pool_cnt[APCP_COMM_POOL_CNT] = {6, 2};
static const aipc_size_t apcp_comm_pool_size[APCP_COMM_POOL_CNT] = {256, 2048};
static const ABLK_LINK_CFG_T apcp_comm_link_cfg[] = {
    {ALTRACE_LINK, LINK_NORMAL, 1},
    {ALMSG_LINK,   LINK_NORMAL, 1},
    {ALDATA_LINK, LINK_NORMAL, 1},
    {STATUS_LINK, LINK_NORMAL, 4},
    {ADIAG_LINK, LINK_NORMAL, 6},
    {RMSG_LINK, LINK_NORMAL, 8}
};
#define APCP_COMM_LINK_CNT    ARRAY_SIZE(apcp_comm_link_cfg)

/*******************************************************************************
 *                          ap-ap ablk pool cfg
 ******************************************************************************/
/*----------------------------------------------------------------------
 * 1. ap ==> ap, debug pool config
 *----------------------------------------------------------------------*/
#define APAP_DEBUG_ABLK_NAME   "apdbg"
#define APAP_DEBUG_POOL_CNT 2

static const u16 apap_debug_pool_cnt[APAP_DEBUG_POOL_CNT] = {6, 4};
static const aipc_size_t apap_debug_pool_size[APAP_DEBUG_POOL_CNT] = {0x200, 0x800};
static const ABLK_LINK_CFG_T apap_debug_link_cfg[] = {
    {ALTRACE_LINK, LINK_NORMAL, 4},
    {ALMSG_LINK,   LINK_NORMAL, 1},
    {ALDATA_LINK, LINK_NORMAL, 6},
};
#define APAP_DEBUG_LINK_CNT ARRAY_SIZE(apap_debug_link_cfg)

/******************************************************************************
 *                    ablk_cfg configuration section.
 ******************************************************************************/
/**
 *ablk_cfg - all the ablk config in current system.
 */
static const ABLK_CFG_T ablk_cfg[] = {
    /* 1. ap-cp at blk */
    {
        APCP_AT_ABLK_NAME,
        AIC_SYS_CP, /* sys */
        AIPC_BRR_AT, /* bearer id */
        APCP_AT_POOL_CNT, /* pool cnt */
        APCP_AT_LINK_CNT, /* link cnt */
        ABLK_NORMAL, /* type */
        ABLK_MAXNUM_ALLOC_RANGE, /* alloc range */
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
        ABLK_MININUM_ALLOC_RANGE, /* alloc range */
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
        ABLK_DEFAULT_ALLOC_RANGE, /* alloc range */
        apcp_comm_pool_cnt, /* pool cnt arrary */
        apcp_comm_pool_size, /* pool size arrary */
        apcp_comm_link_cfg, /* link cfg */
    },
    /* 4. ap-ap debug blk, for ap log. */
    {
        APAP_DEBUG_ABLK_NAME,
        AIC_SYS_AP, /* sys */
        AIPC_BRR_DEBUG, /* bearer id */
        APAP_DEBUG_POOL_CNT, /* pool cnt */
        APAP_DEBUG_LINK_CNT, /* link cnt */
        ABLK_SLNORMAL| ABLK_LOCAL_MEM, /* type */
        ABLK_DEFAULT_ALLOC_RANGE, /* alloc range */
        apap_debug_pool_cnt, /* pool cnt arrary */
        apap_debug_pool_size, /* pool size arrary */
        apap_debug_link_cfg, /* link cfg */
    },
};
#endif /* __AIPC_CUSTOMER_CFG_H__ */

