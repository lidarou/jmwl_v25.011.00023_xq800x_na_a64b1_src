/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of photo converter.
 */

/*********************
 *      INCLUDES
 *********************/

#include <stdio.h>
#include "camera_jpg_generator.h"
#include "jpeg/jpeg-9e/jpeglib.h"
#include "aic_osa_ex.h"
#include "fs_api.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   PUBLIC FUNCTIONS
 **********************/
/**
 * camera_write_jpeg() - write the camera data to a jpeg file.
 * @path: the full path of data to store.
 * @attr: attributes of the data writting
 * @img_data: image data
 * @quality: image quality
 * Return: 0 means success, other means failure.
 */
int camera_write_jpeg(const char *path, const struct cam_image_attr *attr,
                     u8 *img_data, int quality)
{
    struct jpeg_compress_struct cinfo = {0};
    struct jpeg_error_mgr jerr;
    JSAMPROW row[1];
    FILE *fp = NULL;
    int row_stride;

    if (!path || !attr || !img_data) {
        app_loge("%s: param invalid!\n", __func__);
        return -1;
    }

    fp = fopen(path, "wb");
    if (!fp) {
        app_loge("%s: open jpeg file failed\n", __func__);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);

    cinfo.image_width = attr->width;
    cinfo.image_height = attr->height;
    cinfo.input_components = 3;     /* of color components of per pixel */
    cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    row_stride = attr->width * 3; /* JSAMPLEs per row in image buffer */

    while (cinfo.next_scanline < cinfo.image_height) {
        row[0] = &img_data[cinfo.next_scanline * row_stride];
        (void)jpeg_write_scanlines(&cinfo, row, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fflush(fp);
    fclose(fp);

    return 0;
}

/**
 * camera_write_jpeg_yuv422() - write the yuv422 data to a jpeg file.
 * @path: the full path of yuv data to store.
 * @attr: attributes of the yuv data writting
 * @img_data: image data of the yuv writting
 * Return: 0 means success, other means failure.
 */
int camera_write_jpeg_yuv422(const char *path, const struct cam_image_attr *attr,
                            u8 *img_data)
{
    struct jpeg_compress_struct cinfo = {0};
    struct jpeg_error_mgr jerr;
    FILE *fp = NULL;
    JSAMPARRAY yuvbuffer[3];
    JSAMPARRAY y_ptr = 0;
    JSAMPARRAY u_ptr = 0;
    JSAMPARRAY v_ptr = 0;
    u8 *buffer_y = 0, *buffer_u = 0, *buffer_v = 0;
    int width = 0, height = 0;
    int y_row_stride, y_col_stride;
    int u_row_stride, u_col_stride, v_row_stride, v_col_stride;
    int ret = 0;

    if (!path || !attr || !img_data) {
        app_loge("%s: param invalid!\n", __func__);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    width = attr->width;
    height = attr->height;

    buffer_y = (u8 *)aic_malloc(width * height);
    if (!buffer_y) {
        app_loge("%s: no more memory\n", __func__);
        ret = -1;
        goto FAILURE;
    }
    buffer_u = (u8 *)aic_malloc(width * height / 2);
    if (!buffer_u) {
        app_loge("%s: no more memory\n", __func__);
        ret = -1;
        goto FAILURE;
    }
    buffer_v = (u8 *)aic_malloc(width * height / 2);
    if (!buffer_v) {
        app_loge("%s: no more memory\n", __func__);
        ret = -1;
        goto FAILURE;
    }

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width * 2; col++) {
            if (col % 2 == 0)//1)
                buffer_y[row * width + col / 2] = img_data[row * width * 2 + col];
            else if (col % 4 == 1)//0) /* U */
                buffer_u[row * width / 2 + col / 4] = img_data[row * width * 2 + col];
            else if (col % 4 == 3)//2) /* V */
                buffer_v[row * width / 2 + col / 4] = img_data[row * width * 2 + col];
        }
    }

    fp = fopen(path, "wb+");
    if (!fp) {
        app_loge("%s: open jpeg file failed, %s\n", __func__, path);
        ret = -1;
        goto FAILURE;
    }

    jpeg_stdio_dest(&cinfo, fp);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);
    cinfo.raw_data_in = TRUE;
    cinfo.do_fancy_downsampling = FALSE;

    cinfo.comp_info[0].h_samp_factor = 2;                /* for Y */
    cinfo.comp_info[0].v_samp_factor = 1;
    cinfo.comp_info[1].h_samp_factor = 1;                /* for Cb */
    cinfo.comp_info[1].v_samp_factor = 1;
    cinfo.comp_info[2].h_samp_factor = 1;                /* for Cr */
    cinfo.comp_info[2].v_samp_factor = 1;

    app_logi("%s: block size=%d, img_data: %p\n", __func__, cinfo.block_size, img_data);

    cinfo.comp_info[0].width_in_blocks = width / cinfo.block_size;
    cinfo.comp_info[0].height_in_blocks = height / cinfo.block_size;
    cinfo.comp_info[1].width_in_blocks = (width / 2) / cinfo.block_size;
    cinfo.comp_info[1].height_in_blocks = height / cinfo.block_size;
    cinfo.comp_info[2].width_in_blocks = (width / 2) / cinfo.block_size;
    cinfo.comp_info[2].height_in_blocks = height / cinfo.block_size;

    y_row_stride = cinfo.comp_info[0].width_in_blocks * cinfo.block_size;
    y_col_stride = cinfo.comp_info[0].height_in_blocks * cinfo.block_size;
    u_row_stride = cinfo.comp_info[1].width_in_blocks * cinfo.block_size;
    u_col_stride = cinfo.comp_info[1].height_in_blocks * cinfo.block_size;
    v_row_stride = cinfo.comp_info[2].width_in_blocks * cinfo.block_size;
    v_col_stride = cinfo.comp_info[2].height_in_blocks * cinfo.block_size;

    app_logi("y width=%d, height=%d, u width=%d, height=%d, v width=%d, height=%d\n",
             y_row_stride, y_col_stride, u_row_stride, u_col_stride, v_row_stride, v_col_stride);

    y_ptr = (JSAMPROW*)aic_malloc(sizeof(JSAMPROW *) * y_col_stride);
    if (!y_ptr) {
        app_loge("%s: no more memory\n", __func__);
        ret = -1;
        goto FAILURE;
    }
    for (int j = 0; j < y_col_stride; j++)
        y_ptr[j] = buffer_y + y_row_stride * j;

    u_ptr = (JSAMPROW*)aic_malloc(sizeof(JSAMPROW *) * u_col_stride);
    for (int j = 0; j < u_col_stride; j++)
        u_ptr[j] = buffer_u + u_row_stride * j;

    v_ptr = (JSAMPROW*)aic_malloc(sizeof(JSAMPROW *) * v_col_stride);
    for (int j = 0; j < v_col_stride; j++)
        v_ptr[j] = buffer_v + v_row_stride * j;

    yuvbuffer[0] = y_ptr;
    yuvbuffer[1] = u_ptr;
    yuvbuffer[2] = v_ptr;

    jpeg_start_compress(&cinfo, TRUE);

    while (cinfo.next_scanline < cinfo.image_height) {
        jpeg_write_raw_data(&cinfo, yuvbuffer, cinfo.comp_info[0].v_samp_factor * cinfo.block_size);
        yuvbuffer[0] += cinfo.comp_info[0].v_samp_factor * cinfo.block_size;
        yuvbuffer[1] += cinfo.comp_info[1].v_samp_factor * cinfo.block_size;
        yuvbuffer[2] += cinfo.comp_info[2].v_samp_factor * cinfo.block_size;
    }

    jpeg_finish_compress(&cinfo);

FAILURE:
    jpeg_destroy_compress(&cinfo);

    if (y_ptr)
        aic_free(y_ptr);
    if (u_ptr)
        aic_free(u_ptr);
    if (v_ptr)
        aic_free(v_ptr);

    if (buffer_y)
        aic_free(buffer_y);
    if (buffer_u)
        aic_free(buffer_u);
    if (buffer_v)
        aic_free(buffer_v);

    fflush(fp);
    fclose(fp);

    return 0;
}
