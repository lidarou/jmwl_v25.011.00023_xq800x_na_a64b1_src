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
#define AIC_THIS_FILE    0x04dc4f
#define LOG_MODULE    AMOD_GNSS

#include <string.h>

#include "icoe_http_decoder.h"

#define RECVBUFSIZE 64

enum HttpDecodeStat hs;
u8 recvBuf[RECVBUFSIZE];
u8 pin;
s32 contentLen;
s32 recvLen;

static void (*onRtAssistanceCallback)(const u8 * data, s32 len) = NULL;

void SetOnRtAssistanceCallback(void (*p)(const u8 *, s32)){
        onRtAssistanceCallback = p;
}


void httpDecodeInit(void)
{
    hs = HTTPRECVHEAD;
    pin = 0;
    return;
}

s32 httpDecodeHead(void)
{
    s32 respCode;
    char *outrecvBuf;

    if(strlen(recvBuf) <= 1){
        hs = HTTPRECVDATA;
        gps_log(LOG_INFO, "recv http head ok\r\n");
    }
    if(0 == strncmp(recvBuf, "HTTP/", 5)){
        strtok_r(recvBuf, " ", &outrecvBuf);
        respCode = atoi(strtok_r(NULL, " ", &outrecvBuf));
        gps_log(LOG_INFO, "get respCode = %d\r\n", respCode);
        if(respCode != 200){
            gps_log(LOG_ERROR, "Error! get respCode = %d\r\n", respCode);
            return -1;
        }
    }else if(0 == strncmp(recvBuf, "Content-Length:", 14)){
        strtok_r(recvBuf, ":", &outrecvBuf);
        contentLen = atoi(strtok_r(NULL, " ", &outrecvBuf));
        gps_log(LOG_INFO, "contentLen = %d\r\n", contentLen);
    }
    pin = 0;
    return 0;
}

s32 httpAddData(u8 * data, s32 len)
{
    s32 i;
    for(i = 0; i < len; i++){
        switch(hs){
            case HTTPRECVHEAD:
                if(data[i] == 0x0a){
                    recvBuf[pin] = '\0';
                    if(httpDecodeHead() < 0) {
                        gps_log(LOG_ERROR, "error on %s:%d\r\n", __FUNCTION__, __LINE__);
                        return -1;
                    }
                }else{
                    recvBuf[pin] = data[i];
                    pin++;
                    if(pin > RECVBUFSIZE - 1){
                        gps_log(LOG_ERROR, "HTTP data error!\r\n");
                        return -1;
                    }
                }
                break;
            case HTTPRECVDATA:
                if(onRtAssistanceCallback != NULL)
                        onRtAssistanceCallback(data + i, len -1);
                recvLen+=(len-i);
                i = len;
                if(recvLen == contentLen) {
                    recvLen = 0;
                    return 1;
                }
                break;
            default:
                gps_log(LOG_ERROR, "Http Decoder Stat Error!\r\n");
                return -1;
        }
    }
    return 0;
}

