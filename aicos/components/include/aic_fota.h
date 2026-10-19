
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines common MACRO and API prototype for AIC FOTA.
 */

#ifndef __AIC_FOTA_H__
#define __AIC_FOTA_H__
#define FOTA_USER_NAME_LEN          (48 + 1)
#define FOTA_PWD_LEN                (48 + 1)
#define FOTA_URL_LEN                (256 + 1)
#define FOTA_CRT_LEN                2048

enum FOTA_STAGE {
    FOTA_STAGE_BASIC_DIFF_DL,
    FOTA_STAGE_BASIC_UPDATE,
    FOTA_STAGE_EXTEND_DIFF_DL,
    FOTA_STAGE_EXTEND_IMG_DL,
    FOTA_STAGE_EXTEND_UPDATE,
    FOTA_STAGE_ROMFS_DIFF_DL,
    FOTA_STAGE_ROMFS_IMG_DL,
    FOTA_STAGE_ROMFS_UPDATE,
    FOTA_STAGE_APP_IMG_DL,
    FOTA_STAGE_UPDATE_DONE,
    FOTA_STAGE_MAX
};

enum FOTA_START_TYPE {
    FOTA_START_BY_APP,
    FOTA_START_BY_AT,
    FOTA_START_BY_MAX
};

enum FOTA_DL_TYPE {
    FOTA_DL_BASIC_DIFF,
    FOTA_DL_EXT_DIFF,
    FOTA_DL_EXT_FILE,
    FOTA_DL_ROMFS_DIFF,
    FOTA_DL_ROMFS_FILE,
    FOTA_DL_MULTI_MAX,

    FOTA_DL_APP_FILE = 16,
};


enum FOTA_UPGRADE_TYPE {
    FOTA_UPG_ALL = 0,
    FOTA_UPG_APP_FILE = FOTA_STAGE_APP_IMG_DL,
};


enum FOTA_CB_EVENT {
    FOTA_EVENT_CONNECTED,
    FOTA_EVENT_DL_PROGRESS,
    FOTA_EVENT_DL_DONE,
    FOTA_EVENT_CHECK,
    FOTA_EVENT_BROKEN,
    FOTA_EVENT_CERT_FAIL,
    FOTA_EVENT_DL_FAIL,
    FOTA_EVENT_EXT_UPDATE_PROGRESS,
    FOTA_EVENT_RFS_UPDATE_PROGRESS,
    FOTA_EVENT_VER_NOT_MATCH,
    FOTA_EVENT_RESET_BASIC,
};

/**
* FOTA_USER_CB() - callback which is registered through fota_notify_register(), to report FOTA status.
* @user_data: user parameter which is registered through fota_notify_register()
* @event: event type, defined in FOTA_CB_EVENT as above.
* @data1: download progress.
* @data2: download phase, defined in FOTA_DL_TYPE as above.
*/
typedef void (*FOTA_USER_CB)(void *user_data, int event, u32 data1, u32 data2);

struct fota_http_input {
    char url[FOTA_URL_LEN];
    char user_name[FOTA_USER_NAME_LEN];
    char password[FOTA_PWD_LEN];
    u32 port;
};

struct fota_input {
    struct fota_http_input http;
    char crt_url[FOTA_URL_LEN];
    u8 *crt;
    u32 crt_len;

    u32 serial_port_id;
    u32 serial_port_baud;
    u32 dl_urc;
    u32 upgrd_mode;
    u32 upgrd_urc;
    u32 upgrd_type;
    u16 mode;
    u32 timeout;
    u32 prog_bytes;
    u32 ssl_id;
    u32 pkg_size;
    u32 reserved;
};

/**
* fota_start() - start fota download and upgrade.
* @in: input parameter, described as above fota_input.
* Return: 0 means success, others means failed.
*/
int fota_start(struct fota_input *in);

/**
* fota_stop() - stop fota download and upgrade.
* Return: 0 means success, others means failed.
*/
int fota_stop(void);

/**
* fota_notify_register() - register user callback, which to report fota progress and status.
* @cb: user callback.
* @data: user data used in user callback.
* @type: defined in FOTA_START_TYPE as above.
* Return: 0 means success, others means failed.
*/
int fota_notify_register(FOTA_USER_CB cb, void *data, u32 type);
void fota_notify_unregister(u32 type);

/**
* fota_get_stage() - get fota stage.
* Return: fota stage defined in FOTA_STAGE as above.
*/
u32 fota_get_stage(void);

/**
* fota_get_input_param() - get fota input param.
* Return: fota parameter defined in FOTA_INPUT as above.
*/
struct fota_input *fota_get_input_param(void);

/**
* fota_get_download_size() - get fota download size.
* Return: fota image downloading size & total size.
*/
void fota_get_download_size(u32 *dl_size, u32 *total_size);
#endif /* __AIC_FOTA_H__ */

