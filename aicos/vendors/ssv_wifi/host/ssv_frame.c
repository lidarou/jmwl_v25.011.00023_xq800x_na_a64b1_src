/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e5dd9
#define LOG_MODULE    AMOD_VENDOR

#include <log.h>
//#include "lib-impl.h"
#include <ssv_lib.h>
#include <ssv_frame.h>

//#define SSV_FRAME_HEADER_RESV 100

#if (CONFIG_USE_LWIP_PBUF==1)
#include "lwip/lwip_pbuf.h"
#include "lwip/mem.h"
#endif

#include <pbuf.h>
extern OsMutex ssv_frame_mutex;
extern OsMutex ssv_frame_mutex_rx;

#define SSV_FRAME_TYPE_1 0x8001
#define SSV_FRAME_TYPE_2 0x8002

typedef struct ssv_pbuf
{
    u16             len;
    u16             hdr_offset;
    u16             type;
    u16             reserved;
}SSV_pbuf, *PSSV_pbuf;

void _os_frame_free(void *frame)
{
    OsMutex *frame_mutex = 0;

    if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_1)
    {
        frame_mutex = &ssv_frame_mutex;
    }
    else if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_2)
    {
        frame_mutex = &ssv_frame_mutex_rx;
    }
    else
    {
        LOG_PRINTF("Assert!!function: %s, line: %d, ((SSV_pbuf *)frame)->type=%x\n\t", __FUNCTION__, __LINE__, ((SSV_pbuf *)frame)->type); 
        ssv_assert(0);
    }
    
    OS_MutexLock(*frame_mutex);

	if(frame)
		PBUF_MFree(frame);
    OS_MutexUnLock(*frame_mutex);
}

void* os_frame_alloc_fn(ssv_type_u32 size, ssv_type_bool SecPool,const char* file, const int line)
{
    SSV_pbuf * pPKtInfo;

    if (SecPool == false)
    {
        OS_MutexLock(ssv_frame_mutex);
        pPKtInfo = (SSV_pbuf *)PBUF_MAlloc(size ,SSV_NOTYPE_BUF, file, line);
        if(pPKtInfo)
        {   
            pPKtInfo->type = SSV_FRAME_TYPE_1;
        	pPKtInfo->len = size;
        	pPKtInfo->hdr_offset = HEAD_ROOM;
        }
        OS_MutexUnLock(ssv_frame_mutex);
    }
    else
    {
        OS_MutexLock(ssv_frame_mutex_rx);
        pPKtInfo = (SSV_pbuf *)PBUF_MAlloc(size ,SSV_RX_BUF, file, line);
        if(pPKtInfo)
        {   
            pPKtInfo->type = SSV_FRAME_TYPE_2;
        	pPKtInfo->len = size;
        	pPKtInfo->hdr_offset = HEAD_ROOM;
        }
        OS_MutexUnLock(ssv_frame_mutex_rx);
    }
   
	return pPKtInfo;
}

void* os_frame_dup(void *frame)
{
    void * dframe;
    SSV_pbuf * pPKtInfo;
    //OsMutex *frame_mutex = 0;
    OS_MutexLock(ssv_frame_mutex);

	pPKtInfo = (SSV_pbuf *)PBUF_MAlloc((((SSV_pbuf *)frame)->len) ,SSV_NOTYPE_BUF,__FUNCTION__,__LINE__);

	if(pPKtInfo)
    {   
        pPKtInfo->type = SSV_FRAME_TYPE_1;
    	pPKtInfo->len = (((SSV_pbuf *)frame)->len);
    	pPKtInfo->hdr_offset = HEAD_ROOM;
    }
    
	dframe = pPKtInfo;
    
	if(dframe)
		OS_MemCPY(((ssv_type_u8*)(dframe)+(((SSV_pbuf *)dframe)->hdr_offset)), ((ssv_type_u8*)(frame)+(((SSV_pbuf *)frame)->hdr_offset)), (((SSV_pbuf *)frame)->len));

    OS_MutexUnLock(ssv_frame_mutex);
    
	return (void*)dframe;

}

