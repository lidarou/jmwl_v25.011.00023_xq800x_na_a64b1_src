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

#ifndef _GPS_OS_H_
#define _GPS_OS_H_

#include <aic_osa.h>
#include <aic_type.h>
#include "icoe_gps_types.h"

#define DEFAULT_THREAD_STACK_SIZE    2*1024

enum {
        LOG_UNKNOWN, 
        LOG_DEFAULT,
        LOG_VERBOSE, 
        LOG_DEBUG, 
        LOG_INFO, 
        LOG_WARN, 
        LOG_ERROR, 
        LOG_FATAL,
};

#ifndef SUCCESS
#define SUCCESS           (0)
#endif

#ifndef FAIL
#define FAIL              (-1)
#endif

struct GPS_MSG {
    u32 ev;
    u32 param0;
    u8* param1;
    u32 param2;
};

#define HANDLE aos_queue_ptr

#define gps_log(l, pStr, ...) do { \
        switch (l) {    \
        case LOG_WARN: \
            alog_warn(pStr, ##__VA_ARGS__); \
            break; \
        case LOG_ERROR: \
            alog_error(pStr, ##__VA_ARGS__); \
            break; \
        case LOG_INFO: \
            alog_info(pStr, ##__VA_ARGS__); \
            break; \
        case LOG_DEBUG: \
            alog_debug(pStr, ##__VA_ARGS__); \
            break; \
        default: \
            alog_info(pStr, ##__VA_ARGS__); \
        } \
    } while(0)


typedef void* (*gpsThreadEntryT)(void*);

u8 bd_check_eor(s8 *buf, int len);

//u8* strcasestr(u8 *pBuf,u8 *pStr);

//void gps_log(s16 level, const s8 * pStr, ...);

void gps_free(void* buff);

void* gps_malloc(u32 size);

void gps_sleep_ms(u32 ms);

s32 gps_mutex_init(pthread_mutex_t* lock);

s32 gps_mutex_destroy(pthread_mutex_t* lock);

s32 gps_mutex_lock(pthread_mutex_t* lock);

s32 gps_mutex_unlock(pthread_mutex_t* lock);

s32 gps_sem_init(sem_t* semp, u8 value);

s32 gps_sem_wait(sem_t* pSem);

s32 gps_sem_trywait(sem_t* pSem);

s32 gps_sem_timewait(sem_t* semp, u32 msec);

s32 gps_sem_post(sem_t* semp);

s32 gps_sem_destroy(sem_t* semp);

u32 gps_send_qmsg(HANDLE q_ptr, void *msg, u32 wait_optu8n);
u32 gps_receive_qmsg(HANDLE q_ptr, void *msg, u32 wait_optu8n);

HANDLE gps_thread_create(void* entryProc, void* args, u32 stackSize);

#endif /* _GPS_OS_H_ */
