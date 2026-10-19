/************************************************************************ 
* Copyright ? 2022, ICOE (Shanghai) Technologies Co.,Ltd. All rights reserved.              *
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
#define AIC_THIS_FILE    0x11f329
#define LOG_MODULE    AMOD_GNSS

#include <aic_osa.h>
#include <fmd.h>
#include "icoe_gps_os.h"
#include "icoe_gps_serial.h"
#include "icoe_gps_hal.h"
#include "icoe_firmware_loader.h"

#define XMODEM_SOH 0x01
#define XMODEM_STX 0x02
#define XMODEM_EOT 0x04
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18
#define XMODEM_CRC_CHR    'C'
#define XMODEM_CRC_SIZE 2        /* Crc_High Byte + Crc_Low Byte */
#define XMODEM_FRAME_ID_SIZE 2         /* Frame_Id + 255-Frame_Id */
#define XMODEM_DATA_SIZE_SOH 128      /* for Xmodem protocol */
#define XMODEM_DATA_SIZE_STX 1024     /* for 1K xmodem protocol */
#define USE_1K_XMODEM 1          /* 1 for use 1k_xmodem 0 for xmodem */
#define    TIMEOUT_USEC    0
#define    TIMEOUT_SEC    1
#define DEBUG    1
#define ISMENULOADER    0
#if 0
#define GPS_FW_THREAD_PRIORITY       (COS_MMI_TASKS_PRIORITY_BASE + 13)
#define DEFAULT_GPS_FW_STACK_SIZE    2*1024

#define GPS_DLBLOCK_TIME 15*1000
#endif

//need to update according to platform
gpsPltDevCfg rda_platform_config =
{
    .bootloaderPath = "gnss_bl",
    .fwPath = "gnss",
    .bootloaderDownloadSpeed = 115200,
    .fwDownloadSpeed = 2000000,
    .bl_size = 13076,
    .fw_size = 236112,
};


#if (USE_1K_XMODEM)
    #define XMODEM_DATA_SIZE     XMODEM_DATA_SIZE_STX
    #define XMODEM_HEAD        XMODEM_STX
#else
    #define XMODEM_DATA_SIZE     XMODEM_DATA_SIZE_SOH
    #define XMODEM_HEAD         XMODEM_SOH
#endif

#if 0
#define GET_TIME()  { gettimeofday(&time_m, NULL); \
            time_m.tv_sec += TIMEOUT_SEC;\
            }
#define GET_TIME_TIMEOUT(_time_out_)  { gettimeofday(&time_m, NULL); \
            time_m.tv_sec += _time_out_;\
            }

#define CHK_TIME()  { gettimeofday(&time_n, NULL); \
            if(time_n.tv_sec > time_m.tv_sec) { \
                printf("\ntimeout!!!\n\n");\
                close(fd); \
                return -1; \
            } \
             }
#define CHK_TIME_BREAK()  { gettimeofday(&time_n, NULL); \
            if(time_n.tv_sec > time_m.tv_sec) { \
                printf("\ntimeout!!!\n\n");\
                break; \
            } \
             }


#define CHK_TIME1()  { gettimeofday(&time_n, NULL); \
            if(time_n.tv_sec > time_m.tv_sec) { \
                printf("\ntimeout!!!\n\n");\
                if(datafile != NULL) \
                    fclose(datafile); \
                return -1; \
            } \
             }
#endif

int xmodem_send(char *file_path, u32 total_size);

static sem_t semImagDwFinish;
extern u8 firmware_status;

s32 dl_finish_sem_init(void)
{
    s32 ret;
    ret = gps_sem_init(&semImagDwFinish, 0);

    if (ret < 0)
    {
        gps_log(LOG_ERROR, "Init semImagDwFinish fail!\n");
    }

    return ret;
}

int dl_finish_sem_time_wait(s32 msec)
{
    return gps_sem_timewait(&semImagDwFinish, msec);
}

void dl_finish_sem_post(void)
{
    gps_sem_post(&semImagDwFinish);
}

