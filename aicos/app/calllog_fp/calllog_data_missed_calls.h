

#if 0

#ifdef CONFIG_APP_CALLLOG_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_CALLLOG_DATA_MISSED_CALLS_FP_H__
#define __APP_CALLLOG_DATA_MISSED_CALLS_FP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CALLLOG_DATA_MISSED_CALLS_FOLDER_PATH
#define     CALLLOG_DATA_MISSED_CALLS_FOLDER_PATH "/C/missed"
#endif








    uint32_t is_calllog_data_missed_calls_empty();

    int32_t calllog_data_missed_calls_write(calllog_data_param_t * p_data_param,calllog_data_time_t * p_cur_time);

    int32_t calllog_data_missed_calls_read(calllog_data_param_t * p_data_param,uint32_t file_d_off);

    int32_t calllog_data_missed_number_del_file(uint32_t file_d_off);

    int32_t calllog_data_missed_number_folder();





#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_CALLLOG_DATA_MISSED_CALLS_FP_H__ */

#endif

#endif

