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

#ifndef __UM_GPS_RT_H_
#define __UM_GPS_RT_H_

#include <time.h>

#define REQBUFLEN 1024
#define DATABUFSIZE (7*1024)
#define EPH_GP 0x01
#define EPH_BD 0x01 << 1
#define EPH_GL 0x01 << 2
#define EPH_GA 0x01 << 3

typedef struct _RXN_REQ{
    u8 req[REQBUFLEN];
}RXN_Req_T;

typedef struct {
    time_t time;
    int pin;
    char buf[DATABUFSIZE];
}DataBuf;

s32 rt_gnss_init();
s32 rt_gnss_destroy();
void request_assist_data(void);

#endif