void dl_finish_sem_destroy(void)
{
    gps_sem_destroy(&semImagDwFinish);
}

void gps_reset( )
{
    gps_disable_power();
    gps_log(LOG_INFO, "power on gps\n");
    gps_sleep_ms(10);
    gps_enable_power();
    gps_reset_gpio();
}

unsigned short get_crc16 ( char *ptr, unsigned short count )
{
    unsigned short crc, i;

    crc = 0;
    while(count--)
    {
        crc = crc ^ (int) *ptr++ << 8;

        for(i = 0; i < 8; i++)
        {
            if(crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;
        }
    }

    return (crc & 0xFFFF);
}

//extern u32 aic_get_timestamp(void);
int firmware_loader(char *loader_path, char *firmware_path)
{
    int write_number;
    char name[32];
    char response[16];
    int retry = 2;
    int is_get_YC = 0;
    int ret = 0;
    //u32 timer1, timer2;

    gps_log(LOG_INFO, "===== FIREBIRD FIRMWARE UPGRADE =====\n");
    /* open serial port*/
    gps_set_baudrate(rda_platform_config.bootloaderDownloadSpeed);

    gps_log(LOG_INFO, "waiting for YC\n");

    while(retry > 0) {
        memset(response, 0, sizeof(response));
        gps_log(LOG_INFO, "try the %d time\n", 21 - retry);
        gps_reset();

        sprintf(name,"M!TM!TM!T");

        for(int i = 0; i < 9; i++) {
            aos_sleep(2);
            write_number = gps_uart_write(name, 9);
            if(write_number > 0) {
                //gps_log(LOG_INFO, "write %d of %s to uart", write_number, name);
            } else {
                gps_log(LOG_ERROR, "write failed\n");
            }
        }

        ret = gps_uart_read_dir(response, sizeof(response));
        if(ret > 0) {
            if(strchr(response, 'Y') && strchr(response, 'C')) {
                is_get_YC = 1;
                break;
            }
        }

        gps_log(LOG_INFO, "fail to get the response data\n");
        retry --;
    }

    if(!is_get_YC){
        //gps_uart_close();
        return -1;
    }
#if 0
    struct timeval testTime1, testTime2;
    double db_time1, db_time2;
    if (gettimeofday(&testTime1, NULL))
    {
        gps_log(LOG_ERROR, "get current time error.\n");
        return -1;
    }
    db_time1 = testTime1.tv_sec + (double)(testTime1.tv_usec) / 1000000;
    gps_log(LOG_INFO, "download bootloader\n");
#endif
    if(xmodem_send(loader_path, rda_platform_config.bl_size)) {
        gps_log(LOG_ERROR, "bootloader send error!\n");
        return -1;
    }
#if 0
    if (gettimeofday(&testTime2, NULL))
    {
        gps_log(LOG_ERROR, "get current time error.\n");
        return -1;
    }
    db_time2 = testTime2.tv_sec + (double)(testTime2.tv_usec) / 1000000;
    gps_log(LOG_INFO, "***********************D-value for loader %f\n", db_time2 - db_time1);
#endif
    gps_set_baudrate(rda_platform_config.fwDownloadSpeed);
    gps_sleep_ms(60);
#if 0
    if (gettimeofday(&testTime1, NULL))
    {
        gps_log(LOG_ERROR, "get current time error.\n");
        return -1;
    }
    db_time1 = testTime1.tv_sec + (double)(testTime1.tv_usec) / 1000000;
#endif
#if 1
    while(1) {
        memset(response, 0, sizeof(response));
        ret = gps_uart_read(response, sizeof(response));
        if(ret > 0) {
            if(strchr(response, 'Y') || strchr(response, 'C')) {
                break;
            }
        }
    }
#endif
    gps_log(LOG_INFO, "download firmware\n");
    gps_sleep_ms(10);
    //timer1 = aic_get_timestamp();
    if(xmodem_send(firmware_path, rda_platform_config.fw_size) < 0) {
        gps_log(LOG_ERROR, "firmware send error!\n");
        //gps_uart_close();
        return -1;
    }
    //timer2 = aic_get_timestamp();
    //gps_log(LOG_INFO, "download firmware consume time = %d, timer1 = 0x%x, timer2 = 0x%x\n",
    //        timer2 - timer1, timer1, timer2);
#if 0
    if (gettimeofday(&testTime2, NULL))
    {
        printf("get current time error.\n");
        return -1;
    }

    db_time2 = testTime2.tv_sec + (double)(testTime2.tv_usec) / 1000000;
    gps_log(LOG_INFO, "******************************D-value for FW = %f\n", db_time2 - db_time1);
#endif
    return 0;
}

char packet_data[XMODEM_DATA_SIZE];
char frame_data[XMODEM_DATA_SIZE + XMODEM_CRC_SIZE + XMODEM_FRAME_ID_SIZE + 1];
int xmodem_send(char *file_path, u32 total_size)
{
    int pack_count = 0, complete = 0, retry_num = 0;
    unsigned short crc_value;
    unsigned char ack_id;
    int return_value = -1, ret;
    u32 fd, transfer_size;
    int read_number = 0;
    int write_number = 0;
    int send_size = 0;


    ret = fmd_open(file_path, &fd);
    if(ret < 0)
    {
        gps_log(LOG_ERROR, "failed to open file %s retrun %d\n", file_path, fd);
        return -1;
    }

    ack_id = XMODEM_ACK;
    while(!complete)
    {
        switch(ack_id)
        {
            case XMODEM_ACK:
                pack_count ++;
                retry_num = 0;
                transfer_size = total_size > XMODEM_DATA_SIZE ? XMODEM_DATA_SIZE : total_size;
                ret = fmd_read(fd, send_size, transfer_size, packet_data);
                if(transfer_size > 0) {
                    read_number = transfer_size;
                    if(read_number < XMODEM_DATA_SIZE_STX)
                    {
                        //printf("\r\nStart filling the last frame!\n");
                        //is_last = 1;
                        for(; read_number < XMODEM_DATA_SIZE; read_number++)
                            packet_data[read_number] = 0x1A;  // 不足128字节用0x1A填充
                    }

                    frame_data[0] = XMODEM_HEAD;  // 帧开始字符
                    frame_data[1] = (char)pack_count;  // 信息包序号
                    frame_data[2] = (char)(255 - frame_data[1]);  // 信息包序号的补码

                    for(int i=0; i < XMODEM_DATA_SIZE; i++)  // 128字节的数据段
                        frame_data[i+3] = packet_data[i];//把收到的字符和信息头一起打包

                    crc_value = get_crc16(packet_data, XMODEM_DATA_SIZE); // 16位crc校验
                    frame_data[XMODEM_DATA_SIZE+3] = (unsigned char)(crc_value >> 8);// 高八位数据
                    frame_data[XMODEM_DATA_SIZE+4] = (unsigned char)(crc_value);     //低八位数据
                    /* 发送一个包数据，133字节 */
                    write_number = gps_tty_write(frame_data, XMODEM_DATA_SIZE+5);
                    //gps_log(LOG_INFO, "pack: %d size %d", pack_count, write_number);
                    while(gps_uart_read(&ack_id,1) <= 0);
                    if (ack_id == XMODEM_ACK) {
                        //gps_log(LOG_INFO, "ACK OK, Ready sending next package");
                        send_size += transfer_size;
                        total_size -= transfer_size; 
                    }
                    else {
                        gps_log(LOG_ERROR, "ACK Error: %d\n", ack_id);
                    }
                }else {
                    ack_id = XMODEM_EOT;

                    while(ack_id != XMODEM_ACK)
                    {
                        ack_id = XMODEM_EOT;
                        write_number = gps_uart_write(&ack_id,1);
                        while((gps_uart_read(&ack_id, 1)) <= 0);
                    }
                    gps_log(LOG_INFO, "Send file %d successful!!!\n", send_size);
                    complete = 2;
                    //fd = -1;
                }
                break;
                
            case XMODEM_NAK:
                if (retry_num++ > 10)
                {
                    gps_log(LOG_ERROR, "file %s send failed, Quit!\n", file_path);
                    complete = 1;
                }
                else 
                {
                    write_number = gps_tty_write(frame_data, XMODEM_DATA_SIZE+5);
                    //gps_log(LOG_INFO, "pack: %d size %d", pack_count, write_number);
                    while(gps_uart_read(&ack_id,1) <= 0);
                    if (ack_id == XMODEM_ACK) {
                        //gps_log(LOG_INFO, "ACK OK, Ready sending next package");
                        send_size += read_number;
                    }
                    else {
                        gps_log(LOG_ERROR, "ACK Error: %d\n", ack_id);
                    }
                }
                break;
            default:
                gps_log(LOG_ERROR, "fatal error\n");
                complete = 1;
                break;
        }
    }

    if(fd)
        fmd_close(fd);

    if(complete == 2)
        return_value = 0;

    return return_value;
}

void fw_download_thread()
{    
    int count = 0;
    firmware_status = FW_DL_INIT;

    while(firmware_status == FW_DL_INIT) 
    {
        if(firmware_loader(rda_platform_config.bootloaderPath, rda_platform_config.fwPath) != 0) {
            count ++;
            if(count > 2) {
                gps_log(LOG_ERROR, "FW download error\n");
                firmware_status = FW_DL_FAIL;
                break;
            } else{
                continue;
            }
        }
        
        firmware_status = FW_DL_SUCCEED;
        //dl_finish_sem_post();
    }
    
}

s32 wait_for_gps_sleep()
{
    char fw_info[256] = {0};

    gps_get_packet_data(fw_info, 128);
    while(strcasestr(fw_info, "FWVer") == NULL) {
        //gps_sleep_ms(100);
        gps_log(LOG_INFO, "%s\n",fw_info);
        gps_log(LOG_INFO, "wait for fw ready\n");
        gps_get_packet_data(fw_info, 128);
    }
    gps_log(LOG_INFO, "%s", fw_info);
#if 0
    memset(fw_info, 0, 128);
    gps_sleep();
    get_uart_data(fw_info, 128);
    while(strcasestr(fw_info, "$OK") == NULL) {
        gps_log(LOG_INFO, "%s\n",fw_info);
        gps_sleep_ms(100);
        get_uart_data(fw_info, 128);
        gps_log(LOG_INFO, "wait for gps sleep\n");
    }
#endif
    return 0;
}

s32 firmware_download()
{
    s32 ret = -1;
    gps_log(9, "enter %s", __FUNCTION__);
    //HANDLE fwDLHandle = NULL;
    //dl_finish_sem_init();
    //gps_uart_open();

    fw_download_thread();
#if 0
    /*
     * below thread will be suspended for unknown reason on RDA platform
     */
    fwDLHandle = COS_CreateTask(fw_download_thread,
                                NULL, NULL,
                                DEFAULT_GPS_FW_STACK_SIZE,
                                GPS_FW_THREAD_PRIORITY,
                                COS_CREATE_DEFAULT, 0, "GpsFirmware");

    if(dl_finish_sem_time_wait(GPS_DLBLOCK_TIME) < 0) {
#endif

    if(FW_DL_FAIL == firmware_status) {
        gps_log(LOG_ERROR, "GPS FW download timeout\n");
    } else {
        gps_log(LOG_INFO, "GPS FW download success\n");
        ret = 0;
    }

    //dl_finish_sem_destroy();
#if 0
    if(fwDLHandle != NULL) {
        /*wait for task to stop*/
        if(firmware_status == FW_DL_FAIL) {
            COS_StopTask((TASK_HANDLE *)&fwDLHandle);
        }
        gps_sleep_ms(200);
        COS_DeleteTask(fwDLHandle);
        fwDLHandle = NULL;
    }
#endif
    gps_set_baudrate(115200);
    wait_for_gps_sleep();

    return ret;
}

