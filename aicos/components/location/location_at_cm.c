/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file location_at_cm.c
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e7fc7
#define LOG_MODULE    AMOD_SYS

/*********************
 *      INCLUDES
 *********************/
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_osa_ex.h>
#include <aic_type.h>
#include <aic_errno.h>
#include "aic_at_process_common.h"
#include "aic_srv_tele_ex.h"
#include "mbedtls/md5.h"


#include <aic_https_client.h>
#ifdef SUPPORT_CJSON
#include <cJSON.h>
#endif
#include <math.h>
#include "CM_AT_LBS.h"
#include "aic_md_api.h"
#include "aic_md_id.h"

/*********************
 *      DEFINES
 *********************/

#ifdef CONFIG_CUSTOMER_CM_AT

#define AT_MLBS_CFG     "AT+MLBSCFG"
#define AT_MLBS_LOC     "AT+MLBSLOC"

#define RESP_LBS_CFG    "MLBSCFG"
#define RESP_LBS_LOC    "MLBSLOC"

#else

#define AT_MLBS_CFG     "AT+GLBSCFG"
#define AT_MLBS_LOC     "AT+GLBSLOC"

#define RESP_LBS_CFG    "GLBSCFG"
#define RESP_LBS_LOC    "GLBSLOC"

#endif

#define LCS_AT_URC_RES_MAX_LEN    (60)
#define LCS_AT_CM_PID_MAX_LEN     (20)
#define LCS_AT_CM_IMEI_MAX_LEN    (15)
#define LCS_AT_CM_HTTP_NAME       "LBS CM HTTP"

#define LCS_CHECK_POINTER_VALID(ptr) if (NULL == ptr) { \
                app_loge("[%s] pointer is NULL.\r\n", __func__); \
                return AIC_EPERM; }


#define LOCATION_TASK_PRIORITY  20
#define MAX_WAIT_TIME 2000
#define RESPONSE_BUF 300

#define LCS_CM_AT_CELLLIST_STRING_MAX 150
#define LCS_CM_AT_CELLINFO_STRING_MAX 30

#define POST_JSON_FORMAT_SIZE 53
#define POST_JSON_FORMAT "{\"pid\": \"%s\",\"cellList\": \"%s\",\"batchId\": \"%s\"}"
#define LCS_CM_METHOD_CELL 40

#define LON_LAT_AT_LEN 20
#define LON_AND_LAT_AT_LEN 40
#define RADIUS_AT_LEN 20

//errcode
#define AT_ERROR_LCS_CM_ERR_HTTPERROR 101
#define AT_ERROR_LCS_CM_ERR_MISS_APIKEY 102
#define AT_ERROR_LCS_CM_ERR_UNKNOWN 103
#define AT_ERROR_LCS_CM_ERR_WRONG_PARAM 104
#define AT_ERROR_LCS_CM_ERR_LBS_BUSY 105
#define AT_ERROR_LCS_CM_STAT_ERR_TIMEOUT 120
#define AT_ERROR_LCS_CM_STAT_ERR_WRONG_APIKEY 121
#define AT_ERROR_LCS_CM_STAT_ERR_WRONG_PARAM 122
#define AT_ERROR_LCS_CM_STAT_ERR_OUT_OF_DAILY_MAX 123
#define AT_ERROR_LCS_CM_STAT_ERR_OUT_OF_QPS 124
#define AT_ERROR_LCS_CM_STAT_ERR_OUT_OF_TOTAL_MAX 125
#define AT_ERROR_LCS_CM_STAT_ERR_UNKNOWN 126

#define LCM_CM_AT_MAX_ASCII_MD5               16

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    LCS_CM_IDLE,
    LCS_CM_PREACTION,
    LCS_CM_HTTP_DOING,
    LCS_CM_HTTP_DONE
}lcs_cm_status_e;

typedef enum {
    LCS_CM_FIRST_STARTUP = 0,
    LCS_CM_NON_FIRST
}lcs_cm_startup_status_e;

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

const double pi = 3.14159265358979323846;
const double ee = 0.00669342162296594323;
const double a = 6378245.0;
static lcs_cm_status_e s_lcs_cm_status = LCS_CM_IDLE;
static u32 s_lcs_cm_urc_port_id = 0;
static u32 s_lcs_cm_method = 40;/* default: 40 not support others */
static u32 s_lcs_cm_precision = 8;/* range: (1 ~ 8), default: 8 */
static u32 s_lcs_cm_nearbtsen = 0;/* range: (0,1), default: 0 */
static u32 s_lcs_cm_coordinate = 1;/* range: (0,1), default: 1 */
static u8 s_lcs_cm_pid[LCS_AT_CM_PID_MAX_LEN + 1] = {0};/* default: ofvbkP3AU9 */
static u8 s_lcs_cm_pid_default[LCS_AT_CM_PID_MAX_LEN + 1] = "ofvbkP3AU9";/* default: ofvbkP3AU9 */
static u32 s_lcs_cm_http_handle = 0;

