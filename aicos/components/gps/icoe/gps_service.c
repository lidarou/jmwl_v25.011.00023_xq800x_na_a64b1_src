/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0af68d
#define LOG_MODULE    AMOD_GNSS

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_at_process_common.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif
#include <serial_inc.h>
#include "icoe_gps_types.h"
#include "icoe_gps_serial.h"
#include "icoe_gps_hal.h"
#include <icoe_gps_if.h>
#include <gnss_service.h>
#include <timer_inc.h>

//#define GNSS_DEBUG_DATA

struct gnss_svc gnss_service = {
    .name = "gnss_service",
    .output_com = 4,
    .output_enable = 1,
    .at_nmea_output = 0,
};

void gps_location_cb(GpsLocation* location)
{
    int ret;

    ret = aos_get_mutex(gnss_service.mutex, AOS_WAIT_FOREVER);
    AIC_ASSERT(!ret);

    gnss_service.location_status = 1;
    aos_memcpy(&gnss_service.location, location, sizeof(GpsLocation));
    if (gnss_service.gnss_update && gnss_service.gnss_update->location_cb)
        gnss_service.gnss_update->location_cb(location);
    aos_put_mutex(gnss_service.mutex);
#ifdef GNSS_DEBUG_DATA
    aicx_info(0x045c70, "", "GpsLocation:\n");
    aicx_info(0x1bf1c6, "x", "size: 0x%x\n", location->size);
    aicx_info(0x14db42, "x", "flags: 0x%x\n", location->flags);
    aicx_info(0x0ef003, "lf", "latitude: %lf\n", location->latitude);
    aicx_info(0x05fc3f, "lf", "longitude: %lf\n", location->longitude);
    aicx_info(0x012f7a, "lf", "altitude: %lf\n", location->altitude);
    aicx_info(0x0c2b06, "f", "speed: %f\n", location->speed);
    aicx_info(0x0ab865, "f", "bearing: %f\n", location->bearing);
    aicx_info(0x12c810, "f", "accuracy: %f\n", location->accuracy);
    aicx_info(0x180a93, "llx", "timestamp: 0x%llx\n", location->timestamp);
#endif
}

void gps_status_cb(GpsStatus* status)
{
#ifdef GNSS_DEBUG_DATA
    aicx_info(0x09a6e3, "", "GpsStatus:\n");
    aicx_info(0x1bf1c6, "x", "size: 0x%x\n", status->size);
    aicx_info(0x11f6f5, "x", "status: 0x%x\n", status->status);
#endif
    if (gnss_service.gnss_update && gnss_service.gnss_update->status_cb)
        gnss_service.gnss_update->status_cb(status);
}

void gps_gnss_sv_status_cb(GnssSvStatus* sv_info)
{
    int ret;
#ifdef GNSS_DEBUG_DATA
    int i;

    aicx_info(0x137f24, "", "GnssSvStatus:\n");
    aicx_info(0x1bf1c6, "x", "size: 0x%x\n", sv_info->size);
    aicx_info(0x1761c8, "x", "num_svs: 0x%x\n", sv_info->num_svs);
#endif
    ret = aos_get_mutex(gnss_service.mutex, AOS_WAIT_FOREVER);
    AIC_ASSERT(!ret);

    gnss_service.num_svs = sv_info->num_svs;
    aos_memcpy(&gnss_service.sv_info, sv_info, sizeof(GnssSvStatus));
    if (gnss_service.gnss_update && gnss_service.gnss_update->gnss_sv_status_cb)
        gnss_service.gnss_update->gnss_sv_status_cb(sv_info);
    aos_put_mutex(gnss_service.mutex);
#ifdef GNSS_DEBUG_DATA
    for (i = 0; i < sv_info->num_svs; i++) {
        aicx_info(0x12cb52, "fxf", "azimuth: %f, constellation: 0x%x, c_n0_dbhz = %f\n",
            sv_info->gnss_sv_list[i].azimuth,
            sv_info->gnss_sv_list[i].constellation,
            sv_info->gnss_sv_list[i].c_n0_dbhz);
        aicx_info(0x12beca, "fxxx", "elevation = %f,flags =0x%x, size = 0x%x, svid = 0x%x\n",
            sv_info->gnss_sv_list[i].elevation,
            sv_info->gnss_sv_list[i].flags,
            sv_info->gnss_sv_list[i].size,
            sv_info->gnss_sv_list[i].svid);
    }
#endif
}

