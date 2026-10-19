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
#define AIC_THIS_FILE    0x0b0a22
#define LOG_MODULE    AMOD_GNSS

#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_init.h>
#include "icoe_gps_serial.h"
#include "icoe_gps_hal.h"
#include "icoe_gps_nmea.h"
#include "icoe_gps_msg.h"
#include "icoe_gps_if.h"
#include "icoe_gps_rt.h"

static pthread_mutex_t startStopMutex;

enum {
    GPS_DELETE_EPHEMERIS = 1, // 0x0001
    GPS_DELETE_ALMANAC = 2, // 0x0002
    GPS_DELETE_POSITION = 4, // 0x0004
    GPS_DELETE_TIME = 8, // 0x0008
    GPS_DELETE_IONO = 16, // 0x0010
    GPS_DELETE_UTC = 32, // 0x0020
    GPS_DELETE_HEALTH = 64, // 0x0040
    GPS_DELETE_SVDIR = 128, // 0x0080
    GPS_DELETE_SVSTEER = 256, // 0x0100
    GPS_DELETE_SADATA = 512, // 0x0200
    GPS_DELETE_RTI = 1024, // 0x0400
    GPS_DELETE_CELLDB_INFO = 32768, // 0x8000
    GPS_DELETE_ALL = 65535, // 0xFFFF
};

HANDLE gpsTaskq = NULL;

int valid_modes[] = {
    POS_MODE_GN,
    POS_MODE_GPS,
    POS_MODE_BD,
    POS_MODE_GN,
};

GpsCallbacks umGpsCallbacks;

void um_gps_create_task(void)
{
    gpsTaskq = gps_thread_create(gps_hal_state_management, NULL, DEFAULT_THREAD_STACK_SIZE);
}

#if 0
static void cleanup_callback(void)
{
    umGpsCallbacks.size = 0;
    umGpsCallbacks.gnss_sv_status_cb = NULL;
    umGpsCallbacks.nmea_cb = NULL;
    umGpsCallbacks.location_cb = NULL;
    umGpsCallbacks.status_cb = NULL;
}
#endif

int um_gps_start()
{
    int ret = 0;

    gps_log(LOG_INFO, "%s\n", __FUNCTION__);
    gps_mutex_lock(&startStopMutex);
    ret = gps_hal_start(GPS_HSM_STARTMODE_NORMAL);
    gps_status_update(GPS_HAL_STATUS_STARTED);
    gps_mutex_unlock(&startStopMutex);

    return ret;
}

int um_gps_stop()
{
    int ret = 0;
    
    gps_mutex_lock(&startStopMutex);    
    ret = gps_hal_stop();
    gps_status_update(GPS_HAL_STATUS_STOPPED);
    gps_mutex_unlock(&startStopMutex);

    gps_log(LOG_INFO, "%s\n", __FUNCTION__);

    return ret;
}

int um_gps_cleanup()
{
    int ret = 0;

    if(gps_hal_get_state() & GPS_HAL_STATE_STARTED)
        ret = um_gps_stop();
    gps_hal_cleanup();
    gps_status_update(GPS_HAL_STATUS_OFF);
    rt_gnss_destroy();
    gps_mutex_destroy(&startStopMutex);
    gps_log(LOG_INFO, "%s\n", __FUNCTION__);

    return ret;
}

int um_gps_inject_time(GpsUtcTime time, int64_t timeReference, int uncertainty)
{
    (void)time;
    (void)timeReference;
    (void)uncertainty;
    return 0;
}

int um_gps_inject_location(double latitude, double longitude, float accuracy)
{
    (void)latitude;
    (void)longitude;
    (void)accuracy;
    return 0;
}

int um_gps_set_freq(u16 freq)
{
    u8 sum;
    u8 length = 0;
    s8 cmd[GPS_CMD_MAXLEN] = {0};

    sprintf(cmd, "$CFGNAV,1000,%d,3", freq);
    length = strlen(cmd);
    sum = bd_check_eor(cmd + 1, length - 1);
    sprintf(cmd + length, "*%02X\r\n",sum);
    gps_send_cmd(MSG_SET_intERVAL, cmd, strlen(cmd));

    sprintf(cmd, "$CFGSAVE\r\n");
    gps_send_cmd(MSG_SET_intERVAL, cmd, strlen(cmd));


    return 0;
}

int um_gps_set_position_mode(uint16_t mode)
{
    u8 sum;
    u8 length = 0;
    u8 rnss_mode = POS_MODE_GN;
    s8 cmd[GPS_CMD_MAXLEN]; 

    mode &= 0x3;
    if(mode) {
        rnss_mode = valid_modes[mode];
    }
    switch(rnss_mode) {
        case POS_MODE_GPS:
            mode = 1;
            break;
        case POS_MODE_BD:
            mode = 10;
            break;
        case POS_MODE_GN:
            mode = 11;
            break;
        default:
            mode=0;
            break;
        }
    // BD: $CFGSYS,h10*5E
    // GN: $CFGSYS,h11*5F
    if(mode) {
        sprintf(cmd, "$CFGSYS,h%02d", mode);
        length = strlen(cmd);
        sum = bd_check_eor(cmd + 1, length - 1);
        sprintf(cmd + length, "*%02X\r\n",sum);
    } else {
        sprintf(cmd, "$CFGSYS\r\n");
    }

    gps_log(5, "set position mode %d\n", mode);
    gps_send_cmd(MSG_SET_POS_MODE, cmd, strlen(cmd));

    sprintf(cmd, "$CFGSAVE\r\n");
    gps_send_cmd(MSG_SAVE_CFG, cmd, strlen(cmd));


    return 0;
}