static char s_lcs_cm_url[] = "https://os.iot.10086.cn/api/ops/cms/lbs/report";
static u32 s_lcs_cm_url_port = 443;
static char s_lcs_cm_contenttype[] = "application/json";
static aos_thread_id location_thread = AOS_INVALID_THREAD_ID;
static u8 first_startup = LCS_CM_FIRST_STARTUP;
char *jsonstring = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/

void mydtostr(double in,char * out)
{
    int integer_part = (int) in ;
    int fractional_part1 = (int) ((in - integer_part)*10000);
    int fractional_part2 = (int) (((in - integer_part)*10000 - fractional_part1)*10000);
    snprintf(out,LON_LAT_AT_LEN,"%d.%04d%04d",integer_part,fractional_part1,fractional_part2);
    aicx_info(0x02f8fc, "s", "[mydtostr] out(%s)\r\n", out);
}

int32_t lcs_at_cm_save_nv()
{
    LBS_T nv_data = {0};
    int status = 0;

    nv_data.COORDINATE = s_lcs_cm_coordinate;
    nv_data.METHOD = s_lcs_cm_method;
    nv_data.NEARBTSEN = s_lcs_cm_nearbtsen;
    nv_data.PRECISION = s_lcs_cm_precision;
    strncpy(nv_data.PID, s_lcs_cm_pid,strlen(s_lcs_cm_pid));

    status = aic_md_write(MD_ID_CM_AT_LBS, CM_AT_LBS_STRUCT_SIZE, (void *)&nv_data);

    return status;
}

int32_t lcs_at_cm_load_nv()
{
    LBS_T nv_data = {0};
    int status = 0;

    status = aic_md_read(MD_ID_CM_AT_LBS, CM_AT_LBS_STRUCT_SIZE, &nv_data);

    s_lcs_cm_coordinate = nv_data.COORDINATE;
    s_lcs_cm_method = nv_data.METHOD;
    s_lcs_cm_nearbtsen = nv_data.NEARBTSEN;
    s_lcs_cm_precision = nv_data.PRECISION;
    strncpy(s_lcs_cm_pid,nv_data.PID,strlen(nv_data.PID));

    return 0;
}

