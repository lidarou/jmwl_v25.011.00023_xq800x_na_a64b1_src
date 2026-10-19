/*****************************************************************************/
/*                                                                           */
/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

#include "console/console.h"
#include "download/download.h"
#include "download/dload_engine.h"
#include "initcall.h"
#include "platform/platform.h"

struct download *aic_download[DL_MAX];
struct download *cur_dl_chan;

int download_driver_register(uint32_t dl_type, struct download *dl)
{
    if (dl_type >= DL_MAX)
        return -1;

    aic_download[dl_type] = dl;

    return 0;
}

int download_mode_enter(void)
{
    uint32_t dl_type = platform_get_dl_type();

    log_print("dl_type = %d\r\n", dl_type);

    cur_dl_chan = aic_download[dl_type];
    dload_recv_engine();

    return 0;
}

static int download_mode_init(void)
{
    platform_boot_mode_enterpoint_register(download_mode_enter,
                    DOWNLOAD_BOOT);
    return 0;
}

driver_init(download_mode_init);
