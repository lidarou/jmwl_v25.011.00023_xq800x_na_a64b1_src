
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file contains all aipc module config data of cp system.
 *    It contains bellow two parts:
 *      aipc_cfg: Contains all aipc configurations of this system communicating
 *                with other systems.
 *      ablk_cfg: Contains all ablk configurations of this system communicating
 *                with other systems.
 */
#ifndef __AIC_DEFAULT_CFG_H__
#define __AIC_DEFAULT_CFG_H__
#include <aic_type.h>
#include <aic_size.h>

/******************************************************************************
 *                    aipc configuration section.
 ******************************************************************************/
#define AMEM_NORMAL_SIZE    0x0100000
#define AMEM_INTERNAL_SIZE  0x002000

static u32 amem_normal[AMEM_NORMAL_SIZE/sizeof(u32)];
static u32 amem_internal[AMEM_INTERNAL_SIZE/sizeof(u32)];

#define AMEM_NORMAL_BASE      (u32)&amem_normal[0]
#define AMEM_INTERNAL_BASE    (u32)&amem_internal[0]

/**
 *aipc_cfg - used the table to create aipc instacnce.
 */
static const AIPC_CFG_T aipc_cfg[] = {
    /* cp <--> ap ipc */
    {
        AIC_SYS_AP, /* remote sys is ap */
        1, /* is client sys */
        1, /* task pri */
        /* normal share memory */
        {AMEM_NORMAL_BASE, AMEM_NORMAL_SIZE, AMEM_256B_SHIFT},
        /* internal share memory */
        {AMEM_INTERNAL_BASE, AMEM_INTERNAL_SIZE, AMEM_16B_SHIFT},
        "cp_ap" /* name */
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
    },
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
 *                       cp-ap ablk pool and link cfg
 ******************************************************************************/
/* 1. cp ==> ap ,AT commd pool */
#define CPAP_AT_ABLK_NAME   "at"
#define CPAP_AT_POOL_CNT    2
static const u16 cpap_at_pool_cnt[CPAP_AT_POOL_CNT] = {24, 4,};
static const aipc_size_t cpap_at_pool_size[CPAP_AT_POOL_CNT] = {32, 256};
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
#define CPAP_IP_ABLK_NAME   "ip"
#define CPAP_IP_POOL_CNT    1
#define CPAP_IP_LINK_CNT   16

static const u16 cpap_ip_pool_cnt[CPAP_IP_POOL_CNT] = {30};
static const aipc_size_t cpap_ip_pool_size[CPAP_IP_POOL_CNT] = {1600};

/* 3. cp ==> ap, commn, such as nv, cp status */
#define CPAP_COMM_ABLK_NAME "comm"
#define CPAP_COMM_POOL_CNT  3

static const u16 cpap_comm_pool_cnt[CPAP_COMM_POOL_CNT] = {8, 4, 28};
static const aipc_size_t cpap_comm_pool_size[CPAP_COMM_POOL_CNT] = {256, 2048, 4096};
static const ABLK_LINK_CFG_T cpap_comm_link_cfg[] = {
    {ALTRACE_LINK, LINK_NORMAL, 10},
    {ALMSG_LINK,   LINK_NORMAL, 20},
    {ALDATA_LINK,  LINK_NORMAL, 10},
    {STATUS_LINK, LINK_NORMAL, 4},
    {ADIAG_LINK, LINK_NORMAL, 6},
    {RMSG_LINK, LINK_NORMAL, 10}
};

#define CPAP_COMM_LINK_CNT  ARRAY_SIZE(cpap_comm_link_cfg)

/******************************************************************************
 *                    ablk_cfg configuration section.
 ******************************************************************************/
/**
 *ablk_cfg - all the ablk config in current system.
 */
static const ABLK_CFG_T ablk_cfg[] = {
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
        ABLK_MININUM_ALLOC_RANGE, /* alloc range */
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
        ABLK_DEFAULT_ALLOC_RANGE, /* alloc range */
        cpap_comm_pool_cnt, /* pool cnt arrary */
        cpap_comm_pool_size, /* pool size arrary */
        cpap_comm_link_cfg, /* link cfg */
    },
};
#endif /* __AIC_DEFAULT_CFG_H__ */

