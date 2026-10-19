/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05ab6c
#define LOG_MODULE    AMOD_VENDOR

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <cache.h>

#include <stdio.h>
#include <stdlib.h>

#define HAVE_BOOLEAN
#include <jpeglib.h>

#include "rtp.h"
#include "mme.h"

#include <camera_hal.h>

#define RTP_HEADER_MAX_SIZE                 16

#define CAMERA_VIDEO_RTP_END_MARKER         0x80
#define CAMERA_VIDEO_RTP_DATA_MARKER        0
#define CAMERA_VIDEO_MAX_BYTES_PER_PIXEL    2

static camera_id_e camera_video_id = CAMERA_FRONT;

struct img_buf_info {
    char *buf_addr;
    size_t buf_size;
};

struct jpeg_img_info {
    struct img_buf_info buf_info;
    union {
        size_t bytes_already_sent;
        size_t buf_real_size;
    };
};

/* used for RTP read preview */
struct img_buf_info camera_video_w_raw_buf = {AIC_NULL, 0};
struct img_buf_info camera_video_r_raw_buf = {AIC_NULL, 0};

/* used to save data from and to rtp */
struct jpeg_img_info jpeg_from_rtp;
struct jpeg_img_info jpeg_to_send;

const char CodecName[] = "JPEG";

size_t jpeg_to_send_max_size = 0;

unsigned char *yuv420_buffer = AIC_NULL;

static inline void rgb24_to_16(const uint8_t *src, uint8_t *dst, int src_size)
{
    uint16_t *d        = (uint16_t *)dst;
    const uint8_t *s   = src;
    const uint8_t *end = s + src_size;

    while (s < end) {
        const int r = *s++;
        const int g = *s++;
        const int b = *s++;
        *d++        = (b >> 3) | ((g & 0xFC) << 3) | ((r & 0xF8) << 8);
    }
}

int jpeg_img_decode_to_mem(unsigned char *inbuffer,
                                  camera_data_fmt_e dst_data_fmt,
                                  size_t insize, MMEImage *image)
{
    int row_size;
    char *pline, *point;
    char *decompress_buffer;
    struct jpeg_error_mgr jerr;
    struct jpeg_decompress_struct dinfo;

    dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);

    /* set jpeg src file */
    jpeg_mem_src(&dinfo, inbuffer, insize);

    /* read file header */
    jpeg_read_header(&dinfo, AIC_TRUE);

    /* set decompress parameters */

    /* start decompress */
    jpeg_start_decompress(&dinfo);

    /* malloc one line buffer */
    row_size = dinfo.output_width * dinfo.output_components;
    pline = (char *)aos_dma_malloc(row_size);
    if (pline == AIC_NULL) {
        aicx_info(0x033b4d, "", "malloc pline for jpeg_decompress failed!\n");
        return AIC_ENOMEM;
    }

    image->width = dinfo.output_width;
    image->height = dinfo.output_height;

    /* malloc output buffer */
    decompress_buffer = aos_dma_malloc(dinfo.output_width * dinfo.output_height * dinfo.output_components);
    if (decompress_buffer == AIC_NULL) {
        aicx_info(0x125cee, "", "malloc output buffer for jpeg decompress failed!\n");
        return AIC_ENOMEM;
    }
    point = (char *)decompress_buffer;

    /* read line by line */
    while(dinfo.output_scanline < dinfo.output_height) {
        /* read one line */
        jpeg_read_scanlines(&dinfo, (JSAMPARRAY)&pline, 1);
        aos_memcpy(point, pline, row_size);
        point += row_size;
    }
    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);
    aos_dma_free(pline);

    switch (dst_data_fmt) {
    /*case YUV422:
        dst_buffer = aos_dma_malloc(dinfo.output_width * dinfo.output_height * 2);
        if (dst_buffer == AIC_NULL) {
            alog_info("malloc yuv422 dst buffer failed!\n");
            return AIC_ENOMEM;
        }
        rgb24_to_yuv422(decompress_buffer, dst_buffer,
                        dinfo.output_width * dinfo.output_height * dinfo.output_components);
        aos_dma_free(decompress_buffer);
        break;*/

    case RGB565:
        camera_video_w_raw_buf.buf_size = dinfo.output_width * dinfo.output_height * 2;
        rgb24_to_16(decompress_buffer, camera_video_w_raw_buf.buf_addr,
                    dinfo.output_width * dinfo.output_height * dinfo.output_components);
        aos_dma_free(decompress_buffer);
        decompress_buffer = AIC_NULL;
        image->format = 64;
        break;

    default:
        aos_dma_free(decompress_buffer);
        decompress_buffer = AIC_NULL;
        /* do nothing, just copy */
        return AIC_ENODATA;
    }

    image->data = camera_video_w_raw_buf.buf_addr;
    image->bytes = camera_video_w_raw_buf.buf_size;

    return AIC_SUCCESS;
}

