/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file phonebook_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_type.h>
#include "aic_srv_tele.h"
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aic_list.h>
#include <strings.h>

/*********************
 *      DEFINES
 *********************/
#define TELE_PB_SAMPLE_TASK_STK_SIZE (4096u)
#define TELE_PB_SAMPLE_TASK_PRIO (35u)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static ts_pb_status_t s_pb_status = {0};
static int s_pb_read_index = -1;
/**********************
 *   STATIC FUNCTIONS
 **********************/
int32_t tele_pb_srv_callback(void *p_param, uint32_t size)
{
    if (!p_param || !size) {
        return -1;
    }
    uint32_t event = *(uint32_t *)p_param;

    app_logi("[test pb srv]tele_pb_srv_callback event: %d, size: %d .", event, size);
    switch (event) {
    case TS_EVENT_PB_INIT_IND:
        {
            app_logi("[test pb srv]TS_EVENT_PB_INIT_IND received.");
            break;
        }
    case TS_EVENT_PB_WRITE_RESP:
        {
            ts_pb_write_resp_t *resp = (ts_pb_write_resp_t *)p_param;
            app_logi("[test pb srv]TS_EVENT_PB_WRITE_RESP received.");
            app_logi("[test pb srv] ts_pb_write_resp card_id(%d) & written_index(%d)", resp->card_id,
                     resp->written_index);
            break;
        }
    case TS_EVENT_PB_READ_RESP:
        {
            ts_pb_read_resp_t *resp = (ts_pb_read_resp_t *)p_param;
            app_logi("[test pb srv]TS_EVENT_PB_READ_RESP received.");
            app_logi("[test pb srv]ts_pb_read_resp card_id(%d) count(%d) end(%d)", resp->card_id, resp->count,
                     resp->end);
            for (int i = 0; i < resp->count; i++) {
                if (resp->p_contact[i].name[0] != '\0') {
                    app_logi("[test pb srv]ts_pb_read index: %d, name: [%s] number: [%s]", resp->p_contact[i].index,
                             resp->p_contact[i].name, resp->p_contact[i].number);
                    s_pb_read_index = resp->p_contact[i].index;
                }
            }
            break;
        }
    case TS_EVENT_PB_GET_STATUS_RESP:
        {
            ts_pb_get_status_resp_t *resp = (ts_pb_get_status_resp_t *)p_param;
            app_logi("[test pb srv]TS_EVENT_PB_GET_STATUS_RESP received.");
            app_logi("[test pb srv]card_id(%d)", resp->card_id);
            app_logi("[test pb srv]first_index(%d), used(%d), total(%d)", resp->pb_status.first_index,
                     resp->pb_status.used, resp->pb_status.total);

            memcpy(&s_pb_status, &resp->pb_status, sizeof(s_pb_status));
            break;
        }
    default:
        break;
    }

    return 0;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void tele_pb_sample_entry(void *thread_input)
{
    while (1) {
        app_logi("tele_pb sample entry point!\n");

        /* 10 seconds */
        aos_sleep(10000);

        /* get_status */
        app_logi("tele_pb card0 aic_srv_tele_pb_get_status start!\n");
        aic_srv_tele_pb_get_status(0, NULL);

        /* wait to read */
        while (s_pb_status.used == 0) {
            /* 1 seconds */
            aos_sleep(1000);
        }

        aic_srv_tele_pb_read(0, s_pb_status.total, 0, NULL);

        /* write one */
        while (s_pb_read_index < 0) {
            /* 1 seconds */
            aos_sleep(1000);
        }

        ts_pb_info_t wr_contact = {0};
        wr_contact.index = s_pb_read_index;
        strcpy(wr_contact.name, "林黛玉");
        strcpy(wr_contact.number, "+86654321");

        aic_srv_tele_pb_write(0, &wr_contact, NULL);
        aos_sleep(2000);
        /* write before change to card1 */
        wr_contact.index = s_pb_read_index + 1;
        strcpy(wr_contact.name, "京中丽");
        strcpy(wr_contact.number, "+86650021");

        aic_srv_tele_pb_write(1, &wr_contact, NULL);
        aos_sleep(2000);

        /* change to card1 */
        app_logi("tele_pb card1 aic_srv_tele_pb_get_status start!\n");
        s_pb_status.used = 0;
        aic_srv_tele_pb_get_status(1, NULL);

        /* wait ready */
        while (s_pb_status.used == 0) {
            /* 1 seconds */
            aos_sleep(1000);
        }

        wr_contact.index = s_pb_read_index + 2;
        strcpy(wr_contact.name, "中丽");
        strcpy(wr_contact.number, "+8633333");

        aic_srv_tele_pb_write(1, &wr_contact, NULL);

        s_pb_read_index = -1;
    }
}

int tele_pb_init_sample(void)
{
    aic_srv_tele_pb_register(NULL, tele_pb_srv_callback);
    aos_create_thread("tele_pb_sample_task", tele_pb_sample_entry, NULL, TELE_PB_SAMPLE_TASK_STK_SIZE, 0, NULL,
                      TELE_PB_SAMPLE_TASK_PRIO, AOS_AUTO_START);

    return AOS_SUCCESS;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(tele_pb_init_sample);
