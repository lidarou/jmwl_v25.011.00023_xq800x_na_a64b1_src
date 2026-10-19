/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is apis of em
 */

/*********************
 *      INCLUDES
 *********************/
#include "aic_srv_tele.h"
#include "aic_md_api.h"
#include "aic_md_id.h"
#include "app_log.h"
#include "em_flow.h"
#include "app_msg.h"
#include "aic_srv_tele_ex.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "aic_osa.h"
#include "timer_inc.h"
#include "nxd_bsd.h"
/*********************
 *      DEFINES
 *********************/
#define EM_CHECK_POINTER_NULL(ptr) if (NULL == ptr) { \
                app_loge("[%s] pointer is NULL.\r\n", __func__); \
                return -1; }

#define EM_DATA_DEFAULT_PDP_CID         (1)
#define EM_SMS_NUM_MAX_LEN              (21)

#define SPEED_THREAD_STACK_SIZE 1024
#define SPEED_TASK_PRIORITY    10
#define SPEED_TEST_SERVER_PORT 5678
#define SPEED_TEST_SERVER_ADDR "222.71.206.138"

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 *  STATIC PROTOTYPES
 **********************/



/**********************
 *  STATIC VARIABLES
 **********************/
static char g_smssc[EM_SMS_NUM_MAX_LEN + 1] = {0};
static int32_t g_sms_handle = 0;
static int32_t sock_tcp_client = -1;
static uint8_t test_recv_buf[1024];
static uint8_t test_send_buf[] = "test 50K";
static uint32_t first_time_point = 0;
static uint32_t second_time_point = 0;
static aos_thread_id speedtest_thread = AOS_INVALID_THREAD_ID;
static srv_func_callback speed_callback = NULL;


/**********************
 *   STATIC FUNCTIONS
 **********************/
static int32_t handleSmsSettingCallback(void *p_param, uint32_t size)
{
    if(NULL == p_param || 0 == size) {
        APP_LOGE("[%s],p_param is null or size is 0.",__func__);
        return -1;
    }

    uint32_t event = *(uint32_t *)p_param;

    switch (event) {
    case TS_EVENT_SMS_RECEIVE_NEW:
        app_logi("[%s] TS_EVENT_SMS_RECEIVE_NEW", __func__);
        break;
    case TS_EVENT_SMS_GET_LIST_IN_SIM_RESP:
        app_logi("[%s] TS_EVENT_SMS_GET_LIST_IN_SIM_RESP", __func__);
        break;
    case TS_EVENT_SMS_GET_SMSC_RESP:
    {
        ts_sms_get_smsc_resp_t *resp = (ts_sms_get_smsc_resp_t *)p_param;
        app_logi("[%s]TS_EVENT_SMS_GET_SMSC_RESP received. err_code(%d)", __func__, resp->base_info.err_code);
        if (resp->base_info.err_code == 0) {
            app_logi("[%s]TS_EVENT_SMS_GET_SMSC_RESP smsc(%s)", __func__, resp->smsc);
            strncpy(g_smssc, resp->smsc, strlen(resp->smsc));
        }
    }
        break;
    case TS_EVENT_SMS_SEND_RESP:
    {
        ts_sms_send_resp_t *resp = (ts_sms_send_resp_t *)p_param;

        app_logi("[%s] TS_EVENT_SMS_SEND_RESP received. err_code(%d)", __func__, resp->base_info.err_code);
        if (resp->base_info.err_code == 0) {
            app_logi("[sms test] index(%d)", resp->index);
        }
        break;
    }
    default:
        break;
    }

    return 0;

}
__APP_CALLBACK_FUNCTION__(handleSmsSettingCallback);

static int32_t radio_set_filter_resp(void *p_param, uint32_t size)
{
    ts_radio_set_filter_resp_t *p_set_filter_resp = NULL;

    if(NULL == p_param) {
        APP_LOGE("[%s]p_param is NULL.\r\n", __func__);
        return -1;
    }

    p_set_filter_resp = (ts_radio_set_filter_resp_t *)p_param;
    APP_LOGI("[%s]ret_value = %d.\r\n",
             __func__,
             p_set_filter_resp->ret_value);
    return 0;
}
__APP_CALLBACK_FUNCTION__(radio_set_filter_resp);
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/* device info */
int32_t em_flow_get_sn0(char *p_sn0)
{
    char sn0[EM_FLOW_SN_LEN + 1] = {0};
    int ret = 0;

    EM_CHECK_POINTER_NULL(p_sn0);

    ret = aic_md_read(MD_ID_SN0, EM_FLOW_SN_LEN, sn0);
    APP_LOGI("[%s]read sn0 ret = %d,sn0 = %s.", __func__, ret, sn0);
    strncpy(p_sn0, sn0, EM_FLOW_SN_LEN);

    return 0;
}

