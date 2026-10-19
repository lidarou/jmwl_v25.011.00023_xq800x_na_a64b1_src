/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b03a5
#define LOG_MODULE    AMOD_CAMERA
#include <stdio.h>
#include <stdlib.h>
#define    HAVE_BOOLEAN
#include <jpeglib.h>

#include <aic_init.h>
#include <aic_errno.h>

#include <fs_api.h>

#include <timer_inc.h>

#include <display_cfg.h>

#include <camera_fb.h>
#include <camera_hal.h>

extern u32 csi_rgb565_dst[38400];
/* decompress result is RGB888 */
u32 rgb888_buffer[57600] __attribute__((aligned(CACHE_LINE_SIZE)));
u32 yuv420_buffer[28800] __attribute__((aligned(CACHE_LINE_SIZE)));
u32 y_u_v_buffer[28800] __attribute__((aligned(CACHE_LINE_SIZE)));

#define FS_FLASH_CAMERA_TEST_FILE_NAME      "\\test.jpg"

unsigned char *jpeg_dst_demo = AIC_NULL;
size_t jpeg_dst_size_demo = 0;
struct jpeg_compress_struct cinfo;
struct jpeg_decompress_struct dinfo;
u32 jc_t1, jc_t2, jc_delta;

int jpeg_img_decode_to_mem_demo(void)
{
    int row_size;
    char *pline, *point;
    struct jpeg_error_mgr jerr;
#ifdef CONFIG_FS_DEV_FLASH
    int ret;
    char *file_name = AIC_NULL;
    FILE *stream;
#endif

    dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);

#ifdef CONFIG_FS_DEV_FLASH
    /* open jpeg file */
    file_name = aos_zalloc(50);
    aos_memcpy(file_name, FS_FLASH_CAMERA_TEST_FILE_NAME,
               strlen(FS_FLASH_CAMERA_TEST_FILE_NAME));
    stream = fopen(file_name, "r");
    AOS_ASSERT(stream);
    ret = fseek(stream, 0, 0);
    AOS_ASSERT(ret == 0);

    /* set jpeg src file */
    jpeg_stdio_src(&dinfo, stream);
#else
    jpeg_mem_src(&dinfo, jpeg_dst_demo, jpeg_dst_size_demo);
#endif

    /* read file header */
    jpeg_read_header(&dinfo, AIC_TRUE);

    /* set decompress parameters */
    dinfo.out_color_space = JCS_YUYV;

    /* start decompress */
    jpeg_start_decompress(&dinfo);

    /* malloc one line buffer */
    if (dinfo.out_color_space == JCS_YUYV)
        row_size = dinfo.output_width * (dinfo.output_components - 1);
    else
        row_size =  dinfo.output_width * dinfo.output_components;
    pline = (char *)aos_dma_malloc(row_size);
    aos_memset(rgb888_buffer, 0x0, sizeof(rgb888_buffer));
    point = (char *)rgb888_buffer;
    /* read line by line */
    while(dinfo.output_scanline < dinfo.output_height) {
        /* read one line */
        jpeg_read_scanlines(&dinfo, (JSAMPARRAY)&pline, 1);
        aos_memcpy(point, pline, row_size);
        point += row_size;
    }
    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);
    //yuv422_yuyv_to_yuv420_y_uv((unsigned char*)rgb888_buffer, (unsigned char *)y_u_v_buffer, dinfo.output_width, dinfo.output_height);
    aos_dma_free(pline);
#ifdef CONFIG_FS_DEV_FLASH
    fclose(stream);
    aos_free(file_name);
#else
    aos_dma_free(jpeg_dst_demo);
    jpeg_dst_demo = AIC_NULL;
#endif

    return AIC_SUCCESS;
}

