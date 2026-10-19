/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of photo service.
 */

/*********************
 *      INCLUDES
 *********************/
#include "photo_service.h"
#include "aic_list.h"
#include "lvgl.h"
#include "photo_converter.h"
#include "photo_object.h"
#include "aic_osa_ex.h"

/*********************
 *      DEFINES
 *********************/
#define PHOTO_COLOR_DEPTH 24

/**********************
 *      TYPEDEFS
 **********************/
struct image_list {
    struct photo_object obj;
    struct list_head node;
};

struct image_group {
    u32 start_index;
    u32 group_index;
    struct image_list *list_img;
};

struct image_set {
    char dir[PHOTO_DIR_LEN];
    u32 total_num;
    struct list_head list;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void image_list_form(void *file_data, void *param);

/**********************
 *  STATIC VARIABLES
 **********************/
static struct image_group g_img_group;
static struct image_set g_img_set;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void image_list_form(void *file_data, void *param)
{
#ifdef LV_IMG_SJPG_DECODER
    lv_res_t res = LV_RES_INV;
    lv_img_header_t img_header;
#endif /* LV_IMG_SJPG_DECODER */
    struct image_set *img_set = &g_img_set;
    struct image_list *n_obj = NULL;
    struct photo_object *img_obj = NULL;
    struct file_attr *f_attr = (struct file_attr *)file_data;
    char *path_name = (char *)param;

    if (!f_attr || !path_name) {
        app_loge("%s: param invalid!\n", __func__);
        return;
    }

    n_obj = (struct image_list *)aic_malloc(sizeof(struct image_list));
    if (!n_obj) {
        app_loge("%s: no more memory!\n", __func__);
        return;
    }

    img_obj = &n_obj->obj;

    /* check if image is invalid */
#ifdef LV_IMG_SJPG_DECODER
    res = lv_img_decoder_get_info(path_name, &img_header);
    if (res != LV_RES_OK) {
        app_loge("%s: %s not support img\n", __func__, path_name);
        return;
    }

    img_obj->img_attr.width = img_header.w;
    img_obj->img_attr.height = img_header.h;
#else
    if (photo_read_jpeg_header(path_name, &img_obj->img_attr)) {
        app_loge("%s: %s not support img\n", __func__, path_name);
        return;        
    }
#endif /* LV_IMG_SJPG_DECODER */
    app_logi("%s: ori width:%d, ori height:%d\n", __func__,
             img_obj->img_attr.width, img_obj->img_attr.height);

    if (!img_obj->img_attr.width || !img_obj->img_attr.height) {
        app_loge("%s: %s not support img\n", __func__, path_name);
        return;
    }

    memcpy(&img_obj->f_attr, f_attr, sizeof(struct file_attr));
    img_obj->data = 0;

    list_add(&n_obj->node, &img_set->list);

    img_set->total_num++;
}

static void image_service_init(void *param)
{
    g_img_group.start_index = 0;
    g_img_group.group_index = 0;

    g_img_set.total_num = 0;
    memset(g_img_set.dir, '\0', sizeof(g_img_set.dir));

    list_head_init(&g_img_set.list);
}

static void image_service_destory(bool gracefully)
{
    struct list_head *pos = NULL;
    struct image_list *n_obj = NULL;

    g_img_group.start_index = 0;
    g_img_group.group_index = 0;

    g_img_set.total_num = 0;

    list_for_each(pos, &g_img_set.list)
    {
        n_obj = list_entry(pos, struct image_list, node);

        aic_free(n_obj);
    }

    list_del_init(&g_img_set.list);
}

/**********************
 *   PUBLIC FUNCTIONS
 **********************/
/**
 * app_srv_photo_start() - start the photo service.
 * @path: path the full path of photo file stored
 * @arg2: none.
 * Return: none.
 */
void app_srv_photo_start(char *path)
{
    if (!path) {
        app_loge("%s: invalid param\n", __func__);
        return;
    }

    image_service_init(0);

    photo_dir_init(path, image_list_form);

    g_img_group.start_index = 1;
    g_img_group.group_index = 1;

    strcpy(g_img_set.dir, path);
}

/**
 * app_srv_photo_stop() - stop the photo service.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_photo_stop(void)
{
    image_service_destory(1);

    photo_dir_deinit();
    g_img_group.start_index = 0;
    g_img_group.group_index = 0;
}

/**
 * app_srv_photo_obj_get() - Get a selected photo object information
 *                           from photo service.
 * @pos: indicate the location of the photo user want to get
 * @arg2: none.
 * Return: pointer to next photo object information.
 */
struct photo_object *app_srv_photo_obj_get(photo_obj_pos_e pos)
{
    struct photo_object *obj = NULL;
    struct image_group *img_g = &g_img_group;

    switch (pos) {
    case PHOTO_POS_HEAD:
        obj = app_srv_photo_obj_get_by_idx(img_g->start_index);
        img_g->list_img = list_entry(obj, struct image_list, obj);
        break;
    case PHOTO_POS_PREV:
        if (img_g->list_img) {
            img_g->list_img =
                list_prev_entry(img_g->list_img, node, struct image_list);
            if (img_g->list_img)
                obj = &img_g->list_img->obj;
        }
        break;
    case PHOTO_POS_NEXT:
        if (img_g->list_img) {
            img_g->list_img =
                list_next_entry(img_g->list_img, node, struct image_list);
            if (img_g->list_img)
                obj = &img_g->list_img->obj;
        }
        break;
    default:
        app_loge("%s: invalid param\n", __func__);
        return NULL;
    }

