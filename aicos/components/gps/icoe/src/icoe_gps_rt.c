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
#define AIC_THIS_FILE    0x0b01bb
#define LOG_MODULE    AMOD_GNSS

#include <time.h>
#include <nxd_bsd.h>
#include <aic_osa.h>
#include <aic_net_pdp_mgr.h>
#include "icoe_http_decoder.h"
#include "icoe_gps_os.h"
#include "icoe_gps_serial.h"
#include "icoe_gps_hal.h"
#include "icoe_gps_msg.h"
#include "icoe_gps_rt.h"

#define DOWNLOAD_ASSIST_DATA_DLBLOCK_TIME     10*1000
#define DOWNLOAD_ASSIST_DATA_THREAD_PRIORITY  13
#define DOWNLOAD_ASSIST_DATA_STACK_SIZE       2*1024
#define MAX_DATA_LEN                          1024

static int have_req = 0;
static DataBuf *ephemBuf = NULL;
static sem_t assistDataDLFinish;

s32 rt_sem_init(void)
{
    s32 ret;
    ret = gps_sem_init(&assistDataDLFinish, 0);

    if (ret < 0)
    {
        gps_log(LOG_ERROR, "Init semImagDwFinish fail!\n");
    }

    return ret;
}

int rt_sem_time_wait(s32 msec)
{
    return gps_sem_wait(&assistDataDLFinish);
    //return gps_sem_timewait(&assistDataDLFinish, msec);
}

void rt_sem_post(void)
{
    gps_sem_post(&assistDataDLFinish);
}

void rt_sem_destroy(void)
{
    gps_sem_destroy(&assistDataDLFinish);
}

s32 rt_gnss_init()
{
    ephemBuf = (DataBuf*) gps_malloc(sizeof(DataBuf));
    if(NULL == ephemBuf) {
        gps_log(LOG_ERROR, "rt gnss buffer allocate failed\n");
        return -1;
    }
    ephemBuf->time = 0;
    ephemBuf->pin = 0;

    return 0;
}

s32 rt_gnss_destroy()
{
    gps_free(ephemBuf);
    
    return 0;
}

int databuf_add_data(DataBuf * buf, char * data, int count)
{
    int empty = DATABUFSIZE - buf->pin;
    count =  count > empty ? empty:count;
    memcpy(buf->buf + buf->pin, data, count);
    buf->pin = buf->pin + count;

    return count;
}

void databuf_clear(DataBuf * buf)
{
    buf->pin = 0;
    buf->time = 0;
}

void RXN_Callback(const u8 *data, s32 len)
{
    if(ephemBuf != NULL) {
        databuf_add_data(ephemBuf, (char*)data, len);
        databuf_add_data(ephemBuf, "\r\n", 2);
    }
}

static int buffered_ephem_is_useable()
{
    if(NULL == ephemBuf)
        return -1;

    time_t ct = time(NULL);

    if((ct - ephemBuf->time) < 1 * 60 * 60){
        if(ephemBuf->pin > 0)
            return 0;
        else 
            return -1;
    }

    return -1;
}

static s32 init_socket(char *svrFqdn, unsigned short svrPort)
{
    s32 sockfd = -1;
    u8 portText[8] = {0};
    struct addrinfo hint;
    struct addrinfo *ailist = NULL; 
    struct addrinfo *aip    = NULL;
    s32             err     = -1;
  
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_socktype = SOCK_STREAM;
    sprintf(portText, "%d", svrPort);
    err = getaddrinfo(svrFqdn, portText, &hint, &ailist);
    if(err != 0) 
    {
        gps_log(LOG_ERROR, "[%s]: getaddrinfo() err(%d)! errno(%d)\n", 
                         __FUNCTION__, err, errno);
        return -1;
    }

    do
    {
        for(aip = ailist; aip; aip = aip->ai_next) 
        {
            sockfd = socket(aip->ai_family, SOCK_STREAM, 0);
            if(sockfd < 0) 
            {
                gps_log(LOG_ERROR, "error! create socket failed!\n");
                break;
            }

            if(0 == connect(sockfd, aip->ai_addr, aip->ai_addrlen)) 
            {
                gps_log(LOG_INFO, "sucessfully connect to server!\n");
                break;
            }
            else
            {
                gps_log(LOG_ERROR, "connect server failed! errno(%d)\n", errno);
                /* shutdown the created socket! */
                soc_close(sockfd);
                sockfd = -1;
                break;
            }
        }        
    }while(0);

    freeaddrinfo(ailist);

    return sockfd;
}

int get_device_id(u8 *imei)
{
    /*
     * get unique device id from platform
     */
    //CFW_EmodGetIMEI(imei, &length, CFW_SIM_0);
    //if(strncmp(imei, "0000000000000000") == 0)
        //return -1;
    aos_memcpy(imei, "123456789", 10);
    return 0;
}

u8 *RXN_REQ_HEAD_H = "POST /rxn-api/locationApi/rtcm HTTP/1.1\r\nHost: %s\r\nAuthorization:RXN-BASIC cId=%s,mId=%s,dId=%s,pw=%s\r\nContent-type:application/json\r\nAccept:application/octet-stream\r\nContent-length: %d\r\n\r\n%s";

