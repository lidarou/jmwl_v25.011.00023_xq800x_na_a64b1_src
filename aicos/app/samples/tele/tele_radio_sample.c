/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * @file tele_radio_sample.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>

#include "aic_srv_tele.h"


/*********************
 *      DEFINES
 *********************/
#define TELE_RADIO_SAMPLE_TASK_STK_SIZE       (4096u)
#define TELE_RADIO_SAMPLE_TASK_COM_PRIO       (35u)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void get_imei_sample(void)
{
    char imei[20] = {0};

    aic_srv_tele_radio_get_current_slot_imei_sync(imei);
    app_logi("[%s] Get current IMEI is (%s)\n", __func__, imei);
}

void get_reg_status_sample(void)
{
    ts_radio_reg_status_e regStatus = TS_RADIO_REG_STATUS_NO_SERVICE;
    uint8_t cur_card_id = TS_CARD_ID_MAX;

    aic_srv_tele_sim_get_current_card(&cur_card_id);
    aic_srv_tele_radio_get_reg_status_sync(cur_card_id, &regStatus);
    app_logi("[%s] Get reg status: reg_status(%d)\n", __func__, regStatus);
}
void get_current_cellinfo_sample(void)
{
    ts_radio_cell_info_t cell_info_list = {0};

    aic_srv_tele_radio_get_cell_info_sync(0, &cell_info_list);

    app_logi("get_current_cellinfo_sample  \n");
}

void get_neighbor_cellinfo_sample(void)
{
    ts_radio_neighbor_cell_info_list_t cell_info_list = {0};

    aic_srv_tele_radio_get_neighbor_cell_info_list_sync(0, &cell_info_list);

    app_logi("get_neighbor_cellinfo_sample  \n");
}

static void RadioSampleEntry(void *thread_input)
{
    while (1) {
        app_logi("Tele Radio Samplev entry point!\n");
        /* 10 seconds */
        aos_sleep(10000);

        get_reg_status_sample();  /* the sample code for get register status */

        get_imei_sample();  /* the sample code for get imei */

        get_current_cellinfo_sample();  /* the sample code for get current cellinfo */

        get_neighbor_cellinfo_sample();  /* the sample code for get neighbor cellinfo */
    }
}

int radio_init_sample(void)
{
    aos_create_thread(
        "radio_sample_task",
        RadioSampleEntry,
        NULL,
        TELE_RADIO_SAMPLE_TASK_STK_SIZE,
        0,
        NULL,
        TELE_RADIO_SAMPLE_TASK_COM_PRIO,
        AOS_AUTO_START);

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(radio_init_sample);

