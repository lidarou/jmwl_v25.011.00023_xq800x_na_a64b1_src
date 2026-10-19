/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aic cmdline Interface.
 */

#ifndef __AIC_CMDLINE_H__
#define __AIC_CMDLINE_H__

#include <aic_type.h>

/*
 * All the key must define at here.
 * if you want add a key, must add here and
 * add it in the key array s_cmd_key.
 */
typedef enum {
    cl_key_gxem = 0,
    cl_key_32kless,
    cl_key_boardid,
    cl_key_crystal,
    cl_key_bootcnt,
    cl_key_psramid,
    cl_key_bootmode,
    cl_key_usermode,
    cl_key_pwr_sw,
    cl_key_pwr_on,
    cl_key_pwr_off,
    cl_key_dev_type,
    cl_key_dev_com,
    cl_key_dev_baud,
    cl_key_asrt_info,
    cl_key_nv_info,
    cl_key_nvcust_info,
    cl_key_nvdiff_info,
    cl_key_nvcali_info,
    cl_key_nvnorm_info,
    cl_key_snor_cs_num,
    cl_key_snor0_minclk,
    cl_key_snor0_maxclk,
    cl_key_snor1_minclk,
    cl_key_snor1_maxclk,
    cl_key_pmic_version,
    cl_key_vco_val,
    cl_key_dcxo,
    cl_key_cnt
} cl_key_t;

#define KEY_GXEM        "gxem"
#define KEY_32KLESS     "32kl"
#define KEY_BOARDID     "board"
#define KEY_CRYSTAL     "crys"
#define KEY_BOOT_CNT    "bootcnt"
#define KEY_PSRAMID     "psramid"
#define KEY_BOOTMODE    "bootmode"
#define KEY_USERMODE    "usermode"
#define KEY_PWR_SW      "pwr_sw"
#define KEY_PWR_ON      "pwr_on"
#define KEY_PWR_OFF     "pwr_off"
#define KEY_DEV_TYPE    "dev_type"
#define KEY_DEV_COM     "dev_com"
#define KEY_DEV_BAUD    "dev_baud"
#define KEY_ASRT_INFO   "asrt_info"
#define KEY_NV_INFO     "nv"
#define KEY_NVCUST_INFO "nvcust"
#define KEY_NVDIFF_INFO "nvdiff"
#define KEY_NVCALI_INFO "nvcali"
#define KEY_NVNORM_INFO "nvnorm"
#define KEY_SNOR_CS_NUM "snor_csnum"
#define KEY_SNOR0_MINCLK "snor0_minclk"
#define KEY_SNOR0_MAXCLK "snor0_maxclk"
#define KEY_SNOR1_MINCLK "snor1_minclk"
#define KEY_SNOR1_MAXCLK "snor1_maxclk"
#define KEY_PMIC_ID     "pmic_id"
#define KEY_PLL_VCO     "pll_vco"
#define KEY_DCXO_TYPE   "dcxo_type"

/*
 * aic_cmdline_get_value - get a value string of the key.
 *
 * @key: the key define in cl_key_t.
 *
 * @return: the value string pointer.
 */
char *aic_cmdline_get_value(cl_key_t key);

/*
 * aic_cmdline_add_key - Add a cmdline key.
 *
 * @key: the key define in cl_key_t.
 * @value: the value string.
 *
 */
void aic_cmdline_add_key(cl_key_t key, char *value);

/*
 * aic_cmdline_update_key - update a cmdline key,
 * the long of new value can not longer than the long of the old value.
 * if this key is not exist, as same as aic_cmdline_add_key.
 *
 * @key: the key define in cl_key_t.
 * @value: the value string.
 *
 */
void aic_cmdline_update_key(cl_key_t key, char *value);

/*
 * aic_cmdline_add_key_int - Add a cmdline key. will convert the int to string.
 *
 * @key: the key define in cl_key_t.
 * @value: the value string.
 *
 */
void aic_cmdline_add_key_int(cl_key_t key, int value);

/*
 * aic_cmdline_get_value_int - get a value string of the key. will convert the
 * string to int.
 *
 * @key: the key define in cl_key_t.
 * @value: the int value.
 *
 */
int aic_cmdline_get_value_int(cl_key_t key);

/*
 * aic_cmdline_add_assert_info - Add last ap assert info in cmdline.
 *
 * @key: the key define in cl_key_t.
 * @value: the int value.
 *
 */
void aic_cmdline_add_assert_info(void);

/*
 * aic_cmdline_parse - parse all the cmdline, only can be called in system init.
 *
 */
void aic_cmdline_parse(void);

#endif /* __AIC_CMDLINE_H__ */