double location_at_cm_transformlat(double lat, double lng) {
    double ret = -100.0 + 2.0 * lat + 3.0 * lng + 0.2 * lng * lng + 0.1 * lat * lng + 0.2 * sqrt(fabs(lat));
    ret += (20.0 * sin(6.0 * lat * pi) + 20.0 * sin(2.0 * lat * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(lng * pi) + 40.0 * sin(lng / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(lng / 12.0 * pi) + 320 * sin(lng * pi / 30.0)) * 2.0 / 3.0;
    return ret;
}

double location_at_cm_transformlng(double lat, double lng) {
    double ret = 300.0 + lat + 2.0 * lng + 0.1 * lat * lat + 0.1 * lat * lng + 0.1 * sqrt(fabs(lat));
    ret += (20.0 * sin(6.0 * lat * pi) + 20.0 * sin(2.0 * lat * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(lat * pi) + 40.0 * sin(lat / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(lat / 12.0 * pi) + 300.0 * sin(lat / 30.0 * pi)) * 2.0 / 3.0;
    return ret;
}

u8 location_at_cm_out_of_china(double lat, double lng) {
    if (lng < 72.004 || lng > 137.8347) {
        return 1;
    } else if (lat < 0.8293 || lat > 55.8271) {
        return 1;
    }
    return 0;
}

void location_at_cm_wgs84_to_gcj02(double *lat, double *lng) {
    if (location_at_cm_out_of_china(*lat, *lng)) {
        return;
    }
    double dlat = location_at_cm_transformlat(*lat - 35.0, *lng - 105.0);
    double dlng = location_at_cm_transformlng(*lat - 35.0, *lng - 105.0);
    double radlat = *lat / 180.0 * pi;
    double magic = sin(radlat);
    magic = 1 - ee * magic * magic;
    double sqrtmagic = sqrt(magic);
    dlat = (dlat * 180.0) / ((a * (1 - ee)) / (magic * sqrtmagic) * pi);
    dlng = (dlng * 180.0) / (a / sqrtmagic * cos(radlat) * pi);
    *lat = *lat + dlat;
    *lng = *lng + dlng;
}


static u8* lcs_at_cm_get_pid(void)
{
    if(s_lcs_cm_pid[0]==0){
        return s_lcs_cm_pid_default;
    } else{
        return s_lcs_cm_pid;
    }
}

static u32 lcs_at_cm_urc_result(u8 *urc_rest_ptr)
{
    u8 urc_str[AT_MAX_AP_RESP_LENGTH] = {'\0'};

    snprintf(urc_str, AT_MAX_AP_RESP_LENGTH, "%s \r\n", urc_rest_ptr);
    aic_transmit_at_data(s_lcs_cm_urc_port_id, urc_str, strlen(urc_str));
    return AIC_SUCCESS;
}

static u32 lcs_at_cm_resp_ok(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 result_str[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};

    strcpy(result_str, "OK");
    line_pointers[0] = (ulong)result_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}

static u32 lcs_at_cm_resp_result(AT_REQ_RESP *at_req_resp_ptr, u8 *prefix_ptr, u8 *resp_rest_ptr)
{
    u8 ok_str[] = "OK";
    u8 result_str[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    LCS_CHECK_POINTER_VALID(at_req_resp_ptr);
    LCS_CHECK_POINTER_VALID(prefix_ptr);
    LCS_CHECK_POINTER_VALID(resp_rest_ptr);
    line_pointers[1] = (ulong)ok_str;

    snprintf(result_str, sizeof(result_str), "%s: %s", prefix_ptr, resp_rest_ptr);
    line_pointers[0] = (ulong)result_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 2);
    return AIC_SUCCESS;
}
static u32 lcs_at_cm_resp_error_code(AT_REQ_RESP *at_req_resp_ptr, int error)
{
    u8 result_str[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};

    LCS_CHECK_POINTER_VALID(at_req_resp_ptr);
    snprintf(result_str, sizeof(result_str), "+CME ERROR: %d", error);
    line_pointers[0] = (ulong)result_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}
static u32 lcs_at_cm_resp_error_code_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 result_str[] = "ERROR";
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};

    LCS_CHECK_POINTER_VALID(at_req_resp_ptr);
    line_pointers[0] = (ulong)result_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}

static u32 lcs_at_cm_test_result(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 ok_str[10] = "OK";
    u8 result_str[] = "+"RESP_LBS_CFG": \"method\",(40)\r\n+"RESP_LBS_CFG": \"precision\",(1-8)\r\n+"RESP_LBS_CFG": \"pid\",<pid>\r\n+"RESP_LBS_CFG": \"nearbtsen\",(0,1)";
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE] = {0};
    line_pointers[0] = (ulong)result_str;
    line_pointers[1] = (ulong)ok_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 2);
    return AIC_SUCCESS;
}
//lat 纬度，lon 经度
//网络回来的先纬度再经度，AT要求返回先经度再纬度，所以需要解析调换。同时根据精度要求处理位数
static u32 lcs_at_cm_decode_lon_and_lat(u8 *httpcode){

    u8 tmplon[LON_LAT_AT_LEN] = {0};
    u8 tmplat[LON_LAT_AT_LEN] = {0};
    u8* offset = NULL;
    u16 lon_length = 0;
    u16 lat_length = 0;
    u16 i = 0;
    double dLat = 0;
    double dLon = 0;

    offset = strstr(httpcode,",");
    if (NULL == offset) {
        aicx_error(0x16cbee, "d", "[lcs_at_cm_decode_lon_and_lat] offset is NULL line(%d)\r\n", __LINE__);
        return -1;
    }

    lat_length = offset - httpcode;
    offset ++;
    lon_length = strlen(httpcode) - lat_length - 1;
    memcpy(tmplat,httpcode,lat_length);
    memcpy(tmplon,offset,lon_length);

    if(s_lcs_cm_coordinate == 1){
        dLat = atof(tmplat);
        dLon = atof(tmplon);
        location_at_cm_wgs84_to_gcj02(&dLat,&dLon);
#ifdef CONFIG_LINKER_USE_LIBC
        snprintf(tmplat, LON_LAT_AT_LEN, "%f",dLat);
        snprintf(tmplon, LON_LAT_AT_LEN, "%f",dLon);
#else
        mydtostr(dLat,tmplat);
        mydtostr(dLon,tmplon);
#endif
    }

    offset = strstr(tmplon,".");
    if (NULL == offset) {
        aicx_error(0x16cbee, "d", "[lcs_at_cm_decode_lon_and_lat] offset is NULL line(%d)\r\n", __LINE__);
        return -1;
    }
    offset++;
    for(i=0;i<s_lcs_cm_precision;i++){
        if(offset[i]!=0){
            continue;
        }else{
            offset[i] = 48;
        }
    }
    offset[i] = 0;

    offset = strstr(tmplat,".");
    if (NULL == offset) {
        aicx_error(0x16cbee, "d", "[lcs_at_cm_decode_lon_and_lat] offset is NULL line(%d)\r\n", __LINE__);
        return -1;
    }
    offset++;
    for(i=0;i<s_lcs_cm_precision;i++){
        if(offset[i]!=0){
            continue;
        }else{
            offset[i] = 48;
        }
    }
    offset[i] = 0;

    snprintf(httpcode, LON_AND_LAT_AT_LEN, "%s,%s",tmplon,tmplat);
    aicx_info(0x11e711, "s", "[lcs_at_cm_decode_lon_and_lat] httpcode = %s\r\n",httpcode);
    return AIC_SUCCESS;
}
static u32 lcs_at_cm_encode( char *p_imei ,char *p_celllist){
#ifdef SUPPORT_CJSON
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    LCS_CHECK_POINTER_VALID(root);
    cJSON_AddStringToObject(root, "pid", lcs_at_cm_get_pid());
    if(p_celllist[0]!=0)
    cJSON_AddStringToObject(root, "cellList", p_celllist);
    if(p_imei[0]!=0)
    cJSON_AddStringToObject(root, "batchId", p_imei);
    jsonstring = cJSON_Print(root);
    cJSON_Delete(root);
#else
    u16 json_len = 0;
    json_len = POST_JSON_FORMAT_SIZE+  strlen(lcs_at_cm_get_pid()) +  strlen(p_celllist) +  strlen(p_imei);
    jsonstring = aos_malloc(json_len);
    LCS_CHECK_POINTER_VALID(jsonstring);
    snprintf(jsonstring,json_len,POST_JSON_FORMAT,lcs_at_cm_get_pid(),p_celllist,p_imei);
#endif
    aicx_info(0x0a062a, "s", "[lbs at] jsonstring = %s\r\n",jsonstring);
    return AIC_SUCCESS;
}
static void lcs_at_cm_clean_json_string(void){
    if(jsonstring!=NULL){
        free(jsonstring);
        jsonstring = NULL;
    }
}
static u32 lcs_at_cm_get_cellinfo(void)
{
    int32_t ret = 0;
    u16 i = 0;
    char celllist_str[LCS_CM_AT_CELLLIST_STRING_MAX] = {'\0'};
    char cur_str[LCS_CM_AT_CELLINFO_STRING_MAX] = {'\0'};
    char neighbor_string[LCS_CM_AT_CELLINFO_STRING_MAX] = {'\0'};
    ts_radio_neighbor_cell_info_list_t s_lcs_cm_cell_neighbor = {0};
    ts_radio_cell_info_t s_lcs_cm_cell_current = {0};
    u8 s_lcs_cm_imei[LCS_AT_CM_IMEI_MAX_LEN + 1] = {0};

    /* 1. get current cellinfo */
    ret = aic_srv_tele_radio_get_cell_info_sync(0, &s_lcs_cm_cell_current);
    if(ret == -1){
        return AIC_EPERM;
    }
    if(s_lcs_cm_cell_current.tac == 0){
        return AIC_EPERM;
    }
    snprintf(cur_str, sizeof(cur_str), "%s,%s,%ld,%ld, %ld", s_lcs_cm_cell_current.plmn_mcc,s_lcs_cm_cell_current.plmn_mnc,
                                    s_lcs_cm_cell_current.tac, s_lcs_cm_cell_current.cellid,s_lcs_cm_cell_current.rsrp);
    strcpy(celllist_str,  cur_str);

    /* 2. get neighbor cellinfo */
    if (s_lcs_cm_nearbtsen) {
        ret = aic_srv_tele_radio_get_neighbor_cell_info_list_sync(0, &s_lcs_cm_cell_neighbor);
        if(ret == 0){
            for (i = 0; i < s_lcs_cm_cell_neighbor.ncell_num; i++) {
                aicx_info(0x159e39, "ssddld", "[lbs at] mcc %s, mnc %s, lac = %d, cellid = %d ，rsrp = %ld\r\n",
                                s_lcs_cm_cell_neighbor.ncell_info[i].plmn_mcc,
                                s_lcs_cm_cell_neighbor.ncell_info[i].plmn_mnc,
                                s_lcs_cm_cell_neighbor.ncell_info[i].tac,
                                s_lcs_cm_cell_neighbor.ncell_info[i].cellid,
                                s_lcs_cm_cell_neighbor.ncell_info[i].rsrp
                                );
                if(s_lcs_cm_cell_neighbor.ncell_info[i].tac == 0){
                    continue;
                }
                memset(neighbor_string,0,LCS_CM_AT_CELLINFO_STRING_MAX);
                snprintf(neighbor_string, sizeof(neighbor_string), "|%s,%s,%ld,%ld, %ld", s_lcs_cm_cell_neighbor.ncell_info[i].plmn_mcc,s_lcs_cm_cell_neighbor.ncell_info[i].plmn_mnc,
                                                s_lcs_cm_cell_neighbor.ncell_info[i].tac, s_lcs_cm_cell_neighbor.ncell_info[i].cellid,s_lcs_cm_cell_neighbor.ncell_info[i].rsrp);

                if(strlen(celllist_str)+strlen(neighbor_string) > LCS_CM_AT_CELLLIST_STRING_MAX){
                    break;
                }
                strcat(celllist_str,neighbor_string);
            }
        }
    }
    aicx_info(0x17c8ec, "s", "[lbs at] celllist_str = %s\r\n",celllist_str);
    aic_srv_tele_radio_get_current_slot_imei_sync(s_lcs_cm_imei);
    aicx_info(0x0630ff, "s", "[lbs at] IMEI = %s\r\n",s_lcs_cm_imei);

    lcs_at_cm_encode(s_lcs_cm_imei,celllist_str);

    return AIC_SUCCESS;
}


