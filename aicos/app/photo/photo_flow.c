/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of photo browser app
 */

/*********************
 *      INCLUDES
 *********************/
#include "photo_service.h"
#include "photo_flow.h"
#include "photo_converter.h"
#include "../../util/ui_helpers.h"
#include "aic_osa_ex.h"
#include <stdio.h>

/*********************
 *      DEFINES
 *********************/

/* message for photo delete */
#define PHOTO_DEL_PROMPT "are you sure to delete ?"
#define PHOTO_DEL_ALL_PROMPT "are you sure to delete all ?"
#define PHOTO_DEL_APPLY 0
#define PHOTO_DEL_CANCEL 1

/* zoom in/restore */
#define PHOTO_ZOOM_IN 0
#define PHOTO_RESTORE 1
#define PHOTO_SIZE_DOUBLE 512
#define PHOTO_SIZE_RESTORE 256

/* rotation angle */
#define PHOTO_ROTATE_ANGLE 900

/* text length for phone size show */
#define PHOTO_SIZE_LEN 32

/* 3*3 matrix */
#define PHOTO_MTX_X 1
#define PHOTO_MTX_Y 4
#define PHOTO_MTX_SIZE (PHOTO_MTX_X * PHOTO_MTX_Y)
#define PHOTO_MTX_MAP_SIZE (PHOTO_MTX_SIZE + PHOTO_MTX_Y)

/* photo play timer interval (ms) */
#define PHOTO_PLAY_INTERVAL 2000

/* photo button disappear interval */
#define PHOTO_BTNDIS_INTERVAL 5000

#define PHOTO_MTX_X_RIGHT(x)                                                   \
    do {                                                                       \
        x = x + 1;                                                             \
        if (x > PHOTO_MTX_X)                                                   \
            x = 1;                                                             \
    } while (0);

#define PHOTO_MTX_X_LEFT(x)                                                    \
    do {                                                                       \
        x = x - 1;                                                             \
        if (!x)                                                                \
            x = PHOTO_MTX_X;                                                   \
    } while (0);

#define PHOTO_MTX_Y_UP(y)                                                      \
    do {                                                                       \
        y = y - 1;                                                             \
        if (!y)                                                                \
            y = 1;                                                             \
    } while (0);

#define PHOTO_MTX_Y_DOWN(y)                                                    \
    do {                                                                       \
        y = y + 1;                                                             \
        if (y > PHOTO_MTX_Y)                                                   \
            y = PHOTO_MTX_Y;                                                   \
    } while (0);

/**********************
 *      TYPEDEFS
 **********************/
typedef enum photo_browse_type {
    PHOTO_BROWSE_MTX_9,
    PHOTO_BROWSE_LIST,
} photo_browse_type_e;

typedef enum photo_preview_state {
    PHOTO_PREVIEW,
    PHOTO_PREVIEW_PLAY,
} photo_preview_state_e;

struct photo_obj_lvgl {
    lv_obj_t *img;
    lv_obj_t *label;
    lv_obj_t *btnmtx;
    lv_obj_t *panel;
    lv_timer_t *timer;
    lv_timer_t *btntimer;
    lv_coord_t scroll_h;
    lv_img_dsc_t img_dsc;
};

struct photo_info {
    struct photo_obj *img_obj;
    struct photo_obj_lvgl lv_obj;
    photo_browse_type_e type;
    photo_preview_state_e state;
    u32 index;
    u32 total;
};

