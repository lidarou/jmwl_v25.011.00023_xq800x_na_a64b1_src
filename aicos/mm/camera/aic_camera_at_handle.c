/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Camera AT Cmds Parsing Component.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00ade8
#define LOG_MODULE    AMOD_CAMERA

#include <aic_init.h>
#include <aic_type.h>
#include <aic_osa.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_errno.h>
#include <aic_at_process_common.h>

#include <aic_bitmap.h>
#include <display_cfg.h>
#include <display.h>

#ifdef CONFIG_LIBJPEG_SUPPORT
#define    HAVE_BOOLEAN
#include <jpeglib.h>
#endif

#include "camera_hal.h"

#define CAMERA_MAX_AT_RESP_LENGTH        64UL
#define CAMERA_REPLY_LINE_COUNT_ONCE     2UL

u32 aic_camera_open_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[CAMERA_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[CAMERA_REPLY_LINE_COUNT_ONCE];

    snprintf(result_line0, CAMERA_MAX_AT_RESP_LENGTH, "+QCAMOPEN: (240-640),(320-480)");
    strcpy(result_line1, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

    return AIC_SUCCESS;
}

extern u32 csi_rgb565_dst[];

void at_preview_refresh(u32 frame_addr, int width, int height, camera_data_fmt_e format)
{
    enum SCL_MODE scl_mode;
    enum YUYV_MODE yuv_mode;

    if ((width == LCD_PANEL_WIDTH) && (height == LCD_PANEL_HEIGHT))
        scl_mode = NO_SCALER;
    else if ((width / LCD_PANEL_WIDTH == 2) && (height / LCD_PANEL_HEIGHT == 2))
        scl_mode = BINNING;
    else
        scl_mode = BILINEAR_SCALER;

    if (format == YUV422)
        yuv_mode = MODE_YUYV;
    else
        yuv_mode = MODE_Y;

    /* convert to rgb565 */
    yuv422_to_rgb565((unsigned char*)frame_addr, AIC_NULL,
                     width, height,
                     (unsigned short*)csi_rgb565_dst,
                     LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT,
                     yuv_mode, scl_mode,
                     0);

    display_refresh_sync((u32)csi_rgb565_dst);
}

u32 aic_camera_open_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u16 paras_count = 0;
    int ret;
    ulong *paras_arr;
    u8 result_line[CAMERA_MAX_AT_RESP_LENGTH] = {'\0'};
    ulong line_pointers[CAMERA_REPLY_LINE_COUNT_ONCE];

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT+QCAMOPEN=", &paras_count);
    if (paras_arr) {
        if (paras_count == 2) {
            if ((atoi((u8 *) paras_arr[0]) <= CAMERA_SENSOR_OUTPUT_WIDTH) &&
                (atoi((u8 *) paras_arr[1]) <= CAMERA_SENSOR_OUTPUT_HEIGHT)) {
                ret = camera_open(CAMERA_FRONT);
                if (ret < 0) {
                    aicx_error(0x0112b6, "", "camera open set cmd failed!\n");
                    goto PARA_ERROR;
                }
                /* TODO:Use Camera service level API, no need care refresh logic */
                camera_register(at_preview_refresh);
            } else {
                aicx_error(0x181f7d, "", "camera width or height set error!\n");
                goto PARA_ERROR;
            }
        } else {
            aicx_error(0x1b1a7d, "", "camera open set cmd parameter cnt error!\n");
            goto PARA_ERROR;
        }
    }

    aic_free_paras_mem(paras_arr, paras_count);
    strcpy(result_line, "OK");
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;

PARA_ERROR:
    aic_free_paras_mem(paras_arr, paras_count);
    snprintf(result_line, sizeof(result_line),
             "+QCAMOPEN ERROR: %d", AT_ERROR_INVALID_PARAS);
    line_pointers[0] = (ulong)&result_line[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

u32 aic_camera_close_test_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[3] = {'\0'};
    ulong line_pointers[CAMERA_REPLY_LINE_COUNT_ONCE];

    strcpy(result_line0, "OK");
    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

u32 aic_camera_close_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[3] = {'\0'};
    ulong line_pointers[CAMERA_REPLY_LINE_COUNT_ONCE];

    camera_close(CAMERA_FRONT);

    strcpy(result_line0, "OK");
    /* fill response,one lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return AIC_SUCCESS;
}

#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
u32 at_camera_cap_addr;
#else
u32 at_camera_cap_addr[38400] __attribute__((aligned(CACHE_LINE_SIZE)));
#endif

#ifdef CONFIG_LIBJPEG_SUPPORT
unsigned char *yuyv_plane_addr = AIC_NULL;
/*unsigned char yuyv_plane_addr[153600] __attribute__((aligned(CACHE_LINE_SIZE)));*/

unsigned char *jpeg_dst = AIC_NULL;
size_t jpeg_dst_size = 0;

int yuyv_to_jpeg(unsigned char *src_image, int width, int height, camera_data_fmt_e fmt)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int i = 0, j = 0, row_stride;
    unsigned char yuvbuf[width * 3];
    unsigned char *py, *pu, *pv;

    if ((fmt != YUV422) && (fmt != Y)) {
        aicx_error(0x1e0f77, "", "only support yuv422 or y only now!\n");
        return AIC_ENOSYS;
    }

    if (fmt == Y)
        yuyv_plane_addr = aos_dma_malloc(g_camera_info.cam_stream_info.img_size * 2);
    else
        yuyv_plane_addr = aos_dma_malloc(g_camera_info.cam_stream_info.img_size);
    if (yuyv_plane_addr == AIC_NULL) {
        aicx_error(0x0ed443, "", "no enough memory, malloc yuyv_plane_addr failed!\n");
        return AIC_ENOMEM;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, (unsigned char **)&jpeg_dst, &jpeg_dst_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_defaults(&cinfo);

    cinfo.optimize_coding = AIC_TRUE;
    jpeg_set_quality(&cinfo, 99, AIC_TRUE);

    jpeg_start_compress(&cinfo, AIC_TRUE);

    row_stride = cinfo.image_width * 3;

    /* split yuyv to Y U V plane */
    py = yuyv_plane_addr;
    pu = yuyv_plane_addr + cinfo.image_width * cinfo.image_height;
    pv = yuyv_plane_addr + cinfo.image_width * cinfo.image_height * 3 / 2;

    if (fmt == YUV422) {
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
    }

    if (fmt == Y) {
        aos_memcpy(py, src_image, g_camera_info.cam_stream_info.img_size);
        aos_memset(pu, 0x7f, cinfo.image_width * cinfo.image_height);
    }

    j = 0;
    while (cinfo.next_scanline < cinfo.image_height) {
        pu = yuyv_plane_addr + cinfo.image_width * cinfo.image_height + j * cinfo.image_width / 2;
        pv = yuyv_plane_addr + cinfo.image_width * cinfo.image_height * 3 / 2  + j * cinfo.image_width / 2;

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

    aos_dma_free(yuyv_plane_addr);

    return AIC_SUCCESS;
}

u32 aic_camera_testblt_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[CAMERA_MAX_AT_RESP_LENGTH] = {'\0'};
    ulong line_pointers[CAMERA_REPLY_LINE_COUNT_ONCE];
    u8 result_line1[3] = {'\0'};
    camera_data_fmt_e fmt;
    int ret;
#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    void *addr;
#endif

    ret = camera_get_sensor_output_format(CAMERA_FRONT, &fmt);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x1e13d3, "d", "camera %d get sensor output format failed!\n", CAMERA_FRONT);
        goto CAPTURE_ERROR;
    }

    ret = camera_capture_prepare(CAMERA_FRONT, YUV_ONE_PLANE, fmt, CAMERA_SENSOR_OUTPUT_WIDTH, CAMERA_SENSOR_OUTPUT_HEIGHT);
    if (ret < 0) {
        aicx_error(0x14143b, "d", "camera %d capture prepare failed!\n", CAMERA_FRONT);
        goto CAPTURE_ERROR;
    }

#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    addr = aos_dma_malloc(g_camera_info.cam_stream_info.img_size);
    if (addr == AIC_NULL) {
        ret = AIC_ENOMEM;
        aicx_error(0x019a31, "", "no enough memory, malloc at_camera_cap_addr failed!\n");
        goto MEM_ALLOC_FAIL;
    }

    memset(addr, 0x5f, g_camera_info.cam_stream_info.img_size);

    at_camera_cap_addr = (u32)addr;
#else
    memset(at_camera_cap_addr, 0x5f, g_camera_info.cam_stream_info.img_size);
#endif

    ret = camera_capture(CAMERA_FRONT, (u32)at_camera_cap_addr);
    if (ret < 0) {
        aicx_error(0x0b065b, "d", "camera %d capture failed!\n", CAMERA_FRONT);
        goto CAPTURE_ERROR;
    }

    ret = yuyv_to_jpeg((unsigned char *)at_camera_cap_addr, g_camera_info.cam_stream_info.width, g_camera_info.cam_stream_info.height,
                       g_camera_info.cam_stream_info.fmt);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x10ea3b, "", "yuv to jpeg at camera testblt failed!\n");
        goto CAPTURE_ERROR;
    }

    sprintf(result_line0, "QCAMTEST result addr: %x, size: %d",
            (u32)jpeg_dst, jpeg_dst_size);

    ret = aic_transmit_at_data(at_req_res_ptr->serial_port_id, (u8 *)jpeg_dst, (u32)jpeg_dst_size);
    aos_free(jpeg_dst);
    jpeg_dst = AIC_NULL;
    if (ret != AIC_SUCCESS) {
        aicx_error(0x158600, "", "aic transmit jpeg dst through AT interface failed!\n");
        goto CAPTURE_ERROR;
    }

    strcpy(result_line1, "OK");
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    aos_dma_free(addr);
#endif

    return AIC_SUCCESS;

