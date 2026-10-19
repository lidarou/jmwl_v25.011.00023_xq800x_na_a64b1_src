/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file process calibation band command.
 */

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b0275
#define LOG_MODULE    AMOD_PRODINFO

#include <aic_type.h>
#include <diag_cmd.h>
#include <diag_cmd_api.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_math.h>
#include <stdio.h>
#include <aic_list.h>
#include <RF_CalTool_Parameter.h>
#include <RF_COMMON_CALIBRATION.h>
#include <RF_COMMON_DOWNLOADER.h>
#include <RF_LTE_CALIBRATION_BandIndex_00.h>
#include <RF_LTE_CALIBRATION_BandIndex_19.h>
#include <RF_LTE_DOWNLOADER_BandIndex_00.h>
#include <aic_nvram_api.h>
#include <aic_nvram_id.h>
#include "aic_cali_band_cmd.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 * Static Global Variables
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/
/**
 * aic_band_info_init - band information init
 *
 * Parameter:
 *  @band_info: input calibration band information.
 *
 * Return:
 */
void aic_band_info_init(struct lte_cali_band_info *band_info)
{
    u16 cali_item_size;
    RF_COMMON_DOWNLOADER *down_com;
    RF_LTE_DOWNLOADER *down_band;
    u8 *cali_band;
    u32 bandmap, i;
    int status;

    status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_DOWNLOADER,
                                sizeof(RF_COMMON_DOWNLOADER),
                                (void **)&down_com);
    AIC_ASSERT(status == AIC_SUCCESS);

    bandmap = down_com->Bandindex_Map &
              ((1 << LTE_CALI_BAND_INDEX_NUM) - 1);
    if (bandmap) {
        for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
            if (!(bandmap & (1 << i)))
                continue;

            status = aic_nvram_addr_get(NVRAM_ID_RF_LTE_DOWNLOADER_BANDINDEX_00 + i,
                                        sizeof(RF_LTE_DOWNLOADER),
                                        (void **)&down_band);
            AIC_ASSERT(status == AIC_SUCCESS);
            band_info[i].band_num = down_band->ltebandinfo.band.BandNum;
            band_info[i].coband0_num = down_band->ltebandinfo.coband0.BandNum;
            band_info[i].coband1_num = down_band->ltebandinfo.coband1.BandNum;
            band_info[i].coband2_num = down_band->ltebandinfo.coband2.BandNum;
            band_info[i].coband3_num = down_band->ltebandinfo.coband3.BandNum;

            if (i == (LTE_CALI_BAND_INDEX_NUM - 1)) {
                cali_item_size = sizeof(RF_LTE_CALIBRATION_19);
            } else {
                cali_item_size = sizeof(RF_LTE_CALIBRATION);
            }

            status = aic_nvram_addr_get(NVRAM_ID_RF_LTE_CALIBRATION_BANDINDEX_00 + i,
                                        cali_item_size, (void **)&cali_band);
            AIC_ASSERT(status == AIC_SUCCESS);


            if (i == (LTE_CALI_BAND_INDEX_NUM - 1)) {
                band_info[i].cali_flag = ((RF_LTE_CALIBRATION_19 *)cali_band)->LteCal_Flag[0];
            } else {
                band_info[i].cali_flag = ((RF_LTE_CALIBRATION *)cali_band)->LteCal_Flag[0];
            }
        }
    }
}

/**
 * aic_afc_caliinfo_get - get afc calibration information.
 *
 * Parameter:
 *  @afc_info: input afc information ptr.
 *
 * Return:
 */
void aic_afc_caliinfo_get(struct lte_cali_afc_info *afc_info)
{
    RF_COMMON_CALIBRATION *cali_com;
    int status;

    status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_CALIBRATION,
                                sizeof(RF_COMMON_CALIBRATION),
                                (void **)&cali_com);
    AIC_ASSERT(status == AIC_SUCCESS);

    afc_info->cali_flag = cali_com->AfcCalFlag;
    afc_info->cali_freq = cali_com->afccal.CalFreq;
    afc_info->ct = cali_com->afccal.CT;
    afc_info->afc_cal = cali_com->afccal.AFC_Cal;
    afc_info->hpm_32k = cali_com->afccal.HPM_32K;
    afc_info->lpm_32k = cali_com->afccal.LPM_32K;
}

/**
 * aic_rfid_name_get - get rfid name.
 *
 * Parameter:
 *  @hwid_name: input hwid name ptr, length = HW_ID_NAME_LEN.
 *  @rfid_name: input rfid name ptr, length = RF_ID_NAME_LEN;
 *
 * Return:
 */
void aic_rfid_name_get(char *hwid_name, char *rfid_name)
{
    RF_COMMON_DOWNLOADER *down_com;
    int status, hw_id;
    size_t size;

    hw_id = nvram_hw_id_get();
    snprintf(hwid_name, HW_ID_NAME_LEN, "hw_%02d", hw_id);

    status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_DOWNLOADER,
                                sizeof(RF_COMMON_DOWNLOADER),
                                (void **)&down_com);
    AIC_ASSERT(status == AIC_SUCCESS);

    size = strlen(down_com->RFID_Name);
    if (size) {
        aos_memcpy(rfid_name, down_com->RFID_Name, size);
    }
}

/**
 * aic_afc_is_cali - afc is calibration or not.
 *
 * Parameter:
 *
 * Return:
 */
boolean aic_afc_is_cali(void)
{
    struct lte_cali_afc_info afc_info;

    aic_afc_caliinfo_get(&afc_info);

    if (afc_info.cali_flag)
        return true;

    return false;
}

/**
 * aic_band_is_cali - band is calibration or not.
 *
 * Parameter:
 *  @band_num: input band number.
 *
 * Return:
 */
boolean aic_band_is_cali(u8 band_num)
{
    struct lte_cali_band_info *band_info;
    boolean cali_flag = false;
    size_t size;
    u8 i;

    size = sizeof(struct lte_cali_band_info) * LTE_CALI_BAND_INDEX_NUM;
    band_info = aos_zalloc(size);
    AIC_ASSERT(band_info);

    aic_band_info_init(band_info);

    for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
        if ((band_info[i].band_num == band_num) ||
            (band_info[i].coband0_num == band_num) ||
            (band_info[i].coband1_num == band_num) ||
            (band_info[i].coband2_num == band_num) ||
            (band_info[i].coband3_num == band_num)) {
                if ((band_info[i].cali_flag & LTE_CALI_PASS_FLAG) == LTE_CALI_PASS_FLAG) {
                    cali_flag = true;
                }
                break;
            }
    }

    aos_free(band_info);

    return cali_flag;
}

/**
 * aic_calitime_get - get calibration time and toolversion.
 *
 * Parameter:
 *  @calitime: input calibration time ptr.
 *  @toolversion: input to store calibration version ptr.
 *
 * Return:
 */
void aic_calitime_get(struct lte_cali_time_info *calitime, u16 *toolversion)
{
    RF_COMMON_CALIBRATION *cali_com;
    aos_status status;

    status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_CALIBRATION,
                                sizeof(RF_COMMON_CALIBRATION),
                                (void **)&cali_com);
    AIC_ASSERT(status == AIC_SUCCESS);
    aos_memcpy(calitime, &cali_com->cali_time, sizeof(struct lte_cali_time_info));

    *toolversion = cali_com->CalToolVersion;
}

/**
 * aic_caliband_get - get all calibration band.
 *
 * Parameter:
 *  @band_num: input to store band number ptr.
 *  @count: input store band number count.
 *
 * Return:
 */
void aic_caliband_get(u8 *band_num, u8 count)
{
    struct lte_cali_band_info *band_info;
    u8 i, j;

    band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                           LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(band_info);

    aic_band_info_init(band_info);

    aos_memset(band_num, 0, count);

    for (i = 0, j = 0; (i < LTE_CALI_BAND_INDEX_NUM) && (j < count); i++) {
        if ((band_info[i].cali_flag & LTE_CALI_PASS_FLAG) == LTE_CALI_PASS_FLAG) {
            band_num[j++] = band_info[i].band_num;
            if (j >= count)
                break;

            if (band_info[i].coband0_num) {
                band_num[j++] = band_info[i].coband0_num;
                if (j >= count)
                    break;
            }

            if (band_info[i].coband1_num) {
                band_num[j++] = band_info[i].coband1_num;
                if (j >= count)
                    break;
            }

            if (band_info[i].coband2_num) {
                band_num[j++] = band_info[i].coband2_num;
                if (j >= count)
                    break;
            }

            if (band_info[i].coband3_num) {
                band_num[j++] = band_info[i].coband3_num;
                if (j >= count)
                    break;
                }
        }
    }

    aos_free(band_info);
}