//increase space to set header
void* os_frame_push(void *frame, ssv_type_u32 len)
{
    void *pdata;
	SSV_pbuf * pPKtInfo = (SSV_pbuf * )frame;
    OsMutex *frame_mutex = 0;

    if (pPKtInfo->type == SSV_FRAME_TYPE_1)
    {
        frame_mutex = &ssv_frame_mutex;
    }
    else if (pPKtInfo->type == SSV_FRAME_TYPE_2)
    {
        frame_mutex = &ssv_frame_mutex_rx;
    }
    else
    {
        LOG_PRINTF("Assert!!function: %s, line: %d, ((SSV_pbuf *)frame)->type=%x\n\t", __FUNCTION__, __LINE__, ((SSV_pbuf *)frame)->type); 
        ssv_assert(0);
    }
    // LOG_PRINTF("os_frame_push  _len offet %d, hdr_offset %d\n",pPKtInfo->len,pPKtInfo->hdr_offset);
    OS_MutexLock(*frame_mutex);
    
	pPKtInfo->hdr_offset -= len;
	pPKtInfo->len  += len;

	pdata = (ssv_type_u8*)pPKtInfo+pPKtInfo->hdr_offset;
    
    OS_MutexUnLock(*frame_mutex);
   //  LOG_PRINTF("os_frame_push after _len %d,hdr_offset %d\n",pPKtInfo->len,pPKtInfo->hdr_offset);
    return pdata;
}


//decrease data space.
void* os_frame_pull(void *frame, ssv_type_u32 len)
{
    void *pdata;
	SSV_pbuf * pPKtInfo = (SSV_pbuf * )frame;
    OsMutex *frame_mutex = 0;

    if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_1)
    {
        frame_mutex = &ssv_frame_mutex;
    }
    else if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_2)
    {
        frame_mutex = &ssv_frame_mutex_rx;
    }
    else
    {
        LOG_PRINTF("Assert!!function: %s, line: %d, ((SSV_pbuf *)frame)->type=%x\n\t", __FUNCTION__, __LINE__, ((SSV_pbuf *)frame)->type); 
        ssv_assert(0);
    }

    OS_MutexLock(*frame_mutex);
	pPKtInfo->hdr_offset += len;
	pPKtInfo->len  -= len;

	pdata = (ssv_type_u8*)pPKtInfo+pPKtInfo->hdr_offset;

    OS_MutexUnLock(*frame_mutex);
    return pdata;
}

ssv_type_u8* os_frame_get_data_addr(void *frame)
{
    ssv_type_u8* pdata;
    OsMutex *frame_mutex = 0;

    if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_1)
    {
        frame_mutex = &ssv_frame_mutex;
    }
    else if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_2)
    {
        frame_mutex = &ssv_frame_mutex_rx;
    }
    else
    {
        LOG_PRINTF("Assert!!function: %s, line: %d, ((SSV_pbuf *)frame)->type=%x\n\t", __FUNCTION__, __LINE__, ((SSV_pbuf *)frame)->type); 
        ssv_assert(0);
    }
    
    OS_MutexLock(*frame_mutex);
    pdata = ((ssv_type_u8*)(frame)+(((SSV_pbuf *)frame)->hdr_offset));
    OS_MutexUnLock(*frame_mutex);
    
    return pdata;
}

ssv_type_u32 os_frame_get_data_len(void *frame)
{
    ssv_type_u32 len;
    OsMutex *frame_mutex = 0;

    if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_1)
    {
        frame_mutex = &ssv_frame_mutex;
    }
    else if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_2)
    {
        frame_mutex = &ssv_frame_mutex_rx;
    }
    else
    {
        LOG_PRINTF("Assert!!function: %s, line: %d, ((SSV_pbuf *)frame)->type=%x\n\t", __FUNCTION__, __LINE__, ((SSV_pbuf *)frame)->type); 
        ssv_assert(0);
    }
    
    OS_MutexLock(*frame_mutex);
    len = (((SSV_pbuf *)frame)->len);
    OS_MutexUnLock(*frame_mutex);
    
    return len;
}

void os_frame_set_data_len(void *frame, ssv_type_u32 _len)
{
    OsMutex *frame_mutex = 0;

    if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_1)
    {
        frame_mutex = &ssv_frame_mutex;
    }
    else if (((SSV_pbuf *)frame)->type == SSV_FRAME_TYPE_2)
    {
        frame_mutex = &ssv_frame_mutex_rx;
    }
    else
    {
        LOG_PRINTF("Assert!!function: %s, line: %d, ((SSV_pbuf *)frame)->type=%x\n\t", __FUNCTION__, __LINE__, ((SSV_pbuf *)frame)->type); 
        ssv_assert(0);
    }
    
    OS_MutexLock(*frame_mutex);
    (((SSV_pbuf *)frame)->len = _len);
    OS_MutexUnLock(*frame_mutex);
}

//----------------------------------------------

void os_frame_set_debug_flag(void *frame, ssv_type_u32 flag)
{
}



