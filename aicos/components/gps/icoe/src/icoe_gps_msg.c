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
#define AIC_THIS_FILE    0x0cd163
#define LOG_MODULE    AMOD_GNSS

#include "icoe_gps_types.h"
#include "icoe_gps_msg.h"

/* Simple generic message queue -
    pointers are queued, not the entire message; no message copy
    if message is from dynamic memory, sender needs to allocate, receiver needs to free
*/

s32 gps_queue_init(gpsMsgQueueT *queue)
{
#ifdef ANDROID
    if (gps_mutex_init(&queue->lock) != 0)
    {
        return -1;
    }
#endif

    queue->head = NULL;
    queue->tail = NULL;
    queue->count = 0;

    return 0;
}

u32 gps_queue_length(gpsMsgQueueT *queue)
{
    u32 count = 0;

#ifdef ANDROID
    gps_mutex_lock(&queue->lock);
#endif
    count = queue->count;
#ifdef ANDROID
    gps_mutex_unlock(&queue->lock);
#endif

    return count;
}

s32 gps_queue_empty(gpsMsgQueueT *queue)
{
    s32 ret = -1;
#ifdef ANDROID
    gps_mutex_lock(&queue->lock);
#endif
    if( queue->count == 0){
        ret = 0;
    }
#ifdef ANDROID
    gps_mutex_unlock(&queue->lock);
#endif
    return ret;
}
/*
 * add the msg to the end of list
 */
s32 gps_add_msg(gpsMsgQueueT *queue, gpsMsgHdr_t *msg)
{
    //gps_log(5, "add msg %d: in %s", msg->MsgName, __FUNCTION__);
#ifdef ANDROID
    gps_mutex_lock(&queue->lock);
#endif
    msg->next = NULL;
    
    if (queue->head == NULL)
    {
        queue->head = msg;
        queue->tail = msg;
    }
    else
    {
        queue->tail->next = msg;
        queue->tail = msg;
    }
    queue->count +=1;
#ifdef ANDROID
    gps_mutex_unlock(&queue->lock);
#endif
    return 0;
}

/*
 * remove the msg at the beginning of list
 */
s32 gps_remove_msg(gpsMsgQueueT *queue, gpsMsgHdr_t *msg)
{
    gpsMsgHdr_t *entry = NULL;
#ifdef ANDROID
    gps_mutex_lock(&queue->lock);
#endif
    if (queue->head != NULL)
    {
        entry = queue->head;
        queue->head = entry->next;
        entry->next = NULL;
        *((unsigned long*)msg) = (unsigned long)entry;
        //gps_log(5, "remove msg %d: in %s", msg->MsgName, __FUNCTION__);
        queue->count -=1;
    }
#ifdef ANDROID
    gps_mutex_unlock(&queue->lock);
#endif
    return 0;
}

s32 gps_queue_clear(gpsMsgQueueT *queue)
{
    gpsMsgHdr_t *entry = NULL;
#ifdef ANDROID
    gps_mutex_lock(&queue->lock);
#endif
    while(queue->head != NULL)
    {
        entry = queue->head;
        queue->head = entry->next;
        gps_free((gpsCmdMsg_t*)entry);
        queue->count -=1;
    }
#ifdef ANDROID
    gps_mutex_unlock(&queue->lock);
#endif

    return 0;
}
s32 gps_queue_cleanup(gpsMsgQueueT *queue)
{
    queue->head = queue->tail = NULL;
#ifdef ANDROID
    if(gps_mutex_destroy(&queue->lock) != 0)
        return -1;
#endif
    queue->count = 0;

    return 0;
}
