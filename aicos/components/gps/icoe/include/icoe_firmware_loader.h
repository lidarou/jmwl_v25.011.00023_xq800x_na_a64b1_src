/************************************************************************ 
* Copyright © 2022, ICOE (Shanghai) Technologies Co.,Ltd. All rights reserved.              *
* Information in this document is provided in connection with ICOE’s  products.             *
* No license, express or implied, by estoppels or otherwise, to any intellectual property   *
* rights is granted by this document. Except as  provided in ICOE’s Terms and               *
* Conditions of Sale for such products, ICOE assumes no liability whatsoever, and           *
* ICOE disclaims any express or implied warranty, relating to sale and/or use of            *
* ICOE products including liability or warranties relating to fitness for a particular      *
* purpose, merchantability, or infringement of any patent, copyright or other               *
* intellectual property right. ICOE products are not intended for use in    medical, life   *
* saving, or life sustaining applications.                                                  * 
* ICOE (Shanghai) Technologies Co.,Ltd may make changes to specifications and               * 
* product descriptions at any time, without notice.                                         * 
* You may obtain copies of documents which have a document number and are                   *
* referenced in this document, or other ICOE literature may be obtained by                  * 
* mailing to xiaopingjiang@icoe-tech.com.                                                                                                       *
************************************************************************/ 

#ifndef _FIRMWARE_LOADER_
#define _FIRMWARE_LOADER_

#include "icoe_gps_types.h"

#define PATH_LENGTh   64

typedef struct gpsPltDevCfg
{
    s8 bootloaderPath[PATH_LENGTh];
    s8 fwPath[PATH_LENGTh];
    u32 bootloaderDownloadSpeed;
    u32 fwDownloadSpeed;
    u32 bl_size;
    u32 fw_size;
} gpsPltDevCfg;

enum
{
    FW_DL_INIT,
    FW_DL_FAIL,
    FW_DL_SUCCEED
};

s32 firmware_download();

#endif
