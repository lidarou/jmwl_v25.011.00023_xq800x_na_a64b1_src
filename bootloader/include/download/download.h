/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __DOWNLOAD_DOWNLOAD_H__
#define __DOWNLOAD_DOWNLOAD_H__

#include "inttypes.h"

struct download {
    void *data;
    void (*loop_self)(void *data);
    uint32_t (*write)(void *data, uint8_t *buf, uint32_t len);
    uint32_t (*read)(void *data, uint8_t *buf, uint32_t len);
    int (*get_char_blocking)(void *data);
    void (*baudrate_set)(void *data, uint32_t baudrate);
};

int download_driver_register(uint32_t dl_type, struct download *dl);
extern struct download *cur_dl_chan;
#endif /*__DOWNLOAD_DOWNLOAD_H__*/