void gps_nmea_cb(GpsUtcTime timestamp, const char* nmea, int length)
{
#ifdef GNSS_DEBUG_DATA
    aicx_info(0x0e0e0c, "dllxs", "length = %d, 0x%llx: %s\n", length, timestamp, nmea);
#endif
    if (gnss_service.gnss_update && gnss_service.gnss_update->nmea_cb)
        gnss_service.gnss_update->nmea_cb(timestamp, nmea, length);

    if (gnss_service.at_nmea_output) {
        aic_transmit_at_cmd_urc(0, (u8 *)nmea, length);
        aic_transmit_at_cmd_urc(1, (u8 *)nmea, length);
    }

    if (gnss_service.handle)
        uart_write(gnss_service.handle, nmea, length);
}

void gnss_service_enable_agnss(int enable)
{
    gnss_service.agnss = !!enable;
}

int gnss_service_start(void)
{
    return um_gps_start();
}

int gnss_service_stop(void)
{
    int ret;

    ret = um_gps_stop();
    aos_memset(&gnss_service.location, 0, sizeof(GpsLocation));
    aos_memset(&gnss_service.sv_info, 0, sizeof(GnssSvStatus));

    return ret;
}

void gnss_service_cb_register(GpsCallbacks* callbacks)
{
    gnss_service.gnss_update = callbacks;
}

int gnss_service_get_local(GpsLocation* location)
{
    int ret;

    ret = aos_get_mutex(gnss_service.mutex, AOS_WAIT_FOREVER);
    if (ret)
        return AIC_EINTR;
    location = &gnss_service.location;
    aos_put_mutex(gnss_service.mutex);
    return AIC_SUCCESS;
}

int gnss_service_get_sv_status(GnssSvStatus* sv)
{
    int ret;

    ret = aos_get_mutex(gnss_service.mutex, AOS_WAIT_FOREVER);
    if (ret)
        return AIC_EINTR;
    sv = &gnss_service.sv_info;
    aos_put_mutex(gnss_service.mutex);
    return AIC_SUCCESS;
}

int gnss_service_get_sv_info(int *sv_num, int *snr)
{
    GnssSvInfo *sv_ptr;
    int ret, i;

    ret = aos_get_mutex(gnss_service.mutex, AOS_WAIT_FOREVER);
    if (ret)
        return AIC_EINTR;
    *sv_num = gnss_service.sv_info.num_svs;
    sv_ptr = &gnss_service.sv_info.gnss_sv_list[0];
    *snr = sv_ptr->c_n0_dbhz;

    for (i = 1; i < (*sv_num); i++) {
        sv_ptr = &gnss_service.sv_info.gnss_sv_list[i];
        if (*snr < sv_ptr->c_n0_dbhz)
            *snr = sv_ptr->c_n0_dbhz;
    }
    aos_put_mutex(gnss_service.mutex);
    return AIC_SUCCESS;
}

u32 gnss_open_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *result_line0[2] = {"+GNSS: (1, 0)", "OK"};
    ulong *line_pointers;

    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 gnss_open_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *state_string[3] = {"+GNSS: open", "+GNSS: close", "OK"};
    u8 *result_line0[2];
    ulong *line_pointers;

    result_line0[0] = state_string[!gnss_service.gnss_status];
    result_line0[1] = state_string[2];

    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

u32 gnss_open_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *state_string[2] = {"+GNSS: open", "+GNSS: close"};
    u8 at_return_value[50] = {'\0'};
    ulong *paras_arr, *line_pointers;
    u8 *result_line0[2] = {"OK"};
    u16 paras_count, line_num;
    int ret;

     paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT^IGNSS=", &paras_count);
     if (paras_arr && (paras_count == 1)) {
         if (atoi((u8 *) paras_arr[0])) {
            gnss_service.gnss_status = 1;
            ret = um_gps_start();
         } else {
            gnss_service.gnss_status = 0;
            ret = um_gps_stop();
         }
     } else {
        ret = 500;
     }

     if (!ret) {
         result_line0[0] = state_string[!gnss_service.gnss_status];
         result_line0[1] = "OK";
         line_pointers = (ulong *)&result_line0;
         line_num = 2;
     } else {
         snprintf(at_return_value, 50, "+CME ERROR: %d", ret);
         line_pointers = (ulong *)&at_return_value;
         line_num = 1;
     }
     aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, line_num);
     aic_free_paras_mem(paras_arr, paras_count);
     return AIC_SUCCESS;
}

