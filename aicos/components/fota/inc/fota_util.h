/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define common MACRO and API prototype for fota module internel.
 */

#ifndef __FOTA_UTIL_H__
#define __FOTA_UTIL_H__

#include <aic_assert.h>
#include <alog_api.h>
#include <psram_mem_config.h>

/*use rundata if fota not exists */
#define FOTA_PARTITION_NAME        FOTA_EXTIMG_PARTION_NAME
#define FOTA_PARA_PARTITION_NAME   "fota"
#define RUNDATA_PARTITION_NAME     "rundata"

#define FOTA_ROMFS_DIFF_PATH       "C:/etc/diff.rfs"
#define FOTA_ROMFS_PATH            "C:/etc/romfs.bin"
#define FOTA_ROMFS_ROOT            "/C"
#define FOTA_ROMFS_PARTITION_NAME  "romfs"
#define FOTA_APP_PARTITION_NAME    "app"

#define FOTA_BASIC_DIFF_FILE        ".basic_d"
#define FOTA_EXT_DIFF_FILE          ".ext_d"
#define FOTA_EXT_IMG_FILE           ".ext_f"
#define FOTA_ROMFS_DIFF_FILE        ".rfs_d"
#define FOTA_ROMFS_FILE             ".rfs_f"
#define FOTA_APP_FILE               ".app"
#define FOTA_HTTPS_CERT             "ca.der"

#define FOTA_RAM_DATA_READY         0xEEFFBBAA

#define FOTA_LOGIN_USER_NAME    NULL
#define FOTA_LOGIN_PWD          NULL
#define FOTA_PORT               443
#define FOTA_CLIENT             "fota_client"
#define FOTA_CONN_CRT_ID        1

#define FOTA_CONNECT_TIMEOUT    (10 * 1000)

#define FOTA_DL_MEM_SIZE        1500
#define FOTA_FDL_BLOCK_SIZE     4096
#define FOTA_IND_LEN            100

#define FLOGD       alog_debug
#define FLOGI       alog_info
#define FLOGW       alog_warn
#define FLOGE       alog_error

#define FLODX       aicx_debug
#define FLOIX       aicx_info
#define FLOWX       aicx_warn
#define FLOEX       aicx_error

#define FASSERT     AIC_ASSERT

#define fota_malloc aos_malloc
#define fota_free   aos_free

#ifndef mutex_lock
#define mutex_lock(x)   aos_get_mutex(x, AOS_WAIT_FOREVER)
#endif

#ifndef mutex_unlock
#define mutex_unlock    aos_put_mutex
#endif

#define FENTRY  ALOGD("%s entry, line=%d\n", __func__, __LINE__);
#define FDEBUG  ALOGD("%s line=%d\n", __func__, __LINE__);
#define FEXIT   ALOGD("%s exit, line=%d\n", __func__, __LINE__);

#define FOTA_NOTIFY(event, data1, data2) \
    if (fota_ctx.cb) \
        fota_ctx.cb(fota_ctx.user_data, event, (u32)data1, (u32)data2);

extern struct fota_ctx fota_ctx;
extern u32 fota_ftl_handle;
extern u32 fota_romfs_handle;
extern u32 fota_app_handle;

enum FOTA_CONN_TYPE {
    FOTA_CONN_HTTP = 0,
    FOTA_CONN_HTTPS,
    FOTA_CONN_FTP,
    FOTA_CONN_RAM,
    FOTA_CONN_MAX,
};

struct FOTA_CONN_OBJ {
    /* TODO */
    u32 type;
#ifdef CONFIG_FOTA_CMIOT_CMD
    int (*connect)(char *url, u32 port, u8 *crt, u32 crt_len,
                   char *user_name, char *password,
                   u32 start, u32 end, u32 timeout, u32 ssl_id);
    int (*get_data)(u8 *buff, u32 *bytes, u32 timeout);
#else
    int (*connect)(char *url, u32 port, u8 *crt, u32 crt_len,
                   char *user_name, char *password,
                   u32 start, u32 end);
    int (*get_data)(u8 *buff, u32 *bytes);
#endif
    int (*disconnect)(void);
    u32 (*get_total_size)(void);
};

struct FOTA_RAMDATA_INFO {
    u32 addr;
    u32 len;
    u32 is_ready;
};

BOOL fota_is_digit(char *str);
/* len is bytes of buffer */
int fota_string2hex(u8 *str, u8* hex, u32 len);
u16 fota_checksum(u16 *p, u32 len);
int fota_urc_cmiot(u8 state, u32 para1);
#ifdef CONFIG_FOTA_CMIOT_CMD
int fota_connect(char *url, u32 port, u8 *crt, u32 crt_len, char *user_name,
                 char *password, u32 start, u32 end, u32 type, u32 timeout, u32 ssl_id);
int fota_get_data(u8 *buff, u32 *bytes, u32 type, u32 timeout);
#else
int fota_connect(char *url, u32 port, u8 *crt, u32 crt_len, char *user_name,
                 char *password, u32 start, u32 end, u32 type);
int fota_get_data(u8 *buff, u32 *bytes, u32 type);
#endif
u32 fota_get_total_size(u32 type);
int fota_disconnect(u32 type);

/* don't support multi-instance now */
void fota_conn_register(struct FOTA_CONN_OBJ *obj);
BOOL fota_part_exist(void);
int fota_data_erase(u32 offset);
int fota_data_move_head(u32 size);
int fota_get_patch_header(u8 *buff, u32 size);
int fota_param_open(void);
int fota_param_erase(void);
u32 fota_param_close(u32 handle);
int fota_param_read(u8 *buff);
int fota_param_write(u8 *buff);
int fota_fmd_open(char *fota_part_name, u32 *handle);
u32 fota_fmd_close(u32 *handle);
int fota_fmd_erase(u32 handle);
int fota_fmd_read(u32 handle, u8 *buff, u32 offset, u32 size);
int fota_fmd_write(u32 handle, u8 *buff, u32 offset, u32 size);
#endif /* __FOTA_UTIL_H__ */