struct photo_matrix {
    char **map;
    u32 map_size;
    u16 lines;
    u8 x;
    u8 y;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void photo_display(const char *src);
static void photo_matrix_update(lv_obj_t *obj, lv_event_t *e);
static void photo_thumbnail_update(lv_obj_t *obj, lv_event_t *e);
static void photo_matrix_last_page_update(lv_obj_t *obj, int full_page);
static void photo_main_page_init(photo_browse_type_e type);
static void photo_preview_page_init(photo_browse_type_e type);

/**********************
 *  STATIC VARIABLES
 **********************/

extern lv_obj_t *ui_photo_preview;
extern lv_obj_t *ui_photo_main;
extern lv_obj_t *ui_photo_preview_delete_btn;
extern lv_obj_t *ui_photo_preview_return_btn;
extern lv_obj_t *ui_photo_preview_play_btn;
extern lv_obj_t *ui_photo_preview_btn;
extern lv_obj_t * ui_photo_delete_btn;

static struct photo_info g_photo_info = {0};
static struct photo_matrix g_photo_mtx = {0};

/**********************
 *   STATIC FUNCTIONS
 **********************/
static int photo_get_image_dsc(lv_img_dsc_t *img_dsc, char *img_src)
{
    struct photo_attr attr;
    u8 *rgb888_data = 0;
    u16 *rgb565_data = 0;
    u32 rgb888_size = 0;
    u32 rgb565_size = 0;

    if (!img_dsc || !img_src) {
        app_loge("%s: invalid param\n", __func__);
        return -1;
    }

    rgb888_data = photo_read_jpeg(img_src, &attr, &rgb888_size);
    if (!rgb888_data) {
        app_loge("%s: photo decode error\n", __func__);
        return -1;
    }

    app_logi("===>name:%s, w: %d, h:%d\n",
             img_src, attr.width, attr.height);

    rgb565_size = rgb888_size * 2 / 3 + 3;
    rgb565_data = aic_malloc(rgb565_size);
    if (!rgb565_data) {
        aic_free(rgb888_data);
        app_loge("%s: no more memory\n", __func__);
        return -1;
    }

    rgb565_size = photo_rgb888_to_rgb565(rgb888_data, rgb888_size,
                                         rgb565_data, rgb565_size);
    if (!rgb565_size) {
        aic_free(rgb565_data);
        aic_free(rgb888_data);
        app_loge("%s: rgb888 to rgb565 fail\n", __func__);
        return -1;
    }

    aic_free(rgb888_data);

    img_dsc->header.cf = LV_IMG_CF_TRUE_COLOR;
    img_dsc->header.always_zero = 0;
    img_dsc->header.reserved = 0;
    img_dsc->header.w = attr.width;
    img_dsc->header.h = attr.height;
    img_dsc->data_size = rgb565_size * 2;
    img_dsc->data = (u8 *)rgb565_data;

    return 0;
}

static void photo_preview_dis_timer_cb(lv_timer_t *timer)
{
    struct photo_info *info = &g_photo_info;
    lv_obj_t *label = info->lv_obj.label;

    if (label)
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_flag(ui_photo_preview_delete_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_photo_preview_return_btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_photo_preview_play_btn, LV_OBJ_FLAG_HIDDEN);

    lv_timer_del(timer);

    info->lv_obj.btntimer = NULL;
}

static void photo_preview_dis_timer_start(void)
{
    struct photo_info *info = &g_photo_info;
    lv_timer_t *timer = NULL;

    timer = lv_timer_create(photo_preview_dis_timer_cb,
                            PHOTO_BTNDIS_INTERVAL, 0);
    if (!timer) {
        app_loge("%s: lv timer create fail\n", __func__);
        return;
    }

    info->lv_obj.btntimer = timer;
}

static void photo_display(const char *src)
{
    struct photo_info *info = &g_photo_info;
    lv_obj_t *img = info->lv_obj.img;
    lv_obj_t *label = info->lv_obj.label;
    char img_src[PHOTO_PATH_NAME_LEN];

    if (!img || !label) {
        app_loge("%s: lvgl obj not init.\n", __func__);
        return;
    }

    memset(img_src, '\0', sizeof(img_src));
    strcpy(img_src, PHOTO_DISK_LETTER);
    strcat(img_src, PHOTO_DIR);
    strcat(img_src, src);

    if (photo_get_image_dsc(&info->lv_obj.img_dsc, img_src)) {
        app_loge("%s: get image dsc failed\n", __func__);
        return;
    }

    lv_img_set_src(img, &info->lv_obj.img_dsc);

    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    lv_label_set_text_fmt(label, "%u/%u", info->index, info->total);

    lv_obj_align_to(label, img, LV_ALIGN_CENTER, 90, -140);
    lv_obj_move_background(img);

    aic_free(info->lv_obj.img_dsc.data);

    if (info->type == PHOTO_BROWSE_LIST)
        photo_preview_dis_timer_start();
}

static void photo_btnmatrix_init(lv_obj_t *obj, lv_event_t *e)
{
    lv_obj_draw_part_dsc_t *dsc = NULL;

    dsc = lv_event_get_draw_part_dsc(e);

    if (dsc && dsc->class_p == &lv_btnmatrix_class &&
        dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
        dsc->rect_dsc->radius = 0;
        dsc->rect_dsc->border_opa = LV_OPA_TRANSP;
        if (g_photo_info.type == PHOTO_BROWSE_LIST) {
            dsc->rect_dsc->outline_opa = LV_OPA_TRANSP;

            if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
                dsc->rect_dsc->bg_color = lv_palette_darken(LV_PALETTE_ORANGE, 3);
            else
                dsc->rect_dsc->bg_color = lv_palette_main(LV_PALETTE_BLUE_GREY);
        } else {
            if (lv_btnmatrix_get_selected_btn(obj) == dsc->id) {
                dsc->rect_dsc->outline_color = lv_color_make(0xFF, 0xA5, 0x00);
                dsc->rect_dsc->outline_width = 5;
            } else {
                dsc->rect_dsc->outline_opa = LV_OPA_TRANSP;
            }
        }
    }

#ifdef PHOTO_BROSWE_MATRIX_SIMPLE
    int img_total = g_photo_info.total;
    int mtx_total = PHOTO_MTX_SIZE;
    int mtx_page_num = 0;
    int mtx_last_page = 0;
    struct photo_matrix *mtx = &g_photo_mtx;
    mtx_page_num = img_total / mtx_total;
    mtx_last_page = img_total % mtx_total;

    if (mtx_page_num == 0) {
        int line = 0;

        /* number of pix is less than a page */
        for (int i = mtx_last_page; i < mtx_total; i ++) {
            lv_btnmatrix_set_btn_ctrl(obj, i,
                                      LV_BTNMATRIX_CTRL_HIDDEN);
        }

        line = mtx_last_page / PHOTO_MTX_X;
        if (line == 0)
            mtx->lines = line;
        else
            mtx->lines = line + 1;

        /* for debugging */
        if (mtx_last_page == 0) {
            app_loge("%s: should not occur!\n", __func__);
        }
    } else {
        mtx->lines = PHOTO_MTX_Y;
    }
#endif /* !PHOTO_BROSWE_MATRIX_SIMPLE */
}

static void photo_matrix_last_page_update(lv_obj_t *obj, int full_page)
{
    struct photo_matrix *mtx = &g_photo_mtx;
    int total = g_photo_info.total;
    int items_of_last_line = 0;
    int last_btn_id = 0;

    items_of_last_line = total % PHOTO_MTX_X;
    if (!full_page) {
        if (items_of_last_line == 0) {
            /**
             * number of items in last line is equal to
             * PHOTO_MTX_X.
             */
            last_btn_id = mtx->lines * PHOTO_MTX_X;
        } else {
            last_btn_id =
                items_of_last_line + ((mtx->lines - 1) * PHOTO_MTX_X);
        }
    } else {
        /* lines == PHOTO_MTX_Y */
        if (items_of_last_line != 0) {
            last_btn_id = items_of_last_line;
        } else {
            /* full page, donothing */
            return;
        }
    }

    /* update matrix buttons */
    for (int i = last_btn_id; i < total; i++) {
        lv_btnmatrix_set_btn_ctrl(obj, i, LV_BTNMATRIX_CTRL_HIDDEN);
    }
}

static void photo_matrix_update(lv_obj_t *obj, lv_event_t *e)
{
    struct photo_matrix *mtx = &g_photo_mtx;
    struct photo_info *info = &g_photo_info;
    char c = *((char *)lv_event_get_param(e));
    int img_total = info->total;
    int mtx_total = PHOTO_MTX_SIZE;
    int index = info->index;
    int rows, cur_row;

    switch (c) {
    case LV_KEY_UP:
        if (index <= PHOTO_MTX_X) {
            /* reach first line, donothing */
            return;
        }

        index = index - PHOTO_MTX_X;

        if ((img_total - index) < mtx_total) {
            /* last page */
            if (mtx->lines < PHOTO_MTX_Y) {
                /**
                 * Turned directly to the last page,
                 * less than PHOTO_MTX_Y lines.
                 * Maybe only one line is left after
                 * turning the page.
                 */
                photo_matrix_last_page_update(obj, 0);

                if (mtx->y == 1)
                    mtx->lines++;
            } else {
                /* lines == PHOTO_MTX_Y */
                photo_matrix_last_page_update(obj, 1);
            }
        }

        if (mtx->y == 1)
            app_srv_photo_group_update(index, PHOTO_MTX_X, PHOTO_TURN_FORWARD);

        PHOTO_MTX_Y_UP(mtx->y);
        break;
    case LV_KEY_DOWN:
        rows = img_total / PHOTO_MTX_X;
        if ((img_total % PHOTO_MTX_X) != 0) {
            rows = rows + 1;
        }
        cur_row = index / PHOTO_MTX_X;
        if ((index % PHOTO_MTX_X) != 0) {
            cur_row = cur_row + 1;
        }
        if (cur_row == rows) {
            /* last line */
            return;
        }

        index = index + PHOTO_MTX_X;
        if (index > img_total) {
            /* reach last line */
            mtx->x = mtx->x - (index - img_total);
            index = img_total;
        }

        if ((img_total - index) < mtx_total) {
            /* last page */
            if ((mtx->lines > 1) && (mtx->lines < PHOTO_MTX_Y)) {
                /**
                 * Turned directly to the last page, less than
                 * PHOTO_MTX_Y  lines. If there is only one line
                 * left after turning the page, you cannot key
                 * down.
                 */
                photo_matrix_last_page_update(obj, 0);
            } else {
                /* lines == PHOTO_MTX_Y */
                photo_matrix_last_page_update(obj, 1);
            }
        }
        if (mtx->y == 3)
            app_srv_photo_group_update(index, PHOTO_MTX_X, PHOTO_TURN_BACKWARD);
        PHOTO_MTX_Y_DOWN(mtx->y);
        break;
    case LV_KEY_LEFT:
        if (index == 1) {
            /* first item */
            lv_btnmatrix_set_selected_btn(obj, 0);
            return;
        }

        index = index - 1;

        if ((img_total - index) < mtx_total) {
            /* last page */
            if (mtx->lines < PHOTO_MTX_Y) {
                /**
                 * Turned directly to the last page, less than PHOTO_MTX_Y
                 * lines.
                 */
                photo_matrix_last_page_update(obj, 0);

                if (mtx->x == 1) {
                    mtx->lines++;
                }
            } else {
                /* lines == PHOTO_MTX_Y */
                photo_matrix_last_page_update(obj, 1);
            }
        }

        if (mtx->x == 1) {
            if (mtx->y == 1)
                app_srv_photo_group_update(index, PHOTO_MTX_X,
                                           PHOTO_TURN_FORWARD);
            PHOTO_MTX_Y_UP(mtx->y);
        }
        PHOTO_MTX_X_LEFT(mtx->x);
        break;
    case LV_KEY_RIGHT:
        if (index == img_total) {
            /* last item */
            lv_btnmatrix_set_selected_btn(obj, index - 1);
            return;
        }

        index = index + 1;

        if ((img_total - index) < mtx_total) {
            /* last page */
            if (mtx->lines < PHOTO_MTX_Y) {
                /**
                 * Turned directly to the last page, less than PHOTO_MTX_Y
                 * lines.
                 */
                photo_matrix_last_page_update(obj, 0);
            } else {
                /* lines == PHOTO_MTX_Y */
                photo_matrix_last_page_update(obj, 1);
            }
        }
        if (mtx->x == PHOTO_MTX_X) {
            if (mtx->y == PHOTO_MTX_Y)
                app_srv_photo_group_update(index, PHOTO_MTX_X,
                                           PHOTO_TURN_BACKWARD);
            PHOTO_MTX_Y_DOWN(mtx->y);
        }
        PHOTO_MTX_X_LEFT(mtx->x);
        break;
    default:
        return;
    }

    info->index = index;
}

static void photo_mtx_clicked(lv_obj_t *obj, lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    int btn_id = lv_btnmatrix_get_selected_btn(obj);

#ifdef PHOTO_BROSWE_CLICK_SUPPORT
    struct photo_object *img_obj = NULL;
    _ui_screen_change(ui_photo_preview, LV_SCR_LOAD_ANIM_FADE_ON, 50, 0);

    if (info->type == PHOTO_BROWSE_MTX_9)
        photo_preview_page_init(PHOTO_BROWSE_MTX_9);
	else
	    photo_preview_page_init(PHOTO_BROWSE_LIST);

    img_obj = app_srv_photo_obj_get_by_idx(btn_id + 1);
    if (!img_obj) {
        app_loge("%s: no more photos!\n", __func__);
        return;
    }

    photo_display(img_obj->f_attr.name);
#endif /* !PHOTO_BROSWE_CLICK_SUPPORT */

    info->index = btn_id + 1;
}

static void photo_list_update(lv_obj_t *obj, lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_btnmatrix_t *btnm = (lv_btnmatrix_t *)obj;
    int btn_id = lv_btnmatrix_get_selected_btn(obj);
    char c = *((char *)lv_event_get_param(e));

    /* TODO: scrollbar for key responsing */
    switch (c) {
    case LV_KEY_UP:
        lv_obj_scroll_to_y(info->lv_obj.panel,
                           (btnm->btn_id_sel
                           * info->lv_obj.scroll_h / btnm->btn_cnt),
                           LV_ANIM_ON);
        break;
    case LV_KEY_DOWN:
        lv_obj_scroll_to_y(info->lv_obj.panel,
                           ((btnm->btn_id_sel + 1)
                           * info->lv_obj.scroll_h / btnm->btn_cnt),
                           LV_ANIM_ON);
        break;
    case LV_KEY_LEFT:
        break;
    case LV_KEY_RIGHT:
        break;
    default:
        return;
    }

    info->index = btn_id + 1;
}

static void photo_thumbnail_update(lv_obj_t *obj, lv_event_t *e)
{
    lv_area_t a, b;
    lv_draw_img_dsc_t img_draw_dsc;
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    struct tm *date_time = NULL;
    float size = 0.0;

    if (dsc->class_p == &lv_btnmatrix_class &&
        dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
        struct photo_object *img_obj = 0;
        struct photo_attr attr;
        static lv_img_dsc_t img_dsc;
        char img_src[PHOTO_PATH_NAME_LEN];
        u8 *rgb888_data = 0;
        u16 *rgb565_data = 0;
        u32 rgb888_size = 0;
        u32 rgb565_size = 0;

        if (g_photo_info.type == PHOTO_BROWSE_MTX_9) {
            attr.width = 65;
            attr.height = 65;
        } else {
            attr.width = 64;
            attr.height = 64;
        }

        img_obj = app_srv_photo_obj_get_by_idx(dsc->id + 1);
        if (img_obj == NULL) {
            app_logi("%s: no more photos\n", __func__);
            return;
        }

        if (!img_obj->data) {
            memset(img_src, '\0', sizeof(img_src));
            strcpy(img_src, PHOTO_DISK_LETTER);
            strcat(img_src, PHOTO_DIR);
            strcat(img_src, img_obj->f_attr.name);

            rgb888_data = app_srv_photo_generate_thumbnail(img_src, &attr,
                                                           &rgb888_size);
            if (!rgb888_data) {
                app_loge("%s: photo decode error\n", __func__);
                return;
            }

            rgb565_size = rgb888_size * 2 / 3 + 3;
            rgb565_data = aic_malloc(rgb565_size);
            if (!rgb565_data) {
                aic_free(rgb888_data);
                app_loge("%s: no more memory\n", __func__);
                return;
            }

            rgb565_size = photo_rgb888_to_rgb565(rgb888_data, rgb888_size,
                                                rgb565_data, rgb565_size);
            if (!rgb565_size) {
                aic_free(rgb565_data);
                aic_free(rgb888_data);
                app_loge("%s: rgb888 to rgb565 fail\n", __func__);
                return;
            }

            aic_free(rgb888_data);
            img_obj->data = (u8 *)rgb565_data;
            img_obj->size = rgb565_size * 2;
        }

        img_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
        img_dsc.header.always_zero = 0;
        img_dsc.header.reserved = 0;
        img_dsc.header.w = attr.width;
        img_dsc.header.h = attr.height;
        img_dsc.data_size = img_obj->size;
        img_dsc.data = img_obj->data;

        lv_draw_img_dsc_init(&img_draw_dsc);
        img_draw_dsc.recolor = lv_color_black();

        if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
            img_draw_dsc.recolor_opa = LV_OPA_10;

        if (g_photo_info.type == PHOTO_BROWSE_MTX_9) {
            a.x1 = dsc->draw_area->x1 - 30;
            a.x2 = a.x1 + attr.width - 1;
            a.y1 = dsc->draw_area->y1 - 24;
            a.y2 = a.y1 + attr.height - 1;

            lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, &img_dsc);
        } else {
            a.x1 = 0;
            a.x2 = a.x1 + attr.width - 1;
            a.y1 = dsc->draw_area->y1 - 22;
            a.y2 = a.y1 + attr.height - 1;

            lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, &img_dsc);

            size = img_obj->f_attr.size / 1024;
            date_time = gmtime(&img_obj->f_attr.atime);
            char *txt = (char *)aic_malloc(PHOTO_PATH_NAME_LEN + PHOTO_SIZE_LEN);
            if (!txt) {
                app_loge("%s: no more memory\n", __func__);
                return;
            }
            memset(txt, '\0', PHOTO_PATH_NAME_LEN + PHOTO_SIZE_LEN);
            strcpy(txt, img_obj->f_attr.name);

            sprintf(txt + strlen(img_obj->f_attr.name),
                    "\n%0.1fK  %02u/%02u/%04u",
                    size, date_time->tm_mon, date_time->tm_mday,
                    date_time->tm_year + 1900);

            b.x1 = a.x2 + 5;
            b.y1 = dsc->draw_area->y1 - 10;
            b.x2 = a.x2 + 200;
            b.y2 = a.y2;
            lv_draw_label(dsc->draw_ctx, dsc->label_dsc, &b, txt, NULL);

            aic_free(txt);
        }
    }

    /* update the height of scrollbar */
    g_photo_info.lv_obj.scroll_h
                            = lv_obj_get_scroll_bottom(g_photo_info.lv_obj.panel)
                            + lv_obj_get_scroll_top(g_photo_info.lv_obj.panel);
}