int32_t em_flow_get_sn1(char *p_sn1)
{
    char sn1[EM_FLOW_SN_LEN + 1] = {0};
    int ret = 0;

    EM_CHECK_POINTER_NULL(p_sn1);

    ret = aic_md_read(MD_ID_SN1, EM_FLOW_SN_LEN, sn1);
    APP_LOGI("[%s]read sn1 ret = %d,sn1 = %s.", __func__, ret, sn1);
    strncpy(p_sn1, sn1, EM_FLOW_SN_LEN);

    return 0;
}

int32_t em_flow_get_imei(char *p_imei)
{
    int32_t ret = 0;

    EM_CHECK_POINTER_NULL(p_imei);

    ret = aic_srv_tele_radio_get_current_slot_imei_sync(p_imei);

    APP_LOGI("[%s]p_imei = %s.", __func__, p_imei);

    return ret;
}

/* sim info */
int32_t em_flow_get_sim_status(char *p_sim_status)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;
    ts_sim_status_e sim_status = TS_SIM_STATUS_NOT_READY;
    char *p_str_sim_status = NULL;

    EM_CHECK_POINTER_NULL(p_sim_status);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    ret = aic_srv_tele_sim_get_status(current_card_id, &sim_status);
    APP_LOGI("[%s]current_card_id = %d,sim_status = %d.", __func__, current_card_id, sim_status);

    /* not ready/not exist/ready/pin lock/puk lock/puk lock/pin2 lock/puk2 lock */
    switch(sim_status) {
    case TS_SIM_STATUS_NOT_READY:
        p_str_sim_status = "not ready";
        break;
    case TS_SIM_STATUS_NOT_PRESENT:
        p_str_sim_status = "not exist";
        break;
    case TS_SIM_STATUS_READY:
        p_str_sim_status = "ready";
        break;
    case TS_SIM_STATUS_PIN:
        p_str_sim_status = "pin lock";
        break;
    case TS_SIM_STATUS_PUK:
        p_str_sim_status = "puk lock";
        break;
    case TS_SIM_STATUS_PIN2:
        p_str_sim_status = "pin2 lock";
        break;
    case TS_SIM_STATUS_PUK2:
        p_str_sim_status = "puk2 lock";
        break;
    default:
        p_str_sim_status = "not exist";
        break;
    }

    strncpy(p_sim_status, p_str_sim_status, strlen(p_str_sim_status));
    return ret;
}

int32_t em_flow_get_sim_imsi(char *p_sim_imsi)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;

    EM_CHECK_POINTER_NULL(p_sim_imsi);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    ret = aic_srv_tele_sim_get_imsi_sync(current_card_id, p_sim_imsi);
    APP_LOGI("[%s]current_card_id = %d,p_sim_imsi = %d.", __func__, current_card_id, p_sim_imsi);

    return ret;
}
int32_t em_flow_get_sim_phomenum(char *p_sim_phonenum)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;

    EM_CHECK_POINTER_NULL(p_sim_phonenum);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    ret = aic_srv_tele_sim_get_msisdn_sync(current_card_id, p_sim_phonenum);
    APP_LOGI("[%s]current_card_id = %d,p_sim_phonenum = %d.", __func__, current_card_id, p_sim_phonenum);

    return ret;
}
int32_t em_flow_get_regstatus(char *p_reg_status)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;
    ts_radio_reg_status_e reg_status = TS_RADIO_REG_STATUS_POWER_OFF;
    char *p_str_reg_status = NULL;

    EM_CHECK_POINTER_NULL(p_reg_status);

    aic_srv_tele_sim_get_current_card(&current_card_id);

    ret = aic_srv_tele_radio_get_reg_status_sync(current_card_id, &reg_status);

    /* not service/in service/emergency only/power off */
    switch(reg_status) {
        case TS_RADIO_REG_STATUS_NO_SERVICE:
            p_str_reg_status = "not sevice";
        break;
        case TS_RADIO_REG_STATUS_IN_SERVICE:
            p_str_reg_status = "in service";
        break;
        case TS_RADIO_REG_STATUS_EMERGENCY_CALL_ONLY:
            p_str_reg_status = "emergency only";
        break;
        case TS_RADIO_REG_STATUS_POWER_OFF:
            p_str_reg_status = "power off";
        break;
        default:
             p_str_reg_status = "not sevice";
        break;
    }

    strncpy(p_reg_status, p_str_reg_status, strlen(p_str_reg_status));

    return ret;
}
int32_t em_flow_get_operatorname(char *p_opearor_long_name, char *p_operator_short_name)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;
    ts_radio_operator_info_t operator_info = {0};

    EM_CHECK_POINTER_NULL(p_opearor_long_name);
    EM_CHECK_POINTER_NULL(p_operator_short_name);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    ret = aic_srv_tele_radio_get_operator_info(current_card_id, &operator_info);

    strncpy(p_opearor_long_name, operator_info.operator_long_name, strlen(operator_info.operator_long_name));
    strncpy(p_operator_short_name, operator_info.operator_short_name, strlen(operator_info.operator_short_name));

    return ret;
}