u32 gnss_nmea_open_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *result_line0[2] = {"+IGNSSNMEA: (1, 0)", "OK"};
    ulong *line_pointers;

    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 gnss_nmea_open_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 line_num, paras_count, nmea_out;
    u8 *state_str[2] = {"close", "open"};
    ulong *paras_arr, *line_pointers;
    u8 *result_line0[2];

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                            "AT^IGNSSNMEA=", &paras_count);
    if (paras_arr && (paras_count == 1)) {
        nmea_out = atoi((u8 *) paras_arr[0]);
        result_line0[0] = state_str[!!nmea_out];
        gnss_service.at_nmea_output = !!nmea_out;
        result_line0[1] = "OK";
        line_num = 2;
    } else {
        result_line0[0] = "+CME ERROR: 100";
        line_num = 1;
    }

    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, line_num);
    aic_free_paras_mem(paras_arr, paras_count);

    return AIC_SUCCESS;
}

u32 gnss_info_open_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 at_return_value[100] = {'\0'};
    ulong *line_pointers;
    u8 *result_line0[2];
    u16 line_num;

    if (gnss_service.location_status) {
        snprintf(at_return_value, 100, "+IGNSSINFO: N%lf, E%lf, %lf, %lf, %d",
        gnss_service.location.latitude, gnss_service.location.longitude,
        gnss_service.location.altitude, gnss_service.location.speed,
        gnss_service.num_svs);
        result_line0[0] = at_return_value;
        result_line0[1] = "OK";
        line_num = 2;
    } else {
        result_line0[0] = "+CME ERROR: 100";
        line_num = 1;
    }
    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, line_num);
    return AIC_SUCCESS;
}

u32 gnss_bootm_open_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *result_line0[2] = {"+GNSSBOOTM: (2, 1, 0)", "OK"};
    ulong *line_pointers;

    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 gnss_bootm_open_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *state_string[4] = {"+GNSS: hot boot", "+GNSS: cold boot",
                "+GNSS: warm boot", "+CME ERROR: 200"};
    ulong *paras_arr, *line_pointers;
    u8 *result_line0[2];
    u16 paras_count, mode, line_num;
    int ret = AIC_SUCCESS;

     paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT^IGNSSBOOTM=", &paras_count);
     if (paras_arr && (paras_count == 1)) {
         gnss_service.location_status = 0;
         mode = atoi((u8 *) paras_arr[0]);
         um_gps_set_start_mode(mode);
         ret = gps_aidtime();
         if (!ret && gnss_service.agnss)
            um_gps_req_assist(0);
     } else {
        ret = AIC_EINVAL;
     }

     if (!ret) {
         result_line0[0] = state_string[mode];
         result_line0[1] = "OK";
         line_num = 2;
     } else {
         result_line0[0] = state_string[3];
         line_num = 1;
     }

     line_pointers = (ulong *)&result_line0;
     aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, line_num);
     aic_free_paras_mem(paras_arr, paras_count);
     return AIC_SUCCESS;
}

u32 gnss_agnss_open_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *result_line0[2] = {"+AGNSS: (1, 0)", "OK"};
    ulong *line_pointers;

    line_pointers = (ulong *)&result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

u32 gnss_agnss_open_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 *state_string[3] = {"+AGNSS: close", "+AGNSS: open", "+CME ERROR: 300"};
    u16 paras_count, agnss_status, line_num;
    ulong *paras_arr, *line_pointers;
    u8 *result_line0[2];

     paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT^AGNSS=", &paras_count);
     if (paras_arr && (paras_count == 1)) {
         agnss_status = atoi((u8 *) paras_arr[0]);
         gnss_service.agnss = !!agnss_status;
         result_line0[0] = state_string[!!agnss_status];
         result_line0[1] = "OK";
         line_num = 2;
     } else {
         result_line0[0] = state_string[2];
         line_num = 1;
     }

     line_pointers = (ulong *)&result_line0;
     aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, line_num);
     aic_free_paras_mem(paras_arr, paras_count);
     return AIC_SUCCESS;
}

