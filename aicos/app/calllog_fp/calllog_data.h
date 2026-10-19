

#ifdef CONFIG_APP_CALLLOG_FP

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of idle app.
 */
#ifndef __CALLLOG_FP_DATA_H__
#define __CALLLOG_FP_DATA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CALLLOG_DATA_FOLDER_PATH
#define     CALLLOG_DATA_FOLDER_PATH "/C/calllog"
#endif

#ifndef CALLLOG_DATA_FILE_PATH
#define     CALLLOG_DATA_FILE_PATH  "/C/calllog.bin"
#endif


#define     CALLLOG_DATA_NAME_LEN   38
#define     CALLLOG_DATA_LEN        30

#define     CALLLOG_DATA_COUNT_MAX  50



#if 1
    typedef struct _calllog_data_time_h_ {
        uint16_t        year;
        uint8_t         month;
        uint8_t         wday;
        uint8_t         day;
        uint8_t         hour;
        uint8_t         minute;
        uint8_t         second;
        } calllog_data_time_t;


    typedef struct _CALLLOG_DATA__HEAD_H_ {
        uint32_t        flag;
        uint16_t        count;
        uint16_t        used;

        } calllog_data_head_t;


    typedef struct _CALLLOG_DATA_PARAM_H_ {
        uint8_t         name[CALLLOG_DATA_NAME_LEN + 2];
        uint8_t         number[CALLLOG_DATA_LEN + 2];
        uint8_t         card_id;
        uint8_t         call_type;                  /*0:已拨打，1：未接来电，2：已接来电*/
        calllog_data_time_t time;
        } calllog_data_param_t;


#else

    typedef struct _calllog_data_time_h_ {
        uint16_t        year;
        uint8_t         month;
        uint8_t         wday;
        uint8_t         day;
        uint8_t         hour;
        uint8_t         minute;
        uint8_t         second;
        } calllog_data_time_t;


    typedef struct _CALLLOG_DATA_PARAM_H_ {
        uint8_t         name[CALLLOG_DATA_NAME_LEN + 2];
        uint8_t         number[CALLLOG_DATA_LEN + 2];
        uint8_t         card_id;
        } calllog_data_param_t;


#endif


    uint32_t is_calllog_data_file_empty(char * folder_path);
    void calllog_data_folder_init(char * folder_path);
    int32_t calllog_data_write(char * folder_path,calllog_data_param_t * p_data_param,
        calllog_data_time_t * p_cur_time);
    int32_t calllog_data_read(char * folder_path,calllog_data_param_t * p_data_param,uint32_t file_d_off);
    uint8_t calllg_missed_unread_count_get();
    uint32_t is_calllog_data_missed_calls_empty();
    uint32_t is_calllog_data_recevied_calls_empty();
    uint32_t is_calllog_data_dialed_number_empty();
    void calllog_data_init(void);
    int32_t calllog_data_del_number_by_index(uint32_t index);
    void calllg_missed_unread_count_add();
    void calllog_data_reset(void);
    int32_t calllog_data_del_number_by_type(uint8_t call_type);
    int32_t calllog_data_write_number(calllog_data_param_t * p_data_param);
    int32_t calllog_data_read_number(uint32_t index,calllog_data_param_t * p_data_param);
    void calllg_missed_unread_count_reset();

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __CALLLOG_FP_DATA_H__ */

#endif

