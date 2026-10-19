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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0baea7
#define LOG_MODULE    AMOD_GNSS

#include <aic_type.h>
#include <aic_osa.h>
#include "icoe_gps_os.h"

#define GPS_THREAD_PRIORITY       (5)
#define GPS_QUEUE_SIZE       (100)

struct GPS_ICOE {
    aos_thread_id id;
    aos_queue_ptr queue;
};
struct GPS_ICOE gps_icoe;

#if 0
u8* strcasestr(u8 *pBuf,u8 *pStr)
{
    int16_t iBufLen;
    int16_t iStrLen;
    int16_t iBufPos;
    int16_t iStrPos;
    int16_t iMatchCnt;

    iBufLen = iStrLen = 0;
    while(pBuf[iBufLen] != '\0')
    {
        iBufLen++;
    }
    while(pStr[iStrLen] != '\0')
    {
        iStrLen++;
    }

    if(iBufLen < iStrLen) 
        return NULL;

    iMatchCnt = 0;
    iBufPos = 0;
    iStrPos = 0;
    while(iBufPos < iBufLen)
    {
        if(pBuf[iBufPos] == pStr[iStrPos])
        {
            iMatchCnt++;
            if(iMatchCnt == iStrLen)
            {
                //gps_log(1, "gps: found at ", iBufPos-iStrLen+1);
                return pBuf + (iBufPos-iStrLen+1);
            }
            iBufPos++;
            iStrPos++;
        }
        else
        {
            iBufPos++;
            iStrPos = 0;
            iMatchCnt = 0;
        }
    }
    return NULL;
}
#endif

#if 0
/*
 * level need to be in restraint range
 * caller need to guarantee the valid level
 * it will be different for different OS
 */
void gps_log(s16 level, const s8 * pStr, ...)
{
    switch (level) {
    case LOG_WARN:
        alog_warn(pStr, ##__VA_ARGS__);
    case LOG_ERROR:
        alog_error(pStr, ##__VA_ARGS__);
    case LOG_INFO:
        alog_info(pStr, ##__VA_ARGS__);
    case LOG_DEBUG:
        alog_debug(pStr, ##__VA_ARGS__);
    default:
        alog_info(pStr, ##__VA_ARGS__);
    }
}
#endif

u8 bd_check_eor(s8 *buf, int len)
{
    u8 csum = 0;

    while(len--)
        csum ^= buf[len];

    return csum;
}

void* gps_malloc(u32 size)
{
    return aos_zalloc(size);
}

void gps_free(void* buff)
{
    if (buff != NULL)
        aos_free(buff);
}

void gps_sleep_ms(u32 ms)
{
    aos_sleep(ms);
}

// better to check the return value
s32 gps_mutex_init(pthread_mutex_t* mutex)
{
    s32 ret = 0;
    pthread_mutex_init(mutex, NULL);

    return ret;
}

s32 gps_mutex_lock(pthread_mutex_t* mutex)
{
    s32 ret = 0;

    pthread_mutex_lock(mutex);

    return ret;
}

s32 gps_mutex_unlock(pthread_mutex_t* mutex)
{
    s32 ret = 0;
    pthread_mutex_unlock(mutex);

    return ret;
}

s32 gps_mutex_destroy(pthread_mutex_t* mutex)
{
    s32 ret = 0;
    pthread_mutex_destroy(mutex);

    return ret;
}

s32 gps_sem_init(sem_t* semp, u8 value)
{
    s32 ret = 0;

    ret = sem_init(semp, 0, value);

    return ret;
}
    
s32 gps_sem_wait(sem_t* semp)
{
    s32 ret = 0;

    ret = sem_wait(semp);

    return ret;
}

s32 gps_sem_timewait(sem_t* semp, u32 msec)
{
    TX_SEMAPHORE* TheSem = (TX_SEMAPHORE *)semp;
    u32 ret = tx_semaphore_get(TheSem, msec);

    return (ret == TRUE) ? 0 : -1;
}

s32 gps_sem_post(sem_t* semp)
{
    s32 ret = 0;

    ret = sem_post(semp);

    return ret;
}

s32 gps_sem_destroy(sem_t* semp)
{
    s32 ret = 0;

    ret = sem_destroy(semp);

    return ret;
}

u32 gps_send_qmsg(HANDLE q_ptr, void *msg, u32 wait_optu8n)
{
    return aos_send_to_queue(q_ptr, msg, wait_optu8n);
}

u32 gps_receive_qmsg(HANDLE q_ptr, void *msg, u32 wait_optu8n)
{
    return aos_recv_from_queue(q_ptr, msg, wait_optu8n);
}

HANDLE gps_thread_create(void* entryProc, void* args, u32 stackSize)
{
    gps_icoe.queue = aos_create_queue("gps msg queue", sizeof(struct GPS_MSG),100);
    gps_icoe.id = aos_create_thread("GPSTask", entryProc, gps_icoe.queue,
                stackSize, 0, NULL, GPS_THREAD_PRIORITY, AOS_AUTO_START);

    return gps_icoe.queue;
}
