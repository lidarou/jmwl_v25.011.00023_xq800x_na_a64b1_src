/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __CAMERA_JPG_GENERATOR_H__
#define __CAMERA_JPG_GENERATOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/

#include <aic_type.h>
#include <time.h>

/* image directory definition of camera */
#define CAM_IMG_DISK_LETTER "C:"
#define CAM_IMG_DIR "/Images/"
#define CAM_IMG_DIR_LEN 24
#define CAM_IMG_NAME_LEN 12
#define CAM_IMG_PATH_NAME_LEN (CAM_IMG_DIR_LEN + CAM_IMG_NAME_LEN)

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of photo object.
 */

/**
 * struct struct_cam_image_attr - image attributes
 * @width: image width
 * @height: image height
 *
 * This structure is used to describe the image attributes.
 */
struct cam_image_attr {
    u32 width;
    u32 height;
};

/**
 * struct struct_cam_file_attr - file attributes
 * @name: buffer of stored file name
 * @atime: time of last access
 * @size: size of specified file
 *
 * This structure is used to describe the file attributes.
 */
struct cam_file_attr {
    char name[CAM_IMG_NAME_LEN];
    time_t atime;
    u32 size;
};

/**
 * struct struct_cam_image_object - structure of image object
 * @f_attr: file attributes
 * @img_attr: image attributes
 *
 * This structure is used to describe the information of image object.
 */
struct cam_image_object {
    struct cam_file_attr f_attr;
    struct cam_image_attr img_attr;
};

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of camera image generator.
 */

/**
 * camera_write_jpeg() - write the camera data to a jpeg file.
 * @path: the full path of data to store.
 * @attr: attributes of the data writting
 * @img_data: image data
 * @quality: image quality
 * Return: 0 means success, other means failure.
 */
int camera_write_jpeg(const char *path, const struct cam_image_attr *attr,
                     u8 *img_data, int quality);

/**
 * camera_write_jpeg_yuv422() - write the yuv422 data to a jpeg file.
 * @path: the full path of yuv data to store.
 * @attr: attributes of the yuv data writting
 * @img_data: image data of the yuv writting
 * Return: 0 means success, other means failure.
 */
int camera_write_jpeg_yuv422(const char *path, const struct cam_image_attr *attr,
                            u8 *img_data);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __CAMERA_JPG_GENERATOR_H__ */