int32_t em_flow_get_signalinfo(ts_radio_signal_info_t *p_signal_info)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;

    EM_CHECK_POINTER_NULL(p_signal_info);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    ret = aic_srv_tele_radio_get_signal_info_sync(current_card_id, p_signal_info);
    APP_LOGI("[%s]current_card_id = %d,signal_level = %d, signal_level_val = %d, signal_strength= %d.", __func__, current_card_id,
            p_signal_info->signal_level, p_signal_info->signal_level_val, p_signal_info->signal_strength);

    return ret;
}

int32_t em_flow_get_cellinfo(ts_radio_cell_info_t *p_cell_info)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;

    EM_CHECK_POINTER_NULL(p_cell_info);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    ret = aic_srv_tele_radio_get_cell_info_sync(current_card_id, p_cell_info);
    APP_LOGI("[%s]current_card_id = %d,cell id = %d.", __func__, current_card_id, p_cell_info->cellid);

    return ret;
}
int32_t em_flow_get_neighborinfo(ts_radio_neighbor_cell_info_list_t *p_neighbor_info_list)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;

    EM_CHECK_POINTER_NULL(p_neighbor_info_list);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    ret = aic_srv_tele_radio_get_neighbor_cell_info_list_sync(current_card_id, p_neighbor_info_list);
    APP_LOGI("[%s]current_card_id = %d,neighbor list num = %d.", __func__, current_card_id, p_neighbor_info_list->ncell_num);

    return ret;
}

int32_t em_flow_get_pdp_status(char *p_pdp_status)
{
    int32_t ret = 0;
    ts_data_pdp_status_e pdp_status ={0};

    EM_CHECK_POINTER_NULL(p_pdp_status);
    ret =  aic_srv_tele_data_get_pdp_status_sync(EM_DATA_DEFAULT_PDP_CID, &pdp_status);
    APP_LOGI("[%s]pdp_info.pdp_status = %d.", __func__, pdp_status);

    /* active/not active */
    if(TS_DATA_PDP_STATUS_ACTIVED == pdp_status) {
        strncpy(p_pdp_status, "active", strlen("active"));
    } else {
        strncpy(p_pdp_status, "not active", strlen("not active"));
    }

    return ret;
}
int32_t em_flow_get_ip_address(char *p_ip_addr)
{
    int32_t ret = 0;

    EM_CHECK_POINTER_NULL(p_ip_addr);

    ret = aic_srv_tele_data_get_pdp_ip_sync(EM_DATA_DEFAULT_PDP_CID, p_ip_addr);
    APP_LOGI("[%s]ip addr = %s.", __func__, p_ip_addr);

    return ret;
}

int32_t em_flow_sms_register(void)
{
    aic_srv_bus_info_t em_smssetting = {0};
    uint8_t current_card_id = 0;

    em_smssetting.p_client_name = "em";

    g_sms_handle = aic_srv_tele_sms_register(&em_smssetting, __APP_CALLBACK_NAME__(handleSmsSettingCallback));

    app_logi("[%s] sms_handle = 0x%x", __func__, g_sms_handle);
    aic_srv_tele_sms_get_smsc(current_card_id, NULL);
    return g_sms_handle;
}
int32_t em_flow_sms_unregister(void)
{
    app_logi("[%s] enter", __func__);
    return aic_srv_tele_sms_unregister(g_sms_handle);
}

int32_t em_flow_get_smsc(char *p_smsc)
{
    int32_t ret = 0;

    EM_CHECK_POINTER_NULL(p_smsc);
    strncpy(p_smsc, g_smssc, strlen(g_smssc));
    app_logi("[%s] g_smssc = %s", __func__, g_smssc);

    return ret;
}
int32_t em_flow_send_sms(const char *p_dest_num, const char *p_sms_content)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;

    ts_sms_send_info_t send_info = {0};

    EM_CHECK_POINTER_NULL(p_sms_content);

    aic_srv_tele_sim_get_current_card(&current_card_id);
    app_logi("[%s] p_dest_num = %s, p_sms_content = %s", __func__, p_dest_num, p_sms_content);

    send_info.is_write_to_sim = false;
    send_info.p_smsc = g_smssc;
    send_info.p_number = p_dest_num;
    send_info.p_text = p_sms_content;

    ret = aic_srv_tele_sms_send(current_card_id, &send_info, NULL);

    return ret;
}

