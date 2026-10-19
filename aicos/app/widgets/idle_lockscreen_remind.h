

#ifndef __IDLE_LOCK_SCREEN_REMIND_H__
#define __IDLE_LOCK_SCREEN_REMIND_H__
#include <stdint.h>
typedef void(*idle_lock_remind_timer_callback_fun_t) (void * user_data);


enum {
    IDLE_LOCK_SCREEN_TIMER_PUBLIC,
    IDLE_LOCK_SCREEN_TIMER_PRIVATE,
};


typedef struct {
    char *          str;
    uint32_t        tiemr_delay;
    idle_lock_remind_timer_callback_fun_t fun;
    void *          user_data;
    uint8_t         self_destory_flag;
    uint8_t         key;
} idle_lock_screen_remind_param_t;


lv_obj_t * ui_idle_lock_screen_remind_init(idle_lock_screen_remind_param_t * param);
void ui_idle_lock_screen_remind_param_init(idle_lock_screen_remind_param_t * param);





#endif

