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
#define AIC_THIS_FILE    0x11ab04
#define LOG_MODULE    AMOD_GNSS

#include <slp_inc.h>
#include <pthread.h>
#include "icoe_gps_os.h"
#include "icoe_gps_serial.h"
#include "icoe_gps_hal.h"
#include "icoe_gps_msg.h"
#include "icoe_gps_rt.h"
#include "icoe_gps_nmea.h"
#include "icoe_gps_if.h"

#define FW_DL_REQUIRED

#ifdef FW_DL_REQUIRED
#include "icoe_firmware_loader.h"

u8 firmware_status = FW_DL_INIT;
#endif

#define MAX_CMD_LEN            256

#define GPS_UART_NUM                1  

static u32 gpsHalState = GPS_HAL_STATE_INVALID;
gpsMsgQueueT cmdMsgQ;

static pthread_mutex_t stateMutex;
extern struct gnss_svc gnss_service;
extern GpsCallbacks umGpsCallbacks;
extern HANDLE gpsTaskq;
static GpsLocation umGpsLocation;
static GnssSvStatus umGnssSvStatus;
//static gpsNmea_t umGpsNmea;
static GpsStatus umGpsStatus;
static pthread_mutex_t nmeaBufMutex;
GpsStatus save_gpsStatus;
GpsLocation save_gpsLocation;
GnssSvStatus save_gnssSvStatus;

/*
 * to do: support injecting assistance data
 */
s32 gps_send_cmd(u8 type, s8 *cmd, u32 len)
{
    gpsMsg_t content;
    gpsMsgHdr_t header;
    gpsCmdMsg_t *msg = NULL;
    struct GPS_MSG event;

    memset(&content, 0, sizeof(content));
    header.MsgName = type;
    header.next = NULL;
    if(cmd != NULL) {
        if(len > GPS_CMD_MAXLEN - 1) {
            len = GPS_CMD_MAXLEN - 1;
            *(content.cmd + GPS_CMD_MAXLEN - 1) = '\0';
        }
        memcpy(content.cmd, cmd, len);
    }
    msg = (gpsCmdMsg_t *) gps_malloc(sizeof(gpsCmdMsg_t));
    if(msg) {
        memcpy((void*)&msg->hdr, &header, sizeof(gpsMsgHdr_t));
        memcpy((void*)&msg->cmdMsg, &content, sizeof(gpsMsg_t));
        if(gps_add_msg(&cmdMsgQ, (gpsMsgHdr_t *)msg) != 0 ) {
            gps_log(LOG_ERROR, "cmd %d send failed\n", type);
            gps_free(msg);
            return -1;
        }
    }

    event.ev = UM_GPS_CMD_REQUEST;
    gps_send_qmsg(gpsTaskq, &event, AOS_WAIT_FOREVER);

    return 0;
}


s32 gps_hal_init(void)
{
    gps_mutex_init(&stateMutex);    
	gps_mutex_init(&nmeaBufMutex);
    gps_queue_init(&cmdMsgQ);
    
    gps_mutex_lock(&stateMutex);
    gpsHalState = GPS_HAL_STATE_INIT;
    gps_mutex_unlock(&stateMutex);

    return 0;
}

void gps_hal_firmware_load(void)
{
    struct GPS_MSG event;

    event.ev = UM_GPS_FIRMWARE_LOAD;
    gps_send_qmsg(gpsTaskq, &event, AOS_WAIT_FOREVER);
}

s32 gps_hal_start(u32 startMode)
{
    s32 ret = 0;
    gps_mutex_lock(&stateMutex);
    
    if ((gpsHalState & GPS_HAL_STATE_INVALID) || (gpsHalState & GPS_HAL_STATE_DESTROY)) {
        ret = -1;
    } else if (gpsHalState & GPS_HAL_STATE_STARTED) {
        ret = 0;
    } else {
        switch (startMode)
        {            
        case GPS_HSM_STARTMODE_NORMAL:
            if (gpsHalState & GPS_HAL_STATE_STOPPED
               || gpsHalState & GPS_HAL_STATE_INIT
               || gpsHalState & GPS_HAL_STATE_FW_READY)
            {
                aic_get_wakelock();
                gps_enable_power();
                gps_pin_switch_enable();
                gps_uart_open();
                gps_hal_firmware_load();
            }

            break;
        //currently we do not support other start mode
        default:
            break;
        }
        gpsHalState = GPS_HAL_STATE_STARTED;
    }
    gps_mutex_unlock(&stateMutex);
    
    return ret;
}