static u32 lcs_at_cm_req_position(void)
{
    /* 1. request http handle */
    u8 *responsebuf = NULL;
    u32 errcode = AT_ERROR_LCS_CM_STAT_ERR_UNKNOWN;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr = NULL;
#ifdef SUPPORT_CJSON
    cJSON * cjson_result=NULL;
    cJSON * cjson_data=NULL;
    cJSON * cjson_pos=NULL;
    cJSON * cjson_radius=NULL;
#else
    u8* p_pos_str = NULL;
    u8* p_pos_str2 = NULL;
    u16 pos_length = 0;
    char pos_result[LON_AND_LAT_AT_LEN] = {'\0'};
    u8* p_error_str = NULL;
    u8* p_error_str2 = NULL;
    u16 error_length = 0;
    char error_result[RADIUS_AT_LEN] = {'\0'};
#endif
    char * at_response = NULL;
    at_response = aos_malloc(100);
    aos_memset(at_response, 0, 100);

    s_lcs_cm_http_handle = aic_https_client_create(LCS_AT_CM_HTTP_NAME);

    if (0 == s_lcs_cm_http_handle) {
        aicx_error(0x1c9ac8, "", "[lbs at]: http client create error \r\n");
        return AIC_EPERM;
    }
    if(NULL == jsonstring){
        aicx_error(0x0c3985, "", "[lbs at]: jsonString error \r\n");
        return AIC_EPERM;
    }

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)s_lcs_cm_http_handle;
    client_ptr->https_client->auth_type = 0;

    /* 2. start post req */
    aic_https_client_post_start_ext(s_lcs_cm_http_handle,s_lcs_cm_url,
                AIC_NULL,AIC_NULL,s_lcs_cm_url_port,strlen(jsonstring),s_lcs_cm_contenttype,MAX_WAIT_TIME);

    /* 3. put post data */
    aic_https_client_put_data(s_lcs_cm_http_handle,jsonstring, strlen(jsonstring), MAX_WAIT_TIME);
    aos_sleep(10);
    /* 4. get response data */
    responsebuf = aos_malloc(RESPONSE_BUF);
    LCS_CHECK_POINTER_VALID(responsebuf);
    aos_memset(responsebuf, 0, RESPONSE_BUF);
    aic_https_client_response_data_get(s_lcs_cm_http_handle, responsebuf, RESPONSE_BUF, MAX_WAIT_TIME);
    aicx_info(0x00ee08, "s", "[lbs at]responsebuf = %s \r\n",responsebuf);

    if(0 != responsebuf[0]){
#ifdef SUPPORT_CJSON
        cjson_result = cJSON_Parse(responsebuf);

        if(cjson_result!=NULL){
            cjson_data = cJSON_GetObjectItem(cjson_result,"data");
            if(cjson_result!=NULL){
                cjson_pos = cJSON_GetObjectItem(cjson_data,"pos");
                cjson_radius = cJSON_GetObjectItem(cjson_data,"radius");
                if(cjson_pos!=NULL && cjson_radius!=NULL){
                    snprintf(at_response, 100, "+"RESP_LBS_LOC":100,%s,%d",  cjson_pos->valuestring,cjson_radius->valueint);
                }
            }
        }
#else
        p_error_str = strstr(responsebuf,"\"err_code\"");
        if(p_error_str != NULL){
            p_error_str = p_error_str+strlen("\"err_code\": ") - 1;
            p_error_str2 = strstr(p_error_str,",");
            if(p_error_str2 !=NULL){
                error_length = p_error_str2 - p_error_str;
                strncpy(error_result,p_error_str, error_length);
            }
            if (!strcasecmp(p_error_str, "0")){
                p_pos_str = strstr(responsebuf,"\"pos\"");
                if(p_pos_str!=NULL){
                    p_pos_str = p_pos_str+strlen("\"pos\": \"") -1 ;
                    p_pos_str2 = strstr(p_pos_str,"\",");
                    if(p_pos_str2 !=NULL){
                        pos_length = p_pos_str2 - p_pos_str;
                        strncpy(pos_result,p_pos_str, pos_length);
                        lcs_at_cm_decode_lon_and_lat(pos_result);
                    }
                }
                if(pos_result[0]!=0){
                    snprintf(at_response, 100, "\r\n+"RESP_LBS_LOC": 100,%s",pos_result);
                }
            }else if (!strcasecmp(p_error_str, "10000")) {
                errcode = AT_ERROR_LCS_CM_STAT_ERR_WRONG_PARAM;
            } else if (!strcasecmp(p_error_str, "11000")) {
                errcode = AT_ERROR_LCS_CM_STAT_ERR_OUT_OF_QPS;
            } else if (!strcasecmp(p_error_str, "12000")) {
                errcode = AT_ERROR_LCS_CM_STAT_ERR_UNKNOWN;
            } else {
                errcode = AT_ERROR_LCS_CM_STAT_ERR_UNKNOWN;
            }

        }else {
            errcode = AT_ERROR_LCS_CM_STAT_ERR_UNKNOWN;
        }

#endif
    }
    if(0==at_response[0]){
        snprintf(at_response, 100, "\r\n+"RESP_LBS_LOC": %d", errcode);
    }
    lcs_at_cm_urc_result(at_response);
    aos_free(at_response);

    /* 5. delete client */
    aic_https_client_delete(s_lcs_cm_http_handle);
    aos_free(responsebuf);
    return AIC_SUCCESS;

}

