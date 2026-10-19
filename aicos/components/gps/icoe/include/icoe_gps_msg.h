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

#ifndef _GPS_MSG_H_
#define _GPS_MSG_H_

#include "icoe_gps_os.h"

#define GPS_CMD_MAXLEN 32

enum{
    MSG_DELETE_AIDING= 1,
    MSG_SET_intERVAL,
    MSG_SET_POS_MODE,
    MSG_GET_FWINFO,
    MSG_SET_BAUD,
    MSG_SAVE_CFG,
    MSG_REQ_ASSIST,
    MSG_START_DEBUG,
};

typedef struct gpsMsgHdr
{
    u8 MsgName;
    struct gpsMsgHdr* next;
} gpsMsgHdr_t;

typedef struct gpsMsg
{
    s8 cmd[GPS_CMD_MAXLEN];
} gpsMsg_t;

typedef struct
{
    gpsMsgHdr_t  hdr;
    gpsMsg_t cmdMsg;
} gpsCmdMsg_t;

typedef struct
{
    gpsMsgHdr_t* head;     /* head of the list */
    gpsMsgHdr_t* tail;     /* last entry of the list */
    u32 count;
#ifdef ANDROID
    pthread_mutex_t lock;
#endif
} gpsMsgQueueT;

s32 gps_queue_init(gpsMsgQueueT *queue);
s32 gps_queue_empty(gpsMsgQueueT *queue);
u32 gps_queue_length(gpsMsgQueueT *queue);
s32 gps_add_msg(gpsMsgQueueT *queue, gpsMsgHdr_t *msg);
s32 gps_remove_msg(gpsMsgQueueT *queue, gpsMsgHdr_t *msg);
s32 gps_queue_clear(gpsMsgQueueT *queue);
s32 gps_queue_cleanup(gpsMsgQueueT *queue);
#endif