CAPTURE_ERROR:
#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    aos_dma_free(addr);

MEM_ALLOC_FAIL:
#endif
    snprintf(result_line0, sizeof(result_line0),
             "+QCAMTEST ERROR: %d", AT_ERROR_HANDLE_FAILURE);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return ret;
}
#endif

u32 aic_camera_test_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[CAMERA_MAX_AT_RESP_LENGTH] = {'\0'};
    u8 result_line1[3] = {'\0'};
    ulong line_pointers[CAMERA_REPLY_LINE_COUNT_ONCE];
    int i, j, ret, *tmp_addr;
    camera_data_fmt_e fmt;
#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    void *addr;
#endif

    ret = camera_get_sensor_output_format(CAMERA_FRONT, &fmt);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x1e13d3, "d", "camera %d get sensor output format failed!\n", CAMERA_FRONT);
        goto CAPTURE_ERROR;
    }

    ret = camera_capture_prepare(CAMERA_FRONT, YUV_ONE_PLANE, fmt, CAMERA_SENSOR_OUTPUT_WIDTH, CAMERA_SENSOR_OUTPUT_HEIGHT);
    if (ret < 0) {
        aicx_error(0x14143b, "d", "camera %d capture prepare failed!\n", CAMERA_FRONT);
        goto CAPTURE_ERROR;
    }

