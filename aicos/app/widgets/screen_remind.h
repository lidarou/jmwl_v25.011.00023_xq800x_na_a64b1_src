

#ifndef __SCREEN_REMIND_H__
#define __SCREEN_REMIND_H__
#include <stdint.h>
typedef void(*remind_timer_callback_fun_t) (void * user_data);


enum {
    SCREEN_TIMER_PUBLIC,
    SCREEN_TIMER_PRIVATE,
};

typedef struct {
    char *          str;
    uint32_t        tiemr_delay;
    remind_timer_callback_fun_t fun;
    void *          user_data;
    uint8_t         self_destory_flag;
    uint8_t         new_win;
    uint8_t         key_exit_enable;
} screen_remind_param_t;

lv_obj_t * ui_screen_remind_init(screen_remind_param_t * param);
void ui_screen_remind_param_init(screen_remind_param_t * param);
void ui_screen_remind_uninit(lv_obj_t * ui_Screen);

typedef struct {
    uint16_t        format;                         //0:从手机到sim，1：从sim到手机
    uint16_t        copy_del;                       //0:copy    1:del
    void * *        p_state;
    uint32_t        state_count;
    void *          ui_screen;
} phonebook_enter_param_remind_copy_t;

#endif