s32 gps_hal_stop(void)
{
    s32 ret = 0;
    gps_log(LOG_INFO,"enter %s\n", __FUNCTION__);

    gps_mutex_lock(&stateMutex);
    if (gpsHalState & GPS_HAL_STATE_INVALID || gpsHalState & GPS_HAL_STATE_DESTROY)
    {
        ret = -1;
    } else {
        if ((gpsHalState & GPS_HAL_STATE_STARTED)
             || (gpsHalState & GPS_HAL_STATE_INIT)
             || (gpsHalState & GPS_HAL_STATE_FW_READY))
        {
#ifdef FW_DL_REQUIRED
            firmware_status = FW_DL_INIT;
#endif
            if(gpsHalState & GPS_HAL_STATE_STARTED) {
                gps_queue_clear(&cmdMsgQ);
            }
            gps_uart_close();
            gps_pin_switch_disable();
            gps_disable_power();
            aic_put_wakelock();
        }
        gpsHalState = GPS_HAL_STATE_STOPPED;
    }
    gps_mutex_unlock(&stateMutex);
    gps_log(LOG_INFO,"leave %s\n", __FUNCTION__);

    return ret;
}

s32 gps_hal_cleanup(void)
{
    gps_mutex_lock(&stateMutex);
    gpsHalState = GPS_HAL_STATE_DESTROY;
    gps_mutex_unlock(&stateMutex);
    gps_queue_cleanup(&cmdMsgQ);

#ifdef FW_DL_REQUIRED
    firmware_status = FW_DL_INIT;
#endif
    return 0;
}

u32 gps_hal_set_state(u32 state)
{
    gps_mutex_lock(&stateMutex);
    gpsHalState = state;
    gps_mutex_unlock(&stateMutex);

    return gpsHalState;
}

u32 gps_hal_get_state()
{
    u32 state;
    gps_mutex_lock(&stateMutex);
    state = gpsHalState;
    gps_mutex_unlock(&stateMutex);
    
    return state;
}

s32 gps_status_update(gpsHsmStatus_t hsmStatus)
{
    struct GPS_MSG event;
    gps_log(LOG_INFO,"enter %s\n", __FUNCTION__);

    umGpsStatus.size = sizeof(umGpsStatus);
    switch (hsmStatus)
    {
    case GPS_HAL_STATUS_ON:
        umGpsStatus.status = GPS_STATUS_ENGINE_ON;
        break;

    case GPS_HAL_STATUS_OFF:
        umGpsStatus.status = GPS_STATUS_ENGINE_OFF;
        break;

    case GPS_HAL_STATUS_STARTED:
        umGpsStatus.status = GPS_STATUS_SESSION_BEGIN;
        break;

    case GPS_HAL_STATUS_STOPPED:
        umGpsStatus.status = GPS_STATUS_SESSION_END;
        break;

    default:
        umGpsStatus.status = GPS_STATUS_NONE;
        break;
    }

    event.ev = UM_GPS_STATUS_UPDATE;
    gps_send_qmsg(gpsTaskq, &event, AOS_WAIT_FOREVER);
   
    gps_log(LOG_INFO,"leave %s\n", __FUNCTION__);

    return 0;
}

void um_gps_cmd_handling(void)
{
    gpsCmdMsg_t *msg;
    if (gps_hal_get_state() & GPS_HAL_STATE_STARTED)
    {
        while(gps_queue_empty(&cmdMsgQ) != 0)
        {
            gps_remove_msg(&cmdMsgQ, (gpsMsgHdr_t*)&msg);
            if (msg)
            {
                if(msg->hdr.MsgName != MSG_REQ_ASSIST) {
                    gps_log(LOG_INFO, "cmd: %s\n", msg->cmdMsg.cmd);
                    gps_tty_write((s8*)(msg->cmdMsg.cmd), strlen(msg->cmdMsg.cmd));
                } else {
                    request_assist_data();
                }
                gps_free(msg);
            }
        }
    }
}