#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    addr = aos_dma_malloc(g_camera_info.cam_stream_info.img_size);
    if (addr == AIC_NULL) {
        ret = AIC_ENOMEM;
        aicx_error(0x019a31, "", "no enough memory, malloc at_camera_cap_addr failed!\n");
        goto MEM_ALLOC_FAIL;
    }

    memset(addr, 0x5f, g_camera_info.cam_stream_info.img_size);

    tmp_addr = (int *)addr;
    at_camera_cap_addr = (u32)addr;
#else
    memset(at_camera_cap_addr, 0x5f, g_camera_info.cam_stream_info.img_size);
    tmp_addr = (int *)at_camera_cap_addr;
#endif

    ret = camera_capture(CAMERA_FRONT, (u32)at_camera_cap_addr);
    if (ret < 0) {
        aicx_error(0x0b065b, "d", "camera %d capture failed!\n", CAMERA_FRONT);
        goto CAPTURE_ERROR;
    }

    j = 0;
    for (i = 0; i <= g_camera_info.cam_stream_info.width; i++)
        if (tmp_addr[i] == 0x5f5f5f5f)
            j++;

    /* if the half width are all invaid data, error occurs */
    if (j == i) {
        aicx_error(0x0c9904, "d", "camera %d cannot capture image!\n", CAMERA_FRONT);
        ret = AIC_EIO;
        goto CAPTURE_ERROR;
    }

