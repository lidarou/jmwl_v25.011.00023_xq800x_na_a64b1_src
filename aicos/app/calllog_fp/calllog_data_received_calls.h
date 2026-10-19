

#if 0

#ifdef CONFIG_APP_CALLLOG_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_CALLLOG_DATA_REVEIVED_CALLS_FP_H__
#define __APP_CALLLOG_DATA_REVEIVED_CALLS_FP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CALLLOG_DATA_RECEIVEDC_CALLS_FOLDER_PATH
#define     CALLLOG_DATA_RECEIVEDC_CALLS_FOLDER_PATH "/C/received"
#endif

    uint32_t is_calllog_data_recevied_calls_empty();
    int32_t calllog_data_recevied_calls_write(calllog_data_param_t * p_data_param,aos_systime * p_cur_time);
    int32_t calllog_data_recevied_calls_read(calllog_data_param_t * p_data_param,uint32_t file_d_off);
    int32_t calllog_data_recevied_number_del_file(uint32_t file_d_off);
    int32_t calllog_data_recevied_number_folder();








#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_CALLLOG_DATA_REVEIVED_CALLS_FP_H__ */

#endif

#endif

