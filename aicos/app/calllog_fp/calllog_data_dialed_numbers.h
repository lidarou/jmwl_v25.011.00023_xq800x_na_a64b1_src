

#ifdef CONFIG_APP_CALLLOG_FP

#if 0

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __APP_CALLLOG_DATA_DIALED_NUMBER_CALLS_FP_H__
#define __APP_CALLLOG_DATA_DIALED_NUMBER_CALLS_FP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CALLLOG_DATA_DIALED_NUMBER_FOLDER_PATH
#define     CALLLOG_DATA_DIALED_NUMBER_FOLDER_PATH "/C/dialed"
#endif

    uint32_t is_calllog_data_dialed_number_empty();
    int32_t calllog_data_dialed_number_write(calllog_data_param_t * p_data_param,calllog_data_time_t * p_cur_time);
    int32_t calllog_data_dialed_number_read(calllog_data_param_t * p_data_param,uint32_t file_d_off);
    int32_t calllog_data_dialed_number_del_file(uint32_t file_d_off);
    int32_t calllog_data_dialed_number_folder();


#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __APP_CALLLOG_DATA_DIALED_NUMBER_CALLS_FP_H__ */

#endif

#endif

