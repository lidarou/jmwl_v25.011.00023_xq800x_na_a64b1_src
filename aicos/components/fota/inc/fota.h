
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define common MACRO and API prototype for external module.
 */

#ifndef __FOTA_H__
#define __FOTA_H__

#include <aic_fota.h>

#define FOTA_MAGIC_WORD                 0x464f5441
#define FOTA_EXT_IMG_MAGIC              0xAEAEBFBF
#define FOTA_EXT_IMG_HEADER_LEN         8
#define FOTA_CHECK_SUM_SIZE             2
#define FOTA_EXT_IMG_BAK_ATTACH_SIZE    (FOTA_EXT_IMG_HEADER_LEN + FOTA_CHECK_SUM_SIZE)
#define FOTA_BAK_END_FLAG_SIZE          0x1000

enum FOTA_STATE {
    FOTA_STATE_NONE,
    FOTA_STATE_REGISTED,
    FOTA_STATE_DL_STARTED,
    FOTA_STATE_DL_FINISHED,
    FOTA_STATE_ERR,
    FOTA_STATE_DONE = 0x6F746173, /* otas */
    FOTA_STATE_MAX
};

#ifdef CONFIG_FOTA_CMIOT_CMD
enum FOTA_UPGRADE_STATE {
    UPGRADING = 1,
    UPGRADE_SUCCESS,
    UPGRADE_FAIL,
};
#endif

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

struct fota_ctx {
    void *dl_buff;
    FOTA_USER_CB cb;
    void *user_data;
    struct fota_input input;

    BOOL dl_stop;
    BOOL dl_thread_stopped;
    aos_thread_id thread;
    aos_semaphore_ptr sem;
};

u32 fota_get_state(void);
s32 fota_input_para_save(struct fota_input *in);
u16 fota_checksum(u16 *p, u32 len);
s32 fota_uart_data_save(u8 *data, u32 len, u32 total_len);
void fota_uart_data_reset(void);
#endif /* __FOTA_H__ */

