

#ifdef CONFIG_APP_SMS_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_APP_TELE_SMS_DATA_FP_H__
#define __APP_APP_TELE_SMS_DATA_FP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "aic_srv_tele.h"
#include "phonebook.h"

#define     TELE_SMS_DATA_REV_FOLDER_PATH "/C/sms_rev"
#define     TELE_SMS_DATA_SEND_FOLDER_PATH "/C/sms_send"
#define     TELE_SMS_DATA_DRAFT_FOLDER_PATH "/C/sms_draft"

#define     TELE_SMS_DATA_REVICE_BOX_COUNT 100	
#define     TELE_SMS_DATA_SEND_BOX_COUNT 50	
#define     TELE_SMS_DATA_DRAFT_BOX_COUNT 20	

    void tele_sms_data_init();

#if 0
    typedef struct {
        uint8_t         read_flag;
        uint8_t         card_id;
        char            number[TS_SMS_NUM_MAX_LEN + 1];
        ts_sms_time_stamp_t time_stamp;
        } tele_sms_storge_write_param_t;

    typedef struct {
        uint8_t         read_flag;
        uint8_t         card_id;
        char            number[TS_SMS_NUM_MAX_LEN + 1];
        ts_sms_time_stamp_t time_stamp;
        uint32_t        text_start;
        } tele_sms_storge_read_param_t;

#endif

    int32_t tele_sms_data_write_file(ts_sms_read_in_sim_resp_t * p_resp);
    int32_t tele_sms_data_send(const char * p_content,const char * p_number,const char * p_name,bool is_write_to_sim);

    typedef int32_t(*sms_send_func_callback_t) (int32_t send_result,void * user_data); //send_result:0,成功

    typedef struct {
        uint8_t         name[PHONEBOOK_NAME_LEN];
        uint8_t         card_id;
        ts_sms_time_stamp_t time_stamp;
        char            number[TS_SMS_NUM_MAX_LEN + 1];
        uint32_t        text_start;
        } tele_sms_storge_send_param_t;

    typedef struct {
        uint8_t         name[PHONEBOOK_NAME_LEN];
        uint8_t         card_id;
        ts_sms_time_stamp_t time_stamp;
        char            number[TS_SMS_NUM_MAX_LEN + 1];
        uint32_t        text_start;
        } tele_sms_storge_draft_param_t;

    typedef struct {
        uint8_t         name[PHONEBOOK_NAME_LEN];
        ts_sms_read_in_sim_resp_t param;
        } tele_sms_storge_rev_param_t;

    uint32_t tele_sms_rev_unread_count_get();
    void tele_sms_rev_unread_count_reset();
    void tele_sms_rev_unread_count_minus();
    uint32_t is_tele_sms_data_recive_empty();
    uint32_t is_tele_sms_data_send_empty();
    uint32_t is_tele_sms_data_draft_empty();
    int32_t tele_sms_data_set_send_fun_callback(sms_send_func_callback_t func,void * user_data);
    int32_t tele_sms_data_draft_write_file(tele_sms_storge_draft_param_t * p_send);
    void sms_data_box_count_reduce(uint8_t * folder);
    void tele_sms_data_storge_init();
    void sms_data_box_revice_count_reset();
    void sms_data_box_draft_count_reset();
    void sms_data_box_send_count_reset();

#ifdef __cplusplus
} /*extern "C"*/

#endif

#endif /* __APP_APP_TELE_SMS_DATA_FP_H__ */

#endif