int yuv420_to_jpeg(char *src_image, int width, int height)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int i = 0, j = 0, row_stride;
    unsigned char yuvbuf[width * 3];
    unsigned char *py, *pu, *pv;

    aos_memset(yuv420_buffer, 0, sizeof(yuv420_buffer));
    yuv422_yuyv_to_yuv420_y_uv(src_image, (unsigned char *)yuv420_buffer, width, height);
    aos_memset(y_u_v_buffer, 0, sizeof(y_u_v_buffer));
    nv12_to_i420((unsigned char *)yuv420_buffer, (unsigned char *)y_u_v_buffer, width, height);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, (unsigned char **)&jpeg_dst_demo, &jpeg_dst_size_demo);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, 99, AIC_TRUE);

    jpeg_start_compress(&cinfo, AIC_TRUE);

    row_stride = cinfo.image_width * 3;

    py = (unsigned char *)y_u_v_buffer;
    pu = (unsigned char *)y_u_v_buffer + cinfo.image_width * cinfo.image_height;
    pv = (unsigned char *)y_u_v_buffer + cinfo.image_width * cinfo.image_height + cinfo.image_width * cinfo.image_height / 4;

    while (cinfo.next_scanline < cinfo.image_height) {
        pu = (unsigned char *)y_u_v_buffer + cinfo.image_width * cinfo.image_height + j * cinfo.image_width / 4;
        pv = (unsigned char *)y_u_v_buffer + cinfo.image_width * cinfo.image_height * 5 / 4 + j * cinfo.image_width / 4;

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

    return AIC_SUCCESS;
}

