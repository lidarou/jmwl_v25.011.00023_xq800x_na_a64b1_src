/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10b4f5
#define LOG_MODULE    AMOD_KPC

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <slp_inc.h>
#include <aon_glb.h>
#include <kpc_cfg.h>
#include <irq.h>
#include <kpc.h>
#include "pwr_key.h"

#ifdef CONFIG_DRIVER_LCDC
#include "display.h"
#include <display_cfg.h>
#endif
#include <sysoff.h>
#include "kpc_phy.h"
#include "cru_top.h"
#ifdef CONFIG_DRIVER_PMIC_PMU
#include <pmu_user.h>
#include <pmu_driver.h>
#endif
#include <fmd.h>
#ifdef CONFIG_CAMERA_SUPPORT
#include "camera_hal.h"
#endif

#define KPC_BISR_THRD_NAME          "kpc_bisr_thrd"
#define KPC_BISR_THRD_STACK_SIZE    2048
#define KPC_BISR_THRD_QUEUE_SIZE    6
#define KPC_BISR_THRD_QUEUE_NAME    "kpc_bisr_thrd_queue"
#define KPC_BISR_THRD_PRI           29

#define KPC_MAIN_VERSION            1UL
#define KPC_SUB_VERSION             0UL

#define GET_ROW(x)                  (((x) >> 4) & 0x7)
#define GET_COL(x)                  (((x) & 0x7))

#define EXCHANGE(a,b)               {u16 c; c=a; a=b; b=c;}

struct kpc_msg {
    u16 key0_code;
    u16 key0_event;
    u16 key1_code;
    u16 key1_event;
    u16 key2_code;
    u16 key2_event;
    u16 key3_code;
    u16 key3_event;
};

aos_thread_id kpc_bisr_thrd_pid;

struct kpc_config *kpc_config_ptr;

struct kpc_msg g_kpc_msg;

kpc_key_array_t g_key_arr = {0};

u32 g_keyarr_count = 0;

static srv_func_callback g_kpc_callback = NULL;
static int32_t g_kpc_handle = 0;

/* work around for the (0,0) key error when deep sleep */
static bool g_is_special_key = FALSE;
struct kpc_msg g_kpc_msg_store = {0};/* for kpc blt at */

#ifdef CONFIG_KPC_DEMO_FUNC

boolean g_is_camera_open = FALSE;

#ifdef CONFIG_CAMERA_SUPPORT
#define CAMERA_PREVIEW_SIZE_IN_WORD        38400
extern u32 csi_rgb565_dst[CAMERA_PREVIEW_SIZE_IN_WORD];
#endif

#ifdef CONFIG_DRIVER_LCDC
static int kpc_demo_func_show_logo(void)
{
#ifdef CONFIG_CAMERA_SUPPORT
    camera_close(CAMERA_FRONT);
#endif
#ifdef CONFIG_DISPLAY_SHOWLOGO
    extern int show_logo();
    show_logo();
#endif
    return AIC_SUCCESS;
}

#endif

#ifdef CONFIG_CAMERA_SUPPORT

void kpc_preview_refresh(u32 frame_addr, int width, int height, camera_data_fmt_e format)
{
    enum SCL_MODE scl_mode;

    if ((width == LCD_PANEL_WIDTH) && (height == LCD_PANEL_HEIGHT))
        scl_mode = NO_SCALER;
    else if ((width / LCD_PANEL_WIDTH == 2) && (height / LCD_PANEL_HEIGHT == 2))
        scl_mode = BINNING;
    else
        scl_mode = BILINEAR_SCALER;

    /* convert to rgb565 */
    yuv422_to_rgb565((unsigned char*)frame_addr, AIC_NULL,
                     width, height,
                     (unsigned short*)csi_rgb565_dst,
                     LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT,
                     MODE_YUYV, scl_mode,
                     0);

    display_refresh_sync((u32)csi_rgb565_dst);
}