int convert_raw_img_to_jpeg(char *src_image, int width, int height,
                            struct jpeg_img_info *dst_img_info, int dst_buf_offset,
                            camera_data_fmt_e src_img_fmt)
{
    char *dst_buf;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int i = 0, j = 0, row_stride;
    unsigned char yuvbuf[width * 3];
    unsigned char *py, *pu;

    yuv422_yuyv_to_yuv420_y_uv(src_image, (unsigned char *)yuv420_buffer, width, height);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    dst_buf = (char *)dst_img_info->buf_info.buf_addr + dst_buf_offset;

    if (jpeg_to_send_max_size != 0)
        dst_img_info->buf_info.buf_size = jpeg_to_send_max_size;
    jpeg_mem_dest(&cinfo, (unsigned char **)&dst_buf, &dst_img_info->buf_info.buf_size);

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr;
    cinfo.dct_method = JDCT_ISLOW;
    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, 99, AIC_TRUE);

    jpeg_start_compress(&cinfo, AIC_TRUE);

    row_stride = cinfo.image_width * 3;

    py = (unsigned char *)yuv420_buffer;
    pu = (unsigned char *)yuv420_buffer + cinfo.image_width * cinfo.image_height;

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

    aicx_info(0x10d9c8, "d", "mme video jpeg dst size is %d\n", dst_img_info->buf_info.buf_size);

    return AIC_SUCCESS;
}

const char *MME_VideoCodecName(void)
{
    return CodecName;
}

int MME_StartVideo(MMEVideoParam *param)
{
    int ret;
    size_t buf_size;
    void * buf_addr;

    /* open camera */
    ret = camera_open(camera_video_id);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x01aa13, "", "MME start video failed!\n");
        return ret;
    }

    /* init buffer */
    buf_size = param->recvMaxWidth * param->recvMaxHeight *
               CAMERA_VIDEO_MAX_BYTES_PER_PIXEL;
    buf_addr = aos_dma_malloc(buf_size);
    if (buf_addr == AIC_NULL) {
        aicx_error(0x031f73, "", "malloc video buffer jpeg_from_rtp failed!\n");
        return AIC_ENOMEM;
    }
    jpeg_from_rtp.buf_info.buf_addr = buf_addr;
    jpeg_from_rtp.buf_info.buf_size = buf_size;
    jpeg_from_rtp.buf_real_size = 0;

    buf_size = param->sendWidth * param->sendHeight *
               CAMERA_VIDEO_MAX_BYTES_PER_PIXEL;
    /* reserve RTP_HEADER_MAX_SIZE for RTP header */
    buf_addr = aos_dma_malloc(buf_size + RTP_HEADER_MAX_SIZE);
    if (buf_addr == AIC_NULL) {
        aicx_error(0x09ccba, "", "malloc video buffer jpeg_to_send failed!\n");
        goto DMA_FREE_JPEG_FROM_RTP;
    }
    jpeg_to_send.buf_info.buf_addr = buf_addr;
    jpeg_to_send.buf_info.buf_size = buf_size;
    jpeg_to_send.bytes_already_sent = 0;

    jpeg_to_send_max_size = buf_size;

    buf_addr = aos_dma_malloc(buf_size);
    if (buf_addr == AIC_NULL) {
        aicx_error(0x1d43ca, "", "malloc camera_video_w_raw_buf buffer failed!\n");
        goto DMA_FREE_JPEG_TO_SEND;
    }
    camera_video_w_raw_buf.buf_addr = buf_addr;
    camera_video_w_raw_buf.buf_size = buf_size;

    buf_size = param->sendWidth * param->sendHeight * 3 / 2;
    if (yuv420_buffer == AIC_NULL) {
        yuv420_buffer = aos_malloc(buf_size);
        if (yuv420_buffer == AIC_NULL) {
            aicx_error(0x14572d, "", "malloc yuv420 buffer failed!\n");
            goto DMA_FREE_CAMERA_VIDEO_W_RAW_BUF;
        }
    }

    ret = camera_video(camera_video_id, param->sendWidth, param->sendHeight);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x0ef3e4, "", "MME set video mode failed!\n");
        goto DMA_FREE_YUV42O_BUFFER;
    }

    return AIC_SUCCESS;

