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

#ifndef __GPS_HOST_IF_H__
#define __GPS_HOST_IF_H__

#include "icoe_gps_types.h"

enum {
    START_MODE_HOT,
    START_MODE_COLD,
    START_MODE_WARM,
};

struct gnss_svc{
    char *name;
    int output_com;
    u32 output_enable;
    u32 at_nmea_output;
    void *handle;
    u32 gnss_status;
    u32 agnss;
    u32 location_status;
    GpsLocation location;
    GnssSvStatus sv_info;
    int num_svs;
    GpsCallbacks* gnss_update;
    aos_mutex_ptr mutex;
};

void um_gps_create_task(void);
void um_gps_cb_register(GpsCallbacks* callbacks);
int um_gps_start();
int um_gps_stop();
int um_gps_cleanup();
int um_gps_set_freq(u16 freq);
void um_gps_delete_aiding_data(uint16_t flags);
int um_gps_set_positu8n_mode(uint16_t mode);
void um_gps_set_start_mode(uint16_t mode);
void um_gps_req_assist(u8 type);
void um_gps_fw_info();
void um_gps_set_baud(uint32_t baud);
void um_gps_set_nmea_output(u8 type,u8 flag, u8 freq);

#endif