static int kpc_demo_func_camera_preview(void)
{
    int ret = -1;

    ret = camera_open(CAMERA_FRONT);

    if (ret != AIC_SUCCESS) {
        aicx_info(0x0c0d4e, "", "camera open failed!\n");
        return ret;
    }

   /* TODO:Use Camera service level API, no need care refresh logic */
   camera_register(kpc_preview_refresh);

    ret = camera_preview(CAMERA_FRONT, YUV_ONE_PLANE, YUV422, CAMERA_SENSOR_OUTPUT_WIDTH, CAMERA_SENSOR_OUTPUT_HEIGHT);
    if (ret != AIC_SUCCESS) {
        aicx_info(0x0a4521, "", "camera preview failed!\n");
        return ret;
    }

    return AIC_SUCCESS;
}
#endif
#endif


int32_t aic_srv_kpc_register(char *client_name, srv_func_callback _srv_callback)
{
    g_kpc_callback = _srv_callback;
    g_kpc_handle++;
    aicx_info(0x0202e8, "d", "g_kpc_handle = %d \n", g_kpc_handle);
    return g_kpc_handle;
}

int32_t aic_srv_kpc_unregister(int32_t client_handle)
{
    if (client_handle == g_kpc_handle) {
        g_kpc_callback = NULL;
        g_kpc_handle = 0;
    }
    else {
        return -1;
    }
    return 0;
}

/*TODO: Move this api to another source file*/
/**
* kpc_read_key - Brief description of function.
* Parameter: [In]: None
*            [Out]: p_key_arr: the point of the key array
*            [Return]: int: the valid key number
* Description: This function is used to register keypad.
*/
int kpc_read_key(kpc_key_array_t * p_key_arr)
{
    u32 key_count = 0;

    /* read key arr only once if key is valid */
    if(g_keyarr_count != 0)
    {
        aos_memcpy(p_key_arr,&g_key_arr,sizeof(kpc_key_array_t));
        key_count = g_keyarr_count;
    }

    aos_memset(&g_key_arr,0,sizeof(kpc_key_array_t));
    g_keyarr_count = 0;

    return key_count;
}