DMA_FREE_YUV42O_BUFFER:
    aos_dma_free(yuv420_buffer);
    yuv420_buffer = AIC_NULL;

DMA_FREE_CAMERA_VIDEO_W_RAW_BUF:
    aos_free(camera_video_w_raw_buf.buf_addr);
    camera_video_w_raw_buf.buf_addr = AIC_NULL;
    camera_video_w_raw_buf.buf_size = 0;

DMA_FREE_JPEG_TO_SEND:
    aos_dma_free(jpeg_to_send.buf_info.buf_addr);
    jpeg_to_send.buf_real_size = jpeg_to_send.buf_info.buf_size = 0;

DMA_FREE_JPEG_FROM_RTP:
    aos_dma_free(jpeg_from_rtp.buf_info.buf_addr);
    jpeg_from_rtp.buf_real_size = jpeg_from_rtp.buf_info.buf_size = 0;

    jpeg_to_send_max_size = 0;

    return AIC_ENOMEM;
}

struct camera_video_queue_item cv_queue_item = {0};

/* image data is from local camera, MMEImage used for preview  */
int MME_ReadVideoRTP(struct rtp_buf_t *rtp, int rtpBufferSize,
                     int rtpHeaderLength, MMEImage *image, unsigned timeoutMs)
{
    //struct camera_video_queue_item cv_queue_item;
    int rtp_buff_len = rtpBufferSize - rtpHeaderLength;
    size_t unsend_size;
    char *buf_ptr;
    int ret;

    if (camera_video_queue_ptr == AIC_NULL) {
        aicx_error(0x062f87, "", "camera video is not initialized!\n");
        return -1;
    }

    AIC_ASSERT(rtpHeaderLength <= RTP_HEADER_MAX_SIZE);

    //buf_ptr = (char *)rtp->buf;

    /* timeoutMs != 0 means that prepare to send first RTP */
    if (timeoutMs) {
        /* free last preview frame */
        if (cv_queue_item.raw_img_addr) {
            aicx_info(0x18b5a2, "x", "dma free cv_queue_item.raw_img_addr %x!\n", cv_queue_item.raw_img_addr);
            aos_dma_free(cv_queue_item.raw_img_addr);
            cv_queue_item.raw_img_addr = 0;
            cv_queue_item.raw_img_size = 0;
        }

        /* get message from camera video queue */
        ret = aos_recv_from_queue(camera_video_queue_ptr, &cv_queue_item, timeoutMs);
        if (ret != AIC_SUCCESS) {
            aicx_warn(0x1659d6, "", "aos recieve item from queue failed!\n");
            /*if (cv_queue_item != AIC_NULL)
                aos_free(cv_queue_item);*/
            return 0;
        }

        aicx_info(0x011190, "x", "aos recv from queue success raw image addr =%x!\n", cv_queue_item.raw_img_addr);

        /* convert to jpeg and then fill to jpeg_to_send */
        ret = convert_raw_img_to_jpeg((char *)cv_queue_item.raw_img_addr,
                                      cv_queue_item.raw_img_width,
                                      cv_queue_item.raw_img_height,
                                      &jpeg_to_send, rtpHeaderLength,
                                      cv_queue_item.data_fmt);
        if (ret != AIC_SUCCESS) {
            aicx_error(0x064d12, "", "convert raw image to jpeg failed!\n");
            return -1;
        }

        camera_video_r_raw_buf.buf_addr = (char *)cv_queue_item.raw_img_addr;
        camera_video_r_raw_buf.buf_size = cv_queue_item.raw_img_size;

        aicx_info(0x18e1dd, "xxdddx", "image addr is %x, raw_img_addr = %x, raw image size %d, width = %d, height = %d, jpeg_addr = %x\n", image,
                   cv_queue_item.raw_img_addr, cv_queue_item.raw_img_size, cv_queue_item.raw_img_width, cv_queue_item.raw_img_height,
                   jpeg_to_send.buf_info.buf_addr);
        /* filling MMEImage for preview data from sensor */
        if (image != AIC_NULL) {
            image->data = (void *)cv_queue_item.raw_img_addr;
            image->bytes = cv_queue_item.raw_img_size;
            image->width = cv_queue_item.raw_img_width;
            image->height = cv_queue_item.raw_img_height;
            image->format = cv_queue_item.data_fmt;
        }

        if (rtp != AIC_NULL) {
            /* set first RTP type */
            rtp->types = RTP_FIRST;
            /* fill rtp buffer */
            //buf_ptr = jpeg_to_send.buf_info.buf_addr;
            rtp->buf = jpeg_to_send.buf_info.buf_addr;
        }
        buf_ptr = jpeg_to_send.buf_info.buf_addr;
        //if (rtp_buff_len >= jpeg_to_send.buf_info.buf_size + rtpHeaderLength) {
        if(rtp_buff_len >= jpeg_to_send.buf_info.buf_size) {
            /* only one RTP buffer is enough */
            buf_ptr[1] = CAMERA_VIDEO_RTP_END_MARKER;
            //jpeg_to_send.buf_info.buf_size = 0;
            jpeg_to_send.bytes_already_sent = 0;
            aicx_info(0x1192f4, "d", "mme video send to rtp jpeg total size is %d\n", jpeg_to_send.buf_info.buf_size);
            return jpeg_to_send.buf_info.buf_size + rtpHeaderLength;
        } else {
            buf_ptr[1] = CAMERA_VIDEO_RTP_DATA_MARKER;
            /* not include the header */
            jpeg_to_send.bytes_already_sent = rtp_buff_len;
        }
    }else {
        if (jpeg_to_send.bytes_already_sent == 0) {
            aicx_error(0x19441f, "", "The send bytes in the middle or last RTP should not be 0!\n");
            return -1;
        }

        buf_ptr = jpeg_to_send.buf_info.buf_addr + jpeg_to_send.bytes_already_sent;

        if (rtp != AIC_NULL) {
            rtp->types = RTP_IDR;

            /* fill rtp buffer */
            rtp->buf = buf_ptr;
        }
        unsend_size = jpeg_to_send.buf_info.buf_size - jpeg_to_send.bytes_already_sent;
        if (rtp_buff_len >= unsend_size) {
            buf_ptr[1] = CAMERA_VIDEO_RTP_END_MARKER;
            //jpeg_to_send.buf_info.buf_size = 0;
            aicx_info(0x04b2db, "d", "mme video send the last rtp, jpeg total size is %d\n",
                      jpeg_to_send.bytes_already_sent + unsend_size);
            jpeg_to_send.bytes_already_sent = 0;

            return unsend_size + rtpHeaderLength;
        } else {
            buf_ptr[1] = CAMERA_VIDEO_RTP_DATA_MARKER;
            jpeg_to_send.bytes_already_sent += rtp_buff_len;
        }
    }

    return rtpBufferSize;
}

