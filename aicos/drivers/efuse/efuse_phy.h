/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of efuse module.
 *   It includes all the efuse phy function api.
*/
#ifndef __EFUSE_PHY_H__
#define __EFUSE_PHY_H__
#include <aic_bitmap.h>

#define EFUSE_BLK_NUMS      64
#define EFUSE_BLK_LOCK_BIT      31
#define EFUSE_ENOB_LENTH        31
//#define EFUSE_CV_DEBUG
#define EFUSE_INT_WR_FINISH
#define EFUSE_WRITE_MAGIC       0X12EF
#define EFUSE_WAIT_TIME_CNT     100
#define AIC_CACHE_ENABLED       TRUE
#define AIC_AUTO_PD_ENABLED       TRUE

typedef enum {
    AIC_EFUSE_READ_FINISH = 1,
    AIC_EFUSE_WRITE_FINISH = 2,
    AIC_EFUSE_ALL01_CHECK = 4,
    AIC_EFUSE_EXCEPTION = 8,
    AIC_EFUSE_ALL = 0xF
} AIC_EFUSE_INT;

typedef enum {
    AIC_EFUSE_EXC_NONE = 0,
    AIC_EFUSE_ACCESS_SIP_ILLEGAL = BIT(21),
    AIC_EFUSE_ACCESS_SECURITY_ILLEGAL = BIT(22),
    AIC_EFUSE_WRITE_LOCKED_BLOCK = BIT(23),
    AIC_EFUSE_WRITE_WITH_WRONG_MAGIC_DATA = BIT(24)
} AIC_EFUSE_EXC;

typedef enum {
    AIC_EFUSE_INT_S_READ_FINISH = BIT(0),
    AIC_EFUSE_INT_S_WRITE_FINISH = BIT(1),
    AIC_EFUSE_INT_S_ALL01_CHECK_FINISH  = BIT(2),
    AIC_EFUSE_INT_S_ALL01_CHECK_FINISH_PASS_RAW  = BIT(3),
    AIC_EFUSE_INT_S_ALL01_CHECK_FINISH_PASS_ECC  = BIT(4),
    AIC_EFUSE_INT_S_APB_ACCESS_SIP_ILLEGAL =  BIT(21),
    AIC_EFUSE_INT_S_APB_ACCESS_SECURITY_ILLEGAL = BIT(22),
    AIC_EFUSE_INT_S_APB_WRITE_LOCKED_BLOCK = BIT(23),
    AIC_EFUSE_INT_S_WRITE_WITH_WRONG_MAGIC_DATA = BIT(24)
} AIC_EFUSE_INT_MASK;

typedef enum {
    AIC_SEC_IP_BLK = 0,
    AIC_SECURITY_BLK = 1,
    AIC_GENERAL_BLK = 2
}EFUSE_BLK_PROPERTY;

struct efuse_blk_info
{
    u32 blk_id;
    u32 blk_data_read;
    u32 blk_data_written;
    int blk_property;
    int written;
    int write_cnt;
    int locked;
};

/*efuse phy driver interface*/
void aic_efuse_phy_probe(void);
void aic_efuse_phy_remove(void);
int aic_efuse_write(int blk, u32 data);
int aic_efuse_read(int blk,u32 *data);
void aic_efuse_blk_property_scan(struct efuse_blk_info *efuse_fields);

#endif /* __EFUSE_PHY_H__ */