static int kpc_write_key(struct kpc_msg key_msg)
{
    u16 row, col = 0;

    /* TODO:
        1. transfer keyevent
        2. create key ring buffer and add the key read/write operation manage logic */
    memset(&g_key_arr, 0, sizeof(kpc_key_array_t));
    g_keyarr_count = 0;

    if(key_msg.key0_event != 0)
    {
        /* get key matrix */
        row = GET_ROW(key_msg.key0_code);
        col = GET_COL(key_msg.key0_code);
        if((KPC_KEY_ROW_NUM > row) && (KPC_KEY_COL_NUM > col))
        {
            /* get key value*/
            g_key_arr.key[0].key_value = kpc_key_map[row][col];
            g_key_arr.key[0].key_event = key_msg.key0_event;

            aicx_info(0x0e1bfb, "dddd", "row = %d, col = %d, key[0].key_value = %d, key[0].key_event = %d \n",row,col,g_key_arr.key[0].key_value,g_key_arr.key[0].key_event);

            g_keyarr_count ++;
        }
    }

    if(key_msg.key1_event != 0)
    {
        row = GET_ROW(key_msg.key1_code);
        col = GET_COL(key_msg.key1_code);
        if((KPC_KEY_ROW_NUM > row) && (KPC_KEY_COL_NUM > col))
        {
            g_key_arr.key[1].key_value = kpc_key_map[row][col];
            g_key_arr.key[1].key_event = key_msg.key1_event;

            /* TODO: modify the key index logic */
            /* work around for the (0,0) key error when deep sleep */
            /* identify the special key pressed */
            if (((KPC_KEY_EVENT_RELEASE == g_kpc_msg_store.key0_event) || (0 == g_kpc_msg_store.key0_event))
                && (KPC_KEY_EVENT_PRESS == key_msg.key1_event)) {
                g_is_special_key = TRUE;

                /* exchange key[0] and key[1] value */
                EXCHANGE( g_key_arr.key[0].key_value,g_key_arr.key[1].key_value)
                EXCHANGE( g_key_arr.key[0].key_event,g_key_arr.key[1].key_event)

                aicx_info(0x134f5d, "dd", "special pressed,key[0].key_value = %d, key[0].key_event = %d \n", row,col,g_key_arr.key[0].key_value,g_key_arr.key[0].key_event);

            }

            /* identify the special key released */
            if ((KPC_KEY_EVENT_RELEASE == key_msg.key1_event) 
                && (TRUE == g_is_special_key)) {
                g_is_special_key = FALSE;

                /* exchange key[0] and key[1] value */
                EXCHANGE( g_key_arr.key[0].key_value,g_key_arr.key[1].key_value)
                EXCHANGE( g_key_arr.key[0].key_event,g_key_arr.key[1].key_event)

                aicx_info(0x1c3d5a, "dd", "special released,key[0].key_value = %d, key[0].key_event = %d \n", row,col,g_key_arr.key[0].key_value,g_key_arr.key[0].key_event);

            }

            aicx_info(0x017e7f, "dddd", "row = %d, col = %d,key[1].key_value = %d, key[1].key_event = %d \n", row,col,g_key_arr.key[1].key_value,g_key_arr.key[1].key_event);

            g_keyarr_count ++;
        }
    }

    if(key_msg.key2_event != 0)
    {
        row = GET_ROW(key_msg.key2_code);
        col = GET_COL(key_msg.key2_code);
        if((KPC_KEY_ROW_NUM > row) && (KPC_KEY_COL_NUM > col))
        {
            g_key_arr.key[2].key_value = kpc_key_map[row][col];
            g_key_arr.key[2].key_event = key_msg.key2_event;

            aicx_info(0x1d4ff1, "dddd", "row = %d, col = %d,key[2].key_value = %d, key[2].key_event = %d \n", row,col,g_key_arr.key[2].key_value,g_key_arr.key[2].key_event);

            g_keyarr_count ++;
        }
    }

    if(key_msg.key3_event != 0)
    {
        row = GET_ROW(key_msg.key3_code);
        col = GET_COL(key_msg.key3_code);
        if((KPC_KEY_ROW_NUM > row) && (KPC_KEY_COL_NUM > col))
        {
            g_key_arr.key[3].key_value = kpc_key_map[row][col];
            g_key_arr.key[3].key_event = key_msg.key3_event;

            aicx_info(0x1b6ef7, "dddd", "row = %d, col = %d,key[3].key_value = %d, key[3].key_event = %d \n", row,col,g_key_arr.key[3].key_value,g_key_arr.key[3].key_event);

            g_keyarr_count ++;
        }
    }

    return AIC_SUCCESS;
}


#ifdef CONFIG_KPC_DEMO_FUNC
static int kpc_handle_key(void)
{
    u16 key_value = 0;
    u16 key_event = 0;

    /*In 4*4, no em and UI, this function use to excute key demo function,
    here only dispach press event and key[0].key_value*/

    key_value = g_key_arr.key[0].key_value;
    key_event = g_key_arr.key[0].key_event;

    if (KPC_KEY_EVENT_PRESS != key_event) {
        return -1;
    }

    switch (key_value) {
    case KEY_DEL:
#ifdef CONFIG_DRIVER_LCDC
        kpc_demo_func_show_logo();
#else
        aicx_info(0x06a441, "", "can't support show logo! \n");
#endif

        break;

    case KEY_BACK:
#ifdef CONFIG_CAMERA_SUPPORT
        kpc_demo_func_camera_preview();
#else
        aicx_info(0x044157, "", "can't support camera preview! \n");
#endif
        break;

    case KEY_UP:
        {
            aicx_info(0x19834e, "", "sysoff shutdown!\n");
            sysoff_shutdown();
        }
        break;

    /*TODO: add demo function here*/
    default:
        break;
    }

    return AIC_SUCCESS;
}
#endif