void um_gps_delete_aiding_data(uint16_t flags)
{
    u8 sum;
    u8 length = 0;
    s8 cmd[GPS_CMD_MAXLEN];
    u8 bits = 0;

    if(flags == GPS_DELETE_ALL) {
        bits |= 0x95;
    } else if(flags & GPS_DELETE_EPHEMERIS) {
        bits |= 0x01;
    } else if(flags & (GPS_DELETE_POSITION|GPS_DELETE_TIME)) {
        bits |= 0x04;
    } else if(flags & GPS_DELETE_IONO) {
        bits |= 0x10;
    } else if(flags & GPS_DELETE_ALMANAC) {
        bits |= 0x80;
    }
    memset(&cmd, 0, sizeof(cmd));
    sprintf(cmd, "$RESET,0,h%02X", bits);
    length = strlen(cmd);
    sum = bd_check_eor(cmd + 1, length - 1);
    sprintf(cmd + length, "*%02X\r\n",sum);
    gps_send_cmd(MSG_DELETE_AIDING, cmd, strlen(cmd));
}

void um_gps_set_start_mode(uint16_t mode)
{
    u8 sum;
    u8 length = 0;
    s8 cmd[GPS_CMD_MAXLEN];
    u8 bits = START_MODE_HOT;  //default 

    if(mode == START_MODE_COLD) {
        bits |= 0x85;
    } else if(mode == START_MODE_WARM) {
        bits |= 0x01;
    }

    memset(&cmd, 0, sizeof(cmd));
    sprintf(cmd, "$RESET,0,h%02X", bits);
    length = strlen(cmd);
    sum = bd_check_eor(cmd + 1, length - 1);
    sprintf(cmd + length, "*%02X\r\n",sum);
    gps_send_cmd(MSG_DELETE_AIDING, cmd, strlen(cmd));
}

void um_gps_req_assist(u8 type)
{
    s8 cmd[GPS_CMD_MAXLEN] = { 0 };
    
    sprintf(cmd, "ASSIST");
    gps_send_cmd(MSG_REQ_ASSIST, cmd, strlen(cmd));
}

void um_gps_fw_info()
{
    s8 cmd[GPS_CMD_MAXLEN];

    sprintf(cmd, "$PDTINFO\r\n");
    gps_send_cmd(MSG_GET_FWINFO, cmd, strlen(cmd));
}

void um_gps_set_baud(uint32_t baud)
{
    u8 sum;
    u8 len = 0;
    s8 cmd[GPS_CMD_MAXLEN];

    sprintf(cmd,"$CFGPRT,,0,%d,129,3", (int)baud);
    len = strlen(cmd);
    sum = bd_check_eor(cmd + 1, len - 1);
    sprintf(cmd + len, "*%02X\r\n",sum);
    gps_send_cmd(MSG_SET_BAUD, cmd, strlen(cmd));
    aos_sleep(2);
    sprintf(cmd, "$CFGSAVE\r\n");
    gps_send_cmd(MSG_SAVE_CFG, cmd, strlen(cmd));

    gps_set_baudrate(baud);
}

void um_gps_set_nmea_output(u8 type,u8 flag, u8 freq)
{
    u8 sum;
    u8 length = 0;
    s8 cmd[GPS_CMD_MAXLEN] = {0};

    if(flag >= 8) {
        sprintf(cmd, "$CFGMSG,%d,,%d", type, freq);
    } else {
        sprintf(cmd, "$CFGMSG,%d,%d,%d", type, flag, freq);
    }
    length = strlen(cmd);
    sum = bd_check_eor(cmd + 1, length - 1);
    sprintf(cmd + length, "*%02X\r\n",sum);

    gps_send_cmd(MSG_SET_intERVAL, cmd, strlen(cmd));
    sprintf(cmd, "$CFGSAVE\r\n");
    gps_send_cmd(MSG_SET_intERVAL, cmd, strlen(cmd));
}

void um_gps_start_debug()
{
    s8 cmd[GPS_CMD_MAXLEN] = {0};

    sprintf(cmd, "$CFGDEBUG,1,1\r\n");
    gps_send_cmd(MSG_START_DEBUG, cmd, strlen(cmd));
}

void um_gps_cb_register(GpsCallbacks* callbacks)
{
    umGpsCallbacks = *callbacks;
}

int um_gps_init(void)
{
    gps_log(LOG_INFO, "enter %s\n", __FUNCTION__);
    nmea_reader_init();
    rt_gnss_init();
    gps_hal_init();
    gps_mutex_init(&startStopMutex);
    gps_hal_set_state(GPS_HAL_STATE_INIT);
    um_gps_create_task();
    gps_status_update(GPS_HAL_STATUS_ON);
    gps_log(LOG_INFO, "leave %s\n", __FUNCTION__);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_L1_INIT_EXPORT(um_gps_init);
