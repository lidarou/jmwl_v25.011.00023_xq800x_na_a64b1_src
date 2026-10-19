/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __PHOTO_SERVICE_H__
#define __PHOTO_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include "photo_object.h"

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of photo service.
 */

/**
 * @definition of photo relative postion in group
 */
typedef enum photo_obj_pos {
    PHOTO_POS_HEAD,
    PHOTO_POS_PREV,
    PHOTO_POS_NEXT
} photo_obj_pos_e;

/**
 * @definition of photo index relative moving direction
 */
typedef enum photo_turn_dir {
    PHOTO_TURN_FORWARD,
    PHOTO_TURN_BACKWARD,
} photo_turn_dir_e;

/**
 * @definition of photo sort type in list
 */
typedef enum photo_sort_type {
    PHOTO_SORT_BY_NAME,
    PHOTO_SORT_BY_TIME,
} photo_sort_type_e;

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of photo service.
 */

/**
 * app_srv_photo_start() - start the photo service.
 * @path: path the full path of photo file stored
 * @arg2: none.
 * Return: none.
 */
void app_srv_photo_start(char *path);

/**
 * app_srv_photo_stop() - stop the photo service.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void app_srv_photo_stop(void);

/**
 * app_srv_photo_obj_get() - Get a selected photo object information
 *                           from photo service.
 * @pos: indicate the location of the photo user want to get
 * @arg2: none.
 * Return: pointer to next photo object information.
 */
struct photo_object *app_srv_photo_obj_get(photo_obj_pos_e pos);

/**
 * photo_info_get_by_idx() - Get a specified photo object information
 *                           by index.
 * @img: the index specified in photo list
 * @arg2: none.
 * Return: pointer to next photo object information.
 */
struct photo_object *app_srv_photo_obj_get_by_idx(int index);

/**
 * photo_generate_thumbnail() - Generate a thumbnail from the specified
 *                              photo.
 * @path: full path of the specified photo
 * @d_attr: attribute of the thumbnail to generate.
 * @size: size of the thumbnail
 * Return: pointer to the generated thumbnail image data.
 */
u8 *app_srv_photo_generate_thumbnail(const char *path,
                                     const struct photo_attr *d_attr, u32 *size);

/**
 * app_srv_photo_group_update() - update the photo group info caused by
 *                                moving operation.
 * @index: index of current picture selected.
 * @num: number of pictures moved.(1 <= num <= X*Y)
 * @dir: direction of pix movement.
 * Return: none.
 */
void app_srv_photo_group_update(int index, int num, photo_turn_dir_e dir);

/**
 * app_srv_photo_total_number() - number of photos in specified directory.
 * @arg1: none.
 * @arg2: none.
 * Return: return the total number of photoes in directory.
 */
u32 app_srv_photo_total_number(void);

/**
 * app_srv_photo_reorder() - reorder the photo list by the specified type
 * @type: sort type for photo list.
 * @arg2: none.
 * Return: none.
 */
void app_srv_photo_reorder(photo_sort_type_e type);

/**
 * app_srv_photo_remove() - remove the photo corresponding to index from
 *                          the file system
 * @index: index of photo, -1 means all photoes.
 * @arg2: none.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_photo_remove(int index);

/**
 * app_srv_photo_rename() - remove the photo corresponding to index from
 *                          the file system
 * @index:  index of photo.
 * @new_name: pointer to new name of index indicated photo.
 * Return: 0 means success, -1 means failure.
 */
int app_srv_photo_rename(int index, const char *new_name);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PHOTO_SERVICE__ */