static void kpc_phy_get_version(struct ver_info *v_info)
{
    u32 val;

    val = readl_relaxed((u32 *)REG_KPC_KPC_IP_INFO);
    v_info->main_ver = (val & BIT_KPC_IP_MAIN_VERSION(0xff)) >> 8;
    v_info->sub_ver = (val & BIT_KPC_IP_SUB_VERSION(0xff));
}

static void kpc_phy_longpress_en(boolean enable)
{
    if (enable)
        writel_relaxed((u32 *)REG_KPC_KPC_GLB_CTRL_SET, BIT_KPC_KPC_LONGKEY_EN);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_GLB_CTRL_CLR, BIT_KPC_KPC_LONGKEY_EN);
}

static void kpc_phy_auto_slp_en(boolean enable)
{
    if (enable)
        writel_relaxed((u32 *)REG_KPC_KPC_GLB_CTRL_SET, BIT_KPC_KPC_AUTO_SLP_EN);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_GLB_CTRL_CLR, BIT_KPC_KPC_AUTO_SLP_EN);
}

static void kpc_phy_enable(boolean enable)
{
    if (enable)
        writel_relaxed((u32 *)REG_KPC_KPC_GLB_CTRL_SET, BIT_KPC_KPC_EN);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_GLB_CTRL_CLR, BIT_KPC_KPC_EN);
}

static void kpc_phy_row_config(u32 rows)
{
    u32 val;

    val = readl_relaxed((u32 *)REG_KPC_KPC_MTX_EN);
    val &= ~BIT_KPC_KPC_ROW_EN(0xff);
    val |= ((rows << 8) & BIT_KPC_KPC_ROW_EN(0xff));

    writel_relaxed((u32 *)REG_KPC_KPC_MTX_EN, val);
}

static void kpc_phy_col_config(u32 cols)
{
    u32 val;

    val = readl_relaxed((u32 *)REG_KPC_KPC_MTX_EN);
    val &= ~BIT_KPC_KPC_COL_EN(0xff);
    val |= (cols & BIT_KPC_KPC_COL_EN(0xff));

    writel_relaxed((u32 *)REG_KPC_KPC_MTX_EN, val);
}

static void kpc_phy_ghostkey_int_enable(boolean enable)
{
    if (enable)
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_SET, BIT_KPC_KPC_GHOSTKEY_INT_EN);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_CLR, BIT_KPC_KPC_GHOSTKEY_INT_EN);
}

static void kpc_phy_longpress_int_enable(boolean enable)
{
    if (enable)
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_SET, BIT_KPC_KPC_LONG_INT_EN);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_CLR, BIT_KPC_KPC_LONG_INT_EN);
}

static void kpc_phy_release_int_enable(boolean enable)
{
    if (enable)
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_SET, BIT_KPC_KPC_RELEASE_INT_EN);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_CLR, BIT_KPC_KPC_RELEASE_INT_EN);
}

static void kpc_phy_press_int_enable(boolean enable)
{
    if (enable)
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_SET, BIT_KPC_KPC_PRESS_INT_EN);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_INT_EN_CLR, BIT_KPC_KPC_PRESS_INT_EN);
}

static void kpc_phy_polarity(boolean level)
{
    if (level)
        writel_relaxed((u32 *)REG_KPC_KPC_POLARITY_SET, BIT_KPC_KPC_ROW_COL_POL);
    else
        writel_relaxed((u32 *)REG_KPC_KPC_POLARITY_CLR, BIT_KPC_KPC_ROW_COL_POL);
}

static void kpc_phy_longkey_cnt(u32 count)
{
    writel_relaxed((u32 *)REG_KPC_KPC_LONGKEY_CNT, (count & BIT_KPC_KPC_LONGKEY_CNT(0xffff)));
}