/**
 * aic_band_is_nst - band is nst pass or not.
 *
 * Parameter:
 *  @band_num: input band number.
 *
 * Return:
 */
boolean aic_band_is_nst(u8 band_num)
{
    struct lte_cali_band_info *band_info;
    boolean cali_flag = false;
    size_t size;
    u8 i;

    size = sizeof(struct lte_cali_band_info) * LTE_CALI_BAND_INDEX_NUM;
    band_info = aos_zalloc(size);
    AIC_ASSERT(band_info);

    aic_band_info_init(band_info);

    for (i = 0; i < LTE_CALI_BAND_INDEX_NUM; i++) {
        if ((band_info[i].band_num == band_num) ||
            (band_info[i].coband0_num == band_num) ||
            (band_info[i].coband1_num == band_num) ||
            (band_info[i].coband2_num == band_num) ||
            (band_info[i].coband3_num == band_num)) {
                if ((band_info[i].cali_flag & LTE_NST_PASS_FLAG) == LTE_NST_PASS_FLAG) {
                    cali_flag = true;
                }
                break;
            }
    }

    aos_free(band_info);

    return cali_flag;
}

/**
 * aic_nstband_get - get all nst pass band.
 *
 * Parameter:
 *  @band_num: input to store band number ptr.
 *  @count: input store band number count.
 *
 * Return:
 */
void aic_nstband_get(u8 *band_num, u8 count)
{
    struct lte_cali_band_info *band_info;
    u8 i, j;

    band_info = aos_zalloc(sizeof(struct lte_cali_band_info) *
                           LTE_CALI_BAND_INDEX_NUM);
    AIC_ASSERT(band_info);

    aic_band_info_init(band_info);

    aos_memset(band_num, 0, count);

    for (i = 0, j = 0; (i < LTE_CALI_BAND_INDEX_NUM) && (j < count); i++) {
        if ((band_info[i].cali_flag & LTE_NST_PASS_FLAG) == LTE_NST_PASS_FLAG) {
            band_num[j++] = band_info[i].band_num;
            if (j >= count)
                break;

            if (band_info[i].coband0_num) {
                band_num[j++] = band_info[i].coband0_num;
                if (j >= count)
                    break;
            }

            if (band_info[i].coband1_num) {
                band_num[j++] = band_info[i].coband1_num;
                if (j >= count)
                    break;
            }

            if (band_info[i].coband2_num) {
                band_num[j++] = band_info[i].coband2_num;
                if (j >= count)
                    break;
            }

            if (band_info[i].coband3_num) {
                band_num[j++] = band_info[i].coband3_num;
                if (j >= count)
                    break;
                }
        }
    }

    aos_free(band_info);
}

/**
 * aic_band_diag_handle - calibration band diag handle.
 *
 * Parameter:
 * @diag_cmd: receive diag parameter
 * @pri: diag handle private parameter.
 *
 * Return:
 */
