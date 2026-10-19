/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef __SD_H__
#define __SD_H__

#include <aic_type.h>
#include <sdmmc_host.h>
#include <sdmmc_card.h>

#ifdef __cplusplus
extern "C" {
#endif

//int sdmmc_send_if_cond(sdmmc_host *host, uint32_t ocr);
int sdmmc_send_app_op_cond(sdmmc_host *host,          uint32_t ocr, uint32_t *rocr);
int init_sd(sdmmc_host *host, uint32_t ocr, boolean re_init);
int32_t sdmmc_dev_add(sdmmc_card *card);
int32_t sdmmc_dev_remove(sdmmc_card *card);
//void sdmmc_core_powerup(sdmmc_host *host, boolean re_init);

#ifdef __cplusplus
}
#endif

#endif