static void kpc_phy_auto_slp_cnt(u32 count)
{
    writel_relaxed((u32 *)REG_KPC_KPC_AUTO_SLP_CNT, (count & BIT_KPC_KPC_AUTO_SLP_CNT(~0)));
}

static void kpc_phy_clk_div_cnt(u32 div)
{
    writel_relaxed((u32 *)REG_KPC_KPC_CLK_DIV_CNT, (div & BIT_KPC_KPC_CLK_DIV_CNT(0xffff)));
}

static void kpc_phy_clk_select(boolean level)
{
    /*  TRUE: select 32k;
        FALSE: select 26M */

    if (level)
        writel_relaxed((u32 *)REG_CRU_TOP_CLK_AON_KPC_CFG_SET, BIT_CRU_TOP_SEL_CLK_AON_KPC_RF);
    else
        writel_relaxed((u32 *)REG_CRU_TOP_CLK_AON_KPC_CFG_CLR, BIT_CRU_TOP_SEL_CLK_AON_KPC_RF);
}

static void kpc_phy_debounce_cnt(u32 press_debounce_cnt, u32 release_debounce_cnt)
{
    u32 val;

    val = press_debounce_cnt & BIT_KPC_KPC_DEBOUNCE_CNT_PRESS(0xffff);
    val |= (release_debounce_cnt << 16) & BIT_KPC_KPC_DEBOUNCE_CNT_RELEASE(0xffff);

    writel_relaxed((u32 *)REG_KPC_KPC_DEBOUNCE_CNT, val);
}

static void kpc_phy_int_clr_all(void)
{
    writel_relaxed((u32 *)REG_KPC_KPC_INT_CLR,
                  (BIT_KPC_KPC_GHOSTKEY_INT_STATUS_CLR |
                   BIT_KPC_KEY3_INT_STATUS_CLR(0x7) | BIT_KPC_KEY2_INT_STATUS_CLR(0x7) |
                   BIT_KPC_KEY1_INT_STATUS_CLR(0x7) | BIT_KPC_KEY0_INT_STATUS_CLR(0x7)));
}

static int kpc_get_kpc_msg_from_status(void)
{
    u32 val, mtx_val;
    u32 status;

    memset(&g_kpc_msg, 0, sizeof(struct kpc_msg));
    val = readl_relaxed((u32 *)REG_KPC_KPC_KEY_INT_STATUS);
    if (val == 0)
        return AIC_SUCCESS;

    mtx_val = readl_relaxed((u32 *)REG_KPC_KPC_KEY_MATRIX);

    if (val & BIT_KPC_KEY0_INT_STATUS(0x7)) {

        g_kpc_msg.key0_code = mtx_val & 0xFF;
        g_kpc_msg_store.key0_code = mtx_val & 0xFF;

        status = val & BIT_KPC_KEY0_INT_STATUS(0x7);
        g_kpc_msg.key0_event = status;
        g_kpc_msg_store.key0_event = status;
    }

    if (val & BIT_KPC_KEY1_INT_STATUS(0x7)) {

        g_kpc_msg.key1_code = (mtx_val >> 8) & 0xFF;
        g_kpc_msg_store.key1_code = (mtx_val >> 8) & 0xFF;

        status = (val >> 4) & BIT_KPC_KEY0_INT_STATUS(0x7);
        g_kpc_msg.key1_event = status;
        g_kpc_msg_store.key1_event = status;
    }

    if (val & BIT_KPC_KEY2_INT_STATUS(0x7)) {

        g_kpc_msg.key2_code = (mtx_val >> 16) & 0xFF;
        g_kpc_msg_store.key2_code = (mtx_val >> 16) & 0xFF;

        status = (val >> 8) & BIT_KPC_KEY0_INT_STATUS(0x7);
        g_kpc_msg.key2_event = status;
        g_kpc_msg_store.key2_event = status;
    }

    if (val & BIT_KPC_KEY3_INT_STATUS(0x7)) {

        g_kpc_msg.key3_code = (mtx_val >> 24) & 0xFF;
        g_kpc_msg_store.key3_code = (mtx_val >> 24) & 0xFF;

        status = (val >> 12) & BIT_KPC_KEY0_INT_STATUS(0x7);
        g_kpc_msg.key3_event = status;
        g_kpc_msg_store.key3_event = status;
    }

    /*for (i = 0; i < 4; i++)
        if ((val >> (i * 4)) & KEY0_INT_STATUS) {
            row = (val >> (i * 8 + 4)) & KPC_KEY0_COL_NUM;
            col = (val >> (i * 8)) & KPC_KEY0_COL_NUM;
            g_kpc_msg.key_code = MATRIX_SCAN_CODE(row, col, 8);

            status = (val >> (i * 4)) & KEY0_INT_STATUS;
            g_kpc_msg.key_event = (u16) status;
        }*/

    return AIC_SUCCESS;
}

