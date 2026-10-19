/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the memory layout file of the aon ram.
 *    Every region sub region be described like this:
      region name | (base, size)
 */

#ifndef __AONRAM_MEM_CONFIG_H__
#define __AONRAM_MEM_CONFIG_H__
/***********************************************************
             aon ram layout: (0x20000000, 0x00048000)
***********************************************************
------------------------------------------------------------
            cp boot sleep | (0x20000000, 0x00001000)
--------------------------------------------------------
          ap vector sleep | (0x20001000, 0x00001000)
--------------------------------------------------------
             ap sys stack | (0x20002000, 0x00002000)
--------------------------------------------------------
         AIPC share meory | (0x20004000, 0x00008000)
--------------------------------------------------------
  phy buffer/offline dump | (0x2000C000, 0x0003c000)
------------------------------------------------------------
************************************************************/

/*******************************************************
                  ALL aon ram region
********************************************************/
#define AONRAM_REGION_BASE      0x20000000
#define AONRAM_REGION_SIZE      0x00048000

/*******************************************************
                  Cp vctor region
********************************************************/
#define CP_VECTOR_REGION_BASE   AONRAM_REGION_BASE
#define CP_VECTOR_REGION_SIZE   0x00001000

/*******************************************************
                  ap vector memory region
********************************************************/
#define AP_VECTOR_REGION_BASE   (CP_VECTOR_REGION_BASE + CP_VECTOR_REGION_SIZE)
#define AP_VECTOR_REGION_SIZE   0x00002000

/*******************************************************
                  aon ap sys stack memory region
********************************************************/
#define AP_SYSSTACK_REGION_BASE (AP_VECTOR_REGION_BASE + AP_VECTOR_REGION_SIZE)
#define AP_SYSSTACK_REGION_SIZE 0x00001000

/*******************************************************
                  aon aipc shere memory region
********************************************************/
#define AON_SHMEM_REGION_BASE   (AP_SYSSTACK_REGION_BASE + AP_SYSSTACK_REGION_SIZE)
#define AON_SHMEM_REGION_SIZE   0x00008000

/*******************************************************
                  phy buffer region
********************************************************/
#define PHY_BUF_REGION_BASE     (AON_SHMEM_REGION_BASE + AON_SHMEM_REGION_SIZE)
#define PHY_BUF_REGION_SIZE     0x3c000

/***********************************************************
            aipc aon share memory layout:(0x20004000, 0x00008000)
--------------------------------------------------------
            ap --> cp | (0x20004000, 0x00001000)
--------------------------------------------------------
            cp --> ap | (0x20005000, 0x00001000)
--------------------------------------------------------
 reserve(cp has used) | (0x20006000, 0x00006000)
--------------------------------------------------------
************************************************************/
#define AON_AP2CP_BASE          AON_SHMEM_REGION_BASE
#define AON_AP2CP_SIZE          0x00001000

#define AON_CP2AP_BASE          (AON_AP2CP_BASE + AON_AP2CP_SIZE)
#define AON_CP2AP_SIZE          0x00001000

#define AON_RESERVE_BASE        (AON_CP2AP_BASE + AON_CP2AP_SIZE)
#define AON_RESERVE_SIZE        0x00006000

#endif /* __AONRAM_MEM_CONFIG_H__ */