static void lcs_at_cm_thread_entry(void *thread_input)
{
    while(1){
            if(s_lcs_cm_status == LCS_CM_HTTP_DOING){
                // request position
                lcs_at_cm_req_position();
                s_lcs_cm_status = LCS_CM_HTTP_DONE;
                // clear data
                lcs_at_cm_clean_json_string();
                s_lcs_cm_status = LCS_CM_IDLE;
            }
            aos_sleep(10);
        }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

u32 location_at_cm_location_exe_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u32 result = AIC_SUCCESS;
    if(first_startup == LCS_CM_FIRST_STARTUP){
        aicx_error(0x00af2f, "", "[lbs at]: load nv \r\n");
        lcs_at_cm_load_nv();
        first_startup = LCS_CM_NON_FIRST;
    }

    aicx_info(0x161e53, "dds", "[lbs at]config info precision is %d, nearbtsen is %d, pid is %s \r\n",
                                s_lcs_cm_precision, s_lcs_cm_nearbtsen, s_lcs_cm_pid);

    LCS_CHECK_POINTER_VALID(at_req_resp_ptr);
    if(s_lcs_cm_status != LCS_CM_IDLE){
        aicx_info(0x12a45a, "", "lcs is busy \r\n");
        lcs_at_cm_resp_error_code(at_req_resp_ptr,AT_ERROR_LCS_CM_ERR_LBS_BUSY);
        return AIC_EPERM;
    }
    s_lcs_cm_status = LCS_CM_PREACTION;
    // make cellinfo
    s_lcs_cm_urc_port_id = at_req_resp_ptr->serial_port_id;
    result = lcs_at_cm_get_cellinfo();
    if(AIC_SUCCESS != result){
        lcs_at_cm_resp_error_code(at_req_resp_ptr,AT_ERROR_LCS_CM_ERR_UNKNOWN);
        s_lcs_cm_status = LCS_CM_IDLE;
        return AT_ERROR_INVALID_PARAS;
    }
    s_lcs_cm_status = LCS_CM_HTTP_DOING;

    if(location_thread==AOS_INVALID_THREAD_ID){
        location_thread = aos_create_thread("Location Thread",
                        lcs_at_cm_thread_entry, NULL,
                        SIZE_4K, 0, NULL, LOCATION_TASK_PRIORITY,
                        AOS_AUTO_START);

        if (location_thread == AOS_INVALID_THREAD_ID) {
            aicx_error(0x114a80, "", "[lbs at]: create thread is error\r\n");
            lcs_at_cm_resp_error_code(at_req_resp_ptr,AT_ERROR_LCS_CM_ERR_UNKNOWN);
            return AT_ERROR_INVALID_PARAS;
        }
    }

    lcs_at_cm_resp_ok(at_req_resp_ptr);
    return AIC_SUCCESS;
}