void aic_band_diag_handle(diag_send_cmd_t *diag_cmd, void *pri)
{
    struct lte_cali_band_info *band_info;
    struct lte_cali_afc_info afc_info;
    RF_COMMON_CALIBRATION *cali_com;
    RF_COMMON_DOWNLOADER *down_com;
    diag_rsp_cmd_t rsp;
    int status, hw_id;
    size_t size;
    u8 *ptr;

    switch (diag_cmd->subcmd) {
        case BAND_DIAG_READ_SUBCMD:
            {
                size = sizeof(struct lte_cali_band_info) * LTE_CALI_BAND_INDEX_NUM;
                band_info = aos_zalloc(size);
                AIC_ASSERT(band_info);

                aic_band_info_init(band_info);

                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                             diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                             diag_cmd->subcmd, band_info, size);
                diag_cmd_response(&rsp);

                aos_free(band_info);

                break;
            }
        case AFC_DIAG_READ_SUBCMD:
            {
                aic_afc_caliinfo_get(&afc_info);
                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                             diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                             diag_cmd->subcmd, &afc_info,
                             sizeof(struct lte_cali_afc_info));
                diag_cmd_response(&rsp);
                break;
            }
        case RFID_DIAG_READ_SUBCMD:
            {
                hw_id = nvram_hw_id_get();

                status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_DOWNLOADER,
                                            sizeof(RF_COMMON_DOWNLOADER),
                                            (void **)&down_com);
                AIC_ASSERT(status == AIC_SUCCESS);

                size = strlen(down_com->RFID_Name);
                if (size == 0) {
                    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                                diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                                diag_cmd->subcmd, &hw_id, 1);
                    diag_cmd_response(&rsp);
                } else {
                    ptr = aos_zalloc(1 + size);
                    AIC_ASSERT(ptr);

                    *ptr = hw_id;
                    aos_memcpy(ptr + 1, down_com->RFID_Name, size);
                    diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                                diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                                diag_cmd->subcmd, ptr, size + 1);
                    diag_cmd_response(&rsp);

                    aos_free(ptr);
                }
                break;
            }
        case CALITIME_DIAG_READ_SUBCMD:
            {
                status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_CALIBRATION,
                                            sizeof(RF_COMMON_CALIBRATION),
                                            (void **)&cali_com);
                AIC_ASSERT(status == AIC_SUCCESS);

                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                            diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                            diag_cmd->subcmd, (u8 *)&cali_com->cali_time,
                            sizeof(struct lte_cali_time_info));
                diag_cmd_response(&rsp);
                break;
            }
        case TOOLVER_DIAG_READ_SUBCMD:
            {
                status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_CALIBRATION,
                                            sizeof(RF_COMMON_CALIBRATION),
                                            (void **)&cali_com);
                AIC_ASSERT(status == AIC_SUCCESS);

                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                            diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                            diag_cmd->subcmd, (u8 *)&cali_com->CalToolVersion,
                            sizeof(cali_com->CalToolVersion));
                diag_cmd_response(&rsp);
                break;
            }
        case HWDIFF_DIAG_READ_SUBCMD:
            {
                struct list_head *pos, *next, *list = NULL;
                struct hw_diff_info *diff_info;
                u32 cnt, offset;

                status = aic_nvram_addr_get(NVRAM_ID_RF_COMMON_DOWNLOADER,
                                            sizeof(RF_COMMON_DOWNLOADER),
                                            (void **)&down_com);
                AIC_ASSERT(status == AIC_SUCCESS);

                // size = strlen(down_com->RFID_Name);

                nvram_diff_list_get(&list);

                cnt = 0;
                list_for_each_safe(pos, next, list) {
                    cnt++;
                }

                size = (HW_ID_NAME_LEN + RF_ID_NAME_LEN) * cnt;
                ptr = aos_zalloc(size + 4);
                AIC_ASSERT(ptr);

                offset = 0;
                list_for_each_prev(pos, list) {
                    diff_info = list_entry(pos, struct hw_diff_info, node);
                    aos_memcpy(ptr + offset, diff_info->hwid_name,
                        strlen(diff_info->hwid_name));
                    offset += HW_ID_NAME_LEN;
                    if (strlen(diff_info->rfid_name)) {
                        aos_memcpy(ptr + offset, diff_info->rfid_name,
                            strlen(diff_info->rfid_name));
                    } else if (strlen(down_com->RFID_Name)) {
                        aos_memcpy(ptr + offset, down_com->RFID_Name,
                            strlen(down_com->RFID_Name));
                    }
                    offset += RF_ID_NAME_LEN;
                }

                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_SUCC,
                                diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                                diag_cmd->subcmd, ptr, size);
                diag_cmd_response(&rsp);

                aos_free(ptr);
                break;
            }
        default:
            {
                diag_set_rsp(&rsp, diag_cmd->sn, DIAG_RSP_NO_FOUND_SUBCMD,
                             diag_cmd->m2, diag_cmd->sys, DIAG_CMD_BAND,
                             diag_cmd->subcmd, NULL, 0);
                diag_cmd_response(&rsp);
                break;
            }
    }
}

/**
 * aic_band_diag_cmd_init - band diag cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_band_diag_cmd_init(void)
{
    diag_register_cmd_handler(DIAG_CMD_BAND, aic_band_diag_handle, NULL);

    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P2_INIT_EXPORT(aic_band_diag_cmd_init);

