//at and urc

#include <aic_size.h>
#include <aic_list.h>
#include <aipc_api.h>
#include "aic_d2d_l3_type.h"
#include "aic_at_process_common.h"

#ifndef AIC_D2D_AT_H
#define AIC_D2D_AT_H

#define AT_D2D_URC_MT "+OMT"
#define AT_D2D_URC_RELEASE "+OREL"
#define AT_D2D_URC_ONLINE "+OONLINE"
#define AT_D2D_URC_OFFLINE "+OOFFLINE"
#define AT_D2D_URC_RSSI "^D2D"

typedef struct{
	u8 flag;
    u32 time;
	Td2d_l3_target Td2d_info;
} Td2d_info_e;

enum {
    AT_D2D_URC_MT_MSG_TYPE = 0x0106,
    AT_D2D_URC_ONLINE_MSG_TYPE,
    AT_D2D_URC_RELEASE_MSG_TYPE,
    AT_D2D_URC_OFFLINE_MSG_TYPE,
	AT_D2D_URC_RSSI_MSG_TYPE,
};

u32 aic_handle_radio_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_radio_set_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_dev_query_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_find_request_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_link_request_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_screen_status_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_encrypt_trans_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_release_cmd(AT_REQ_RESP *at_req_ptr);
u32 aic_handle_write_d2dId_cmd(AT_REQ_RESP *at_req_ptr);

int monitor_init();
int compareD2dInfo(Td2d_l3_target info1, Td2d_l3_target info2);
void line_monitor();
void sendOnline(void* d2d_msg_ptr);
void updataOffline(void* d2d_msg_ptr);
void sendOffline(void* input);
void mtInd(void* d2d_msg_ptr);
void releaseInd(void* d2d_msg_ptr);
u32 timerCompara(aos_systime time);
void inital_timer();
void init_at();
u32 stop_d2d_modem();
u32 aic_handle_set_power_cmd(AT_REQ_RESP *at_req_ptr);
#endif