GnssSvStatus sv_status;
s32 gps_hal_update_svstatus(GnssSvInfo sv_list[])
{
    struct GPS_MSG event;
    s32 i = 0;
    s32 n = 0;
	/*
	* as hal maintain MAX_SVS SvInfo for compatible with tradition PRN(QZSS PRN will be 195) and system
	* but at specific time, there will be less than half of them.
	* so we put half of them(GNSS_MAX_SVS:128) to report to application
	*/
	memset(&sv_status, 0, sizeof(sv_status));
	for(i = 0; i < MAX_SVS && n < GNSS_MAX_SVS; i++) {
		if(sv_list[i].svid > 0) {
			// Tweak Beidou PRN
			if(sv_list[i].constellation == GNSS_CONSTELLATION_BEIDOU
				&& sv_list[i].svid > BDS_SV_OFFSET) {
				sv_list[i].svid -= BDS_SV_OFFSET;
			} else if(sv_list[i].constellation == GNSS_CONSTELLATION_GALILEO
				&& sv_list[i].svid > GAL_SV_OFFSET) {
					sv_list[i].svid -= GAL_SV_OFFSET;
				}

			memcpy(&sv_status.gnss_sv_list[n], sv_list + i, sizeof(GnssSvInfo));
			sv_status.gnss_sv_list[n].size = sizeof(GnssSvInfo);
			n++;
		}
	}
	sv_status.num_svs = n;
	sv_status.size = sizeof(sv_status);

    umGnssSvStatus = sv_status;

    event.ev = UM_GPS_SVSTATUS_UPDATE;
    gps_send_qmsg(gpsTaskq, &event, AOS_WAIT_FOREVER);

    return 0;
}

s32 gps_hal_update_location(GpsLocation fix)
{
    struct GPS_MSG event;

    umGpsLocation = fix;

    event.ev = UM_GPS_LOCATION_UPDATE;
    gps_send_qmsg(gpsTaskq, &event, AOS_WAIT_FOREVER);

    return 0;
}

void gps_hal_state_management(void)
{
    struct GPS_MSG msg;
    u32 length;

    gps_log(LOG_INFO, "enter gps HAL SM\n");
    while(1)
    {
        gps_receive_qmsg(gpsTaskq, &msg, AOS_WAIT_FOREVER);
        //gps_log(LOG_ERROR, "gpsTaskq msg.ev= 0x%x, buf = 0x%x, length = 0x%x\n", msg.ev, msg.param1, msg.param0);
        switch (msg.ev)
        {
            #ifdef FW_DL_REQUIRED
            case UM_GPS_FIRMWARE_LOAD:
                if (gps_firmware_download_enable()) {
                    if(firmware_status != FW_DL_SUCCEED)
                        firmware_download();

                    if (firmware_status != FW_DL_SUCCEED)
                        break;
                }

                gps_aidtime();
                gps_uart_start_receive();
                if (gnss_service.agnss)
                    um_gps_req_assist(0);
                break;
            #endif

            case UM_GPS_STATUS_UPDATE:
                memcpy(&save_gpsStatus, &umGpsStatus, sizeof(GpsStatus));
                if (umGpsCallbacks.status_cb == NULL) {
                    gps_log(LOG_ERROR, "status call back is null\n");
                } else {
                    umGpsCallbacks.status_cb(&save_gpsStatus);
                }
                break;
            case UM_GPS_LOCATION_UPDATE:
                memcpy(&save_gpsLocation, &umGpsLocation, sizeof(GpsLocation));
                if (umGpsCallbacks.location_cb == NULL) {
                    gps_log(LOG_ERROR, "location call back is null\n");
                } else {
                    umGpsCallbacks.location_cb(&save_gpsLocation);
                }
                break;
            case UM_GPS_SVSTATUS_UPDATE:
                memcpy(&save_gnssSvStatus, &umGnssSvStatus, sizeof(GnssSvStatus));
                if (umGpsCallbacks.gnss_sv_status_cb == NULL) {
                    gps_log(LOG_ERROR, "svstatus call back is null\n");
                } else {
                    umGpsCallbacks.gnss_sv_status_cb(&save_gnssSvStatus);
                }
                break;
            case UM_GPS_CMD_REQUEST:
                um_gps_cmd_handling();
                break;
            case UM_GPS_RECV_IND:
                length = (u32) msg.param0;
                um_gps_nmea_parse(msg.param1, length);
                gps_free_buffer(msg.param2);
                break;
            default:
                break;
        }
    }
    gps_log(LOG_INFO, "exit gps HAL SM\n");
}
