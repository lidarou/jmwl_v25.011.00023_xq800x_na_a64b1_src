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

#include "photo_converter.h"
#include "photo_object.h"
#include "fs_api.h"
#include "jpeg/jpeg-9e/jpeglib.h"
#include "aic_osa_ex.h"

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
/**
 * quadratic linear interpolation
 *         P0  P2
 *         P1  P3
 *	       pm0=(1-u)*(1-v);
 *		   pm1=v*(1-u);
 *		   pm2=u*(1-v);
 *		   pm3=u*v;
 *         dst = P0*pm0 + P1*pm1 + P2*pm2 + P3*pm3;
 * **/
static u8 photo_quadratic_linear_interpolation(u8 *pline1, u8 *pline2,
                                               int channels, int u, int v)
{
    int pm3 = u * v;
    int pm2 = (u << 8) - pm3;
    int pm1 = (v << 8) - pm3;
    int pm0 = (1 << 16) - pm1 - pm2 - pm3;

    u8 result = (u8)(((pm0 * pline1[0] + pm2 * pline1[channels] + pm1 * pline2[0]
                     + pm3 * pline2[channels]) >> 16) & 0xff);

    return result;
}

/**********************
 *   PUBLIC FUNCTIONS
 **********************/
/**
* photo_read_jpeg_header() - test if the file is a valid jpeg file
* @path: the full path of photo stored
* @attr: attributes read from the specified jpeg file
* Return: 0 means success, -1 means failure.
*/
int photo_read_jpeg_header(const char *path, struct photo_attr *attr)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int fd = -1;

    if (!path || !attr) {
        app_loge("%s: param invalid!\n", __func__);
        return -1;
    }

    fd = fs_open(path, FS_O_RDWR);
    if (fd < 0) {
        app_loge("%s: open jpeg file failed\n", __func__);
        return -1;
    }

    fs_seek(fd, 0, FS_SEEK_BEGIN);

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, &fd);

    (void)jpeg_read_header(&cinfo, 1);

    attr->width = cinfo.image_width;
    attr->height = cinfo.image_height;

    fs_close(fd);

    jpeg_destroy_decompress(&cinfo);

    return 0;
}

/**
 * photo_read_jpeg() - read the jpeg file attributes and data from the
 *                     specified path.
 * @path: the full path of photo stored
 * @attr: attributes read from the specified jpeg file
 * @size: jpeg file data size
 * Return: return the data decompressed from the specified jpeg file.
 */
u8 *photo_read_jpeg(const char *path, struct photo_attr *attr, u32 *size)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    u8 *img_data = NULL;
    u8 *row = NULL;
    u32 data_size = 0, h, w;
    int fd = -1;
    int ret = 0;

    if (!path || !attr) {
        app_loge("%s: param invalid!\n", __func__);
        return NULL;
    }

    fd = fs_open(path, FS_O_RDWR);
    if (fd < 0) {
        app_loge("%s: open jpeg file failed\n", __func__);
        return NULL;
    }

    fs_seek(fd, 0, FS_SEEK_BEGIN);

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, &fd);

    ret = jpeg_read_header(&cinfo, 1);
    if (ret != JPEG_HEADER_OK) {
        app_loge("%s: jpeg read header failed\n", __func__);
        goto FAILURE;
    }

    ret = jpeg_start_decompress(&cinfo);
    if (!ret) {
        app_loge("%s: jpeg start decompress failed\n", __func__);
        goto FAILURE;
    }

    w = attr->width = cinfo.output_width;
    h = attr->height = cinfo.output_height;

    data_size = w * h * cinfo.num_components;
    img_data = (u8 *)aic_malloc(data_size);
    if (!img_data) {
        app_loge("%s: image data alloc failed\n", __func__);
        goto FAILURE;
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        row = img_data + cinfo.output_scanline * w * cinfo.num_components;
        jpeg_read_scanlines(&cinfo, &row, 1);
    }

    *size = data_size;

    jpeg_finish_decompress(&cinfo);

FAILURE:
    fs_close(fd);

    jpeg_destroy_decompress(&cinfo);

    return img_data;
}

/**
 * photo_write_jpeg() - write the photo data to a jpeg file.
 * @path: the full path of photo to store.
 * @attr: attributes of the photo writting
 * @img_data: image data of the photo writting
 * @quality: image quality of the photo writting
 * Return: 0 means success, other means failure.
 */