u8 * vendorID = "IcoeTempID1Expire20230430";
u8 * modelID = "123456789";
u8 * passWord = "NW4zOFJjY2NvaFhTUmkzWA==";

u8 * RXN_URL = "icoe-api.rx-networks.cn";

u8 rxn_content[256];
int rxn_construct_req(RXN_Req_T *req, char *dId, u8 type){
    u8 *content = rxn_content;

    if(0 == type) 
        return -1;

    sprintf(content, "[{\"rtAssistance\":{\"format\":\"rtcm\",\"msgs\":[");
    if(type & EPH_GP)
        sprintf(content + strlen(content), "\"GPS:1NAF\",");
    if(type & EPH_BD)
        sprintf(content + strlen(content), "\"BDS:2NAF\",");
    if(type & EPH_GL)
        sprintf(content + strlen(content), "\"GLO:2NAF\",");
    if(type & EPH_GA)
        sprintf(content + strlen(content), "\"GAL:2NAF\",");
    sprintf(content + strlen(content) - 1, "]");
    sprintf(content + strlen(content), "}}]\r\n");
    sprintf(req->req, RXN_REQ_HEAD_H, RXN_URL, vendorID, modelID, dId, passWord,strlen(content), content);

    return strlen(req->req);
}

u8 assist_data_buf[MAX_DATA_LEN + 1];
void get_assist_data()
{
    int MAX_TRY = 3;
    int ret = 0;
    int sockfd = -1;
    int retry = 0;
    RXN_Req_T req;
    unsigned int reqlen = 0;
    u8 *buf = assist_data_buf;
    char imei[32];
    memset(imei, 0, sizeof(imei));

    if(get_device_id(imei) != 0) {
        gps_log(LOG_ERROR, "fail to get IMEI\n");
        return;
    }

    for(retry = 0; retry < MAX_TRY; retry++) {
        sockfd = init_socket(RXN_URL, 80);
        if(sockfd < 0){
            gps_log(LOG_ERROR, "fail to set up the socket to agnss server\n");
            if(retry == MAX_TRY - 1) {
                rt_sem_post();
                return;
            } else {
                gps_sleep_ms(100);
            }
        } else {
            break;
        }
    }

    reqlen = rxn_construct_req(&req, imei, EPH_GP | EPH_BD | EPH_GL);

    //gps_log(LOG_ERROR, "send the requirement: %s", req.req);
    send(sockfd, req.req, reqlen, 0);
    httpDecodeInit();
    SetOnRtAssistanceCallback(RXN_Callback);
    while(1)
    {
        int recv_len = recv(sockfd, buf, MAX_DATA_LEN, 0);
        if(recv_len < 0){
            gps_log(LOG_ERROR, "recv data error\r\n");
            break;
        } else {
            ret = httpAddData(buf, recv_len);
            if(ret == 1) {
                ephemBuf->time = time(NULL);
                gps_log(LOG_INFO, "assist data decode end!\n");
                break;
            } else if(ret < 0) {
                gps_log(LOG_ERROR, "http interface return error\r\n");
                break;
            }
        } // end of if
    }

    rt_sem_post();

    soc_close(sockfd);
    
    return;
}

static s32 inject_assist_data()
{
    s32 ret = -1;

    if(gps_hal_get_state() & GPS_HAL_STATE_STARTED) {
        gps_log(LOG_INFO, "inject assist data:%d\n", ephemBuf->pin);
        ret = gps_tty_write(ephemBuf->buf, ephemBuf->pin);
        if(!ret) {
            gps_log(LOG_INFO, "succeed to inject data length: %d\n", ephemBuf->pin);
            ret = 0;
        } else {
            gps_log(LOG_ERROR, "inject data failed: %d\n", ret);
            ret = -1;
        }
    }

    return ret;
}

void request_assist_data(void)
{
    aos_thread_id assistHandle = AOS_INVALID_THREAD_ID;

    if (!aic_is_default_pdp_activated())
        return;

    rt_sem_init();

    if(buffered_ephem_is_useable() != 0) {
        gps_log(LOG_INFO, "request for assist data\n");
        assistHandle = aos_create_thread("GPS Assist Data Download", get_assist_data,
                        NULL, DOWNLOAD_ASSIST_DATA_STACK_SIZE,
                        0, NULL, DOWNLOAD_ASSIST_DATA_THREAD_PRIORITY,
                        AOS_AUTO_START);
            
        if(rt_sem_time_wait(DOWNLOAD_ASSIST_DATA_DLBLOCK_TIME) < 0 && ephemBuf->pin != 0) {
            gps_log(LOG_ERROR, "fail to download assist data from server\n");
        } else {
            if(inject_assist_data() == 0) {
                have_req = 0;
            } else {
                have_req = 1;
            }
        }
    } else {
        //if(have_req) {
            if(inject_assist_data() == 0) {
                have_req = 0;
            }
        //}
    }
    rt_sem_destroy();

    if(assistHandle != AOS_INVALID_THREAD_ID) {
        /*wait for task to stop*/
        gps_sleep_ms(200);
        aos_delete_thread(assistHandle);
        assistHandle = AOS_INVALID_THREAD_ID;
    }
    return;
}