int32_t em_flow_set_radio_report_status(bool report_status)
{
    int32_t ret = 0;
    uint8_t current_card_id = 0;
    aic_srv_tele_sim_get_current_card(&current_card_id);
    if(true == report_status) {
        ret = aic_srv_tele_radio_set_filter(current_card_id, 0xFFFFFFFF, __APP_CALLBACK_NAME__(radio_set_filter_resp));
    } else {
        ret = aic_srv_tele_radio_set_filter(current_card_id, 0, __APP_CALLBACK_NAME__(radio_set_filter_resp));
    }
    return ret;
}

static void speed_test_thread(void *thread_input)
{
    int32_t status;
    struct sockaddr_in serv_addr;
    uint32_t dl_bytes = 0;
    uint32_t dlspeed = 0;
    em_network_speed_t * resp = NULL;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SPEED_TEST_SERVER_PORT);
    inet_pton(AF_INET, SPEED_TEST_SERVER_ADDR, &serv_addr.sin_addr);
    resp = aic_zalloc(sizeof(em_network_speed_t));
    resp->speed = 0;

    sock_tcp_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_tcp_client == -1) {
        app_loge("[speed_test_thread] IPv4 TCP client socket create fail!\n");
        resp->status = EM_CREATE_SOCKET_ERROR;
        speed_callback(resp,sizeof(em_network_speed_t));
        speed_callback = NULL;
        return ;
    }

    /* Now connect this client to the server */
    status = connect(sock_tcp_client, (struct sockaddr *)&serv_addr,
                     sizeof(serv_addr));
    if (status != OK) {
        app_loge("[speed_test_thread] IPv4 TCP client socket connect fail 0x%x!\n",
               status);
        resp->status = status;
        speed_callback(resp,sizeof(em_network_speed_t));
        speed_callback = NULL;
        status = soc_close(sock_tcp_client);
        return ;
    }
    status = send(sock_tcp_client, test_send_buf, strlen(test_send_buf), 0);
    if (status == ERROR) {
        app_loge("[speed_test_thread] IPv4 TCP client socket %d send fail %d!\n",
               sock_tcp_client, errno);
    }

    while (1) {
        if (sock_tcp_client == -1) {
            break;
        }

        status = recvfrom(sock_tcp_client, (void *)test_recv_buf,1024, 0, NULL, NULL);
        if (status < 0) {
            if (errno == EAGAIN)
                continue;
            app_loge("[speed_test_thread] IPv4 TCP error on recv socket %d 0x%x 0x%x\n",
                   sock_tcp_client, status, errno);
            aos_sleep(3);
            break;
        } else {
            app_loge("[speed_test_thread] IPv4 TCP client %d recv %d bytes\n",
                   sock_tcp_client, status);
            dl_bytes = dl_bytes + status;
        }
    }

    /* close this client socket */
    status = soc_close(sock_tcp_client);
    if (status != ERROR)
        app_logi("[speed_test_thread] IPv4 TCP client socket %d closed!\n",
                  sock_tcp_client);
    else
        app_loge("[speed_test_thread] IPv4 TCP client socket %d closed failed!\n",
                 sock_tcp_client);
    sock_tcp_client = -1;
    second_time_point = aic_get_timestamp();
    uint32_t utime = second_time_point - first_time_point;
    uint32_t mtime = utime/1000;
    dlspeed = dl_bytes*1000/mtime;
    app_logi("[speed_test_thread] first_time_point = %d,second_time_point = %d,dl_bytes = %d\n",first_time_point,second_time_point,dl_bytes);
    app_logi("[speed_test_thread] mtime = %d,dlspeed = %d\n",mtime,dlspeed);
    if(speed_callback != NULL){
        resp->status = EM_SPEED_NO_ERROR;
        resp->speed = dlspeed;
        speed_callback(resp,sizeof(em_network_speed_t));
        speed_callback = NULL;
    }
}


int32_t em_flow_get_download_speed(srv_func_callback callback)
{
    app_logi("em_flow_get_download_speed");

    if (speed_callback != NULL) {
       app_loge("speed test is doing");
       return -1;
    } else {
        speed_callback = callback;
    }

    speedtest_thread = aos_create_thread("network_speed_task", speed_test_thread, NULL, SPEED_THREAD_STACK_SIZE, 0, NULL,
                      SPEED_TASK_PRIORITY, AOS_AUTO_START);
    if (speedtest_thread == AOS_INVALID_THREAD_ID) {
        app_loge("[speed test]: create thread is error\r\n");
        return -1;
    }

    first_time_point = aic_get_timestamp();

    return 0;
}