int photo_write_jpeg(const char *path, const struct photo_attr *attr,
                     u8 *img_data, int quality)
{
    struct jpeg_compress_struct cinfo;
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
 * photo_write_jpeg_yuv420() - write the yuv422 data to a jpeg file.
 * @path: the full path of yuv data to store.
 * @attr: attributes of the yuv data writting
 * @img_data: image data of the yuv writting
 * Return: 0 means success, other means failure.
 */
int photo_write_jpeg_yuv422(const char *path, const struct photo_attr *attr,
                            u8 *img_data)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    u8 *buffer_y = 0, *buffer_u = 0, *buffer_v = 0;
    int width = 0, height = 0;
    FILE *fp = NULL;

    if (!path || !attr || !img_data) {
        app_loge("%s: param invalid!\n", __func__);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    width = attr->width;
    height = attr->height;

    buffer_y = (u8 *)aic_malloc(width * height);
    buffer_u = (u8 *)aic_malloc(width * height / 2);
    buffer_v = (u8 *)aic_malloc(width * height / 2);
    if (!buffer_y || !buffer_u || !buffer_v) {
        app_loge("%s: no more memory\n", __func__);
        return -1;
    }

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width * 2; col++) {
            if (col % 2 == 0)//1)
                buffer_y[row * width + (col - 1) / 2] = img_data[row * width * 2 + col];
            else if (col % 4 == 1)//0) /* U */
                buffer_u[row * width / 2 + col / 4] = img_data[row * width * 2 + col];
            else if (col % 4 == 3)//2) /* V */
                buffer_v[row * width / 2 + col / 4] = img_data[row * width * 2 + col];
        }
    }

    fp = fopen(path, "wb+");
    if (!fp) {
        app_loge("%s: open jpeg file failed, %s\n", __func__, path);
        jpeg_destroy_compress(&cinfo);
        aic_free(buffer_y);
        aic_free(buffer_u);
        aic_free(buffer_v);
        return -1;
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

    JSAMPARRAY yuvbuffer[3];
    JSAMPARRAY y_ptr;
    JSAMPARRAY u_ptr;
    JSAMPARRAY v_ptr;

    int y_row_stride, y_col_stride;
    int u_row_stride, u_col_stride, v_row_stride, v_col_stride;

    y_row_stride = cinfo.comp_info[0].width_in_blocks * cinfo.block_size;
    y_col_stride = cinfo.comp_info[0].height_in_blocks * cinfo.block_size;
    u_row_stride = cinfo.comp_info[1].width_in_blocks * cinfo.block_size;
    u_col_stride = cinfo.comp_info[1].height_in_blocks * cinfo.block_size;
    v_row_stride = cinfo.comp_info[2].width_in_blocks * cinfo.block_size;
    v_col_stride = cinfo.comp_info[2].height_in_blocks * cinfo.block_size;

    app_logi("y width=%d, height=%d, u width=%d, height=%d, v width=%d, height=%d\n",
             y_row_stride, y_col_stride, u_row_stride, u_col_stride, v_row_stride, v_col_stride);

    y_ptr = (JSAMPROW*)aic_malloc(sizeof(JSAMPROW *) * y_col_stride);
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

    aic_free(y_ptr);
    aic_free(u_ptr);
    aic_free(v_ptr);
 
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    aic_free(buffer_y);
    aic_free(buffer_u);
    aic_free(buffer_v);

    fflush(fp);
    fclose(fp);

    return 0;
}

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
                    const struct photo_attr *s_attr, u8 *img_data, u32 *size)
{
    int d_w, d_h, s_w, s_h;
    int B, N, x, y;
    int pixel_size = 0;
    int img_size = 0;
    u8 *line_pre, *line_next;
    u8 *d_img_data = NULL;
    u8 *a, *b, *c, *d;
    u8 *cur;

    if (!d_attr || !s_attr || !img_data) {
        app_loge("%s: param invalid!\n", __func__);
        return 0;
    }

    d_w = d_attr->width - 1;
    d_h = d_attr->height - 1;
    s_w = s_attr->width - 1;
    s_h = s_attr->height - 1;

    pixel_size = depth >> 3;

    img_size = d_attr->width * d_attr->height * pixel_size;
    d_img_data = (u8 *)aic_malloc(img_size);
    if (!d_img_data) {
        app_loge("%s: aic_malloc failed\n", __func__);
        return 0;
    }
    *size = img_size;

    for (int i = 0; i < d_h; ++i) {
        cur = d_img_data + (i * d_attr->width * pixel_size);
        y = (i * s_h) / d_h;
        N = d_h - (i * s_h) % d_h;
        line_pre = img_data + (y * s_attr->width * pixel_size);
        y++;
        line_next =
            (N == d_h) ? line_pre : (img_data + y * s_attr->width * pixel_size);

        for (int j = 0; j < d_w; ++j) {
            x = j * s_w / d_w * pixel_size;
            B = d_w - j * s_w % d_w;
            a = line_pre + x;
            b = a + pixel_size;
            c = line_next + x;
            d = c + pixel_size;
            if (B == d_w) {
                b = a;
                d = c;
            }

            for (int k = 0; k <= pixel_size; ++k) {
                *cur++ = (u8)(int)((B * N * (*a - (*b) - (*c) + (*d)) +
                                    d_w * N * (*b) + d_h * B * (*c) +
                                    (d_w * d_h - d_h * B - d_w * N) * (*d) +
                                    d_w * d_h / 2) / (d_w * d_h));
                a++;
                b++;
                c++;
                d++;
            }
        }
    }

    return d_img_data;
}

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
                       const struct photo_attr *s_attr, u8 *img_data, u32 *size)
{
    u8 *pos, *out_data, *pt;
    u8 *cur = img_data;
    int pixel_size = 0;
    int src_byte_per_line = 0;
    int dst_byte_per_line = 0;
    int srcy_16 = 0, v_8 = 0, srcx_16 = 0;
    int stepw = 0, steph = 0;
    int img_size = 0;

    if (!d_attr || !s_attr || !img_data) {
        app_loge("%s: param invalid\n", __func__);
        return NULL;
    }

    pixel_size = depth >> 3;
    if (pixel_size != 1 && pixel_size != 3) {
        app_loge("%s: pixel size %d error\n", __func__, pixel_size);
        return NULL;
    }

    src_byte_per_line = s_attr->width * pixel_size;
    dst_byte_per_line = d_attr->width * pixel_size;
    stepw = ((s_attr->width - 1) << 16) / d_attr->width + 1;
    steph = ((s_attr->height - 1) << 16) / d_attr->height + 1;

    img_size = dst_byte_per_line * d_attr->height;
    out_data = (u8 *)aic_malloc(img_size);
    if (!out_data) {
        app_loge("%s: no more memory\n", __func__);
        return NULL;
    }
    pos = out_data;
    *size = img_size;

    srcy_16 = 0;
    for (int j = 0; j < d_attr->height; j++) {
        v_8 = (srcy_16 & 0xFFFF) >> 8;
        cur = img_data + src_byte_per_line * (srcy_16 >> 16);
        srcx_16 = 0;
        for (int i = 0; i < dst_byte_per_line; i += pixel_size) {
            pt = &cur[(srcx_16>>16)*pixel_size];

            for (int m = 0; m < pixel_size; m++) {
                pos[i+m] = photo_quadratic_linear_interpolation(&pt[m], &pt[m] + src_byte_per_line,
                                                                pixel_size, (srcx_16 & 0xFFFF) >> 8, v_8);
            }

            srcx_16 += stepw;
        }
        srcy_16 += steph;

        pos += dst_byte_per_line;
    }

    return out_data;
}

/**
 * photo_rgb888_to_rgb565() - RGB888 to RGB565 format.
 * @rgb888: pointer to RGB888 data
 * @rgb888_size: size of RGB888 data
 * @rgb565: point to RGB565
 * @rgb565_size: size of RGB565
 * Return: return rbg565 data size, 0 means failure.
 */
int photo_rgb888_to_rgb565(u8 *rgb888, u32 rgb888_size, u16 *rgb565,
                           u32 rgb565_size)
{
    u8 red = 0, green = 0, blue = 0;
    int count = 0;

    if (!rgb888 || !rgb888_size || !rgb565 || !rgb565_size ||
        rgb565_size < ((rgb888_size / 3) * 2)) {
        app_loge("%s: param invalid!\n", __func__);
        return 0;
    }

    for (int i = 0; i < rgb888_size; i += 3) {
        red = rgb888[i] >> 3;
        green = rgb888[i + 1] >> 2;
        blue = rgb888[i + 2] >> 3;

        rgb565[count++] = (red << 11) | (green << 5) | blue;
    }

    return count;
}