static void photo_thumbnail_display_event_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (!obj || !e) {
        app_loge("%s: param error!\n", __func__);
        return;
    }

    switch (code) {
    case LV_EVENT_DRAW_PART_BEGIN:
        photo_btnmatrix_init(obj, e);
        break;
    case LV_EVENT_DRAW_PART_END:
        photo_thumbnail_update(obj, e);
        break;
    case LV_EVENT_KEY:
        if (info->type == PHOTO_BROWSE_LIST)
            photo_list_update(obj, e);
        else
            photo_matrix_update(obj, e);
        break;
    case LV_EVENT_CLICKED:
        photo_mtx_clicked(obj, e);
        break;
    default:
        break;
    }
}

static void photo_thumbnail_next_page_event_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_obj_t *obj = info->lv_obj.btnmtx;
    int mtx_total = PHOTO_MTX_SIZE;
    int img_total = info->total;
    int index = info->index;

    /* check if last page */
    if ((img_total - index) < mtx_total) {
        app_logi("%s: last page !\n", __func__);
        return;
    }

    index = index + mtx_total;
    if (index > img_total) {
        index = img_total;
    }

    /* update last page */
    if ((img_total - index) < mtx_total)
        photo_matrix_last_page_update(obj, 0);
    else
        photo_matrix_last_page_update(obj, 1);

    /* update group from service */
    app_srv_photo_group_update(index, mtx_total, PHOTO_TURN_BACKWARD);

    /* send event to update thumbnails */
    lv_event_send(obj, LV_EVENT_DRAW_PART_END, 0);

    info->index = index;
}