/* if image as null */
int MME_WriteVideoRTP(void *rtp, int rtpHeaderLength, int rtpLen, MMEImage *image)
{
    int rtp_buff_len, ret;
    char *rtp_marker;
    char *jpeg_header1, *jpeg_header2;

    if (rtp == AIC_NULL) {
        aicx_error(0x099d71, "", "rtp is null in write video!\n");
        return -1;
    }

    AIC_ASSERT(rtpHeaderLength <= RTP_HEADER_MAX_SIZE);
    jpeg_header1 = (char *)rtp + rtpHeaderLength;
    jpeg_header2 = (char *)rtp + rtpHeaderLength + 1;
    if ((*jpeg_header1 == 0xff) && (*jpeg_header2 == 0xd8)) {
        aicx_info(0x110390, "", "received jpeg header success!\n");
        jpeg_from_rtp.buf_real_size = 0;

    }

    rtp_buff_len = rtpLen - rtpHeaderLength;

    rtp_marker = (char *)rtp + 1;
    /* last rtp packet, may be only one RTP, write whole image info to MMEImage */
    if (*rtp_marker & CAMERA_VIDEO_RTP_END_MARKER) {
        if (image == AIC_NULL) {
            aicx_error(0x0c80af, "", "MMEImage is NULL, just discard this buffer!\n");
            jpeg_from_rtp.buf_real_size = 0;
            return 0;
        }

        /* if the recieved size is larger than the total buffer */
        if (jpeg_from_rtp.buf_real_size + rtp_buff_len > jpeg_from_rtp.buf_info.buf_size) {
            aicx_error(0x1de9c1, "", "image buffer is full, cannot save last rtp!\n");
            jpeg_from_rtp.buf_real_size = 0;
            return -1;
        }

        aos_memcpy(jpeg_from_rtp.buf_info.buf_addr + jpeg_from_rtp.buf_real_size,
                   (char *)rtp + rtpHeaderLength, rtp_buff_len);

        /* calc real image size */
        jpeg_from_rtp.buf_real_size += rtp_buff_len;

        ret = jpeg_img_decode_to_mem(jpeg_from_rtp.buf_info.buf_addr, RGB565, jpeg_from_rtp.buf_real_size, image);
        if (ret != AIC_SUCCESS) {
            aicx_error(0x0a859d, "", "decode jpeg image from RTP buffer to memory failed!\n");
            jpeg_from_rtp.buf_real_size = 0;
            return -1;
        }

        aicx_info(0x04d65a, "", "get rtp end marker and decode image to mem success!\n");

        jpeg_from_rtp.buf_real_size = 0;
    } else {
        if (jpeg_from_rtp.buf_real_size + rtp_buff_len > jpeg_from_rtp.buf_info.buf_size) {
            aicx_error(0x0c68a6, "", "image buffer is full, cannot save midle RTP!\n");
            aicx_info(0x0344d3, "ddd", "buf real size %d, rtp buff len %d, buf total size %d",
                      jpeg_from_rtp.buf_real_size, rtp_buff_len, jpeg_from_rtp.buf_info.buf_size);
            jpeg_from_rtp.buf_real_size = 0;
            return -1;
        }

        aicx_info(0x0344d3, "ddd", "buf real size %d, rtp buff len %d, buf total size %d",
                  jpeg_from_rtp.buf_real_size, rtp_buff_len, jpeg_from_rtp.buf_info.buf_size);

        aos_memcpy(jpeg_from_rtp.buf_info.buf_addr + jpeg_from_rtp.buf_real_size,
                   (char *)rtp + rtpHeaderLength, rtp_buff_len);

        /* calc real image size */
        jpeg_from_rtp.buf_real_size += rtp_buff_len;
    }

    return 0;
}

