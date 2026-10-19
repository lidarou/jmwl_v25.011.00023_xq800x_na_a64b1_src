/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file aic_wifi_scan.h
 *
 */

#ifndef __AIC_WIFI_SCAN_H__
#define __AIC_WIFI_SCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <aic_type.h>
#include <aic_errno.h>
#include "aic_srv_bus.h"

/*********************
 *      DEFINES
 *********************/
#define WIFI_SCAN_MAC_MAX_LEN               (18)
#define WIFI_SCAN_SSID_MAX_LEN              (32)
#define WIFI_SCAN_CHANNEL_MAX_NUM           (13) /* 2.4G wifi channel*/
#define WFS_EVENT_SCAN_STRUCT_HEAD wfs_event_scan_e event_id;

/**********************
 *      TYPEDEFS
 **********************/

typedef enum
{
    /* the result report when wifiscan finish, with wfs_scan_result_t data */
    WFS_EVENT_SCAN_RESULT,

    /*TODO: the response of start wifiscan, with wfs_scan_start_response_t data */
    WFS_EVENT_SCAN_START_RESP,

    /*TODO: the response of stop wifiscan, with wfs_scan_stop_response_t data */
    WFS_EVENT_SCAN_STOP_RESP,

    /*TODO: WFS_EVENT_SCAN_INTERRUPT  */
    /*TODO: WFS_EVENT_SCAN_FAIL */
    WFS_EVENT_SCAN_MAX
}wfs_event_scan_e;

/**
 * struct wfs_scan_ap_t - the ap info of scan result
 * @channel: the channel of the wifi ap, value:1~13.
 * @mac: the mac address string of the wifi ap.
 * @ssid: the ssid string of the wifi ap.
 * @rssi: the signal strenth of the wifi ap.
 * @security: the security type of the wifi ap.
 *
 * wifi ap info.
 */
typedef struct {
    uint8_t  channel;
    uint8_t  mac[WIFI_SCAN_MAC_MAX_LEN];
    uint8_t  ssid[WIFI_SCAN_SSID_MAX_LEN];
    int32_t rssi;
    uint32_t security;
}wfs_scan_ap_t;

/**
 * struct wfs_scan_result_t - the info of scan result
 * @p_ap_arrayp: the array pointer of the wifi ap.
 * @total_ap_num: the scanned total number of wifi ap.
 * Note: the p_ap_array is already malloced accoding with the total_ap_num,
 *       ande it will be released until next time scan.
 *
 * wifi scan result.
 */
typedef struct {
    /* head macro with event id */
    WFS_EVENT_SCAN_STRUCT_HEAD

    wfs_scan_ap_t *p_ap_array;
    uint32_t total_ap_num;
    uint32_t current_round;
    /* TODO: scan time */
}wfs_scan_result_t;

/**
 * struct wfs_scan_config_t - wifi scan config info.
 * @scan_channel: the channel index array which need scan.
 *               scan_channel[0]=0: scan all channel
 *               scan_channel[]={1,6,11,0,5}: only scan 1,6,11 channel
 * @mac: filter ap which match with this mac address.if none, don't filter.
 * @ssid: filter ap which match with this ssid.if none, don't filter.
 * @time_out: scan timeout. range: 2000~10000 default:5000 unit: millisecond
 * @max_ap_num: the max ap number of scan.
 *
 * wifi scan config info.
 */
typedef struct {
    uint8_t  scan_channel[WIFI_SCAN_CHANNEL_MAX_NUM];    /* range: 1~13 default:scan all*/
    uint8_t  mac[WIFI_SCAN_MAC_MAX_LEN];     /* default: don't filter mac*/
    uint8_t  ssid[WIFI_SCAN_SSID_MAX_LEN];   /* default: don't filter mac*/
    uint32_t time_out;       /* range: 2000~10000 default:5000 unit: millisecond */
    uint32_t max_ap_num;     /* range: 1~100 default:10 */
    uint32_t round;          /* range: 1~3 default:1 */
    bool     ssid_enable;    /* range: 0: disable ssid,not parse ssid. 1: enable ssid,parse ssid. */
}wfs_scan_config_t;

/*TODO: move to correct header file */
typedef int32_t (*srv_func_callback)(void *p_param, uint32_t size);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * aic_srv_wifi_scan_register - register a new client to recevie notification from wifi scan service
 * @p_bus_info: [IN] client information. Reserved for future use
 * @_srv_callback: [IN] client callback
 * Return: wifi scan handle which used to stop scan.
 *         value:valid handle > 0; AIC_EPERM: param error; AIC_EBUSY: scan busy.
 * Description: This function is used to stop wifi scan.
 */
int32_t aic_srv_wifi_scan_register(aic_srv_bus_info_t *p_bus_info, srv_func_callback _srv_callback);

/**
 * aic_srv_wifi_scan_unregister - deregister a client
 * @client_handle: [IN] client handle
 * Return: 0 if success, otherwise -1
 */
int32_t aic_srv_wifi_scan_unregister(int32_t client_handle);

/**
* aic_srv_wifi_scan_start - start wifi scan.
* @p_wifi_scan_cfg: [IN] wifi scan config info.
* @_response_callback: [IN] wifi scan call back to get scan result.
* Return: 0 if success, otherwise -1
* Description: This function is used to start wifi scan.
*/
int32_t aic_srv_wifi_scan_start(wfs_scan_config_t *p_wifi_scan_cfg, srv_func_callback _response_callback);

/**
* aic_srv_wifi_scan_stop - stop wifi scan.
* @wifi_scan_handle: [IN] wifi scan handle when start create.
* @_response_callback: [IN] wifi scan call back to get option result.
* Return: 0 if success, otherwise -1
* Description: This function is used to stop wifi scan.
*/
int32_t aic_srv_wifi_scan_stop(srv_func_callback _response_callback);

/**
* aic_srv_wifi_scan_get_config - get wifi scan config info.
* @p_wifi_scan_cfg: [OUT] out put wifi scan config info.
* Return: 0 if success, otherwise -1
* Description: This function is used to get wifi scan config info.
*/
int32_t aic_srv_wifi_scan_get_config(wfs_scan_config_t *p_wifi_scan_cfg);

/**
* aic_srv_wifi_scan_get_result - return last wifi scan result.
* @p_wifi_scan_result: [OUT] out put wifi scan result.
* Return: 0 if success, otherwise -1
* Description: This function is used to get last wifi scan result.
*/
int32_t aic_srv_wifi_scan_get_result(wfs_scan_result_t *p_wifi_scan_result);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __AIC_WIFI_SCAN_H__ */
