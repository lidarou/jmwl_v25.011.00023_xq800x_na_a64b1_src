

#if 0
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>
#include <aos_thread.h>
#include <pthread.h>
#include "calllog_data.h"
#include "calllog_data_missed_calls.h"
#include "fs_api.h"

#include "app_log.h"

#define __inf                   APP_LOGI




uint32_t is_calllog_data_missed_calls_empty()
{
    return is_calllog_data_file_empty(CALLLOG_DATA_MISSED_CALLS_FOLDER_PATH);
}


int32_t calllog_data_missed_calls_write(calllog_data_param_t * p_data_param,calllog_data_time_t * p_cur_time)
{
    return calllog_data_write(CALLLOG_DATA_MISSED_CALLS_FOLDER_PATH,p_data_param,p_cur_time);
}


int32_t calllog_data_missed_calls_read(calllog_data_param_t * p_data_param,uint32_t file_d_off)
{
    return calllog_data_read(CALLLOG_DATA_MISSED_CALLS_FOLDER_PATH,p_data_param,file_d_off);
}


int32_t calllog_data_missed_number_del_file(uint32_t file_d_off)
{
    return fp_del_file_by_folder_off(CALLLOG_DATA_MISSED_CALLS_FOLDER_PATH,file_d_off);
}


int32_t calllog_data_missed_number_folder()
{
    return fp_fs_remove_folder(CALLLOG_DATA_MISSED_CALLS_FOLDER_PATH);
}


#endif