static void photo_thumbnail_pre_page_event_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_obj_t *obj = info->lv_obj.btnmtx;
    int mtx_total = PHOTO_MTX_SIZE;
    int index = info->index;

    /* check if the first page */
    if (index <= mtx_total) {
        app_logi("%s: first page !\n", __func__);
        return;
    }

    index = index - mtx_total;

    /* update group from service */
    app_srv_photo_group_update(index, mtx_total,
                               PHOTO_TURN_FORWARD);

    /* send event to update thumbnails */
    lv_event_send(obj, LV_EVENT_DRAW_PART_END, 0);

    info->index = index;
}

static void photo_thumbnail_delete_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_obj_t *obj = NULL;
    int mtx_total = PHOTO_MTX_SIZE;
    int img_total = info->total;
    int index = info->index;
    int chosen = PHOTO_DEL_CANCEL;

    obj = lv_event_get_current_target(e);
    if (!obj) {
        app_loge("%s, obj get fail\n", __func__);
        return;
    }

    chosen = lv_msgbox_get_active_btn(obj);
    if (chosen == PHOTO_DEL_CANCEL) {
        lv_msgbox_close(obj);
        return;
    }

    if (app_srv_photo_remove(index)) {
        app_loge("%s: photo num:%d removes failed\n",
        __func__, index);
        return;
    }

    /* check if last one removed */
    if (index == img_total)
        index = index - 1;

    img_total = img_total - 1;

    info->total = img_total;
    info->index = index;

    /* update last page */
    if ((img_total - index) < mtx_total)
        photo_matrix_last_page_update(obj, 0);
    else
        photo_matrix_last_page_update(obj, 1);

    /* send event to update thumbnails */
    /**
     * TODO: photo drawing start position
     * should be optimized
     */
    lv_event_send(obj, LV_EVENT_DRAW_PART_END, 0);
}

