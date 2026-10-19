/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
/**
 * @file lv_port_indev_keypad.c
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x058d54
#define LOG_MODULE    AMOD_LVGL

/*********************
 *      INCLUDES
 *********************/
#ifndef WIN32
#endif
#include "lvgl.h"
#include "kpc_codes.h"
#include "kpc.h"
#include "lv_port_indev_keypad.h"
#include "pwr_key.h"
#include "display.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    int32_t lv_key;
    int32_t kpc_key;
}keypad_keymap_t;

typedef struct
{
    uint32_t lv_key;
    lv_indev_state_t state;
}lv_keypad_info_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void keypad_read(lv_indev_drv_t *p_indev_drv, lv_indev_data_t *p_data);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_indev_t *s_indev_keypad_ptr = NULL;
static uint32_t lv_last_key = 0;

/* kpc key map to lv key. */
static keypad_keymap_t s_keymap_arr[] = {
    {LV_KEY_UP,        KEY_UP},
    {LV_KEY_DOWN,      KEY_DOWN},
    {LV_KEY_LEFT,      KEY_LEFT},
    {LV_KEY_RIGHT,     KEY_RIGHT},
    {LV_KEY_ENTER,     KEY_ENTER},
    {LV_KEY_ESC,       KEY_ESC},
    {LV_KEY_BACKSPACE, KEY_BACK},
    {LV_KEY_HOME,      KEY_HOME},
    {LV_KEY_GREEN,     KEY_GREEN}, /* TODO: research map relative key */
    {LV_KEY_RED,       KEY_RED},   /* TODO: research map relative key */
    {LV_KEY_END,       KEY_END},
    {LV_KEY_DEL,       KEY_DEL},
    {LV_KEY_STAR,      KEY_STAR},
    {LV_KEY_HASH,      KEY_HASH},
    {LV_KEY_0,         KEY_0},
    {LV_KEY_1,         KEY_1},
    {LV_KEY_2,         KEY_2},
    {LV_KEY_3,         KEY_3},
    {LV_KEY_4,         KEY_4},
    {LV_KEY_5,         KEY_5},
    {LV_KEY_6,         KEY_6},
    {LV_KEY_7,         KEY_7},
    {LV_KEY_8,         KEY_8},
    {LV_KEY_9,         KEY_9},
    {LV_KEY_END,       KEY_POWER},
    /* TODO: add more keys */
};

static lv_keypad_info_t g_keypad_array[KPC_KEY_PRESSED_MAX_KEY_NUM] = {0};

/**********************
 *   STATIC FUNCTIONS
 **********************/
