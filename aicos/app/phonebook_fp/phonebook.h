

#ifndef __PHONEBOOK_H__
#define __PHONEBOOK_H__

#include "src/misc/lv_ll.h"

#define PHONEBOOK_NAME_LEN      (38 + 2)
#define PHONEBOOK_NUMBER_LEN    30
#define PHONEBOOK_PINYIN_LEN    40


#define PHONEBOOK_FLASH_MAX_COUNT 500
#define PHONEBOOK_SIM_MAX_COUNT 500

//#define PHONEBOOK_USER_RAM_CACHE
typedef struct _PHONEBOOK_HEAD_H_ {
    uint32_t        flag;
    uint16_t        count;
    uint16_t        used;

} phonebook_head_t;

typedef struct _PHONEBOOK_PARAM_H_ {
    uint8_t         name[PHONEBOOK_NAME_LEN + 2];
    uint8_t         number[PHONEBOOK_NUMBER_LEN + 2];
    uint16_t        index;
    uint8_t         strage_format;                  //0:存储在手机上,1:卡1，卡2
    uint8_t         name_pinyin[PHONEBOOK_PINYIN_LEN + 2];
} phonebook_param_t;

typedef struct _PHONEBOOK_MAP_HEAD_H_ {
    uint32_t        flag;
    uint16_t        count;
    uint16_t        used;

} phonebook_map_head_t;

typedef struct _PHONEBOOK_MAP_PARAM_H_ {
    uint16_t        index;
    uint8_t         strage_format;                  //0:存储在手机上,1:卡1，卡2
} phonebook_map_param_t;

#define PHONEBOOK_PARAM_ADD_INDEX_VALUE 0xFFFF


enum {
    PHONEBOOK_RET_FS = -2,
    PHONEBOOK_RET_ERROR = -1,
    PHONEBOOK_RET_OK,
};

typedef struct {
#ifdef PHONEBOOK_USER_RAM_CACHE
    uint8_t         name[PHONEBOOK_NAME_LEN + 2];

    //uint8_t number[PHONEBOOK_NUMBER_LEN+2];
#endif

    uint16_t        index;
    uint8_t         strage_format;                  //0:存储在手机上,1:卡1，卡2
} phonebook_map_node_t;

uint32_t get_phonebook_storge_used();
int32_t read_phonebook_param_storge(phonebook_map_node_t * p_cur_node,phonebook_param_t * p);
int32_t edit_phonebook_param_storge(phonebook_map_node_t * p_edit_node,phonebook_param_t * p);
int32_t add_phonebook_param_storge(phonebook_param_t * p);

uint32_t get_tele_phonebook_sim_status();
uint32_t get_phonebook_param_add_index();

uint32_t phonebook_data_is_init();
void phone_msg_get_name_by_number_action(uint8_t * number);
void phonebook_data_copy_storge(uint32_t direction,phonebook_map_node_t * *p_node,uint32_t node_count);
void phonebook_data_delete_storge(phonebook_map_node_t * *p_node,uint32_t node_count);
lv_ll_t * get_phonebook_storge_ll();
uint8_t tele_phone_data_get_display_mode();
void tele_phone_data_set_display_mode(uint8_t display_mode);
void get_phonebook_head_local(phonebook_head_t * phonebook_head);
void get_phonebook_head_sim(phonebook_head_t * phonebook_head);

#define PHONEBOOK_NODE_MAX_SIZE (PHONEBOOK_FLASH_MAX_COUNT+PHONEBOOK_SIM_MAX_COUNT)



enum {
    pb_init_step_tele_data_reset = 0,
    pb_init_step_tele_get_sim_exit,
    pb_init_step_tele_get_status,
    pb_init_step_tele_wait_status,
    pb_init_step_tele_get_pb_data,
    pb_init_step_tele_wait_pb_data,
    pb_init_step_tele_pb_data,
    pb_init_step_sort_data,
    pb_init_step_end,
};

typedef struct {
    uint32_t        msg_id;
    void *          p_param;
    uint32_t        size;
} phone_queue_param_t;

enum {
    PHONEBOOK_TASK_QUEUE_COPY = 0x01,
    PHONEBOOK_TASK_QUEUE_DEL,

    PHONEBOOK_TASK_QUEUE_EMPTY=0xFFFFFF
};

typedef int32_t(*phonebook_copy_callback_t) (uint32_t copy_count,uint32_t copy_status,void * user_data);

typedef struct __PHONEBOOK_QUEUE_COPY_PARAM_H_ {
    uint32_t        direction;
    phonebook_map_node_t * *p_node;
    uint32_t        node_count;
    phonebook_copy_callback_t fun;
    void *          user_data;
} phonebook_queue_copy_param_t;

#define PHONE_CFG_TASK_QUEUE_NUM (5)
typedef int32_t(*phone_get_name_func_callback_t) (void * user_data,uint8_t * name);
void phone_set_get_name_callback_fun(phone_get_name_func_callback_t fun,void * user_data);

uint32_t phone_data_msg_recv(phone_queue_param_t * p_app_msg,uint32_t wait_option);
int32_t phone_data_msg_send_number_get_name(uint8_t * number,uint32_t number_size);
void tele_phonebook_reset_init_steps();
void sim_write_create_folder_init();
uint32_t phonebook_data_is_init();
int32_t phone_get_name_by_number(uint8_t * number,uint8_t * name);
void tele_phonebook_copy_init();
int phone_data_init(void);
int32_t phone_data_msg_send_del(phonebook_map_node_t **p_node,uint32_t node_count,phonebook_copy_callback_t fun,void *user_data);
int32_t phone_data_msg_send_copy(uint32_t direction,phonebook_map_node_t **p_node,uint32_t node_count,phonebook_copy_callback_t fun,void *user_data);
void tele_phonebook_copy_stop();
uint32_t get_phonebook_ui_used();
phonebook_map_node_t * get_phonebook_next_node(phonebook_map_node_t * p_cur_node);
phonebook_map_node_t * get_phonebook_head_node();
phonebook_map_node_t * get_phonebook_prev_node(phonebook_map_node_t * p_cur_node);
phonebook_map_node_t * get_phonebook_prev_count_node(phonebook_map_node_t * p_cur_node,uint32_t count);
phonebook_map_node_t * get_phonebook_next_count_node(phonebook_map_node_t * p_cur_node,uint32_t count);
phonebook_map_node_t * get_phonebook_next_count_node1(phonebook_map_node_t * p_cur_node,uint32_t count,
    uint8_t display_flash_flag,uint8_t display_sim_flag);
phonebook_map_node_t * get_phonebook_prev_count_node1(phonebook_map_node_t * p_cur_node,uint32_t count,
    uint8_t display_flash_flag,uint8_t display_sim_flag);
phonebook_map_node_t * get_phonebook_tail_node();
uint32_t phonebook_data_is_get_pb_data_start();

#endif