static u32 atblt_gnss_test_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    int ret, sv_num, snr, search_sv = 0, search_sv_snr = 0;
    u64 timeout = 60000, cur_time;
    u16 count, line_num = 1;
    u8 *result_line[2], result_str[100] = {'\0'};
    ulong *paras_arr, rsp  = (ulong)result_line;

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT^GNSSTEST=", &count);
    if (!paras_arr || count != 2) {
        result_line[0] = "ERROR: arguments error";
        goto AT_REPLY;
    }

    ret = gnss_service_start();
    if (ret) {
        result_line[0] = "ERROR: GNSS open fail";
        goto AT_REPLY;
    }

    sv_num = strtoul((char *)paras_arr[0], NULL, 10);
    snr = strtoul((char *)paras_arr[1], NULL, 10);

    if ((sv_num < 0) || (snr < 0)) {
        result_line[0] = "ERROR: arguments error, sv_num or snr < 0";
        goto AT_REPLY;
    }

    result_line[1] = "PASS";
    cur_time = aic_get_tick();
    timeout = cur_time + timeout;
    result_line[0] = "GNSS search wait(1 min)......";
    aic_fill_transmit_cmd_resp(at_req_res_ptr, (ulong *)rsp, line_num);
    while (((sv_num > search_sv) || (snr > search_sv_snr)) &&
        (timeout > cur_time)) {
        gnss_service_get_sv_info(&search_sv, &search_sv_snr);
        cur_time = aic_get_tick();
    }

    if (timeout <= cur_time)
        result_line[1] = "FAIL: gnss search timeout";

    line_num = 2;
    snprintf(result_str, 99, "search satellite num = %d, max snr = %d",
        search_sv, search_sv_snr);
    aicx_info(0x090928, "s", "%s\n", result_str);
    result_line[0] = result_str;
AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req_res_ptr, (ulong *)rsp, line_num);
}

static u32 atblt_gnss_off_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong rsp;
    int ret;

    rsp = (ulong)"SUCCESS";
    ret = gnss_service_stop();
    if (ret)
        rsp = (ulong)"ERROR: GNSS open fail";

    return aic_fill_transmit_cmd_resp(at_req_res_ptr, &rsp, 1);
}

static AT_CMD_HANDLE gnss_at_handles[] = {
    {"AT^IGNSS", gnss_open_cmd, gnss_open_query_cmd,
                gnss_open_set_cmd, AIC_NULL},
    {"AT^IGNSSINFO", AIC_NULL, AIC_NULL, gnss_info_open_set_cmd, AIC_NULL},
    {"AT^IGNSSNMEA", gnss_nmea_open_cmd, AIC_NULL, gnss_nmea_open_set_cmd, AIC_NULL},
    {"AT^IGNSSBOOTM", gnss_bootm_open_cmd, AIC_NULL, gnss_bootm_open_set_cmd, AIC_NULL},
    {"AT^AGNSS", gnss_agnss_open_cmd, AIC_NULL, gnss_agnss_open_set_cmd, AIC_NULL},
    /* a: spec min numbers >=b; b: spec max snr >=c */
    {"AT^GNSSTEST", AIC_NULL, AIC_NULL, atblt_gnss_test_set_cmd, AIC_NULL},
    {"AT^GNSSOFF", AIC_NULL, AIC_NULL, atblt_gnss_off_set_cmd, AIC_NULL},
    {AIC_NULL},
};

GpsCallbacks gps_cb = {
    .location_cb =  gps_location_cb,
    .status_cb = gps_status_cb,
    .gnss_sv_status_cb = gps_gnss_sv_status_cb,
    .nmea_cb = gps_nmea_cb,
};

int gps_init(void)
{
    gnss_service.mutex = aos_create_mutex(gnss_service.name);
    AIC_ASSERT(gnss_service.mutex);

    if (gnss_service.output_enable) {
        gnss_service.handle = uart_open(gnss_service.output_com);
        if (gnss_service.handle)
            uart_set_baud_rate(gnss_service.handle, 115200);
    }
    um_gps_cb_register(&gps_cb);
    aic_add_at_cmd_handle(gnss_at_handles);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_L2_INIT_EXPORT(gps_init);
