/******************************************************************************/
/*                                                                            */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
#ifndef __AIC_D2D_L3_FUNC_H__
#define __AIC_D2D_L3_FUNC_H__

aos_status aic_fd2d_dummy_init_req_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_dummy_stop_req_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_dummy_pbch_cfg_req_hdlr(d2d_msg_header * d2d_msg_ptr);

void aic_fd2d_dummy_l1_main(void *param);
void aic_fd2d_dummy_l2_main(void *param);

void aic_fd2d_test_init(void);
void aic_fd2d_test_main(void *thread_input);
void aic_fd2d_l3_main(void *param);
void aic_fd2d_gen_pbch_freq_pattern(void);
u32 aic_fd2d_get_d2d_id(void);
void aic_fd2d_set_d2d_id(u32 d2d_id);
void aic_fd2d_set_pbch_freq_group(u8 pbch_freq_group);
u8 aic_fd2d_get_pbch_freq_group(void);
void aic_fd2d_set_bch_report_period(u8 period);
u8 aic_fd2d_get_bch_report_period(void);
void aic_fd2d_set_out_of_sync_para_tout(u8 tout);
u8 aic_fd2d_get_out_of_sync_para_tout(void);
void aic_fd2d_set_tx_power(s16 tx_power, s16 max_tx_power);
s16 aic_fd2d_get_tx_power(void);
s32 aic_fd2d_l3_init();
aos_status aic_fd2d_l3_delete();

void aic_fd2d_enc_d2d_id(u8 *out_ptr, u32 in_d2d_id);

void aic_fd2d_l3_enc_broadcast(Td2d_l3_codec *out_ptr, u32 d2d_id, u16 hash);
void aic_fd2d_l3_enc_paging(Td2d_l3_codec *out_ptr, u32 d2d_id, Td2d_ie_service_type_e service_type);
void aic_fd2d_l3_enc_conn_release(Td2d_l3_codec *out_ptr, Td2d_ie_service_type_e service_type, u8 qos_id, Td2d_ie_rel_reason_e reason);
void aic_fd2d_l3_enc_call_setup_request(Td2d_l3_codec *out_ptr, Td2d_ie_service_type_e service_type, u8 qos_id, Td2d_ul_dl_alloc_e ul_dl_alloc);
void aic_fd2d_l3_enc_l3_ack(Td2d_l3_codec *out_ptr, Td2d_ie_msg_name_e msg_name);
void aic_fd2d_l3_enc_mt_data_accept(Td2d_l3_codec *out_ptr, u8 qos_id);

void aic_fd2d_l3_msg_l3_ack_hdlr(u8 msg_type);
void aic_fd2d_l3_msg_call_setup_request_hdlr(u8 *msg_content_ptr);
void aic_fd2d_l3_msg_conn_release_hdlr(u8 *msg_content_ptr);
void aic_fd2d_l3_msg_mt_data_accept_hdlr(u8 *msg_content_ptr);

u8 aic_fd2d_l3_check_other_call_exist(u8 qos_id);
void aic_fd2d_l3_call_end(u8 qos_id, u8 is_paging_abort);
u8 aic_fd2d_l3_get_qos_id_with_state(Td2d_qos_state_e qos_state);
void aic_fd2d_l3_update_qos_info(u8 option, u8 qos_id, u8 service_type, u8 mode);
void aic_fd2d_l3_tmr_exp_callback(void *param_ptr);
void aic_fd2d_l3_initiate_call(u8 qos_id);
void aic_fd2d_l3_terminate_call(u8 qos_id);
void aic_fd2d_l3_terminate_all_call(void);
void aic_fd2d_l3_terminate_all_call_local(void);
u8 aic_fd2d_l3_check_bch_list(Td2d_l3_target *target_ptr);
void aic_fd2d_l3_continue_call(void);
u8 aic_fd2d_l3_check_qos_id(u8 qos_id);
void aic_fd2d_l3_conn_state_hdlr(Td2d_state_e entered_state);
aos_status aic_fd2d_l3_sig_tx_list_push(Td2d_lcid_e lcid, u16 msg_sn, u16 msg_id);
u16 aic_fd2d_l3_sig_tx_list_pop(Td2d_lcid_e lcid, u16 msg_sn);
void aic_fd2d_l3_l3_sig_tx_list_empty(void);
void aic_fd2d_l3_call_active_hdlr(u8 qos_id);

aos_status aic_fd2d_l3_send_msg_to_l2(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);
aos_status aic_fd2d_l3_send_msg_to_l1(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);
aos_status aic_fd2d_l3_send_msg_to_app_queue(void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);
aos_status aic_fd2d_l3_init_req_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l3_stop_req_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l3_init_cnf_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l3_stop_cnf_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l3_send_sig(u16 lcid, Td2d_l3_codec *in_sig_ptr);
aos_status aic_fd2d_api_send_msg_to_l3(aos_thread_id src_id, void *d2d_msg_ptr, u16 d2d_msg_len, aos_prim_id prim_id);
aos_status aic_fd2d_api_wait_app_msg(aos_prim_id prim_id);
aos_status aic_fd2d_l3_re_est_tmr_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_l3_bch_report_tmr_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_l3_bch_check_tmr_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_l3_mo_guard_tmr_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_l3_wait_rel_tmr_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);

aos_status aic_fd2d_l3_send_at_urc(u8 *reply_str,         u32 reply_size);
aos_status aic_fd2d_l3_send_d2d_status_to_at_urc(u8 d2d_state_index);
aos_status aic_fd2d_l3_qos_set_req_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l3_call_req_hdlr(d2d_msg_header * d2d_msg_ptr);

void aic_fd2d_at_init();

aos_status aic_fd2d_config_l1_pbch_para(u8 is_init_cfg);
aos_status aic_fd2d_config_l1_pbch_para_cnf(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l1_paging_ind_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l3_rpt_bch_info(Td2d_l3_target *target_ptr);
aos_status aic_fd2d_l1_bch_ind_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_rel_l1_chnl();
aos_status aic_fd2d_rel_l1_chnl_cnf(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_config_l1_ptch_para(void);
aos_status aic_fd2d_config_l1_ptch_para_cnf(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_l1_chnl_release_ind_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_l1_paging_abort_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);
aos_status aic_fd2d_l1_cfg_para();
aos_status aic_fd2d_l1_re_search_cmd_hdlr(d2d_msg_header * d2d_rcv_msg_ptr);

aos_status aic_fd2d_l2_data_ind_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l2_cfg_cnf_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l2_data_cnf_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l2_release_cnf_hdlr(d2d_msg_header * d2d_msg_ptr);
aos_status aic_fd2d_l3_send_rel_req_to_l2();
void aic_fd2d_l3_init_qos_para(void);
void aic_fd2d_l3_cfg_l2_cch_and_all_qos(void);

#endif

