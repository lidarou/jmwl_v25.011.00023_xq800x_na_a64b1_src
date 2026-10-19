/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log mudule config header file.
 */
#ifndef __ALOG_CFG_H__
#define __ALOG_CFG_H__
#include <aic_bitmap.h>
#include <aipc_api.h>
#include <aipc_brr_inputer.h>
#include <aic_module.h>

/**
 * we used ALOG_LV_BITS bit to indicate a level.
*/
#define ALOG_LV_BITS     4
#define ALOG_LV_SIZE     BITS_TO_LONGS(MAX_AMOD_CNT * ALOG_LV_BITS)

/* outputer_dev, bit0 ~ bit3, dev type, bit4~bit7 dev port. */
#define ALOP_GET_DEV_TYPE(outputer_dev)  ((outputer_dev) >> 4)
#define ALOP_GET_DEV_COM(outputer_dev)  ((outputer_dev) & 0xf)
#define ALOP_MAKE_OUTDEV(type, com)  (((type) << 4) | (com & 0xf))

/* log_switch, BIT0:trace, BIT1: message: BIT2:cap. */
#define ALOG_TRACE_SW   BIT_0
#define ALOG_MSG_PEER   BIT_1
#define ALOG_MSG_PARA   BIT_2
#define ALOG_DATA_SW    BIT_3
#define ALOG_MSG_SW     (ALOG_MSG_PEER | ALOG_MSG_PARA)

#define ALOG_ENA_ALL    0xff
#define ALOG_DIS_ALL    0x00

#define ALOG_NV_MAGIC   0x5A5A

#define MAX_MSG_TASK    256
#define TASK_MSG_SIZE   BITS_TO_LONGS(MAX_MSG_TASK)

/* Subsystem differential configuration. */
typedef struct ALOG_BUF_CFG {
    u16  buf_cnt;/* number of bufs */
    u16  offset; /* loop override skips the first offset buffers */
    u16  size;  /* buf size */
    u16  priority; /* the priority the buffer mgr task */
} ALOG_BUF_CFG_T;

/* alog brr config */
typedef struct ALBRR_CFG {
    u8  sys;
    u8  brr;
} ALBRR_CFG_T;

/* alog config, now is as smame as alog breare config. */
typedef struct ALBRR_CFG ALOG_CFG_T;

const ALOG_BUF_CFG_T *alog_get_buf_cfg(u8 type);
const ALOG_CFG_T *alog_get_cfg(void);
const ALBRR_CFG_T *alog_get_inputter_brr_cfg(u8 *cnt);

u8 alog_get_log_switch_ex(void);
void alog_set_log_switch_ex( u8 value);
bool alog_get_log_switch(u8 mask);
void alog_set_log_switch(bool enable, u8 mask);
#define alog_is_trace_enable()          alog_get_log_switch(ALOG_TRACE_SW)
#define alog_is_data_enable()           alog_get_log_switch(ALOG_DATA_SW)
#define alog_set_trace_enable(en)       alog_set_log_switch((en), ALOG_TRACE_SW)
#define alog_set_para_enable(en)        alog_set_log_switch((en), ALOG_MSG_PARA)
#define alog_set_peer_enable(en)        alog_set_log_switch((en), ALOG_MSG_PEER)
#define alog_set_data_enable(en)        alog_set_log_switch((en), ALOG_DATA_SW)

unsigned long *alog_get_level_bitmap(void);
unsigned long *alog_get_msg_task_bitmap(void);
#endif /* __ALOG_CFG_H__ */