static void photo_matrix_map_update()
{
    struct photo_matrix *mtx = &g_photo_mtx;
    struct photo_info *info = &g_photo_info;
    int mtx_size = 0, map_cnt = 0;
    int raws = info->total / PHOTO_MTX_X;
    int left = info->total % PHOTO_MTX_X;

    /* free before map first */
    if (mtx->map_size)
        aic_free(mtx->map);

    if (left != 0)
        raws = raws + 1;

    if (info->type == PHOTO_BROWSE_MTX_9) {
        map_cnt = info->total + (PHOTO_MTX_X - left);
        mtx_size = map_cnt + raws;
    } else {
        map_cnt = info->total;
        mtx_size = info->total + raws;
    }

    mtx->map = (char **)aic_malloc((mtx_size + 1) * sizeof(char *));
    if (!mtx->map) {
        app_loge("%s: no more memory\n", __func__);
        return;
    }

    for (int i = 1; i < mtx_size; i++) {
        if ((i % (PHOTO_MTX_X + 1)) == 0)
            mtx->map[i - 1] = "\n";
        else
            mtx->map[i - 1] = " ";
    }

    mtx->map[mtx_size - 1] = "";
    mtx->map_size = mtx_size;

    lv_btnmatrix_set_map(info->lv_obj.btnmtx, (const char **)mtx->map);

    if (info->type == PHOTO_BROWSE_MTX_9) {
        lv_obj_set_size(info->lv_obj.btnmtx, LV_PCT(100), 26 * map_cnt);

        for (int i = info->total; i < map_cnt; i++)
            lv_btnmatrix_set_btn_ctrl(info->lv_obj.btnmtx, i,
                                      LV_BTNMATRIX_CTRL_HIDDEN);
    } else {
        lv_obj_set_size(info->lv_obj.btnmtx, LV_PCT(100), 64 * map_cnt);
    }

    lv_group_focus_obj(info->lv_obj.btnmtx);
}

static void photo_preview_delete_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    struct photo_object *img_obj = 0;
    lv_obj_t *obj = NULL;
    u32 index = info->index;
    u32 img_total = info->total;
    int chosen = PHOTO_DEL_CANCEL;

    obj = lv_event_get_current_target(e);
    if (!obj) {
        app_loge("%s, obj get fail\n", __func__);
        return;
    }

    chosen = lv_msgbox_get_active_btn(obj);
    if (chosen == PHOTO_DEL_CANCEL) {
        lv_msgbox_close(obj);
        return;
    }

    if (app_srv_photo_remove(index)) {
        app_loge("%s: photo num:%d removes failed\n",
        __func__, index);
        return;
    }

    /* check if last one removed */
    if (index == img_total)
        index = index - 1;

    img_total = img_total - 1;

    info->total = img_total;
    info->index = index;

    lv_msgbox_close(obj);

    /* show next page */
    if (img_total != 0) {
        img_obj = app_srv_photo_obj_get_by_idx(index);
        if (img_obj)
            photo_display(img_obj->f_attr.name);
    } else {
        /* no more images, return to main page */
        flow_photo_preview_exit_evt_cb(0);

        lv_obj_t *btm = info->lv_obj.btnmtx;
        lv_obj_t *label = lv_label_create(lv_obj_get_parent(btm));
        lv_label_set_text(label, "No photos");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_del(btm);

        lv_obj_add_state(ui_photo_preview_btn, LV_STATE_DISABLED);
        lv_obj_add_state(ui_photo_delete_btn, LV_STATE_DISABLED);
    }
}

static void photo_list_delete_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_obj_t *obj = NULL;
    u32 index = info->index;
    u32 img_total = info->total;
    int chosen = PHOTO_DEL_CANCEL;

    obj = lv_event_get_current_target(e);
    if (!obj) {
        app_loge("%s, obj get fail\n", __func__);
        return;
    }

    chosen = lv_msgbox_get_active_btn(obj);
    if (chosen == PHOTO_DEL_CANCEL) {
        lv_msgbox_close(obj);
        return;
    }

    if (app_srv_photo_remove(index)) {
        app_loge("%s: photo num:%d removes failed\n",
        __func__, index);
        return;
    }

    /* check if last one removed */
    if (index == img_total)
        index = index - 1;

    img_total = img_total - 1;

    info->total = img_total;
    info->index = index;

    lv_msgbox_close(obj);

    /* update list */
    obj = info->lv_obj.btnmtx;
    if (img_total != 0) {
        photo_matrix_map_update();

        lv_group_focus_obj(obj);
        lv_btnmatrix_set_selected_btn(obj, index - 1);
    } else {
        lv_obj_t *label = lv_label_create(lv_obj_get_parent(obj));
        lv_label_set_text(label, "No photos");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_del(obj);

        /* disable preview */
        lv_obj_add_state(ui_photo_preview_btn, LV_STATE_DISABLED);
        lv_obj_add_state(ui_photo_delete_btn, LV_STATE_DISABLED);
    }
}

static void photo_delete_all_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_obj_t *obj = NULL;
    int chosen = PHOTO_DEL_CANCEL;
    int index = info->index;

    obj = lv_event_get_current_target(e);
    if (!obj) {
        app_loge("%s, obj get fail\n", __func__);
        return;
    }

    chosen = lv_msgbox_get_active_btn(obj);
    if (chosen == PHOTO_DEL_CANCEL) {
        lv_msgbox_close(obj);
        return;
    }

    if (app_srv_photo_remove(-1)) {
        app_loge("%s: photos remove failed\n",
        __func__, index);
        return;
    }

    info->total = 0;
    info->index = 0;

    /* TODO: destroy current page ? */
    /* lv_obj_del(lv_scr_act()); */
}