void kpc_send_msg_thread(void *param)
{
    aicx_info(0x04b21d, "s", "Key_code [%s]\n", __func__);
    //alog_info("key0_code = [0x%x], key0_event = %d \n", g_kpc_msg.key0_code,g_kpc_msg.key0_event);
    //alog_info("key1_code = [0x%x], key1_event = %d \n", g_kpc_msg.key1_code,g_kpc_msg.key1_event);
    //alog_info("key2_code = [0x%x], key2_event = %d \n", g_kpc_msg.key2_code,g_kpc_msg.key2_event);
    //alog_info("key3_code = [0x%x], key3_event = %d \n", g_kpc_msg.key3_code,g_kpc_msg.key3_event);

    /* transfer key matrix to key value and Write key buffer */
    kpc_write_key(g_kpc_msg);

    /*  dispatch key to registed client */
    if (g_kpc_callback) {
        int32_t ret = 0;
        ret = g_kpc_callback(&g_key_arr, sizeof(kpc_key_array_t));
        aicx_info(0x05ca50, "d", "g_kpc_callback ret is %d\n", ret);
    }

#ifdef CONFIG_KPC_DEMO_FUNC
    kpc_handle_key();
#endif
}

static ISR_RET_E kpc_phy_isr_handler(void *param)
{
    u32 val;

    val = readl_relaxed((u32 *)REG_KPC_KPC_KEY_INT_STATUS);
    if (val & BIT_KPC_KPC_GHOSTKEY_INT_STATUS) {
        /* CLear all interrupts */
        kpc_phy_int_clr_all();
        return NONE_WORKER;
    }

    kpc_get_kpc_msg_from_status();
    kpc_phy_int_clr_all();

    return CALL_LWORKER;
}

static void kpc_slp_resume(void *ptr)
{
    /* select 32k when sleep resume */
    kpc_phy_enable(FALSE);
    kpc_phy_clk_select(TRUE);
    kpc_phy_enable(TRUE);
}
#ifdef CONFIG_PWR_KEY_LONG_PRESS_SHUTDOWN

static int32_t kpc_pwr_key_handle(void *para, uint32_t size)
{
    int ret = 0;
    int pwr_status = 0;

    memcpy(&pwr_status, para, size);
    memset(&g_key_arr, 0, sizeof(kpc_key_array_t));

    switch (pwr_status) {
    case AIC_PWR_KEYDOWN:
        g_key_arr.key[4].key_value = KEY_POWER;
        g_key_arr.key[4].key_event = KPC_KEY_EVENT_PRESS;
        break;

    case AIC_PWR_KEYUP:
        g_key_arr.key[4].key_value = KEY_POWER;
        g_key_arr.key[4].key_event = KPC_KEY_EVENT_RELEASE;
        break;
#ifdef CONFIG_FP
    case AIC_PWR_KEYLONG:
        g_key_arr.key[4].key_value = KEY_POWER;
        g_key_arr.key[4].key_event = KPC_KEY_EVENT_LONGKEY;
        break;
#endif

    default:
        aicx_error(0x0ebbfc, "d", "kpc_pwr_key_handle error pwr_status is %d\n", pwr_status);
        return -1;
        break;
    }

    /* dispatch power key event */
    if (g_kpc_callback) {
        int32_t ret = 0;
        ret = g_kpc_callback(&g_key_arr, sizeof(kpc_key_array_t));
        aicx_info(0x05ca50, "d", "g_kpc_callback ret is %d\n", ret);
    }
    return ret;
}
#endif

