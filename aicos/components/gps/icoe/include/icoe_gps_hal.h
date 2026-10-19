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

#ifndef _GPS_HAL_H_
#define _GPS_HAL_H_

#include "icoe_gps_types.h"
#include "icoe_gps_os.h"

typedef struct
{
    int             prn;
    float           cno;
    float           elevation; /* degrees */
    float           azimuth; /* degrees */
} gpsSvInfo_t;

typedef struct
{
    u16             num_svs;
    gpsSvInfo_t     sv_list[GPS_MAX_SVS];
    u32             ephemeris_mask;
    u32             almanac_mask;
    u32             used_in_fix_mask;
} gpsSvStatus_t;

typedef struct
{
    u16             num_svs_gln;
    gpsSvInfo_t     gln_sv_list[GLN_MAX_SVS];
    u32             gln_ephemeris_mask;
    u32             gln_almanac_mask;
    u32             gln_used_in_fix_mask;
} glnSvStatus_t;

typedef struct
{
    gpsSvStatus_t gps_sv_status;
    glnSvStatus_t gln_sv_status;
} gnssSvStatus_t;

typedef struct
{
    DBL             utcTime;
    u16             msgLen;
    s8              nmeaInfo[GPS_NMEA_STRING_MAXLEN];
} gpsNmea_t;


enum
{
    GPS_HSM_STARTMODE_NORMAL,
    GPS_HSM_STARTMODE_TEST_COLD,
    GPS_HSM_STARTMODE_TEST_HOT,
    GPS_HSM_STARTMODE_TEST_WARM,
};

//#define GPS_CMD_MAXLEN      (32)
#define MAX_NMEA_BUF_NUM    (2)
/*
 * Android status
 */
typedef enum
{
    GPS_HAL_STATUS_INVALID,
    GPS_HAL_STATUS_ON,
    GPS_HAL_STATUS_STARTED,
    GPS_HAL_STATUS_STOPPED,
    GPS_HAL_STATUS_OFF
} gpsHsmStatus_t;

enum
{
    GPS_HAL_STATE_INVALID  = 0x00000001,
    GPS_HAL_STATE_DESTROY  = 0x00000010,
    GPS_HAL_STATE_INIT     = 0x00000100,
    GPS_HAL_STATE_FW_READY = 0x00001000,
    GPS_HAL_STATE_STARTED  = 0x00010000,
    GPS_HAL_STATE_STOPPED  = 0x00100000,
};

enum
{
    UM_GPS_NONE            = 0x00000000,
    UM_GPS_STATUS_UPDATE   = 0x00000001,
    UM_GPS_LOCATION_UPDATE = 0x00000010,
    UM_GPS_SVSTATUS_UPDATE = 0x00000100,
    UM_GPS_NMEA_UPDATE     = 0x00001000,
    UM_GPS_CMD_REQUEST     = 0x00010000,
    UM_GPS_RECV_IND        = 0x00100000,
    UM_GPS_FIRMWARE_LOAD   = 0x01000000,
    UM_GPS_THREAD_EXIT     = 0x80000000,
};

typedef struct
{
    u16      usYear;                             /* Year                */
    u8       ucMonth;                            /* Month (1-12)        */
    u8       ucDay;                              /* Day (1-31)          */
    u8       ucHour;                             /* Hour (0-23)         */
    u8       ucMin;                              /* Minute (0-59)       */
    u8       ucSec;                              /* Second (0-59)       */
    u16      usMSec;                             /* Millisecond (0-999) */
    u32      offset;
} gpsHsmUtcTimeT;

s32 gps_send_cmd(u8 type, s8 *cmd, u32 len);
s32 gps_hal_init(void);
s32 gps_hal_start(u32 startMode);
s32 gps_hal_stop(void);
u32 gps_hal_set_state(u32 state);
u32 gps_hal_get_state();
s32 gps_status_update(gpsHsmStatus_t hsmStatus);
s32 gps_hal_update_svstatus(GnssSvInfo sv_list[]);
s32 gps_hal_update_location(GpsLocation fix);
void gps_hal_state_management(void);
s32 gps_hal_cleanup(void);
#endif