static int32_t keypad_transfer_to_lv_key(int32_t kpc_key)
{
    uint32_t keymap_idx = 0;
    for (keymap_idx = 0; keymap_idx < (sizeof(s_keymap_arr) / sizeof(s_keymap_arr[0])); keymap_idx++) {
        if (kpc_key == s_keymap_arr[keymap_idx].kpc_key) {
            return s_keymap_arr[keymap_idx].lv_key;
        }
    }
    return -1;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
* keypad_write - Brief description of function.
* Parameter: [In]: void * p_data
*            [Out]: None
*            [Return]: if success return 0, else return other value.
* Description: This function is used to write keypad.
*/
int keypad_write(void * p_data)
{
    int32_t lv_key = -1;
    uint32_t key_idx = 0;
    kpc_key_array_t *p_kpc_key = NULL;

    if(NULL == p_data) {
        aicx_error(0x17d154, "s", "[%s] p_data is NULL!!!", __func__);
        return -1;
    }

    p_kpc_key = (kpc_key_array_t *)p_data;

    for (key_idx = 0; key_idx < KPC_KEY_PRESSED_MAX_KEY_NUM; key_idx++) {
        if (0 != p_kpc_key->key[key_idx].key_event) {
            /* transfer to lv info */
            lv_key = keypad_transfer_to_lv_key(p_kpc_key->key[key_idx].key_value);

            aicx_info(0x194442, "sddd", "[%s]kpc key_val = %d, key_event = %d, lv_key = %d.\r\n", __func__, p_kpc_key->key[key_idx].key_value, p_kpc_key->key[key_idx].key_event, lv_key);

            g_keypad_array[key_idx].lv_key = lv_key;
            if (KPC_KEY_EVENT_PRESS == p_kpc_key->key[key_idx].key_event) {
                g_keypad_array[key_idx].state = LV_INDEV_STATE_PR;
            } else if (KPC_KEY_EVENT_RELEASE == p_kpc_key->key[key_idx].key_event) {
                g_keypad_array[key_idx].state = LV_INDEV_STATE_REL;
#ifdef CONFIG_FP
            } else if ((KPC_KEY_EVENT_LONGKEY == p_kpc_key->key[key_idx].key_event) && (lv_key == LV_KEY_END)) {
                g_keypad_array[key_idx].state = LV_INDEV_STATE_REL;
                return 1;
#endif
            } else {
                aicx_error(0x00b471, "sd", "[%s]Current state(%d) no need to handle.\r\n", __func__, p_kpc_key->key[key_idx].key_event);
            }
            aicx_info(0x154764, "sddd", "[%s]Current key idx(%d), lv key is %d, state is %d.\r\n", __func__, key_idx, g_keypad_array[key_idx].lv_key, g_keypad_array[key_idx].state);
        }
    }
    return 0;
}


/**
* keypad_init - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to initialize keypad.
*/
void keypad_init(void)
{
    /*Your code comes here*/

}

/**
* keypad_register - Brief description of function.
* Parameter: [In]: None
*            [Out]: None
*            [Return]: None
* Description: This function is used to register keypad.
*/
void keypad_register(void)
{
    static lv_indev_drv_t keypad_indev_drv = {0};
    lv_group_t *p_global_group = NULL;

    lv_indev_drv_init(&keypad_indev_drv);
    keypad_indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    keypad_indev_drv.read_cb = keypad_read;
    s_indev_keypad_ptr = lv_indev_drv_register(&keypad_indev_drv);
    aicx_info(0x1b8b69, "s", "[%s] Call lv_indev_drv_register end.\r\n", __func__);
    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/
    p_global_group = lv_group_get_default();
    if (!p_global_group) {
        p_global_group = lv_group_create();
        lv_group_set_default(p_global_group);
    }
    lv_indev_set_group(s_indev_keypad_ptr, p_global_group);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void keypad_read(lv_indev_drv_t *p_indev_drv, lv_indev_data_t *p_data)
{
    uint8_t key_idx = 0;
    uint8_t key_data_num = 0;
    uint8_t key_valid_idx = 0;
    bool last_key_found = false;

    if (NULL == p_data) {
        aicx_error(0x012174, "s", "[%s]p_data is NULL.\r\n", __func__);
        return;
    }

    /* get the key data number in global array */
    for (key_idx = 0; key_idx < KPC_KEY_PRESSED_MAX_KEY_NUM; key_idx++) {
        if (0 != g_keypad_array[key_idx].lv_key) {
            key_valid_idx = key_idx;/* remember first valid key_idx */
            key_data_num++;
        }
    }

    /* if there is no key data, return directly. */
    if (0 == key_data_num) {
        return;
    } else {

        if (0 == lv_last_key) {/* no key pressed before */

            aicx_info(0x0b96d2, "sddd", "[%s]first valid key g_keypad_array[%d].lv_key = %d, state = %d.\r\n",
                      __func__,
                      key_valid_idx,
                      g_keypad_array[key_valid_idx].lv_key,
                      g_keypad_array[key_valid_idx].state);

            /* find the first valid key and report state */
            if (LV_INDEV_STATE_PR == g_keypad_array[key_valid_idx].state) {
                p_data->key = g_keypad_array[key_valid_idx].lv_key;
                p_data->state = LV_INDEV_STATE_PR;
                lv_last_key = p_data->key;
            } else {
                aicx_error(0x18a49c, "sd", "[%s] no key pressed before, but state comes released, ignore and reset key(%d).\r\n", __func__, g_keypad_array[key_valid_idx].lv_key);
                memset(&g_keypad_array[key_valid_idx], 0, sizeof(lv_keypad_info_t));
            }

        } else {/* last key pressed before */

            for (key_idx = 0; key_idx < KPC_KEY_PRESSED_MAX_KEY_NUM; key_idx++) {
                if ((0 != lv_last_key) && 
                    (lv_last_key == g_keypad_array[key_idx].lv_key)) {/* find the last key and report state */

                    aicx_info(0x0468b0, "sdd", "[%s] find lv_last_key (%d), state is %d.\r\n", __func__, lv_last_key, g_keypad_array[key_idx].state);
                    last_key_found = true;

                    if (LV_INDEV_STATE_PR == g_keypad_array[key_idx].state) {/* the last key still pressed */
                        p_data->key = g_keypad_array[key_idx].lv_key;
                        p_data->state = LV_INDEV_STATE_PR;

                    } else {/* the last key released */
                        p_data->key = g_keypad_array[key_idx].lv_key;
                        p_data->state = LV_INDEV_STATE_REL;
                        memset(&g_keypad_array[key_idx], 0, sizeof(lv_keypad_info_t));
                        lv_last_key = 0;
                    }
                } else if ((0 != g_keypad_array[key_idx].lv_key)
                            && (LV_INDEV_STATE_REL == g_keypad_array[key_idx].state)) {/* clear the other valid key if released */
                    aicx_info(0x03691b, "sd", "[%s] clear lv_key (%d).\r\n", __func__, g_keypad_array[key_idx].lv_key);
                    memset(&g_keypad_array[key_idx], 0, sizeof(lv_keypad_info_t));
                }
            }

            if (false == last_key_found) {/* can not find the last key, report released and reset last key */
                p_data->key = lv_last_key;
                p_data->state = LV_INDEV_STATE_REL;
                lv_last_key = 0;
                aicx_error(0x1a744e, "sd", "[%s]lv_last_key (%d) not find, report last key released.\r\n", __func__, lv_last_key);
            }
        }

        aicx_info(0x1d8bdd, "sddd", "[%s]key_data_num is %d, report lv_key (%d), state (%d).\r\n", __func__, key_data_num, p_data->key, p_data->state);

    }
}

