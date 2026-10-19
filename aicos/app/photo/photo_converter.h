/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __PHOTO_CONVERTER_H__
#define __PHOTO_CONVERTER_H__

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
 *    This file defines the apis of photo converter.
 */

/**
* photo_read_jpeg_header() - test if the file is a valid jpeg file
* @path: the full path of photo stored
* @attr: attributes read from the specified jpeg file
* Return: 0 means success, -1 means failure.
*/
int photo_read_jpeg_header(const char *path, struct photo_attr *attr);

/**
 * photo_read_jpeg() - read the jpeg file attributes and data from the
 *                     specified path.
 * @path: the full path of photo stored
 * @attr: attributes read from the specified jpeg file
 * @size: jpeg file data size
 * Return: return the data decompressed from the specified jpeg file.
 */
u8 *photo_read_jpeg(const char *path, struct photo_attr *attr, u32 *size);

/**
 * photo_write_jpeg() - write the photo data to a jpeg file.
 * @path: the full path of photo to store.
 * @attr: attributes of the photo writting
 * @img_data: image data of the photo writting
 * @quality: image quality of the photo writting
 * Return: 0 means success, other means failure.
 */
int photo_write_jpeg(const char *path, const struct photo_attr *attr,
                     u8 *img_data, int quality);

/**
 * photo_write_jpeg_yuv420() - write the yuv422 data to a jpeg file.
 * @path: the full path of yuv data to store.
 * @attr: attributes of the yuv data writting
 * @img_data: image data of the yuv writting
 * Return: 0 means success, other means failure.
 */
int photo_write_jpeg_yuv422(const char *path, const struct photo_attr *attr,
                            u8 *img_data);

/**
 * photo_zoom_jpeg() - scale the photo linearly.
 * @d_attr: target attributes after the photo is scaled
 * @depth: bit depth of target photo
 * @s_attr: source attributes of input photo data
 * @img_data: source photo data
 * @size: size of target data
 * Return: return the scaled data.
 */
u8 *photo_zoom_jpeg(const struct photo_attr *d_attr, const int depth,
                    const struct photo_attr *s_attr, u8 *img_data, u32 *size);

/**
 * photo_zoom_jpeg_ex() - scale the photo by quadratic linear interpolation.
 * @d_attr: target attributes after the photo is scaled
 * @depth: bit depth of target photo
 * @s_attr: source attributes of input photo data
 * @img_data: source photo data
 * @size: size of target data
 * Return: return the scaled data.
 */
u8 *photo_zoom_jpeg_ex(const struct photo_attr *d_attr, const int depth,
                       const struct photo_attr *s_attr, u8 *img_data, u32 *size);

/**
 * photo_rgb888_to_rgb565() - RGB888 to RGB565 format.
 * @rgb888: pointer to RGB888 data
 * @rgb888_size: size of RGB888 data
 * @rgb565: point to RGB565
 * @rgb565_size: size of RGB565
 * Return: return rbg565 data size, 0 means failure.
 */
int photo_rgb888_to_rgb565(u8 *rgb888, u32 rgb888_size, u16 *rgb565,
                           u32 rgb565_size);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PHOTO_CONVERTER_H__ */
