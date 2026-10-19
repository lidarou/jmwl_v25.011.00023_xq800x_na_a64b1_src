

#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

    void fp_calllog_menu_ui_event_actions(lv_event_t * e);
    void fp_calllog_missed_calls_ui_event_actions(lv_event_t * e);
    void fp_missed_calls_option_ui_event_actions(lv_event_t * e);
    void fp_calllog_dial_calls_ui_event_actions(lv_event_t * e);
    void fp_calllog_recevied_calls_ui_event_actions(lv_event_t * e);
    void fp_dialed_calls_option_ui_event_actions(lv_event_t * e);
    void fp_received_calls_option_ui_event_actions(lv_event_t * e);
    void fp_calllog_del_file_del_ui_event_actions(lv_event_t * e);
    void fp_calllog_del_ui_event_actions(lv_event_t * e);



#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif

