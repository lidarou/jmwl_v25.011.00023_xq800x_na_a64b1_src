

#ifndef _UI_EVENTS_H
#define _UI_EVENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

    void fp_incoming_ui_event_actions(lv_event_t * e);
    void fp_on_the_phone_ui_event_actions(lv_event_t * e);
    void fp_dial_ui_event_actions(lv_event_t * e);
    void fp_hangup_ui_event_actions(lv_event_t * e);
    void fp_on_the_phone_option_ui_event_actions(lv_event_t * e);



#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif

