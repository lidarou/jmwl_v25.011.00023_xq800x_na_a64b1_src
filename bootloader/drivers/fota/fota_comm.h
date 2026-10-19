
/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define common MACRO and API prototype for external module.
 *    Definitions in this file should always be same with component/fota.h.
 */

#ifndef __FOTA_COMM_H__
#define __FOTA_COMM_H__

#ifdef CONFIG_FOTA_BY_MCU
/*
 * the fota input param has change form V0004 to higher version,
 * For compatible with ota update form V0004 to higher version,
 * Add the old fota input param define. Used it to try the old fota
 * param that has been saved by the old softversion.
 */
#define FOTA_OLD_USER_NAME_LEN      64
#define FOTA_OLD_PWD_LEN            64
#define FOTA_OLD_URL_LEN            512
#endif

#define FOTA_MAGIC_WORD             0x464f5441
#define FOTA_USER_NAME_LEN          (48 + 1)
#define FOTA_PWD_LEN                (48 + 1)
#define FOTA_URL_LEN                (256 + 1)
#define FOTA_CRT_LEN                2048

enum FOTA_STATE {
    FOTA_STATE_NONE,
    FOTA_STATE_REGISTED,
    FOTA_STATE_DL_STARTED,
    FOTA_STATE_DL_FINISHED,
    FOTA_STATE_ERR,
    FOTA_STATE_DONE = 0x6F746173, /* otas */
    FOTA_STATE_MAX
};

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
};

#ifdef CONFIG_FOTA_BY_MCU
struct fota_old_http_input {
    char url[FOTA_OLD_URL_LEN];
    char user_name[FOTA_OLD_USER_NAME_LEN];
    char password[FOTA_OLD_PWD_LEN];
    u32 port;
};

struct fota_old_input {
    struct fota_old_http_input http;
    char crt_url[FOTA_OLD_URL_LEN];
    u8 *crt;
    u32 crt_len;

    u32 serial_port_id;
    u32 serial_port_baud;
    u32 dl_urc;
    u32 upgrd_mode;
    u32 upgrd_urc;
    u32 upgrd_type;
    u32 reserved;
};

struct fota_old_param {
    u32 state;
    u32 cur_ver_id;
    u32 new_ver_id;
    u32 dl_size;
    u32 total_size;
    u32 stage;
    u32 st_type;
    u16 checksum;
    u16 reserved;
    u32 reserve[8];  /* 64 bytes totally for download */

    struct fota_old_input input;  /* here in after is for update */
    u32 finished_parti_cnt;
    u32 magic;
};
#endif

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

struct fota_param {
    u32 state;
    u32 cur_ver_id;
    u32 new_ver_id;
    u32 dl_size;
    u32 total_size;
    u32 stage;
    u32 st_type;
    u16 checksum;
    u16 reserved;
    u32 reserve[8];  /* 64 bytes totally for download */

    struct fota_input input;  /* here in after is for update */
    u32 finished_parti_cnt;
    u32 magic;
};

struct fota_patch_header {
    u32 ver_id;
    u32 header_size;
    u32 data_size;
    u16 check_val;
};

#endif /* __FOTA_COMM_H__ */

