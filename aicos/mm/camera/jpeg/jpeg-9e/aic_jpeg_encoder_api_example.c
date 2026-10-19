/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01e024
#define LOG_MODULE    AMOD_CAMERA

#include <stdio.h>
#include <stdlib.h>
#define    HAVE_BOOLEAN
#include <jpeglib.h>

#include <aic_init.h>
#include <aic_errno.h>
#include <aic_osa.h>

#include <timer_inc.h>

#include <display_cfg.h>

#include <camera_fb.h>
#include <camera_hal.h>

/* yuv422 to jpeg example */
int yuv422_to_jpeg(unsigned char *src_image, int width, int height, unsigned char **jpeg_img)
{
    unsigned char *y_u_v_buf = AIC_NULL;
    struct jpeg_compress_struct cinfo;
    unsigned char yuvbuf[width * 3];
    int i = 0, j = 0, row_stride;
    unsigned char *py, *pu, *pv;
    struct jpeg_error_mgr jerr;
    size_t jpeg_img_size = 0;
    JSAMPROW row_pointer[1];

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, jpeg_img, &jpeg_img_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_defaults(&cinfo);

    cinfo.optimize_coding = AIC_TRUE;
    jpeg_set_quality(&cinfo, 80, AIC_TRUE);

    jpeg_start_compress(&cinfo, AIC_TRUE);

    row_stride = cinfo.image_width * 3;

    y_u_v_buf = aos_dma_malloc(cinfo.image_width * cinfo.image_height * cinfo.input_components);
    /* split yuyv to Y U V plane */
    py = y_u_v_buf;
    pu = y_u_v_buf + cinfo.image_width * cinfo.image_height;
    pv = y_u_v_buf + cinfo.image_width * cinfo.image_height * 3 / 2;

    for (i = 0; i < cinfo.image_height; i++) {
        for (j = 0; j < cinfo.image_width * 2; j++) {
            if (j % 2 == 0)
                py[i * cinfo.image_width + j/2] = src_image[i * cinfo.image_width * 2 + j];
            else if (j % 4 == 1) /* U */
                pu[i * cinfo.image_width/2 + j/4] = src_image[i * cinfo.image_width * 2 + j];
            else if (j % 4 == 3) /* V */
                pv[i * cinfo.image_width/2 + j/4] = src_image[i * cinfo.image_width * 2 + j];
        }
    }

    j = 0;
    while (cinfo.next_scanline < cinfo.image_height) {
        pu = y_u_v_buf + cinfo.image_width * cinfo.image_height + j * cinfo.image_width / 2;
        pv = y_u_v_buf + cinfo.image_width * cinfo.image_height * 3 / 2  + j * cinfo.image_width / 2;

        for (i = 0; i < cinfo.image_width; i += 2) {
            yuvbuf[i*3] = *py++;
            yuvbuf[i*3 + 1] = *pu;
            yuvbuf[i*3 + 2] = *pv;

            yuvbuf[i*3 + 3] = *py++;
            yuvbuf[i*3 + 4] = *pu++;
            yuvbuf[i*3 + 5] = *pv++;
        }

        row_pointer[0] = yuvbuf;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        j++;
    }

    jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);

    aos_dma_free(y_u_v_buf);

    return jpeg_img_size;
}

/* yuv420 to jpeg example */
int nv12_to_jpeg(unsigned char *src_img, int width, int height, unsigned char **jpeg_img)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    size_t jpeg_img_size = 0;
    JSAMPROW row_pointer[1];
    int i = 0, j = 0, row_stride;
    unsigned char yuvbuf[width * 3];
    unsigned char *py, *pu;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    jpeg_mem_dest(&cinfo, jpeg_img, &jpeg_img_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, 80, AIC_TRUE);

    jpeg_start_compress(&cinfo, AIC_TRUE);

    row_stride = cinfo.image_width * 3;

    py = (unsigned char *)src_img;
    pu = (unsigned char *)src_img + cinfo.image_width * cinfo.image_height;

    while (cinfo.next_scanline < cinfo.image_height) {
        int idx = 0;
        for (i = 0; i < cinfo.image_width; i++) {
            yuvbuf[idx++] = py[i + j * cinfo.image_width];
            yuvbuf[idx++] = pu[j/2*cinfo.image_width + (i/2)*2];
            yuvbuf[idx++] = pu[j/2*cinfo.image_width + (i/2)*2 + 1];
        }
        row_pointer[0] = yuvbuf;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        j++;
    }

    jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);

    return AIC_SUCCESS;
}

/* y only to jpeg example */
int y_only_to_jpeg(unsigned char *src_img, int width, int height, unsigned char **jpeg_img)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    size_t jpeg_img_size = 0;
    JSAMPROW row_pointer[1];
    int i = 0, row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    jpeg_mem_dest(&cinfo, jpeg_img, &jpeg_img_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 1;
    cinfo.in_color_space = JCS_GRAYSCALE;
    cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, 80, AIC_TRUE);

    jpeg_start_compress(&cinfo, AIC_TRUE);

    row_stride = cinfo.image_width * 1;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = src_img + i * cinfo.image_width;
        i++;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);

    return jpeg_img_size;
}