static void photo_preview_play_timer_cb(lv_timer_t *timer)
{
    struct photo_info *info = &g_photo_info;
    struct photo_object *img_obj = NULL;
    u32 index = info->index;

    img_obj = app_srv_photo_obj_get_by_idx(index);
    if (img_obj)
        photo_display(img_obj->f_attr.name);

    index++;

    if (index > info->total)
        index = 1;

    info->index = index;

    lv_timer_reset(timer);
}

static void photo_preview_play_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_timer_t *timer = NULL;

    timer = lv_timer_create(photo_preview_play_timer_cb,
                            PHOTO_PLAY_INTERVAL, 0);
    if (!timer) {
        app_loge("%s: lv timer create fail\n",
                   __func__);
        return;
    }

    info->state = PHOTO_PREVIEW_PLAY;
    info->lv_obj.timer = timer;

    lv_timer_ready(timer);
}

static void photo_remove_prompt(const char *prompt, lv_event_cb_t evt_cb)
{
    static const char *btns[] = {"Apply", "Cancel", ""};
    lv_obj_t *mbox = NULL;

    mbox = lv_msgbox_create(NULL, NULL, prompt, btns, false);
    if (!mbox) {
        app_loge("%s: mbox create failed\n", __func__);
        return;
    }

    lv_obj_add_event_cb(mbox, evt_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_center(mbox);
    lv_obj_align(((lv_msgbox_t *)mbox)->text, LV_ALIGN_CENTER, 20, 0);
    lv_obj_set_size(((lv_msgbox_t *)mbox)->btns, LV_PCT(100), 30);
}

static void photo_preview_img_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    lv_event_code_t event_code = lv_event_get_code(e);

    if (event_code == LV_EVENT_CLICKED) {
        if (info->state == PHOTO_PREVIEW) {
            photo_preview_dis_timer_start();
        } else {
            flow_photo_preview_exit_evt_cb(0);
        }

        lv_obj_t *label = info->lv_obj.label;

        if (label)
            lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(ui_photo_preview_delete_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_photo_preview_return_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_photo_preview_play_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

static void photo_preview_page_init(photo_browse_type_e type)
{
    struct photo_obj_lvgl *lv_obj = &g_photo_info.lv_obj;

    if (!lv_obj->img) {
        lv_obj->img = lv_img_create(ui_photo_preview);
        if (!lv_obj->img) {
            app_loge("%s: create img obj failed\n", __func__);
            return;
        }
    }

    if (!lv_obj->label) {
        lv_obj->label = lv_label_create(ui_photo_preview);
        if (!lv_obj->label) {
            app_loge("%s: create label obj failed\n", __func__);
        }
    }

    lv_obj_add_flag(lv_obj->img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(lv_obj->img, photo_preview_img_evt_cb, LV_EVENT_ALL,
                        NULL);

    if (type == PHOTO_BROWSE_LIST) {
        lv_obj_clear_flag(ui_photo_preview_delete_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_photo_preview_return_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_photo_preview_play_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

static void photo_preview_page_deinit(void)
{
    struct photo_obj_lvgl *lv_obj = &g_photo_info.lv_obj;

    if (lv_obj->label) {
        lv_obj_del(lv_obj->label);
        lv_obj->label = NULL;
    }

    if (lv_obj->img) {
        lv_obj_del(lv_obj->img);
        lv_obj->img = NULL;
    }
}

static void photo_main_page_init(photo_browse_type_e type)
{
    struct photo_matrix *mtx = &g_photo_mtx;
    lv_obj_t *btm = NULL, *panel = NULL;
    struct photo_info *info = &g_photo_info;
    int mtx_size = 0, map_cnt = 0;
    int raws = 0, left = 0;

    panel = lv_obj_create(lv_scr_act());
    if (!panel) {
        app_loge("%s: panel create failed\n", __func__);
        return;
    }
    lv_obj_remove_style(panel, NULL, LV_PART_MAIN);
    if (type == PHOTO_BROWSE_MTX_9)
        lv_obj_set_size(panel, LV_PCT(100), 320);
    else
        lv_obj_set_size(panel, LV_PCT(100), 270);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 0);

    info->lv_obj.panel = panel;

    /* check if photo directory is empty */
    if (info->total == 0) {
        /* no photos */
        lv_obj_t *label = lv_label_create(panel);
        lv_label_set_text(label, "No photos");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

        /* disable preview */
        lv_obj_add_state(ui_photo_preview_btn, LV_STATE_DISABLED);
        lv_obj_add_state(ui_photo_delete_btn, LV_STATE_DISABLED);
        return;
    }

    btm = lv_btnmatrix_create(panel);
    if (!btm) {
        app_loge("%s: btnmatrix create failed\n", __func__);
        return;
    }

    raws = info->total / PHOTO_MTX_X;
    left = info->total % PHOTO_MTX_X;
    if (left != 0)
        raws = raws + 1;

    if (type == PHOTO_BROWSE_MTX_9) {
        map_cnt = info->total + (PHOTO_MTX_X - left);
        mtx_size = map_cnt + raws;
    } else {
        map_cnt = info->total;
        mtx_size = info->total + raws;
    }

    mtx->map = (char **)aic_malloc((mtx_size + 1) * sizeof(char *));
    if (!mtx->map) {
        app_loge("%s: no more memory\n", __func__);
        return;
    }

    for (int i = 1; i < mtx_size; i++) {
        if ((i % (PHOTO_MTX_X + 1)) == 0)
            mtx->map[i - 1] = "\n";
        else
            mtx->map[i - 1] = " ";
    }

    mtx->map[mtx_size - 1] = "";
    mtx->map_size = mtx_size;

    lv_btnmatrix_set_map(btm, (const char **)mtx->map);
    lv_obj_add_event_cb(btm, photo_thumbnail_display_event_cb, LV_EVENT_ALL,
                        NULL);
    lv_btnmatrix_set_btn_ctrl_all(btm, LV_BTNMATRIX_CTRL_CLICK_TRIG |
                                           LV_BTNMATRIX_CTRL_NO_REPEAT);
    if (type == PHOTO_BROWSE_MTX_9) {
        lv_obj_set_size(btm, LV_PCT(100), 26 * map_cnt);

        for (int i = info->total; i < map_cnt; i++)
            lv_btnmatrix_set_btn_ctrl(btm, i, LV_BTNMATRIX_CTRL_HIDDEN);
    } else {
        lv_obj_remove_style(btm, NULL, LV_PART_MAIN);
        lv_obj_set_size(btm, LV_PCT(100), 64 * map_cnt);
    }

    lv_group_focus_obj(btm);

    info->lv_obj.btnmtx = btm;
    info->type = type;
}

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

/**
 * flow_photo_enter() - enter photo app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_enter(void *param)
{
    struct photo_matrix *mtx = &g_photo_mtx;
    struct photo_info *info = &g_photo_info;
    char path[PHOTO_DIR_LEN];

    memset(path, '\0', PHOTO_DIR_LEN);
    strcpy(path, PHOTO_DISK_LETTER);
    strcat(path, PHOTO_DIR);
    app_srv_photo_start(path);

    info->img_obj = NULL;
    info->index = 1;
    info->state = PHOTO_PREVIEW;
    info->total = app_srv_photo_total_number();

    mtx->lines = 0;
    mtx->x = 1;
    mtx->y = 1;

    photo_main_page_init(PHOTO_BROWSE_LIST);
}

/**
 * flow_photo_leave() - leavel photo app flow.
 * @param: pointer to user data, reserved.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_leave(void *param)
{
    struct photo_matrix *mtx = &g_photo_mtx;
    struct photo_info *info = &g_photo_info;

    app_srv_photo_stop();

    if (info->lv_obj.btnmtx)
        lv_obj_del(info->lv_obj.btnmtx);
    if (info->lv_obj.panel)
        lv_obj_del(info->lv_obj.panel);
    if (info->lv_obj.label)
        lv_obj_del(info->lv_obj.label);
    if (info->lv_obj.img)
        lv_obj_del(info->lv_obj.img);
    if (info->lv_obj.timer)
        lv_timer_del(info->lv_obj.timer);
    if (info->lv_obj.btntimer)
        lv_timer_del(info->lv_obj.btntimer);

    info->img_obj = NULL;
    info->index = 0;
    info->total = 0;

    mtx->lines = 0;
    mtx->x = 0;
    mtx->y = 0;
}

/**
 * flow_photo_preview_enter_evt_cb() - responding to preview
 *                                     events.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_enter_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    struct photo_object *img_obj = NULL;

    if (!info->total) {
        app_logi("no images !\n");
        return;
    }

    info->state = PHOTO_PREVIEW;

    photo_preview_page_init(PHOTO_BROWSE_LIST);

    img_obj = app_srv_photo_obj_get_by_idx(info->index);
    if (!img_obj) {
        app_loge("%s: no more photos!\n", __func__);
        return;
    }

    photo_display(img_obj->f_attr.name);
}

/**
 * flow_photo_preview_exit_evt_cb() - responding to preview
 *                                    exit events.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_exit_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    struct photo_obj_lvgl *obj = &info->lv_obj;

    photo_preview_page_deinit();

    if (info->total)
        photo_matrix_map_update();

    if (info->state == PHOTO_PREVIEW_PLAY) {
        if (info->lv_obj.timer) {
            lv_timer_del(info->lv_obj.timer);
        }
    }

    if (info->lv_obj.btntimer)
        lv_timer_del(info->lv_obj.btntimer);

    if (obj->btnmtx) {
        lv_group_focus_obj(obj->btnmtx);
        lv_btnmatrix_set_selected_btn(obj->btnmtx,
                                      info->index - 1);
        lv_obj_scroll_to_y(info->lv_obj.panel,
                           ((info->index - 1)
                           * info->lv_obj.scroll_h
                           / info->total),
                           LV_ANIM_ON);
    }

    _ui_screen_change(ui_photo_main,
                      LV_SCR_LOAD_ANIM_FADE_ON, 50, 0);
}

/**
 * flow_photo_prev_key_evt_cb - event callback of previous photo.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_prev_key_evt_cb(lv_event_t *e)
{
    struct photo_matrix *mtx = &g_photo_mtx;
    struct photo_info *info = &g_photo_info;
    struct photo_object *img_obj = NULL;
    int index = info->index;

    index = index - 1;
    if (index == 0) {
        app_logi("%s: reach first photo!\n", __func__);
        return;
    }

    img_obj = app_srv_photo_obj_get(index);
    if (!img_obj) {
        app_loge("%s: no more photos!\n", __func__);
        return;
    }

    if (mtx->x == 1) {
        if (mtx->y == 1)
            app_srv_photo_group_update(index, PHOTO_MTX_X,
                                       PHOTO_TURN_FORWARD);

        PHOTO_MTX_Y_UP(mtx->y);
    }
    PHOTO_MTX_X_LEFT(mtx->x);

    info->index = index;

    photo_display(img_obj->f_attr.name);
}

/**
 * flow_photo_next_key_evt_cb() - event callback of next photo.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_next_key_evt_cb(lv_event_t *e)
{
    struct photo_matrix *mtx = &g_photo_mtx;
    struct photo_info *info = &g_photo_info;
    struct photo_object *img_obj = NULL;
    int index = info->index;

    index = index + 1;
    if (index == info->total) {
        app_logi("%s: reach last photo!\n", __func__);
        return;
    }

    img_obj = app_srv_photo_obj_get_by_idx(index);
    if (!img_obj) {
        app_loge("%s: no more photos!\n", __func__);
        return;
    }

    if (mtx->x == PHOTO_MTX_X) {
        if (mtx->y == PHOTO_MTX_Y)
            app_srv_photo_group_update(index, PHOTO_MTX_X,
                                       PHOTO_TURN_BACKWARD);

        PHOTO_MTX_Y_DOWN(mtx->y);
    }
    PHOTO_MTX_X_RIGHT(mtx->x);

    info->index = index;

    photo_display(img_obj->f_attr.name);
}

/**
 * flow_photo_prev_page_evt_cb - event callback of previous page.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_prev_page_evt_cb(lv_event_t *e)
{
    photo_thumbnail_pre_page_event_cb(e);
}

/**
 * flow_photo_next_page_evt_cb() - event callback of next page.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_next_page_evt_cb(lv_event_t *e)
{
    photo_thumbnail_next_page_event_cb(e);
}

/**
 * flow_photo_rename_evt_cb() - event callback of photo rename
 *                              operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_rename_evt_cb(lv_event_t *e)
{
    int index = g_photo_info.index;
    char *new_name = NULL;

    /* TODO: get the name from input */
    if (app_srv_photo_rename(index, new_name)) {
        app_loge("%s: photo num:%d removes failed\n",
                    __func__, index);
        return;
    }
}

/**
 * flow_photo_list_remove_evt_cb() - event callback of photo list
 *                                   remove operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_list_remove_evt_cb(lv_event_t *e)
{
    if (!g_photo_info.total) {
        /* TODO: disable image button */
        return;
    }

    photo_remove_prompt(PHOTO_DEL_PROMPT, photo_list_delete_evt_cb);
}

/**
 * flow_photo_preview_remove_evt_cb() - event callback of photo preview
 *                                      remove operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_remove_evt_cb(lv_event_t *e)
{
    if (g_photo_info.state == PHOTO_PREVIEW_PLAY) {
        app_logi("%s: preview play, can't op",
                  __func__);
        return;
    }

    photo_remove_prompt(PHOTO_DEL_PROMPT,
                        photo_preview_delete_evt_cb);
}

/**
 * flow_photo_preview_play_evt_cb() - event callback of photo preview
 *                                    remove operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_preview_play_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;

    if (info->total <= 1) {
        app_logi("%s: just %d pixes\n", __func__);
        return;
    }

    if (info->state == PHOTO_PREVIEW_PLAY) {
        app_logi("%s: is playing\n", __func__);
        return;
    }

    photo_preview_play_evt_cb(e);
}

/**
 * flow_photo_thumbnail_remove_evt_cb() - event callback of remove
 *                                        chosen thumbnail operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_thumbnail_remove_evt_cb(lv_event_t *e)
{
    photo_remove_prompt(PHOTO_DEL_PROMPT, photo_thumbnail_delete_evt_cb);
}

/**
 * flow_photo_remove_all_evt_cb() - event callback of remove all
 *                                  photos operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_remove_all_evt_cb(lv_event_t *e)
{
    photo_remove_prompt(PHOTO_DEL_ALL_PROMPT, photo_delete_all_evt_cb);
}

/**
 * flow_photo_details_display_evt_cb() - event callback of photo
 *                                       details display.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_details_display_evt_cb(lv_event_t *e)
{
    lv_obj_t *label;
    struct tm *date_time = NULL;
    struct photo_object *img_obj = 0;
    int index = g_photo_info.index;

    img_obj = app_srv_photo_obj_get_by_idx(index);
    if (!img_obj) {
        app_loge("%s: photo num:%d get failed\n",
                   __func__, index);
        return;
    }

    /* display details info on label */
    date_time = gmtime(&img_obj->f_attr.atime);
    if (!date_time) {
        app_loge("%s: time_t convert failed\n", __func__);
        return;
    }

    label = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(label, "name: %s, width: %u, \
                          height: %u, size: %u,\
                          date: %u%02u%02u %02u:%02u:%02u",
                          img_obj->f_attr.name,
                          img_obj->img_attr.width,
                          img_obj->img_attr.height,
                          img_obj->f_attr.size,
                          date_time->tm_year,
                          date_time->tm_mon,
                          date_time->tm_mday,
                          date_time->tm_hour,
                          date_time->tm_min,
                          date_time->tm_sec);

    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

/**
 * flow_photo_zoom_evt_cb() - event callback of photo zoom in and
 *                            restore.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_zoom_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    static int zoom = PHOTO_ZOOM_IN;
    struct photo_object *img_obj = 0;
    lv_obj_t *img = info->lv_obj.img;
    lv_obj_t *label = info->lv_obj.label;
    char img_src[PHOTO_PATH_NAME_LEN];

    if (!img || !label) {
        app_loge("%s: lvgl obj not init.\n", __func__);
        return;
    }

    img_obj = app_srv_photo_obj_get_by_idx(info->index);
    if (img_obj != NULL) {
        app_loge("%s: no more photos\n", __func__);
        return;
    }

    memset(img_src, '\0', sizeof(img_src));
    strcpy(img_src, PHOTO_DISK_LETTER);
    strcat(img_src, PHOTO_DIR);
    strcat(img_src, img_obj->f_attr.name);

    if (photo_get_image_dsc(&info->lv_obj.img_dsc, img_src)) {
        app_loge("%s: get image data failed\n", __func__);
        return;
    }

    lv_img_set_src(img, &info->lv_obj.img_dsc);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, -20);

    if (zoom == PHOTO_ZOOM_IN) {
        lv_img_set_zoom(img, PHOTO_SIZE_DOUBLE);
        zoom = PHOTO_RESTORE;
    } else {
        lv_img_set_zoom(img, PHOTO_SIZE_RESTORE);
        zoom = PHOTO_ZOOM_IN;
    }

    lv_label_set_text_fmt(label, "%u/%u", info->index,
                          info->total);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    aic_free(info->lv_obj.img_dsc.data);
}

/**
 * flow_photo_rotate_evt_cb() - event callback of photo rotate operation.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_rotate_evt_cb(lv_event_t *e)
{
    struct photo_info *info = &g_photo_info;
    struct photo_object *img_obj = 0;
    lv_obj_t *label = info->lv_obj.label;
    lv_obj_t *img = info->lv_obj.img;
    char img_src[PHOTO_PATH_NAME_LEN];

    img_obj = app_srv_photo_obj_get_by_idx(info->index);
    if (img_obj != NULL) {
        app_loge("%s: no more photos\n", __func__);
        return;
    }

    memset(img_src, '\0', sizeof(img_src));
    strcpy(img_src, PHOTO_DISK_LETTER);
    strcat(img_src, PHOTO_DIR);
    strcat(img_src, img_obj->f_attr.name);

    if (photo_get_image_dsc(&info->lv_obj.img_dsc, img_src)) {
        app_loge("%s: get image data failed\n", __func__);
        return;
    }

    lv_img_set_src(img, &info->lv_obj.img_dsc);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, -20);

    lv_img_set_angle(img, PHOTO_ROTATE_ANGLE);

    lv_label_set_text_fmt(label, "%u/%u", info->index,
                          info->total);
    lv_obj_align_to(label, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    aic_free(info->lv_obj.img_dsc.data);
}

/**
 * flow_photo_matrix_display_evt_cb() - event callback of 3*3 matrix display.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_matrix_display_evt_cb(lv_event_t *e)
{
    lv_obj_del(lv_scr_act());

    photo_main_page_init(PHOTO_BROWSE_MTX_9);
}

/**
 * flow_photo_back_key_evt_cb() - event callback for back key response.
 * @e: pointer to event object.
 * @arg2: none.
 * Return: none.
 */
void flow_photo_back_key_evt_cb(lv_event_t *e)
{
    flow_photo_leave(0);

    lv_obj_del(ui_photo_preview);
//    lv_obj_del_async(ui_photo_main);
}