    return obj;
}

/**
 * photo_info_get_by_idx() - Get a specified photo object information
 *                           by index.
 * @img: the index specified in photo list
 * @arg2: none.
 * Return: pointer to next photo object information.
 */
struct photo_object *app_srv_photo_obj_get_by_idx(int index)
{
    struct image_list *n_obj = NULL;
    struct list_head *pos = NULL;
    int idx_iter = 0;

    list_for_each(pos, &g_img_set.list)
    {
        n_obj = list_entry(pos, struct image_list, node);

        idx_iter++;
        if (idx_iter == index) {
            break;
        }
    }

    if (!n_obj)
        return NULL;
    else
        return &n_obj->obj;
}

/**
 * photo_generate_thumbnail() - Generate a thumbnail from the specified
 *                              photo.
 * @path: full path of the specified photo
 * @d_attr: attribute of the thumbnail to generate.
 * @size: size of the thumbnail
 * Return: pointer to the generated thumbnail image data.
 */
u8 *app_srv_photo_generate_thumbnail(const char *path,
                                     const struct photo_attr *d_attr, u32 *size)
{
    struct photo_attr attr;
    u32 img_size;
    u8 *img_data = NULL;
    u8 *thumbnail = NULL;

    if (!path || !d_attr) {
        app_loge("%s: param invalid!\n", __func__);
        return 0;
    }

    img_data = photo_read_jpeg(path, &attr, &img_size);
    if (!img_data) {
        app_loge("%s: read jpeg failed!\n", __func__);
        return NULL;
    }

    thumbnail = photo_zoom_jpeg_ex(d_attr, PHOTO_COLOR_DEPTH, &attr, img_data, &img_size);
    if (!thumbnail) {
        app_loge("%s: image zoom failed!\n", __func__);
        aic_free(img_data);
        return NULL;
    }
    *size = img_size;

    aic_free(img_data);

    return thumbnail;
}

/**
 * app_srv_photo_group_update() - update the photo group info caused by
 *                                moving operation.
 * @index: index of current picture selected.
 * @num: number of pictures moved.(1 <= num <= X*Y)
 * @dir: direction of pix movement.
 * Return: none.
 */
void app_srv_photo_group_update(int index, int num, photo_turn_dir_e dir)
{
    struct image_group *img_g = &g_img_group;
    int start_index = img_g->start_index;
    int total = g_img_set.total_num;

    if (num <= 0 || (dir != PHOTO_TURN_FORWARD && dir != PHOTO_TURN_BACKWARD)) {
        app_loge("%s: invalid param!\n", __func__);
        return;
    }

    if (dir == PHOTO_TURN_FORWARD) {
        start_index = start_index - num;
        if (start_index < 0)
            start_index = 1;
    } else {
        start_index = start_index + num;
        if (start_index > total)
            start_index = total;
    }

    img_g->group_index = index - start_index + 1;
}

/**
 * app_srv_photo_total_number() - number of photos in specified directory.
 * @arg1: none.
 * @arg2: none.
 * Return: return the total number of photoes in directory.
 */
u32 app_srv_photo_total_number(void)
{
    return g_img_set.total_num;
}

/**
 * app_srv_photo_reorder() - reorder the photo list by the specified type
 * @type: sort type for photo list.
 * @arg2: none.
 * Return: none.
 */
void app_srv_photo_reorder(photo_sort_type_e type)
{
    return;
}

/**
 * app_srv_photo_remove() - remove the photo corresponding to index from
 *                          the file system
 * @index: index of photo, -1 means all photoes.
 * @arg2: none.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_photo_remove(int index)
{
    struct image_set *img_set = &g_img_set;
    struct image_list *n_obj = NULL;
    struct list_head *pos = NULL;
    char path_name[PHOTO_PATH_NAME_LEN];
    int idx_iter = 0;

    if (index <= 0 || index > img_set->total_num) {
        app_loge("%s:invalid param\n", __func__);
        return -1;
    }

    list_for_each(pos, &img_set->list) {
        n_obj = list_entry(pos, struct image_list, node);

        idx_iter++;
        if (idx_iter == index)
            break;
    }

    if (!n_obj)
        return -1;

    /* remove from photo directory */
    memset(path_name, '\0', sizeof(path_name));
    strcpy(path_name, img_set->dir);
    strcat(path_name, n_obj->obj.f_attr.name);

    if (photo_delete(path_name))
        return -1;

    list_del(pos);
    aic_free(n_obj);

    return 0;
}

/**
 * app_srv_photo_rename() - remove the photo corresponding to index from
 *                          the file system
 * @index:  index of photo.
 * @new_name: pointer to new name of index indicated photo.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_photo_rename(int index, const char *new_name)
{
    struct image_set *img_set = &g_img_set;
    struct image_list *n_obj = NULL;
    struct list_head *pos = NULL;
    char old_path_name[PHOTO_PATH_NAME_LEN];
    char new_path_name[PHOTO_PATH_NAME_LEN];

    int idx_iter = 0;

    if (index <= 0 || index > img_set->total_num || !new_name) {
        app_loge("%s:invalid param\n", __func__);
        return -1;
    }

    list_for_each(pos, &img_set->list)
    {
        n_obj = list_entry(pos, struct image_list, node);

        idx_iter++;
        if (idx_iter == index)
            break;
    }

    if (!n_obj)
        return -1;

    memset(old_path_name, '\0', sizeof(old_path_name));
    memset(new_path_name, '\0', sizeof(new_path_name));
    strcpy(old_path_name, img_set->dir);
    strcpy(new_path_name, img_set->dir);

    strcat(old_path_name, n_obj->obj.f_attr.name);
    strcat(new_path_name, new_name);

    if (photo_rename(old_path_name, new_path_name))
        return -1;

    return 0;
}
