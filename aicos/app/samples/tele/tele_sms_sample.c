/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file tele_sms_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>
#include <pthread.h>
#include "aic_srv_tele.h"


/*********************
 *      DEFINES
 *********************/
#define TELE_SMS_SAMPLE_TASK_STK_SIZE (4096u)
#define TELE_SMS_SAMPLE_TASK_COM_PRIO (35u)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static pthread_mutex_t g_tele_sms_sample_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_tele_sms_sample_cond = PTHREAD_COND_INITIALIZER;
static char g_tele_sms_old_smsc[TS_SMS_NUM_MAX_LEN + 1] = {0};
static int32_t *g_tele_sms_list = NULL;
static int32_t g_tele_sms_count = 0;

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
static void tele_sms_sample_wait_for_response(void)
{
    app_logi("[test sms srv]tele_sms_sample_wait_for_response waiting");
    pthread_mutex_lock(&g_tele_sms_sample_mutex);
    pthread_cond_wait(&g_tele_sms_sample_cond, &g_tele_sms_sample_mutex);
    pthread_mutex_unlock(&g_tele_sms_sample_mutex);
    app_logi("[test sms srv]tele_sms_sample_wait_for_response received response");
}

static int32_t tele_sms_sample_callback(void *p_param, uint32_t size)
{
    if (!p_param || !size) {
        return -1;
    }
    uint32_t event = *(uint32_t *)p_param;
    bool is_handled = true;
    switch (event) {
    case TS_EVENT_SMS_SEND_WITH_PDU_RESP:
        {
            ts_sms_send_with_pdu_resp_t *resp = (ts_sms_send_with_pdu_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_SEND_WITH_PDU_RESP received.");
            app_logi("[test sms srv]err_code(%d) & index(%d)", resp->base_info.err_code, resp->index);
        }
        break;
    case TS_EVENT_SMS_WRITE_WITH_PDU_TO_SIM_RESP:
        {
            ts_sms_write_with_pdu_to_sim_resp_t *resp = (ts_sms_write_with_pdu_to_sim_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_WRITE_WITH_PDU_TO_SIM_RESP received.");
            app_logi("[test sms srv]err_code(%d) & index(%d)", resp->base_info.err_code, resp->index);
        }
        break;
    case TS_EVENT_SMS_DEL_IN_SIM_WITH_PDU_RESP:
        {
            ts_sms_del_in_sim_with_pdu_resp_t *resp = (ts_sms_del_in_sim_with_pdu_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_DEL_IN_SIM_WITH_PDU_RESP received.");
            app_logi("[test sms srv]err_code(%d)", resp->base_info.err_code);
        }
        break;
    case TS_EVENT_SMS_SET_SMSC_WITH_PDU_RESP:
        {
            ts_sms_set_smsc_with_pdu_resp_t *resp = (ts_sms_set_smsc_with_pdu_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_SET_SMSC_WITH_PDU_RESP received.");
            app_logi("[test sms srv]err_code(%d)", resp->base_info.err_code);
        }
        break;
    case TS_EVENT_SMS_GET_SMSC_WITH_PDU_RESP:
        {
            ts_sms_get_smsc_with_pdu_resp_t *resp = (ts_sms_get_smsc_with_pdu_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_GET_SMSC_WITH_PDU_RESP received.");
            app_logi("[test sms srv]err_code(%d) & smsc(%s)", resp->base_info.err_code, resp->smsc);
            if (resp->base_info.err_code == 0) {
                /* backup the old smsc */
                strcpy(g_tele_sms_old_smsc, resp->smsc);
            }
        }
        break;
    case TS_EVENT_SMS_GET_LIST_IN_SIM_WITH_PDU_RESP:
        {
            uint16_t i = 0;
            ts_sms_get_list_in_sim_with_pdu_resp_t *resp = (ts_sms_get_list_in_sim_with_pdu_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_GET_INFO_LIST_SIM_RESP received.");
            app_logi("[test sms srv]err_code(%d)", resp->base_info.err_code);
            if (resp->base_info.err_code == 0) {
                app_logi("[test sms srv]total count(%d) & box(%d)", resp->count, (int32_t)resp->box);
                ts_sms_entry_t *list = (ts_sms_entry_t *)(&resp->list_start);
                for (uint16_t i = 0; i < resp->count; i++) {
                    app_logi("[test sms srv]SMS INFO LIST - #[%d] index(%d) status(%d)", i, list[i].index,
                             (int32_t)list[i].status);
                }
                if (g_tele_sms_list) {
                    free(g_tele_sms_list);
                    g_tele_sms_list = NULL;
                    g_tele_sms_count = 0;
                }
                if (resp->count > 0) {
                    g_tele_sms_list = calloc(1, resp->count * sizeof(int32_t));
                    if (g_tele_sms_list) {
                        for (i = 0; i < resp->count; i++) {
                            g_tele_sms_list[i] = list[i].index;
                        }
                        g_tele_sms_count = resp->count;
                    } else {
                        app_loge("[test sms srv]TS_EVENT_SMS_GET_INFO_LIST_SIM_RESP no memory.");
                    }
                }
            }
        }
        break;
    case TS_EVENT_SMS_READ_IN_SIM_WITH_PDU_RESP:
        {
            ts_sms_read_in_sim_with_pdu_resp_t *resp = (ts_sms_read_in_sim_with_pdu_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_READ_IN_SIM_WITH_PDU_RESP received.");
            app_logi("[test sms srv]err_code(%d)", resp->base_info.err_code);
            if (resp->base_info.err_code == 0) {
                app_logi("[test sms srv]READ SIM IN SIM: index(%d) & pdu(%s)", resp->index, resp->pdu);
            }
        }
        break;
    case TS_EVENT_SMS_NEW_WITH_IDX_IND:
        {
            ts_sms_new_with_idx_ind_t *ind = (ts_sms_new_with_idx_ind_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_NEW_WITH_PDU_IND received.");
            app_logi("[test sms srv]SMS NEW IND: index(%d)", ind->index);
            /* don't trigger semaphore */
            is_handled = false;
        }
        break;
    case TS_EVENT_SMS_STATUS_REPORT_WITH_PDU_IND:
        {
            ts_sms_status_report_with_pdu_ind_t *resp = (ts_sms_status_report_with_pdu_ind_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_STATUS_REPORT_WITH_PDU_IND received.");
            app_logi("[test sms srv]STATUS REPORT IND: pdu(%d)", resp->pdu);
            /* don't trigger semaphore */
            is_handled = false;
        }
        break;
    case TS_EVENT_SMS_NEW_WITH_PDU_IND:
        {
            int32_t err_code = -1;
            ts_sms_new_with_pdu_ind_t *new_ind = (ts_sms_new_with_pdu_ind_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_NEW_WITH_PDU_IND received. tpdu_len(%d), pdu(%s)", new_ind->tpdu_len,
                     new_ind->pdu);
            /* send ack */
            err_code = aic_srv_tele_sms_send_ack_with_pdu(0, true, NULL);
            if (err_code != 0) {
                app_loge("[test sms srv]aic_srv_tele_sms_send_ack_with_pdu req sent error");
            } else {
                app_logi("[test sms srv]aic_srv_tele_sms_send_ack_with_pdu ack sent OK");
            }
            /* don't trigger semaphore */
            is_handled = false;
        }
        break;
    case TS_EVENT_SMS_SEND_ACK_WITH_PDU_RESP:
        {
            ts_sms_send_ack_with_pdu_resp_t *resp = (ts_sms_send_ack_with_pdu_resp_t *)p_param;
            app_logi("[test sms srv]TS_EVENT_SMS_SEND_ACK_WITH_PDU_RESP received. err_code(%d)", resp->base_info.err_code);
            /* don't trigger semaphore */
            is_handled = false;
        }
        break;
    case TS_EVENT_SMS_STORAGE_STATUS_WITH_PDU_IND:
        {
            /* don't trigger semaphore */
            is_handled = false;
        }
        break;
    default:
        is_handled = false;
        break;
    }
    if (is_handled) {
        app_logi("[test sms srv]pthread_cond_signal triggered");
        pthread_cond_signal(&g_tele_sms_sample_cond);
    } else {
        app_logi("[test sms srv]pthread_cond_signal not triggered");
    }
    return 0;
}

static int32_t test_sms_send_pdu(bool is_write_to_sim)
{
    int32_t ret = 0;
    app_logi("[test sms srv]test_sms_send_pdu start");
    ts_sms_send_with_pdu_info_t pdu_info = {0};
    ts_radio_operator_info_t oper_info = {0};
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    pdu_info.is_write_to_sim = is_write_to_sim;
    pdu_info.p_smsc = NULL;
    aic_srv_tele_radio_get_operator_info(cur_card_id, &oper_info);
    if (0 == strcmp(oper_info.plmn_mcc, "460")) {
        if (0 == strcmp(oper_info.plmn_mnc, "00")) {
            pdu_info.p_pdu = "150105810180F6000802044F59989D";  /* send to 10086 with "余额" */
        } else if (0 == strcmp(oper_info.plmn_mnc, "11")) {
            pdu_info.p_pdu = "010005810100F100000331580C";  /* send to 10001 with "101" */
        } else {
            pdu_info.p_pdu = "010005810110F0000004637CBE0C"; /* send to 10010 with "cxye" */
        }
    }
    ret = aic_srv_tele_sms_send_with_pdu(cur_card_id, &pdu_info, NULL);

    return ret;
}

static int32_t test_sms_get_smsc(void)
{
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    app_logi("[test sms srv]test_sms_get_smsc start");
    return aic_srv_tele_sms_get_smsc_with_pdu(cur_card_id, NULL);
}

static int32_t test_sms_set_smsc(const char *new_smsc)
{
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    app_logi("[test sms srv]test_sms_set_smsc start");
    return aic_srv_tele_sms_set_smsc_with_pdu(cur_card_id, new_smsc, NULL);
}

static int32_t test_sms_write_pdu(ts_sms_rec_status_e status)
{
    int32_t ret = 0;
    ts_sms_write_with_pdu_to_sim_info_t pdu_info = {0};
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    app_logi("[test sms srv]test_sms_write_pdu start");
    pdu_info.status = status;
    pdu_info.p_smsc = NULL;
    pdu_info.p_pdu = "010005810110F000001654747A0E4ACF416110B53CA7839AE5F93C7C2E03"; /* send to 10010 with "This
                                                                                                is a Test Message" */
    ret = aic_srv_tele_sms_write_with_pdu_to_sim(cur_card_id, &pdu_info, NULL);

    return ret;
}

static int32_t test_sms_get_list(ts_sms_box_type_e box)
{
    int32_t ret = 0;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    app_logi("[test sms srv]test_sms_get_list start");
    ret = aic_srv_tele_sms_get_list_in_sim_with_pdu(cur_card_id, box, NULL);
    return ret;
}

static int32_t test_sms_read_pdu(void)
{
    int32_t ret = -1;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    if (g_tele_sms_list && g_tele_sms_count > 0) {
        ret = aic_srv_tele_sms_read_in_sim_with_pdu(cur_card_id, g_tele_sms_list[0], NULL);
    } else {
        app_logi("[test sms srv]test_sms_read_pdu no available pdu to read.");
    }
    return ret;
}

static int32_t test_sms_delete_pdu(void)
{
    int32_t ret = -1;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    app_logi("[test sms srv]test_sms_delete_pdu start");
    /* delete all last read */
    if (g_tele_sms_list) {
        ret = aic_srv_tele_sms_del_in_sim_with_pdu(cur_card_id, g_tele_sms_list, g_tele_sms_count, NULL);
    } else {
        app_loge("[test sms srv]test_sms_delete_pdu no record to delete");
    }
    return ret;
}

static void TeleSmsSampleEntry(void *thread_input)
{
    int32_t ret = 0;
    aic_srv_tele_sms_with_pdu_register(NULL, tele_sms_sample_callback);

    app_logi("[test sms srv]Tele sms sample entry point!\n");
    /* 10 seconds */
    /* wait pin ready */
    while (1) {
        ts_sim_status_e sim_status;
        static int32_t count = 0;
        uint8_t cur_card_id = TS_CARD_ID_MAX;

        aos_sleep(10000);
        aic_srv_tele_sim_get_current_card(&cur_card_id);
        aic_srv_tele_sim_get_status(cur_card_id, &sim_status);
        if (TS_SIM_STATUS_READY == sim_status) {
            break;
        } else {
            count++;
            app_logi("[test sms srv]TeleSmsSampleEntry PIN NOT READY. waited for %ds", count * 10);
        }
    }

    /* test: get smsc */
    ret = test_sms_get_smsc();
    if (!ret) {
        app_logi("[test sms srv]test_sms_get_smsc req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_get_smsc req sent error");
    }

    if (g_tele_sms_old_smsc[0] != 0) {
        /* test: set smsc */
        ret = test_sms_set_smsc("+861380000000");
        if (!ret) {
            app_logi("[test sms srv]test_sms_get_smsc req sent ok");
            tele_sms_sample_wait_for_response();
        } else {
            app_loge("[test sms srv]test_sms_get_smsc req sent error");
        }

        /* test: restore old smsc */
        ret = test_sms_set_smsc(g_tele_sms_old_smsc);
        if (!ret) {
            app_logi("[test sms srv]test_sms_get_smsc req sent ok");
            tele_sms_sample_wait_for_response();
        } else {
            app_loge("[test sms srv]test_sms_get_smsc req sent error");
        }
    }

    /* test: write unsent sms pdu into sim */
    ret = test_sms_write_pdu(TS_SMS_REC_STATUS_UNSENT);
    if (!ret) {
        app_logi("[test sms srv]test_sms_write_pdu req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_write_pdu req sent error");
    }

    /* test: write sent sms pdu into sim */
    ret = test_sms_write_pdu(TS_SMS_REC_STATUS_SENT);
    if (!ret) {
        app_logi("[test sms srv]test_sms_write_pdu req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_write_pdu req sent error");
    }

    /*test: get sms inbox list */
    ret = test_sms_get_list(TS_SMS_BOX_TYPE_INBOX);
    if (!ret) {
        app_logi("[test sms srv]test_sms_get_list inbox req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_get_list req sent error");
    }

    /*test: get sms outbox list */
    ret = test_sms_get_list(TS_SMS_BOX_TYPE_OUTBOX);
    if (!ret) {
        app_logi("[test sms srv]test_sms_get_list outbox req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_get_list outbox req sent error");
    }

    /*test: get sms all list */
    ret = test_sms_get_list(TS_SMS_BOX_TYPE_ALL);
    if (!ret) {
        app_logi("[test sms srv]test_sms_get_list all boxes req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_get_list all boxes req sent error");
    }

    /* test: read sms pdu */
    /* read the first pdu rec from last fetched list */
    ret = test_sms_read_pdu();
    if (!ret) {
        app_logi("[test sms srv]test_sms_read_pdu req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_read_pdu req sent error");
    }

    /* test: delete sms pdu in sim */
    /* Note: the list to delete depends on the last getting list result */
    ret = test_sms_delete_pdu();
    if (!ret) {
        app_logi("[test sms srv]test_sms_delete_pdu req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_delete_pdu req sent error");
    }

    /* assume the sms service in modem is ready in 10s */
    aos_sleep(10000);

    /* test: send pdu without saving */
    ret = test_sms_send_pdu(false);
    if (!ret) {
        app_logi("[test sms srv]test_sms_send_pdu req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_send_pdu req sent error");
    }

    /* test: send pdu with saving */
    ret = test_sms_send_pdu(true);
    if (!ret) {
        app_logi("[test sms srv]test_sms_send_pdu req sent ok");
        tele_sms_sample_wait_for_response();
    } else {
        app_loge("[test sms srv]test_sms_send_pdu req sent error");
    }

    app_logi("[test sms srv] test finished");
}

int tele_sms_init_sample(void)
{
    aos_create_thread("tele_sms_sample_task", TeleSmsSampleEntry, NULL, TELE_SMS_SAMPLE_TASK_STK_SIZE, 0, NULL,
                      TELE_SMS_SAMPLE_TASK_COM_PRIO, AOS_AUTO_START);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(tele_sms_init_sample);
