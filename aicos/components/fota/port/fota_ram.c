/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is ram type access for FOTA download.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x061e3e
#define LOG_MODULE    AMOD_FOTA

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_size.h>

#include <aic_at_cmd_handle.h>
#include <sys_mem_api.h>

#include "fota_util.h"

struct fota_ram_ctx {
    struct FOTA_RAMDATA_INFO *info;
    u32 start_addr;
    u32 read_len;
};

static struct fota_ram_ctx g_fota_ram;

/* test for the RAM Client get method. */
static int fota_ram_connect(char *url, u32 port, u8 *crt, u32 crt_len,
                              char *user_name, char *password,
                              u32 start, u32 end)
{
    u32 ram_addr = 0;
    u32 ram_size = 0;
    u32 offset = sizeof(struct FOTA_RAMDATA_INFO);

    sys_get_save_mcu_fota_pkg_area((addr_t *)&ram_addr, &ram_size);

    /* check if ram is valid and ram is not empty */
    g_fota_ram.info = (struct FOTA_RAMDATA_INFO *)ram_addr;
    if (g_fota_ram.info->is_ready != FOTA_RAM_DATA_READY) {
        FLOIX(0x0b91fd, "", "fota: ram data is not ready\n");
        return -1;
    }

    if (start >= ram_size) {
        FLOIX(0x160dbe, "dd", "fota: start pos: %d, ram size:%d\n", start, ram_size);
        start = 0;
    }

    g_fota_ram.start_addr = g_fota_ram.info->addr + offset + start;
    g_fota_ram.read_len = 0;

    FLOIX(0x1d77a5, "x", "fota: ram start read addr: %x\n", g_fota_ram.start_addr);

    return 0;
}

/**
 *  return:
 *       < 0: error code
 *       >=0: data size
 */
static int fota_ram_get_data(u8 *buff, u32 *bytes)
{
    u32 left_len = 0;

    if (!g_fota_ram.info || g_fota_ram.info->is_ready != FOTA_RAM_DATA_READY) {
        FLOIX(0x19ffce, "", "fota: ram not init\n");
        return -1;
    }

    /* read data from ram */
    left_len = g_fota_ram.info->len - g_fota_ram.read_len;
    if (*bytes <= left_len) {
        memcpy(buff, (u8 *)g_fota_ram.start_addr + g_fota_ram.read_len, *bytes);
        g_fota_ram.read_len += *bytes;
    } else {
        memcpy(buff, (u8 *)g_fota_ram.start_addr + g_fota_ram.read_len, left_len);
        *bytes = left_len;
        g_fota_ram.read_len += left_len;
    }

    FLOIX(0x023351, "d", "fota: ram total read: %d\n", g_fota_ram.read_len);

    return *bytes;
}

static u32 fota_ram_get_total_size(void)
{
    /* return the total size of data from AT command */
    if (!g_fota_ram.info || g_fota_ram.info->is_ready != FOTA_RAM_DATA_READY) {
        FLOEX(0x19ffce, "", "fota: ram not init\n");
        return 0;
    }

    return g_fota_ram.info->len;
}

static int fota_ram_disconnect(void)
{
    /* set ram addr to null */
    g_fota_ram.info = 0;
    g_fota_ram.read_len = 0;
    g_fota_ram.start_addr = 0;

    return 0;
}

struct FOTA_CONN_OBJ fota_conn_ram = {
    .type = FOTA_CONN_RAM,
    .connect = fota_ram_connect,
    .get_data = fota_ram_get_data,
    .disconnect = fota_ram_disconnect,
    .get_total_size = fota_ram_get_total_size,
};

int fota_ram_init(void)
{
    g_fota_ram.info = 0;
    g_fota_ram.read_len = 0;
    g_fota_ram.start_addr = 0;

    fota_conn_register(&fota_conn_ram);

    return 0;
}
DIS_CHARGE_COMP_P1_INIT_EXPORT(fota_ram_init);