int kpc_init(void)
{
    struct ver_info v_info;
    u32 kpc_slp_handler = 0;

    kpc_config_ptr = &kpc_config;

    /* enable top */
    writel_relaxed((void *)REG_AON_GLB_KPC_EB_SET, BIT_AON_GLB_KPC_EB);

    kpc_phy_get_version(&v_info);
    if ((v_info.main_ver != KPC_MAIN_VERSION) || (v_info.sub_ver != KPC_SUB_VERSION))
        return AIC_ENXIO;

    kpc_phy_enable(FALSE);

    kpc_phy_clk_div_cnt(0);
    kpc_phy_clk_select(TRUE);

    /* int config */
    kpc_phy_press_int_enable(TRUE);
    kpc_phy_release_int_enable(TRUE);
    kpc_phy_longpress_int_enable(kpc_config_ptr->longkey_en);
    kpc_phy_ghostkey_int_enable(kpc_config_ptr->need_ghost_filter);

    /* function configure */
    kpc_phy_auto_slp_en(kpc_config_ptr->auto_sleep);
    kpc_phy_longpress_en(kpc_config_ptr->longkey_en);

    /* matrix config */
    kpc_phy_row_config(kpc_config_ptr->rows);
    kpc_phy_col_config(kpc_config_ptr->cols);

    /* polarity */
    kpc_phy_polarity(kpc_config_ptr->level);
    /* tmp clear for fpga cannot disable kpc clock */
    kpc_phy_int_clr_all();

    /* longkey cnt*/
    kpc_phy_longkey_cnt(kpc_config_ptr->long_press_cnt);

    /* auto sleep cnt */
    kpc_phy_auto_slp_cnt(kpc_config_ptr->auto_slp_cnt);

    /* debounce */
    kpc_phy_debounce_cnt(kpc_config_ptr->press_debounce_cnt,
                         kpc_config_ptr->release_debounce_cnt);

    /* register isr */
    register_irq(IRQ_KPD, (UISR_FUNC)&kpc_phy_isr_handler, NULL, kpc_send_msg_thread, NULL);

    /* TODO: add task for APPs which need callback interface.
    kpc_bisr_thrd_pid = aos_create_thread(KPC_BISR_THRD_NAME, &kpc_thread_entry, NULL,
                      KPC_BISR_THRD_STACK_SIZE, KPC_BISR_THRD_QUEUE_SIZE,
                      KPC_BISR_THRD_QUEUE_NAME,KPC_BISR_THRD_PRI,
                      AOS_AUTO_START);
    */

    kpc_phy_press_int_enable(TRUE);
    kpc_phy_release_int_enable(TRUE);
    kpc_phy_enable(TRUE);

    aicx_info(0x180d66, "s", "The kpc model has been initialled in [%s]\n", __func__);

    kpc_slp_handler = slp_register_sleep_hook(NULL, NULL,kpc_slp_resume, NULL);
    if (0 == kpc_slp_handler) {
        aicx_info(0x142ac6, "", "kpc_slp_handler = 0, register fail \n");
    }

#ifdef CONFIG_PWR_KEY_LONG_PRESS_SHUTDOWN
    register_pwr_key_monitor("kpc", kpc_pwr_key_handle);
#endif
    return AIC_SUCCESS;
}

AIC_DRV_P1_INIT_EXPORT(kpc_init);

