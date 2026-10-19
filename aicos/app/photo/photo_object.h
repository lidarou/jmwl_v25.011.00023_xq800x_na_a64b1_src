/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __PHOTO_OBJECT_H__
#define __PHOTO_OBJECT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include "aic_type.h"
#include "photo_dir_mgr.h"
#include <time.h>

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of photo object.
 */

/**
 * struct struct_photo_attr - image attributes
 * @width: image width
 * @height: image height
 *
 * This structure is used to describe the image attributes.
 */
struct photo_attr {
    u32 width;
    u32 height;
};

/**
 * struct struct_file_attr - file attributes
 * @name: buffer of stored file name
 * @atime: time of last access
 * @size: size of specified file
 *
 * This structure is used to describe the file attributes.
 */
struct file_attr {
    char name[PHOTO_NAME_LEN];
    time_t atime;
    u32 size;
};

/**
 * struct struct_photo_object - structure of image object
 * @f_attr: file attributes
 * @img_attr: image attributes
 *
 * This structure is used to describe the information of image object.
 */
struct photo_object {
    struct file_attr f_attr;
    struct photo_attr img_attr;
    u8 *data;
    u32 size;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PHOTO_OBJECT_H__ */