#ifdef CONFIG_LIBJPEG_SUPPORT
    ret = yuyv_to_jpeg((unsigned char *)at_camera_cap_addr, g_camera_info.cam_stream_info.width, g_camera_info.cam_stream_info.height,
                       g_camera_info.cam_stream_info.fmt);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0ca35c, "", "yuv to jpeg at camera test failed!\n");
        goto CAPTURE_ERROR;
    }

    sprintf(result_line0, "QCAMTEST result addr: %x, size: %d",
            (u32)jpeg_dst, jpeg_dst_size);

    ret = aic_transmit_at_data(at_req_res_ptr->serial_port_id, (u8 *)jpeg_dst, (u32)jpeg_dst_size);
    aos_free(jpeg_dst);
    jpeg_dst = AIC_NULL;
    if (ret != AIC_SUCCESS) {
        aicx_error(0x158600, "", "aic transmit jpeg dst through AT interface failed!\n");
        goto CAPTURE_ERROR;
    }
#else
    sprintf(result_line0, "QCAMTEST result addr: %x, size: %d",
            (u32)at_camera_cap_addr, g_camera_info.cam_stream_info.img_size);
#endif

    ret = camera_preview(CAMERA_FRONT, YUV_ONE_PLANE, fmt, LCD_PANEL_WIDTH, LCD_PANEL_HEIGHT);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0aa19b, "d", "camera %d enter preview failed!\n", CAMERA_FRONT);
        goto CAPTURE_ERROR;
    }

    strcpy(result_line1, "OK");
    line_pointers[0] = (ulong)&result_line0[0];
    line_pointers[1] = (ulong)&result_line1[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 2);

#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    aos_dma_free(addr);
#endif

    return AIC_SUCCESS;

CAPTURE_ERROR:
#ifdef AT_CAMERE_CAP_ADDR_DYNAMIC
    aos_dma_free(addr);

MEM_ALLOC_FAIL:
#endif
    snprintf(result_line0, sizeof(result_line0),
             "+QCAMTEST ERROR: %d", AT_ERROR_HANDLE_FAILURE);
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    return ret;
}

static AT_CMD_HANDLE camera_at_handles[] = {
    {"AT+QCAMOPEN", aic_camera_open_test_cmd, AIC_NULL,
     aic_camera_open_set_cmd, AIC_NULL},
    {"AT+QCAMCLOSE", aic_camera_close_test_cmd, AIC_NULL,
     aic_camera_close_set_cmd, AIC_NULL},
    {"AT+QCAMTEST", AIC_NULL, AIC_NULL,
     aic_camera_test_set_cmd, AIC_NULL},
#ifdef CONFIG_LIBJPEG_SUPPORT
    {"AT+QCAMTESTBLT", AIC_NULL, AIC_NULL,
     aic_camera_testblt_set_cmd, AIC_NULL},
#endif

    {AIC_NULL},
};

int camera_at_cmd_init(void)
{
    aic_add_at_cmd_handle(camera_at_handles);
    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_L2_INIT_EXPORT(camera_at_cmd_init);
