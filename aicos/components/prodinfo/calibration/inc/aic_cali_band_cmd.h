/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file export the calibration band base operation struct and interface.
 **---------------------------------------------------------------------------*/

#ifndef __AIC_CALI_CMD_H__
#define __AIC_CALI_CMD_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_bitmap.h>
#include <aic_list.h>

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define LTE_CALI_BAND_INDEX_NUM                 20

/* BIT0 - BIT3 for AGC */
#define LTE_CALI_AGC_REF_FLAG                   BIT_0
#define LTE_CALI_AGC_COMP_FLAG                  BIT_1
#define LTE_CALI_IIP2_FLAG                      BIT_2   /* only FDD do. */
#define LTE_CALI_AGC_REF_CUSTOM_FLAG            BIT_3   /* only FDD do. */

/* BIT4 - BIT7 for AGC */
#define LTE_CALI_APC_REF_FLAG                   BIT_4
#define LTE_CALI_APC_COMP_FLAG                  BIT_5

/* BIT8 - BIT11 for NST */
#define LTE_NST_FLAG                            BIT_8

#define LTE_CALI_PASS_FLAG                      (LTE_CALI_AGC_REF_FLAG | \
                                                 LTE_CALI_AGC_COMP_FLAG | \
                                                 LTE_CALI_APC_REF_FLAG | \
                                                 LTE_CALI_APC_COMP_FLAG)

#define LTE_NST_PASS_FLAG                       (LTE_CALI_PASS_FLAG | \
                                                 LTE_NST_FLAG)

#define HW_ID_NAME_LEN                          22
#define RF_ID_NAME_LEN                          32

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
struct lte_cali_band_info {
    u8  band_num;
    u8  coband0_num;
    u8  coband1_num;
    u8  coband2_num;
    u8  coband3_num;
    u16 cali_flag;
    u8  reserved;
};

struct lte_cali_afc_info {
    u16 cali_flag;
    u16 cali_freq;
    u16 ct;
    u16 afc_cal;
    u16 hpm_32k;
    u16 lpm_32k;
};

struct lte_cali_time_info {
    u16 year;
    u8 mon;
    u8 day;
    u8 hour;
    u8 min;
    u8 sec;
};

typedef enum {
    BAND_DIAG_READ_SUBCMD = 0,
    AFC_DIAG_READ_SUBCMD,
    RFID_DIAG_READ_SUBCMD,
    HWDIFF_DIAG_READ_SUBCMD,
    CALITIME_DIAG_READ_SUBCMD,
    TOOLVER_DIAG_READ_SUBCMD,
} LTE_BAND_CALI_DIAG_SUBCMD;

struct hw_diff_info {
    u8 hwid_name[HW_ID_NAME_LEN];
    u8 rfid_name[RF_ID_NAME_LEN];
    struct list_head node;
};

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_band_is_cali - band is calibration or not.
 *
 * Parameter:
 *  @band_num: input band number.
 *
 * Return:
 */
boolean aic_band_is_cali(u8 band_num);

/**
 * aic_caliband_get - get all calibration band.
 *
 * Parameter:
 *  @band_num: input to store band number ptr.
 *  @count: input store band number count.
 *
 * Return:
 */
void aic_caliband_get(u8 *band_num, u8 count);

/**
 * aic_band_is_nst - band is nst pass or not.
 *
 * Parameter:
 *  @band_num: input band number.
 *
 * Return:
 */
boolean aic_band_is_nst(u8 band_num);

/**
 * aic_nstband_get - get all nst pass band.
 *
 * Parameter:
 *  @band_num: input to store band number ptr.
 *  @count: input store band number count.
 *
 * Return:
 */
void aic_nstband_get(u8 *band_num, u8 count);

/**
 * aic_afc_is_cali - afc is calibration or not.
 *
 * Parameter:
 *
 * Return:
 */
boolean aic_afc_is_cali(void);

/**
 * aic_afc_caliinfo_get - get afc calibration information.
 *
 * Parameter:
 *  @afc_info: input afc information ptr.
 *
 * Return:
 */
void aic_afc_caliinfo_get(struct lte_cali_afc_info *afc_info);

/**
 * aic_rfid_name_get - get rfid name.
 *
 * Parameter:
 *  @hwid_name: input hwid name ptr, length = HW_ID_NAME_LEN.
 *  @rfid_name: input rfid name ptr, length = RF_ID_NAME_LEN;
 *
 * Return:
 */
void aic_rfid_name_get(char *hwid_name, char *rfid_name);

/**
 * aic_calitime_get - get calibration time and toolversion.
 *
 * Parameter:
 *  @calitime: input calibration time ptr.
 *  @toolversion: input to store calibration version ptr.
 *
 * Return:
 */
void aic_calitime_get(struct lte_cali_time_info *calitime, u16 *toolversion);

/**
 * aic_caliband_at_get - process at+CALIBAND?.
 *
 * Parameter:
 *  @rsp_str: at response.
 *  @rsp_str: response length.
 *
 * Return:
 *  AIC_SUCCESS is success, other fail.
 */
u32 aic_caliband_at_get(char *rsp_str, u32 rsp_len);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_CALI_CMD_H__ */