int MME_ResetVideo(unsigned char sendFps, unsigned short sendGopMs, unsigned short sendKbps)
{
    return 0;
}

void MME_StopVideo(void)
{
    /* close camera */
    camera_close(camera_video_id);

    /* free last preview frame */
    if (cv_queue_item.raw_img_addr) {
        aos_dma_free(cv_queue_item.raw_img_addr);
        cv_queue_item.raw_img_addr = 0;
        cv_queue_item.raw_img_size = 0;
    }

    /* free unsed memory */
    if (jpeg_to_send.buf_info.buf_addr != AIC_NULL) {
        aos_dma_free(jpeg_to_send.buf_info.buf_addr);
        jpeg_to_send.buf_info.buf_addr = 0;
        jpeg_to_send.buf_info.buf_size = jpeg_to_send.bytes_already_sent = 0;
    }

    if (jpeg_from_rtp.buf_info.buf_addr != AIC_NULL) {
        aos_dma_free(jpeg_from_rtp.buf_info.buf_addr);
        jpeg_from_rtp.buf_info.buf_addr = 0;
        jpeg_from_rtp.buf_info.buf_size = jpeg_from_rtp.buf_real_size = 0;
    }

    if (camera_video_w_raw_buf.buf_addr != AIC_NULL) {
        aos_dma_free(camera_video_w_raw_buf.buf_addr);
        camera_video_w_raw_buf.buf_addr = 0;
        camera_video_w_raw_buf.buf_size = 0;
    }

    if (yuv420_buffer != AIC_NULL) {
        aos_free(yuv420_buffer);
        yuv420_buffer = AIC_NULL;
    }
}

int MME_SetCamera(int face)
{
    switch (face) {
    case 0:
        camera_video_id = CAMERA_FRONT;
        break;

    case 1:
        camera_video_id = CAMERA_BACK;
        break;

    default:
        aicx_error(0x0d3505, "d", "MME camera id %d is not support!\n", face);
        return AIC_ENODEV;
    }

    return AIC_SUCCESS;
}

void MME_OnVideoStats(unsigned nowMs, char sendLoss, unsigned sendKbps,
                      unsigned rttMs, unsigned recvJitterMs, unsigned recvFPS)
{
    /* Do nothing */
    return;
}