u32 location_at_cm_config_test_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    lcs_at_cm_test_result(at_req_resp_ptr);

    return AIC_SUCCESS;
}
void  lcs_at_cm_hex_ascii_convert(char *source, u32 source_length, char *destination)
{
    u32    i,j;
    u8    digit;

    /* Setup destination index.  */
    j =  0;

    /* Loop to process the entire source string.  */
    for (i = 0; i < source_length; i++)
    {

        /* Pickup the first nibble.  */
        digit =  (source[i] >> 4) & 0xF;

        /* Convert to ASCII and store.  */
        if (digit <= 9)
            destination[j++] =  (CHAR)(digit + '0');
        else
            destination[j++] =  (CHAR)(digit + 'A' - 10);

        /* Pickup the second nibble.  */
        digit =  source[i] & 0xF;

        /* Convert to ASCII and store.  */
        if (digit <= 9)
            destination[j++] =  (CHAR)(digit + '0');
        else
            destination[j++] =  (CHAR)(digit + 'A' - 10);
    }

    /* Finally, place a NULL in the destination string.  */
    destination[j] = 0;
}

u32 location_at_cm_md5(char *dest,char *src)
{
    u8 md5_binary[LCM_CM_AT_MAX_ASCII_MD5 + 1] = {0};

    mbedtls_md5((unsigned char *)src,strlen(src),md5_binary);
    lcs_at_cm_hex_ascii_convert(md5_binary, LCM_CM_AT_MAX_ASCII_MD5, dest);
    return AIC_SUCCESS;

}
u32 location_at_cm_config_set_cmd(AT_REQ_RESP *at_req_resp_ptr)
{
    u16 cnt = 0;
    ulong *lcs_paras_arr = NULL;
    u32 lcs_cm_method = 40;/* default: 40 not support others */
    u32 lcs_cm_precision = 8;/* range: (1 ~ 8), default: 8 */
    u32 lcs_cm_nearbtsen = 0;/* range: (0,1), default: 0 */
    u32 lcs_cm_coordinate = 0;/* range: (0,1), default: 1 */
    u8 result_str[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    u8 lcs_cm_pid_md5[LCM_CM_AT_MAX_ASCII_MD5 + 1] = {0};/* default: ofvbkP3AU9 */

    if(first_startup == LCS_CM_FIRST_STARTUP){
        aicx_error(0x00af2f, "", "[lbs at]: load nv \r\n");
        lcs_at_cm_load_nv();
        first_startup = LCS_CM_NON_FIRST;
    }
    LCS_CHECK_POINTER_VALID(at_req_resp_ptr);

    s_lcs_cm_urc_port_id = at_req_resp_ptr->serial_port_id;
    lcs_paras_arr = aic_split_paras_to_str_array(at_req_resp_ptr->at_req,
                                             "AT+"RESP_LBS_CFG"=",
                                             &cnt);
    if (!lcs_paras_arr) {
        aicx_error(0x0423e1, "", "[lbs at]: error config param \r\n");
        lcs_at_cm_resp_error_code_set_cmd(at_req_resp_ptr);
        return AT_ERROR_INVALID_PARAS;
    }
    if (!strcasecmp((char*)lcs_paras_arr[0], "pid")) {
        if (cnt >=2) {
            if (!strcasecmp((char*)lcs_paras_arr[1], "0")){
                strcpy(s_lcs_cm_pid, s_lcs_cm_pid_default);
                lcs_at_cm_resp_ok(at_req_resp_ptr);
            } else {
                strcpy(s_lcs_cm_pid, (char*)lcs_paras_arr[1]);
                lcs_at_cm_resp_ok(at_req_resp_ptr);
            }
            lcs_at_cm_save_nv();
            aic_free_paras_mem(lcs_paras_arr, cnt);
            return AIC_SUCCESS;
        }
        location_at_cm_md5(lcs_cm_pid_md5, lcs_at_cm_get_pid());
        aicx_info(0x0e6d9a, "s", "[lbs at]: pid = %s \r\n",lcs_cm_pid_md5);
        snprintf(result_str, sizeof(result_str), "\"%s\",%s", (char*)lcs_paras_arr[0], lcs_cm_pid_md5);
    } else if (!strcasecmp((char*)lcs_paras_arr[0], "oneoscoordinate")) {
        if (cnt >=2) {
            lcs_cm_coordinate = (u32)atoi((u8 *)lcs_paras_arr[1]);
            if ((0 != lcs_cm_coordinate) && (1 != lcs_cm_coordinate)) {

                lcs_at_cm_resp_error_code_set_cmd(at_req_resp_ptr);
                aic_free_paras_mem(lcs_paras_arr, cnt);
                return AT_ERROR_INVALID_PARAS;
            }
            s_lcs_cm_coordinate = lcs_cm_coordinate;
            lcs_at_cm_resp_ok(at_req_resp_ptr);
            lcs_at_cm_save_nv();
            aic_free_paras_mem(lcs_paras_arr, cnt);
            return AIC_SUCCESS;
        }

        aicx_info(0x073c36, "d", "[lbs at]:set coordinate =  %d \r\n",s_lcs_cm_coordinate);
        snprintf(result_str, sizeof(result_str), "\"%s\",%d", (char*)lcs_paras_arr[0], s_lcs_cm_coordinate);
    } else if (!strcasecmp((char*)lcs_paras_arr[0], "nearbtsen")) {
        if (cnt >=2) {
            lcs_cm_nearbtsen = (u32)atoi((u8 *)lcs_paras_arr[1]);
            if ((0 != lcs_cm_nearbtsen) && (1 != lcs_cm_nearbtsen)) {

                lcs_at_cm_resp_error_code_set_cmd(at_req_resp_ptr);
                aic_free_paras_mem(lcs_paras_arr, cnt);
                return AT_ERROR_INVALID_PARAS;
            }
            s_lcs_cm_nearbtsen = lcs_cm_nearbtsen;
            lcs_at_cm_resp_ok(at_req_resp_ptr);
            lcs_at_cm_save_nv();
            aic_free_paras_mem(lcs_paras_arr, cnt);
            return AIC_SUCCESS;
        }

        aicx_info(0x14423f, "d", "[lbs at]:set nearbtsen =  %d \r\n",s_lcs_cm_nearbtsen);
        snprintf(result_str, sizeof(result_str), "\"%s\",%d", (char*)lcs_paras_arr[0], s_lcs_cm_nearbtsen);
    } else if (!strcasecmp((char*)lcs_paras_arr[0], "precision")) {
        if (cnt >=2) {
            lcs_cm_precision = (u32)atoi((u8 *)lcs_paras_arr[1]);
            if ((lcs_cm_precision < 1) || (lcs_cm_precision > 8)) {

                lcs_at_cm_resp_error_code_set_cmd(at_req_resp_ptr);
                aic_free_paras_mem(lcs_paras_arr, cnt);
                return AT_ERROR_INVALID_PARAS;
            }
            s_lcs_cm_precision = lcs_cm_precision;
            lcs_at_cm_resp_ok(at_req_resp_ptr);
            lcs_at_cm_save_nv();
            aic_free_paras_mem(lcs_paras_arr, cnt);
            return AIC_SUCCESS;
        }
        aicx_info(0x041898, "d", "[lbs at]:set precision =  %d \r\n",s_lcs_cm_precision);
        snprintf(result_str, sizeof(result_str), "\"%s\",%d", (char*)lcs_paras_arr[0], s_lcs_cm_precision);
    } else if (!strcasecmp((char*)lcs_paras_arr[0], "method")) {
        if (cnt >=2) {
            lcs_cm_method = (u32)atoi((u8 *)lcs_paras_arr[1]);
            if ((lcs_cm_method != LCS_CM_METHOD_CELL)) {
                lcs_at_cm_resp_error_code_set_cmd(at_req_resp_ptr);
                aic_free_paras_mem(lcs_paras_arr, cnt);
                return AT_ERROR_INVALID_PARAS;
            }
            s_lcs_cm_method = lcs_cm_method;
            lcs_at_cm_resp_ok(at_req_resp_ptr);
            lcs_at_cm_save_nv();
            aic_free_paras_mem(lcs_paras_arr, cnt);
            return AIC_SUCCESS;
        }
        aicx_info(0x16eec1, "d", "[lbs at]:set method =  %d \r\n",s_lcs_cm_method);
        snprintf(result_str, sizeof(result_str), "\"%s\",%d", (char*)lcs_paras_arr[0], s_lcs_cm_method);
    } else if ((char*)lcs_paras_arr[0] == NULL){
        aicx_error(0x0155d5, "", "[lbs at]: lcs_paras_arr[0] is NULL\r\n");
        lcs_at_cm_resp_error_code_set_cmd(at_req_resp_ptr);
        aic_free_paras_mem(lcs_paras_arr, cnt);
        return AT_ERROR_INVALID_PARAS;
    } else {
        aicx_error(0x19fd4f, "s", "[lbs at]: error config param %s \r\n",(char*)lcs_paras_arr[0]);
        lcs_at_cm_resp_error_code_set_cmd(at_req_resp_ptr);
        aic_free_paras_mem(lcs_paras_arr, cnt);
        return AT_ERROR_INVALID_PARAS;
    }

    lcs_at_cm_resp_result(at_req_resp_ptr,"+"RESP_LBS_CFG,result_str);
    aic_free_paras_mem(lcs_paras_arr, cnt);
    return AIC_SUCCESS;
}


static AT_CMD_HANDLE location_at_cm_handles[] = {
    {AT_MLBS_CFG, location_at_cm_config_test_cmd,AIC_NULL,
                    location_at_cm_config_set_cmd,AIC_NULL},
    {AT_MLBS_LOC, AIC_NULL,AIC_NULL,
                    AIC_NULL,location_at_cm_location_exe_cmd},

    {AIC_NULL},
};

int location_at_cm_init(void)
{
    aic_add_at_cmd_handle(location_at_cm_handles);
    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_L2_INIT_EXPORT(location_at_cm_init);