int save_to_jpeg_img(char *src_image)
{
    struct jpeg_error_mgr jerr;
    //JSAMPROW row_pointer[1]; /* pointer to a single row */
    int j, row, col;
    u8 *buffer_y, *buffer_u, *buffer_v;
    JSAMPARRAY yuvbuffer[3];
    JSAMPARRAY y_ptr;
    JSAMPARRAY u_ptr;
    JSAMPARRAY v_ptr;
    int y_row_stride, y_col_stride;
    int u_row_stride, u_col_stride, v_row_stride, v_col_stride;
    struct camera_fb_info *fb_info;

    int ret = AIC_SUCCESS;
#ifdef CONFIG_FS_DEV_FLASH
    ulong actual;
    char *file_name = AIC_NULL;
    FILE *stream;
    struct stat buf;

    file_name = aos_zalloc(50);
    aos_memcpy(file_name, FS_FLASH_CAMERA_TEST_FILE_NAME,
               strlen(FS_FLASH_CAMERA_TEST_FILE_NAME));
    stream = fopen(file_name, "w+");
    AOS_ASSERT(stream);
    ret = fseek(stream, 0, 0);
    AOS_ASSERT(ret == 0);
#endif

    jc_t1 = aic_get_timestamp();
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
#ifdef CONFIG_FS_DEV_FLASH
    jpeg_stdio_dest(&cinfo, stream);
#else
    jpeg_mem_dest(&cinfo, (unsigned char **)&jpeg_dst_demo, &jpeg_dst_size_demo);
#endif
    cinfo.image_width = 240;
    cinfo.image_height = 320;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;

    fb_info = camera_get_fb_info(0);

    /*buffer_y = (u8 *)malloc(cinfo.image_width * cinfo.image_height);
    buffer_u = (u8 *)malloc(cinfo.image_width * cinfo.image_height/2);
    buffer_v = (u8 *)malloc(cinfo.image_width * cinfo.image_height/2);*/
    buffer_y = (u8 *)fb_info->fb_addr;
    buffer_u = (u8 *)(fb_info->fb_addr + fb_info->fb_size/2);
    buffer_v = (u8 *)(fb_info->fb_addr + fb_info->fb_size/2 + fb_info->fb_size/4);

    for (row = 0; row < cinfo.image_height; row++) {
        for (col = 0; col < cinfo.image_width * 2; col++) {
            if (col % 2 == 0)
                buffer_y[row * cinfo.image_width + col/2] = src_image[row * cinfo.image_width * 2 + col];
            else if (col % 4 == 1) /* U */
                buffer_u[row * cinfo.image_width/2 + col/4] = src_image[row * cinfo.image_width * 2 + col];
            else if (col % 4 == 3) /* V */
                buffer_v[row * cinfo.image_width/2 + col/4] = src_image[row * cinfo.image_width * 2 + col];
        }
    }

    jpeg_set_defaults(&cinfo);
    cinfo.raw_data_in = TRUE;
    cinfo.do_fancy_downsampling = FALSE;

    /* for Y */
    cinfo.comp_info[0].h_samp_factor = 2;
    cinfo.comp_info[0].v_samp_factor = 1;
    /* for Cb */
    cinfo.comp_info[1].h_samp_factor = 1;
    cinfo.comp_info[1].v_samp_factor = 1;
    /* for Cr */
    cinfo.comp_info[2].h_samp_factor = 1;
    cinfo.comp_info[2].v_samp_factor = 1;

    cinfo.comp_info[0].width_in_blocks = cinfo.image_width / cinfo.block_size;
    cinfo.comp_info[0].height_in_blocks = cinfo.image_height / cinfo.block_size;
    cinfo.comp_info[1].width_in_blocks = (cinfo.image_width / 2) / cinfo.block_size;
    cinfo.comp_info[1].height_in_blocks = cinfo.image_height / cinfo.block_size;
    cinfo.comp_info[2].width_in_blocks = (cinfo.image_width / 2) / cinfo.block_size;
    cinfo.comp_info[2].height_in_blocks = cinfo.image_height / cinfo.block_size;

    y_row_stride = cinfo.comp_info[0].width_in_blocks * cinfo.block_size;
    y_col_stride = cinfo.comp_info[0].height_in_blocks * cinfo.block_size;
    u_row_stride = cinfo.comp_info[1].width_in_blocks * cinfo.block_size;
    u_col_stride = cinfo.comp_info[1].height_in_blocks * cinfo.block_size;
    v_row_stride = cinfo.comp_info[2].width_in_blocks * cinfo.block_size;
    v_col_stride = cinfo.comp_info[2].height_in_blocks * cinfo.block_size;

    y_ptr = (JSAMPROW *)aos_dma_malloc(sizeof(JSAMPROW *) * y_col_stride);
    for (j = 0; j < y_col_stride; j++)
        y_ptr[j] = buffer_y + y_row_stride *j;

    u_ptr = (JSAMPROW *)aos_dma_malloc(sizeof(JSAMPROW *) * u_col_stride);
    for (j = 0; j < u_col_stride; j++)
        u_ptr[j] = buffer_u + u_row_stride * j;

    v_ptr = (JSAMPROW *)aos_dma_malloc(sizeof(JSAMPROW *) * v_col_stride);
    for (j = 0; j < v_col_stride; j++)
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
    aos_dma_free(y_ptr);
    aos_dma_free(u_ptr);
    aos_dma_free(v_ptr);
    v_ptr = u_ptr = y_ptr = AIC_NULL;
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    /*row_stride = cinfo.image_width * 2;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &src_image[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }*/
#ifdef CONFIG_FS_DEV_FLASH
    /* need seek to the begining of file */
    ret = fseek(stream, 0, 0);
    AOS_ASSERT(ret == 0);
    /* get jpeg file size */
    ret = stat(file_name, &buf);
    AOS_ASSERT(ret == AIC_SUCCESS);
    jpeg_dst_size_demo = buf.st_size;
    jpeg_dst_demo = aos_dma_malloc(jpeg_dst_size_demo);
    ret = fread(jpeg_dst_demo, 1, jpeg_dst_size_demo, stream);
    AOS_ASSERT(ret >= 0);
    actual = ret;

    /*ret = fs_flush(fd);
    AOS_ASSERT(ret == AIC_SUCCESS);*/

    ret = fclose(stream);
    AOS_ASSERT(ret == AIC_SUCCESS);

    aos_free(file_name);

    aos_dma_free(jpeg_dst_demo);
    jpeg_dst_demo = AIC_NULL;
#endif

    jc_t2 = aic_get_timestamp();
    jc_delta = jc_t2 - jc_t1;
    //jpeg_destroy_compress(&cinfo);

    return ret;
}

static aos_thread_id jpeg_thread_id = AOS_INVALID_THREAD_ID;

static void jpeg_thread_entry(void *data)
{
    int ret;

    ret = camera_open(CAMERA_FRONT);

    if (ret == AIC_SUCCESS) {
        camera_capture_prepare(CAMERA_FRONT, YUV_ONE_PLANE, YUV422, CAMERA_SENSOR_OUTPUT_WIDTH, CAMERA_SENSOR_OUTPUT_HEIGHT);
        camera_capture(CAMERA_FRONT, (u32)csi_rgb565_dst);
        camera_close(CAMERA_FRONT);
        //yuv420_to_jpeg((char *)csi_rgb565_dst, CAMERA_SENSOR_OUTPUT_WIDTH, CAMERA_SENSOR_OUTPUT_HEIGHT);
        save_to_jpeg_img((char *)csi_rgb565_dst);
        jpeg_img_decode_to_mem_demo();
    }
}

int jpeglib_demo_init(void)
{
    jpeg_thread_id = aos_create_thread_simply("jpeg thread",
                                              jpeg_thread_entry,
                                              NULL, 3072, 30);

    return AIC_SUCCESS;
}

AIC_NORMAL_APP_L2_INIT_EXPORT(jpeglib_demo_init);
