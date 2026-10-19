/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00e9d4
#define LOG_MODULE    AMOD_CAMERA

#include <aic_errno.h>
#include <aic_type.h>

#include "format_convert.h"

#define FIX_BITS 10
#define CLIP(v, v_min, v_max)  ( ((v)<(v_min)) ? (v_min) : (((v) > (v_max)) ? (v_max) : (v)) )

static int calc_rot_addr(uint8_t rot_mode, int x, int y, int blk_width, int blk_height)
{
    int rot_x = 0, rot_y = 0, rot_width = 0, rot_height = 0;
    int dst_pos = 0;

    switch (rot_mode) {
    case ROT_MODE_NONE:
        rot_width = blk_width;
        rot_height = blk_height;
        rot_x = x;
        rot_y = y;
        break;
    case ROT_MODE_ANTI_CW90:
        rot_width = blk_height;
        rot_height = blk_width;
        rot_x = y;
        rot_y = rot_height - x - 1;
        break;
    case ROT_MODE_ANTI_CW270:
        rot_width = blk_height;
        rot_height = blk_width;
        rot_x = rot_width - y - 1;
        rot_y = x;
        break;
    }
    dst_pos = rot_y * rot_width + rot_x;

    return dst_pos;
}

static uint8_t argb_linear_avg(uint8_t pix1, uint8_t pix2, uint32_t frac)
{
    uint8_t value, value1, value2;
    uint32_t tmp;

    value1 = pix1 & 0xFF;
    value2 = pix2 & 0xFF;
    tmp = (value1 * ((1 << FIX_BITS) - frac) + value2 * frac) >> FIX_BITS;
    if (tmp > 255)
        value = 255;
    else
        value = tmp & 0xFF;

    return value;
}

int32_t bilinear_scaling_for_yuv422(unsigned char* p_src_y, unsigned char* p_src_uv,
                                    uint32_t src_width, uint32_t src_height,
                                    unsigned short* p_dst, uint32_t dst_width, uint32_t dst_height,
                                    enum YUYV_MODE uv_mode, enum ROT_MODE rot_mode)
{
    uint8_t Y00, Y01, Y10, Y11, U00, U01, U10, U11, V00, V01, V10, V11;
    uint32_t y_step = (src_height << FIX_BITS) / dst_height;
    uint32_t x_step = (src_width << FIX_BITS) / dst_width;
    int rot_pos = 0, src_pos = 0;
    int32_t Y, U, V, R, G, B;
    uint32_t x_frac, y_frac;
    uint32_t pixh0, pixh1;
    uint32_t y_f, x_f;
    uint32_t y_o, x_o;
    uint32_t y_i, x_i;
    unsigned short RGB;
    uint8_t uv_flag = 0;

    /* for one plane */
    int src_pitch = src_width << 1;

    U01 = U10 = U11 = V01 = V10 = V11 = 0;

    /* for two plane and only Y */
    if ((uv_mode == MODE_UV) || (uv_mode == MODE_VU) || (uv_mode == MODE_Y)) {
        src_pitch = src_width;
    }

    for (y_o = 0; y_o < dst_height; y_o++) {
        for (x_o = 0; x_o < dst_width; x_o++) {
            y_f = y_o * y_step;
            x_f = x_o * x_step;

            y_i = y_f >> FIX_BITS;
            x_i = x_f >> FIX_BITS;
            y_frac = y_f & ((1 << FIX_BITS) - 1);
            x_frac = x_f & ((1 << FIX_BITS) - 1);

            uv_flag = 0;

            switch (uv_mode) {
            case MODE_YUYV:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 2);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 2);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 1);
                    U10 = *(p_src_y + src_pos + src_pitch + 1);
                }
                else {
                    U00 = *(p_src_y + src_pos - 1);
                    U01 = *(p_src_y + src_pos + 3);
                    U10 = *(p_src_y + src_pos + src_pitch - 1);
                    U11 = *(p_src_y + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 3);
                    V10 = *(p_src_y + src_pos + src_pitch + 3);
                }
                else {
                    V00 = *(p_src_y + src_pos + 1);
                    V01 = *(p_src_y + src_pos + 5);
                    V10 = *(p_src_y + src_pos + src_pitch + 1);
                    V11 = *(p_src_y + src_pos + src_pitch + 5);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }

                break;
            case MODE_YVYU:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 2);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 2);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 3);
                    U10 = *(p_src_y + src_pos + src_pitch + 3);
                }
                else {
                    U00 = *(p_src_y + src_pos + 1);
                    U01 = *(p_src_y + src_pos + 5);
                    U10 = *(p_src_y + src_pos + src_pitch + 1);
                    U11 = *(p_src_y + src_pos + src_pitch + 5);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 1);
                    V10 = *(p_src_y + src_pos + src_pitch + 1);
                }
                else {
                    V00 = *(p_src_y + src_pos - 1);
                    V01 = *(p_src_y + src_pos + 3);
                    V10 = *(p_src_y + src_pos + src_pitch - 1);
                    V11 = *(p_src_y + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }

                break;
            case MODE_UYVY:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 1);
                Y01 = *(p_src_y + src_pos + 3);
                Y10 = *(p_src_y + src_pos + src_pitch + 1);
                Y11 = *(p_src_y + src_pos + src_pitch + 3);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 0);
                    U10 = *(p_src_y + src_pos + src_pitch + 0);
                }
                else {
                    U00 = *(p_src_y + src_pos - 2);
                    U01 = *(p_src_y + src_pos + 2);
                    U10 = *(p_src_y + src_pos + src_pitch - 2);
                    U11 = *(p_src_y + src_pos + src_pitch + 2);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 2);
                    V10 = *(p_src_y + src_pos + src_pitch + 2);
                }
                else {
                    V00 = *(p_src_y + src_pos + 0);
                    V01 = *(p_src_y + src_pos + 4);
                    V10 = *(p_src_y + src_pos + src_pitch + 0);
                    V11 = *(p_src_y + src_pos + src_pitch + 4);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }

                break;
            case MODE_VYUY:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 1);
                Y01 = *(p_src_y + src_pos + 3);
                Y10 = *(p_src_y + src_pos + src_pitch + 1);
                Y11 = *(p_src_y + src_pos + src_pitch + 3);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 2);
                    U10 = *(p_src_y + src_pos + src_pitch + 2);
                }
                else {
                    U00 = *(p_src_y + src_pos + 0);
                    U01 = *(p_src_y + src_pos + 4);
                    U10 = *(p_src_y + src_pos + src_pitch + 0);
                    U11 = *(p_src_y + src_pos + src_pitch + 4);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 0);
                    V10 = *(p_src_y + src_pos + src_pitch + 0);
                }
                else {
                    V00 = *(p_src_y + src_pos - 2);
                    V01 = *(p_src_y + src_pos + 2);
                    V10 = *(p_src_y + src_pos + src_pitch - 2);
                    V11 = *(p_src_y + src_pos + src_pitch + 2);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }
                break;
            case MODE_UV:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                src_pos = y_i * src_width + ((x_i >> 1) << 1);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_uv + src_pos + 0);
                    U10 = *(p_src_uv + src_pos + src_pitch + 0);
                }
                else {
                    U00 = *(p_src_uv + src_pos + 0);
                    U01 = *(p_src_uv + src_pos + 2);
                    U10 = *(p_src_uv + src_pos + src_pitch + 0);
                    U11 = *(p_src_uv + src_pos + src_pitch + 2);
                    }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_uv + src_pos + 1);
                    V10 = *(p_src_uv + src_pos + src_pitch + 1);
                }
                else {
                    V00 = *(p_src_uv + src_pos + 1);
                    V01 = *(p_src_uv + src_pos + 3);
                    V10 = *(p_src_uv + src_pos + src_pitch + 1);
                    V11 = *(p_src_uv + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }
                break;
            case MODE_VU:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                src_pos = y_i * src_width + ((x_i >> 1) << 1);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_uv + src_pos + 1);
                    U10 = *(p_src_uv + src_pos + src_pitch + 1);
                }
                else {
                    U00 = *(p_src_uv + src_pos + 1);
                    U01 = *(p_src_uv + src_pos + 3);
                    U10 = *(p_src_uv + src_pos + src_pitch + 1);
                    U11 = *(p_src_uv + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_uv + src_pos + 0);
                    V10 = *(p_src_uv + src_pos + src_pitch + 0);
                }
                else {
                    V00 = *(p_src_uv + src_pos + 0);
                    V01 = *(p_src_uv + src_pos + 2);
                    V10 = *(p_src_uv + src_pos + src_pitch + 0);
                    V11 = *(p_src_uv + src_pos + src_pitch + 2);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }
                break;
            case MODE_Y:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                U00 = 128;
                V00 = 128;

                uv_flag = 2;
                break;
            default:
                return AIC_ENOEXEC;
            }

            /* for bilinear */
            pixh0 = argb_linear_avg(Y00, Y01, x_frac);
            pixh1 = argb_linear_avg(Y10, Y11, x_frac);
            Y = argb_linear_avg(pixh0, pixh1, y_frac);
            if (uv_flag == 0) {
                U = argb_linear_avg(U00, U10, y_frac);
            }
            else if (uv_flag == 1) {
                pixh0 = argb_linear_avg(U00, U01, x_frac);
                pixh1 = argb_linear_avg(U10, U11, x_frac);
                U = argb_linear_avg(pixh0, pixh1, y_frac);
            }
            else {
                U = U00;
            }
            if (uv_flag == 0) {
                V = argb_linear_avg(V00, V10, y_frac);
            }
            else if (uv_flag == 1) {
                pixh0 = argb_linear_avg(V00, V01, x_frac);
                pixh1 = argb_linear_avg(V10, V11, x_frac);
                V = argb_linear_avg(pixh0, pixh1, y_frac);
            }
            else {
                V = V00;
            }

            /* BT601 full range */
            U = U - 128;
            V = V - 128;
            R = Y + ((360 * V + 128) >> 8);
            G = Y - ((88 * U + 184 * V + 128) >> 8);
            B = Y + ((455 * U + 128) >> 8);
            R = CLIP(R, 0, 255);
            G = CLIP(G, 0, 255);
            B = CLIP(B, 0, 255);
            RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
            if (ROT_MODE_NONE == rot_mode) {
                *p_dst++ = RGB;
            }
            else {
                rot_pos = calc_rot_addr(rot_mode, x_o, y_o, dst_width, dst_height);
                *(p_dst + rot_pos) = RGB;
            }
        }
    }

    return 0;
}

int32_t bilinear_scaling_for_yuv420(unsigned char* p_src_y, unsigned char* p_src_uv,
                                    uint32_t src_width, uint32_t src_height,
                                    unsigned short* p_dst, uint32_t dst_width, uint32_t dst_height,
                                    enum YUYV_MODE uv_mode, enum ROT_MODE rot_mode)
{
    uint8_t Y00, Y01, Y10, Y11, U00, U01, U10, U11, V00, V01, V10, V11;
    uint32_t y_step = (src_height << FIX_BITS) / dst_height;
    uint32_t x_step = (src_width << FIX_BITS) / dst_width;
    int rot_pos = 0, src_pos = 0;
    int32_t Y, U, V, R, G, B;
    uint32_t x_frac, y_frac;
    uint32_t pixh0, pixh1;
    uint32_t y_f, x_f;
    uint32_t y_o, x_o;
    uint32_t y_i, x_i;
    unsigned short RGB;
    uint8_t uv_flag = 0;

    int src_pitch = src_width;

    U01 = U10 = U11 = V01 = V10 = V11 = 0;

    for (y_o = 0; y_o < dst_height; y_o++) {
        for (x_o = 0; x_o < dst_width; x_o++) {
            y_f = y_o * y_step;
            x_f = x_o * x_step;

            y_i = y_f >> FIX_BITS;
            x_i = x_f >> FIX_BITS;
            y_frac = y_f & ((1 << FIX_BITS) - 1);
            x_frac = x_f & ((1 << FIX_BITS) - 1);

            uv_flag = 0;

            switch (uv_mode) {
            case MODE_UV:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                src_pos = (y_i >> 1) * src_width + ((x_i >> 1) << 1);

                if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 0)) {
                    U00 = *(p_src_uv + src_pos + 0);
                }
                else if (((x_i & 0x1) == 1) && ((y_i & 0x1) == 0)) {
                    U00 = *(p_src_uv + src_pos + 0);
                    U01 = *(p_src_uv + src_pos + 2);
                }
                else if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 1)) {
                    U00 = *(p_src_uv + src_pos + 0);
                    U10 = *(p_src_uv + src_pos + src_pitch + 0);
                }
                else {
                    U00 = *(p_src_uv + src_pos + 0);
                    U01 = *(p_src_uv + src_pos + 2);
                    U10 = *(p_src_uv + src_pos + src_pitch + 0);
                    U11 = *(p_src_uv + src_pos + src_pitch + 2);
                }

                if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 0)) {
                    V00 = *(p_src_uv + src_pos + 1);
                }
                else if (((x_i & 0x1) == 1) && ((y_i & 0x1) == 0)) {
                    V00 = *(p_src_uv + src_pos + 1);
                    V01 = *(p_src_uv + src_pos + 3);
                }
                else if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 1)) {
                    V00 = *(p_src_uv + src_pos + 1);
                    V10 = *(p_src_uv + src_pos + src_pitch + 1);
                }
                else {
                    V00 = *(p_src_uv + src_pos + 1);
                    V01 = *(p_src_uv + src_pos + 3);
                    V10 = *(p_src_uv + src_pos + src_pitch + 1);
                    V11 = *(p_src_uv + src_pos + src_pitch + 3);
                }

                if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 0)) {
                    uv_flag = 0;
                }
                else if (((x_i & 0x1) == 1) && ((y_i & 0x1) == 0)) {
                    uv_flag = 1;
                }
                else if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 1)) {
                    uv_flag = 2;
                }
                else {
                    uv_flag = 3;
                }
                break;
            case MODE_VU:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                src_pos = (y_i >> 1) * src_width + ((x_i >> 1) << 1);

                if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 0)) {
                    V00 = *(p_src_uv + src_pos + 0);
                }
                else if (((x_i & 0x1) == 1) && ((y_i & 0x1) == 0)) {
                    V00 = *(p_src_uv + src_pos + 0);
                    V01 = *(p_src_uv + src_pos + 2);
                }
                else if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 1)) {
                    V00 = *(p_src_uv + src_pos + 0);
                    V10 = *(p_src_uv + src_pos + src_pitch + 0);
                }
                else {
                    V00 = *(p_src_uv + src_pos + 0);
                    V01 = *(p_src_uv + src_pos + 2);
                    V10 = *(p_src_uv + src_pos + src_pitch + 0);
                    V11 = *(p_src_uv + src_pos + src_pitch + 2);
                }

                if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 0)) {
                    U00 = *(p_src_uv + src_pos + 1);
                }
                else if (((x_i & 0x1) == 1) && ((y_i & 0x1) == 0)) {
                    U00 = *(p_src_uv + src_pos + 1);
                    U01 = *(p_src_uv + src_pos + 3);
                }
                else if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 1)) {
                    U00 = *(p_src_uv + src_pos + 1);
                    U10 = *(p_src_uv + src_pos + src_pitch + 1);
                }
                else {
                    U00 = *(p_src_uv + src_pos + 1);
                    U01 = *(p_src_uv + src_pos + 3);
                    U10 = *(p_src_uv + src_pos + src_pitch + 1);
                    U11 = *(p_src_uv + src_pos + src_pitch + 3);
                }

                if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 0)) {
                    uv_flag = 0;
                }
                else if (((x_i & 0x1) == 1) && ((y_i & 0x1) == 0)) {
                    uv_flag = 1;
                }
                else if (((x_i & 0x1) == 0) && ((y_i & 0x1) == 1)) {
                    uv_flag = 2;
                }
                else {
                    uv_flag = 3;
                }
                break;
            case MODE_Y:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                U00 = 128;
                V00 = 128;

                uv_flag = 0;
                break;
            default:
                return AIC_ENOEXEC;
            }

            /* for bilinear */
            pixh0 = argb_linear_avg(Y00, Y01, x_frac);
            pixh1 = argb_linear_avg(Y10, Y11, x_frac);
            Y = argb_linear_avg(pixh0, pixh1, y_frac);
            if (uv_flag == 0) {
                U = U00;
                V = V00;
            }
            else if (uv_flag == 1) {
                U = argb_linear_avg(U00, U01, x_frac);
                V = argb_linear_avg(V00, V01, x_frac);
            }
            else if (uv_flag == 2) {
                U = argb_linear_avg(U00, U10, y_frac);
                V = argb_linear_avg(V00, V10, y_frac);
            }
            else {
                pixh0 = argb_linear_avg(U00, U01, x_frac);
                pixh1 = argb_linear_avg(U10, U11, x_frac);
                U = argb_linear_avg(pixh0, pixh1, y_frac);

                pixh0 = argb_linear_avg(V00, V01, x_frac);
                pixh1 = argb_linear_avg(V10, V11, x_frac);
                V = argb_linear_avg(pixh0, pixh1, y_frac);
            }

            /* BT601 full range */
            U = U - 128;
            V = V - 128;
            R = Y + ((360 * V + 128) >> 8);
            G = Y - ((88 * U + 184 * V + 128) >> 8);
            B = Y + ((455 * U + 128) >> 8);
            R = CLIP(R, 0, 255);
            G = CLIP(G, 0, 255);
            B = CLIP(B, 0, 255);
            RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
            if (ROT_MODE_NONE == rot_mode) {
                *p_dst++ = RGB;
            }
            else {
                rot_pos = calc_rot_addr(rot_mode, x_o, y_o, dst_width, dst_height);
                *(p_dst + rot_pos) = RGB;
            }
        }
    }

    return 0;
}


/*
 * src_ptr: src image data buffer, if 2plane, it's y plane buffer.
 * src_uv_ptr: if 2plane, it's y plane buffer; if 1plane, it's invalid.
 * dst_ptr: dst image data buffer
 * width: image width
 * height: image height
 * uv_mode: 0: YUYV, 1: YVYU, 2: UYVY, 3: VYUY; 4: UV, 5:VU, 6:Y
 * scl_mode: 0: no scaler, 1: bilinear scaler, 2: binning
 * rot_mode: 0: 0, 1: 90, 2: 270 */
int yuv422_to_rgb565(unsigned char* src_ptr, unsigned char* src_uv_ptr,
                     int src_width, int src_height,
                     unsigned short* dst_ptr, int dst_width, int dst_height,
                     enum YUYV_MODE uv_mode, enum SCL_MODE scl_mode,
                     enum ROT_MODE rot_mode)
{
    int Y00, Y01, Y10, Y11, U0, V0, U1, V1, Y0, Y1, U, V, R, G, B;
    unsigned char* in_uv_ptr = AIC_NULL;
    unsigned short* out_ptr = AIC_NULL;
    unsigned char* in_ptr = AIC_NULL;
    int rot_pos = 0, src_pos = 0;
    unsigned short RGB;
    int i, j;

    /* for one plane */
    int src_pitch = src_width << 1;

    /* for two plane and only Y */
    if ((uv_mode == MODE_UV) || (uv_mode == MODE_VU) || (uv_mode == MODE_Y)) {
        src_pitch = src_width;
    }

    /* for bilinear */
    if (scl_mode == 1) {
        in_ptr = src_ptr;
        in_uv_ptr = src_uv_ptr;
        out_ptr = dst_ptr;

        bilinear_scaling_for_yuv422(in_ptr, in_uv_ptr, src_width, src_height,
                                    out_ptr, dst_width, dst_height, uv_mode, rot_mode);
    } else if (scl_mode == 2) { /* for binning */
        in_ptr = src_ptr;
        in_uv_ptr = src_uv_ptr;
        out_ptr = dst_ptr;
        for (i = 0; i < src_height; i += 2) {
            for (j = 0; j < src_width; j += 2) {
                switch (uv_mode) {
                case MODE_YUYV:
                    src_pos = (i * src_width + j) << 1;
                    Y00 = *(in_ptr + src_pos + 0);
                    U0 = *(in_ptr + src_pos + 1);
                    Y01 = *(in_ptr + src_pos + 2);
                    V0 = *(in_ptr + src_pos + 3);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    U1 = *(in_ptr + src_pos + src_pitch + 1);
                    Y11 = *(in_ptr + src_pos + src_pitch + 2);
                    V1 = *(in_ptr + src_pos + src_pitch + 3);
                    break;
                case MODE_YVYU:
                    src_pos = (i * src_width + j) << 1;
                    Y00 = *(in_ptr + src_pos + 0);
                    V0 = *(in_ptr + src_pos + 1);
                    Y01 = *(in_ptr + src_pos + 2);
                    U0 = *(in_ptr + src_pos + 3);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    V1 = *(in_ptr + src_pos + src_pitch + 1);
                    Y11 = *(in_ptr + src_pos + src_pitch + 2);
                    U1 = *(in_ptr + src_pos + src_pitch + 3);
                    break;
                case MODE_UYVY:
                    src_pos = (i * src_width + j) << 1;
                    U0 = *(in_ptr + src_pos + 0);
                    Y00 = *(in_ptr + src_pos + 1);
                    V0 = *(in_ptr + src_pos + 2);
                    Y01 = *(in_ptr + src_pos + 3);
                    U1 = *(in_ptr + src_pos + src_pitch + 0);
                    Y10 = *(in_ptr + src_pos + src_pitch + 1);
                    V1 = *(in_ptr + src_pos + src_pitch + 2);
                    Y11 = *(in_ptr + src_pos + src_pitch + 3);
                    break;
                case MODE_VYUY:
                    src_pos = (i * src_width + j) << 1;
                    V0 = *(in_ptr + src_pos + 0);
                    Y00 = *(in_ptr + src_pos + 1);
                    U0 = *(in_ptr + src_pos + 2);
                    Y01 = *(in_ptr + src_pos + 3);
                    V1 = *(in_ptr + src_pos + src_pitch + 0);
                    Y10 = *(in_ptr + src_pos + src_pitch + 1);
                    U1 = *(in_ptr + src_pos + src_pitch + 2);
                    Y11 = *(in_ptr + src_pos + src_pitch + 3);
                    break;
                case MODE_UV:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    Y11 = *(in_ptr + src_pos + src_pitch + 1);
                    U0 = *(in_uv_ptr + src_pos + 0);
                    V0 = *(in_uv_ptr + src_pos + 1);
                    U1 = *(in_uv_ptr + src_pos + src_pitch + 0);
                    V1 = *(in_uv_ptr + src_pos + src_pitch + 1);
                    break;
                case MODE_VU:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    Y11 = *(in_ptr + src_pos + src_pitch + 1);
                    V0 = *(in_uv_ptr + src_pos + 0);
                    U0 = *(in_uv_ptr + src_pos + 1);
                    V1 = *(in_uv_ptr + src_pos + src_pitch + 0);
                    U1 = *(in_uv_ptr + src_pos + src_pitch + 1);
                    break;
                case MODE_Y:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    Y11 = *(in_ptr + src_pos + src_pitch + 1);
                    V0 = 128;
                    U0 = 128;
                    V1 = 128;
                    U1 = 128;
                    break;
                default:
                    return AIC_ENOEXEC;
                }

                //for binning
                Y0 = (Y00 + Y01 + Y10 + Y11 + 2) >> 2;
                U = (U0 + U1 + 1) >> 1;
                V = (V0 + V1 + 1) >> 1;

                //BT601 full range
                U = U - 128;//Cb
                V = V - 128;//Cr
                R = Y0 + ((360 * V + 128) >> 8);
                G = Y0 - ((88 * U + 184 * V + 128) >> 8);
                B = Y0 + ((455 * U + 128) >> 8);
                R = CLIP(R, 0, 255);
                G = CLIP(G, 0, 255);
                B = CLIP(B, 0, 255);
                RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                rot_pos = calc_rot_addr(rot_mode, j >> 1, i >> 1, src_width >> 1, src_height >> 1);
                *(out_ptr + rot_pos) = RGB;
            }
        }
    }
    else {
        for (i = 0; i < src_height; i++) {
            in_ptr = src_ptr + i * src_pitch;
            in_uv_ptr = src_uv_ptr + i * src_pitch;
            out_ptr = dst_ptr;
            for (j = 0; j < src_width / 2; j++) {
                switch (uv_mode) {
                case MODE_YUYV:
                    Y0 = *in_ptr++;
                    U = *in_ptr++;
                    Y1 = *in_ptr++;
                    V = *in_ptr++;
                    break;
                case MODE_YVYU:
                    Y0 = *in_ptr++;
                    V = *in_ptr++;
                    Y1 = *in_ptr++;
                    U = *in_ptr++;
                    break;
                case MODE_UYVY:
                    U = *in_ptr++;
                    Y0 = *in_ptr++;
                    V = *in_ptr++;
                    Y1 = *in_ptr++;
                    break;
                case MODE_VYUY:
                    V = *in_ptr++;
                    Y0 = *in_ptr++;
                    U = *in_ptr++;
                    Y1 = *in_ptr++;
                    break;
                case MODE_UV:
                    Y0 = *in_ptr++;
                    Y1 = *in_ptr++;
                    U = *in_uv_ptr++;
                    V = *in_uv_ptr++;
                    break;
                case MODE_VU:
                    Y0 = *in_ptr++;
                    Y1 = *in_ptr++;
                    V = *in_uv_ptr++;
                    U = *in_uv_ptr++;
                    break;
                case MODE_Y:
                    Y0 = *in_ptr++;
                    Y1 = *in_ptr++;
                    V = 128;
                    U = 128;
                    break;
                default:
                    return AIC_ENOEXEC;
                }

                //BT601 full range
                U = U - 128;//Cb
                V = V - 128;//Cr
                R = Y0 + ((360 * V + 128) >> 8);
                G = Y0 - ((88 * U + 184 * V + 128) >> 8);
                B = Y0 + ((455 * U + 128) >> 8);
                R = CLIP(R, 0, 255);
                G = CLIP(G, 0, 255);
                B = CLIP(B, 0, 255);
                RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                rot_pos = calc_rot_addr(rot_mode, j << 1, i, src_width, src_height);
                *(out_ptr + rot_pos) = RGB;

                R = Y1 + ((360 * V + 128) >> 8);
                G = Y1 - ((88 * U + 184 * V + 128) >> 8);
                B = Y1 + ((455 * U + 128) >> 8);
                R = CLIP(R, 0, 255);
                G = CLIP(G, 0, 255);
                B = CLIP(B, 0, 255);
                RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                rot_pos = calc_rot_addr(rot_mode, (j << 1) + 1, i, src_width, src_height);
                *(out_ptr + rot_pos) = RGB;
            }
        }
    }

    return AIC_SUCCESS;
}

/*
 * src_ptr: src image data buffer, if 2plane, it's y plane buffer.
 * src_uv_ptr: if 2plane, it's y plane buffer; if only y, it's invalid.
 * dst_ptr: dst image data buffer
 * width: image width
 * height: image height
 * uv_mode: 4: UV, 5:VU, 6:Y
 * scl_mode: 0: no scaler, 1: bilinear scaler, 2: binning
 * rot_mode: 0: 0, 1: 90, 2: 270 */
int yuv420_to_rgb565(unsigned char* src_ptr, unsigned char* src_uv_ptr,
                     int src_width, int src_height,
                     unsigned short* dst_ptr, int dst_width, int dst_height,
                     enum YUYV_MODE uv_mode, enum SCL_MODE scl_mode, enum ROT_MODE rot_mode)
{
    int Y00, Y01, Y10, Y11, U0, V0, Y, U, V, R, G, B;
    unsigned char* in_uv_ptr = AIC_NULL;
    unsigned short* out_ptr = AIC_NULL;
    unsigned char* in_ptr = AIC_NULL;
    int rot_pos = 0, src_pos = 0;
    unsigned short RGB;
    int i, j;

    int src_pitch = src_width;

    /* for bilinear */
    if (scl_mode == 1) {
        in_ptr = src_ptr;
        in_uv_ptr = src_uv_ptr;
        out_ptr = dst_ptr;

        bilinear_scaling_for_yuv420(in_ptr, in_uv_ptr, src_width, src_height,
                                    out_ptr, dst_width, dst_height, uv_mode, rot_mode);
    } else if (scl_mode == 2) { /* for binning */
        in_ptr = src_ptr;
        in_uv_ptr = src_uv_ptr;
        out_ptr = dst_ptr;
        for (i = 0; i < src_height; i += 2) {
            for (j = 0; j < src_width; j += 2) {
                switch (uv_mode) {
                case MODE_UV:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    Y11 = *(in_ptr + src_pos + src_pitch + 1);
                    src_pos = (i >> 1) * src_width + j;
                    U0 = *(in_uv_ptr + src_pos + 0);
                    V0 = *(in_uv_ptr + src_pos + 1);
                    break;

                case MODE_VU:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    Y11 = *(in_ptr + src_pos + src_pitch + 1);
                    src_pos = (i >> 1) * src_width + j;
                    V0 = *(in_uv_ptr + src_pos + 0);
                    U0 = *(in_uv_ptr + src_pos + 1);
                    break;

                case MODE_Y:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    Y10 = *(in_ptr + src_pos + src_pitch + 0);
                    Y11 = *(in_ptr + src_pos + src_pitch + 1);
                    V0 = 128;
                    U0 = 128;
                    break;

                default:
                    return AIC_ENOEXEC;
                }

                /* for binning */
                Y = (Y00 + Y01 + Y10 + Y11 + 2) >> 2;
                U = U0;
                V = V0;

                /* BT601 full range */
                U = U - 128;
                V = V - 128;
                R = Y + ((360 * V + 128) >> 8);
                G = Y - ((88 * U + 184 * V + 128) >> 8);
                B = Y + ((455 * U + 128) >> 8);
                R = CLIP(R, 0, 255);
                G = CLIP(G, 0, 255);
                B = CLIP(B, 0, 255);
                RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                rot_pos = calc_rot_addr(rot_mode, j >> 1, i >> 1, src_width >> 1, src_height >> 1);
                *(out_ptr + rot_pos) = RGB;
            }
        }
    }
    else {
        in_ptr = src_ptr;
        in_uv_ptr = src_uv_ptr;
        out_ptr = dst_ptr;
        for (i = 0; i < src_height; i++) {
            for (j = 0; j < src_width; j += 2) {
                switch (uv_mode) {
                case MODE_UV:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    src_pos = (i >> 1) * src_width + j;
                    U0 = *(in_uv_ptr + src_pos + 0);
                    V0 = *(in_uv_ptr + src_pos + 1);
                    break;
                case MODE_VU:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    src_pos = (i >> 1) * src_width + j;
                    V0 = *(in_uv_ptr + src_pos + 0);
                    U0 = *(in_uv_ptr + src_pos + 1);
                    break;
                case MODE_Y:
                    src_pos = i * src_width + j;
                    Y00 = *(in_ptr + src_pos + 0);
                    Y01 = *(in_ptr + src_pos + 1);
                    V0 = 128;
                    U0 = 128;
                    break;
                default:
                    return AIC_ENOEXEC;
                }

                U = U0;
                V = V0;

                /* BT601 full range */
                U = U - 128;
                V = V - 128;
                R = Y00 + ((360 * V + 128) >> 8);
                G = Y00 - ((88 * U + 184 * V + 128) >> 8);
                B = Y00 + ((455 * U + 128) >> 8);
                R = CLIP(R, 0, 255);
                G = CLIP(G, 0, 255);
                B = CLIP(B, 0, 255);
                RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                rot_pos = calc_rot_addr(rot_mode, j, i, src_width, src_height);
                *(out_ptr + rot_pos) = RGB;

                R = Y01 + ((360 * V + 128) >> 8);
                G = Y01 - ((88 * U + 184 * V + 128) >> 8);
                B = Y01 + ((455 * U + 128) >> 8);
                R = CLIP(R, 0, 255);
                G = CLIP(G, 0, 255);
                B = CLIP(B, 0, 255);
                RGB = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
                rot_pos = calc_rot_addr(rot_mode, j + 1, i, src_width, src_height);
                *(out_ptr + rot_pos) = RGB;
            }
        }
    }

    return AIC_SUCCESS;
}

int32_t yuv422_yuyv_to_yuv420_y_uv(uint8_t* p_src, uint8_t* p_dst, uint16_t width, uint16_t height)
{
    uint32_t y_size = width * height;
    uint8_t* y_420 = p_dst;
    uint8_t* uv_420 = y_420 + y_size;
    int32_t x, y;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x += 2) {
            *y_420++ = p_src[(y * width + x) * 2 + 0];
            *y_420++ = p_src[(y * width + x) * 2 + 2];
            if ((y & 0x1) == 0) {
                *uv_420++ = (p_src[(y * width + x) * 2 + 1] + p_src[((y + 1) * width + x) * 2 + 1]) >> 1;
                *uv_420++ = (p_src[(y * width + x) * 2 + 3] + p_src[((y + 1) * width + x) * 2 + 3]) >> 1;
            }
        }
    }

    return 0;
}

int32_t yuv422_y_uv_to_yuv420_y_uv(uint8_t* p_src, uint8_t* p_dst, uint16_t width, uint16_t height)
{
    uint32_t y_size = width * height;
    uint8_t* y_420 = p_dst;
    uint8_t* uv_420 = y_420 + y_size;
    uint8_t* uv_422 = p_src + y_size;
    int32_t x, y;

    /* copy y data */
    if (p_src != p_dst) {
        memcpy(y_420, p_src, y_size * sizeof(uint8_t));
    }

    for (y = 0; y < height; y += 2) {
        for (x = 0; x < width; x += 2) {
            *uv_420++ = (uv_422[(y * width + x) * 1 + 0] + uv_422[((y + 1) * width + x) * 1 + 0]) >> 1;
            *uv_420++ = (uv_422[(y * width + x) * 1 + 1] + uv_422[((y + 1) * width + x) * 1 + 1]) >> 1;
        }
    }

    return 0;
}

int yuv422_scaling_by_bilinear(unsigned char* p_src_y, unsigned char* p_src_uv, uint32_t src_width, uint32_t src_height,
    unsigned char* p_dst_y, unsigned char* p_dst_uv, uint32_t dst_width, uint32_t dst_height, enum YUYV_MODE yuyv_in_mode, enum YUYV_MODE yuyv_out_mode)
{
    uint32_t y_f, x_f;
    uint32_t x_frac, y_frac;
    uint32_t y_o, x_o;
    uint32_t y_i, x_i;
    uint8_t Y00, Y01, Y10, Y11, U00, U01, U10, U11, V00, V01, V10, V11;
    int32_t Y, U, V;
    uint32_t pixh0, pixh1;
    uint32_t x_step = (src_width << FIX_BITS) / dst_width;
    uint32_t y_step = (src_height << FIX_BITS) / dst_height;
    uint8_t uv_flag = 0;
    uint8_t* dst_y = NULL;
    uint8_t* dst_u = NULL;
    uint8_t* dst_v = NULL;

    int src_pos = 0;
    /* for one plane */
    int src_pitch = src_width << 1;

    /* for two plane and only Y */
    if ((yuyv_in_mode == MODE_UV) || (yuyv_in_mode == MODE_VU) || (yuyv_in_mode == MODE_Y)) {
        src_pitch = src_width;
    }

    dst_y = p_dst_y;

    U01 = U10 = U11 = V01 = V10 = V11 = 0;

    switch (yuyv_out_mode) {
    case MODE_UV:
        dst_u = (uint8_t*)p_dst_uv;
        dst_v = (uint8_t*)p_dst_uv + 1;
        break;

    case MODE_VU:
        dst_v = (uint8_t*)p_dst_uv;
        dst_u = (uint8_t*)p_dst_uv + 1;
        break;

    case MODE_YUYV:
        dst_u = (uint8_t*)p_dst_y + 1;
        dst_v = (uint8_t*)p_dst_y + 3;
        break;

    case MODE_YVYU:
        dst_v = (uint8_t*)p_dst_y + 1;
        dst_u = (uint8_t*)p_dst_y + 3;
        break;

    case MODE_UYVY:
        dst_y = (uint8_t*)p_dst_y + 1;
        dst_u = (uint8_t*)p_dst_y + 0;
        dst_v = (uint8_t*)p_dst_y + 2;
        break;

    case MODE_VYUY:
        dst_y = (uint8_t*)p_dst_y + 1;
        dst_v = (uint8_t*)p_dst_y + 0;
        dst_u = (uint8_t*)p_dst_y + 2;
        break;

    case MODE_Y:
        break;

    default:
        return AIC_ENOTSUP;
    }

    for (y_o = 0; y_o < dst_height; y_o++) {
        for (x_o = 0; x_o < dst_width; x_o++) {
            y_f = y_o * y_step;
            x_f = x_o * x_step;

            y_i = y_f >> FIX_BITS;
            x_i = x_f >> FIX_BITS;
            y_frac = y_f & ((1 << FIX_BITS) - 1);
            x_frac = x_f & ((1 << FIX_BITS) - 1);

            uv_flag = 0;

            switch (yuyv_in_mode) {
            case MODE_YUYV:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 2);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 2);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 1);
                    U10 = *(p_src_y + src_pos + src_pitch + 1);
                }
                else {
                    U00 = *(p_src_y + src_pos - 1);
                    U01 = *(p_src_y + src_pos + 3);
                    U10 = *(p_src_y + src_pos + src_pitch - 1);
                    U11 = *(p_src_y + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 3);
                    V10 = *(p_src_y + src_pos + src_pitch + 3);
                }
                else {
                    V00 = *(p_src_y + src_pos + 1);
                    V01 = *(p_src_y + src_pos + 5);
                    V10 = *(p_src_y + src_pos + src_pitch + 1);
                    V11 = *(p_src_y + src_pos + src_pitch + 5);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }

                break;
            case MODE_YVYU:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 2);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 2);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 3);
                    U10 = *(p_src_y + src_pos + src_pitch + 3);
                }
                else {
                    U00 = *(p_src_y + src_pos + 1);
                    U01 = *(p_src_y + src_pos + 5);
                    U10 = *(p_src_y + src_pos + src_pitch + 1);
                    U11 = *(p_src_y + src_pos + src_pitch + 5);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 1);
                    V10 = *(p_src_y + src_pos + src_pitch + 1);
                }
                else {
                    V00 = *(p_src_y + src_pos - 1);
                    V01 = *(p_src_y + src_pos + 3);
                    V10 = *(p_src_y + src_pos + src_pitch - 1);
                    V11 = *(p_src_y + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }

                break;
            case MODE_UYVY:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 1);
                Y01 = *(p_src_y + src_pos + 3);
                Y10 = *(p_src_y + src_pos + src_pitch + 1);
                Y11 = *(p_src_y + src_pos + src_pitch + 3);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 0);
                    U10 = *(p_src_y + src_pos + src_pitch + 0);
                }
                else {
                    U00 = *(p_src_y + src_pos - 2);
                    U01 = *(p_src_y + src_pos + 2);
                    U10 = *(p_src_y + src_pos + src_pitch - 2);
                    U11 = *(p_src_y + src_pos + src_pitch + 2);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 2);
                    V10 = *(p_src_y + src_pos + src_pitch + 2);
                }
                else {
                    V00 = *(p_src_y + src_pos + 0);
                    V01 = *(p_src_y + src_pos + 4);
                    V10 = *(p_src_y + src_pos + src_pitch + 0);
                    V11 = *(p_src_y + src_pos + src_pitch + 4);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }

                break;
            case MODE_VYUY:
                src_pos = (y_i * src_width + x_i) << 1;
                Y00 = *(p_src_y + src_pos + 1);
                Y01 = *(p_src_y + src_pos + 3);
                Y10 = *(p_src_y + src_pos + src_pitch + 1);
                Y11 = *(p_src_y + src_pos + src_pitch + 3);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_y + src_pos + 2);
                    U10 = *(p_src_y + src_pos + src_pitch + 2);
                }
                else {
                    U00 = *(p_src_y + src_pos + 0);
                    U01 = *(p_src_y + src_pos + 4);
                    U10 = *(p_src_y + src_pos + src_pitch + 0);
                    U11 = *(p_src_y + src_pos + src_pitch + 4);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_y + src_pos + 0);
                    V10 = *(p_src_y + src_pos + src_pitch + 0);
                }
                else {
                    V00 = *(p_src_y + src_pos - 2);
                    V01 = *(p_src_y + src_pos + 2);
                    V10 = *(p_src_y + src_pos + src_pitch - 2);
                    V11 = *(p_src_y + src_pos + src_pitch + 2);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }
                break;
            case MODE_UV:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                src_pos = y_i * src_width + ((x_i >> 1) << 1);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_uv + src_pos + 0);
                    U10 = *(p_src_uv + src_pos + src_pitch + 0);
                }
                else {
                    U00 = *(p_src_uv + src_pos + 0);
                    U01 = *(p_src_uv + src_pos + 2);
                    U10 = *(p_src_uv + src_pos + src_pitch + 0);
                    U11 = *(p_src_uv + src_pos + src_pitch + 2);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_uv + src_pos + 1);
                    V10 = *(p_src_uv + src_pos + src_pitch + 1);
                }
                else {
                    V00 = *(p_src_uv + src_pos + 1);
                    V01 = *(p_src_uv + src_pos + 3);
                    V10 = *(p_src_uv + src_pos + src_pitch + 1);
                    V11 = *(p_src_uv + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }
                break;
            case MODE_VU:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                src_pos = y_i * src_width + ((x_i >> 1) << 1);

                if ((x_i & 0x1) == 0) {
                    U00 = *(p_src_uv + src_pos + 1);
                    U10 = *(p_src_uv + src_pos + src_pitch + 1);
                }
                else {
                    U00 = *(p_src_uv + src_pos + 1);
                    U01 = *(p_src_uv + src_pos + 3);
                    U10 = *(p_src_uv + src_pos + src_pitch + 1);
                    U11 = *(p_src_uv + src_pos + src_pitch + 3);
                }

                if ((x_i & 0x1) == 0) {
                    V00 = *(p_src_uv + src_pos + 0);
                    V10 = *(p_src_uv + src_pos + src_pitch + 0);
                }
                else {
                    V00 = *(p_src_uv + src_pos + 0);
                    V01 = *(p_src_uv + src_pos + 2);
                    V10 = *(p_src_uv + src_pos + src_pitch + 0);
                    V11 = *(p_src_uv + src_pos + src_pitch + 2);
                }

                if ((x_i & 0x1) == 0) {
                    uv_flag = 0;
                }
                else {
                    uv_flag = 1;
                }
                break;
            case MODE_Y:
                src_pos = y_i * src_width + x_i;
                Y00 = *(p_src_y + src_pos + 0);
                Y01 = *(p_src_y + src_pos + 1);
                Y10 = *(p_src_y + src_pos + src_pitch + 0);
                Y11 = *(p_src_y + src_pos + src_pitch + 1);

                U00 = 128;
                V00 = 128;

                uv_flag = 2;
                break;
            default:
                return AIC_ENOEXEC;
            }

            /* for bilinear */
            pixh0 = argb_linear_avg(Y00, Y01, x_frac);
            pixh1 = argb_linear_avg(Y10, Y11, x_frac);
            Y = argb_linear_avg(pixh0, pixh1, y_frac);
            if (uv_flag == 0) {
                U = argb_linear_avg(U00, U10, y_frac);
            }
            else if (uv_flag == 1) {
                pixh0 = argb_linear_avg(U00, U01, x_frac);
                pixh1 = argb_linear_avg(U10, U11, x_frac);
                U = argb_linear_avg(pixh0, pixh1, y_frac);
            }
            else {
                U = U00;
            }
            if (uv_flag == 0) {
                V = argb_linear_avg(V00, V10, y_frac);
            }
            else if (uv_flag == 1) {
                pixh0 = argb_linear_avg(V00, V01, x_frac);
                pixh1 = argb_linear_avg(V10, V11, x_frac);
                V = argb_linear_avg(pixh0, pixh1, y_frac);
            }
            else {
                V = V00;
            }

            if (yuyv_out_mode < MODE_Y) {
                *dst_y = Y;
                dst_y += 2;
                if ((x_o & 0x1) == 0) {
                    *dst_u = U;
                }
                else {
                    *dst_u = (*dst_u + U + 1) >> 1;
                    dst_u += 4;
                }
                if ((x_o & 0x1) == 0) {
                    *dst_v = V;
                }
                else {
                    *dst_v = (*dst_v + V + 1) >> 1;
                    dst_v += 4;
                }
            } else if (yuyv_out_mode == MODE_Y) {
                *dst_y++ = Y;
            } else {
                *dst_y++ = Y;
                if ((x_o & 0x1) == 0) {
                    *dst_u = U;
                }
                else {
                    *dst_u = (*dst_u + U + 1) >> 1;
                    dst_u += 2;
                }
                if ((x_o & 0x1) == 0) {
                    *dst_v = V;
                }
                else {
                    *dst_v = (*dst_v + V + 1) >> 1;
                    dst_v += 2;
                }
            }
        }
    }

    return AIC_SUCCESS;
}

int yuv422_scaling_by_bining(unsigned char* p_src_y, unsigned char* p_src_uv, uint32_t src_width, uint32_t src_height,
    unsigned char* p_dst_y, unsigned char* p_dst_uv, enum YUYV_MODE yuyv_in_mode, enum YUYV_MODE yuyv_out_mode)
{
    uint8_t* dst_y = AIC_NULL;
    uint8_t* dst_u = AIC_NULL;
    uint8_t* dst_v = AIC_NULL;
    int Y00, Y01, Y10, Y11, U0, V0, U1, V1, Y0, U, V;
    unsigned char* in_ptr = AIC_NULL;
    unsigned char* in_uv_ptr = AIC_NULL;
    int i, j;
    int src_pos = 0;
    /* for one plane */
    int src_pitch = src_width << 1;

    /* for two plane and only Y */
    if ((yuyv_in_mode == MODE_UV) || (yuyv_in_mode == MODE_VU)) {
        src_pitch = src_width;
    }

    in_ptr = p_src_y;
    in_uv_ptr = p_src_uv;

    dst_y = p_dst_y;

    switch (yuyv_out_mode) {
    case MODE_UV:
        dst_u = (uint8_t*)p_dst_uv;
        dst_v = (uint8_t*)p_dst_uv + 1;
        break;

    case MODE_VU:
        dst_v = (uint8_t*)p_dst_uv;
        dst_u = (uint8_t*)p_dst_uv + 1;
        break;

    case MODE_YUYV:
        dst_u = (uint8_t*)p_dst_y + 1;
        dst_v = (uint8_t*)p_dst_y + 3;
        break;

    case MODE_YVYU:
        dst_v = (uint8_t*)p_dst_y + 1;
        dst_u = (uint8_t*)p_dst_y + 3;
        break;

    case MODE_UYVY:
        dst_y = (uint8_t*)p_dst_y + 1;
        dst_u = (uint8_t*)p_dst_y + 0;
        dst_v = (uint8_t*)p_dst_y + 2;
        break;

    case MODE_VYUY:
        dst_y = (uint8_t*)p_dst_y + 1;
        dst_v = (uint8_t*)p_dst_y + 0;
        dst_u = (uint8_t*)p_dst_y + 2;
        break;

    case MODE_Y:
        break;

    default:
        return AIC_ENOTSUP;
    }

    for (i = 0; i < src_height; i += 2) {
        for (j = 0; j < src_width; j += 2) {
            switch (yuyv_in_mode) {
            case MODE_YUYV:
                src_pos = (i * src_width + j) << 1;
                Y00 = *(in_ptr + src_pos + 0);
                U0 = *(in_ptr + src_pos + 1);
                Y01 = *(in_ptr + src_pos + 2);
                V0 = *(in_ptr + src_pos + 3);
                Y10 = *(in_ptr + src_pos + src_pitch + 0);
                U1 = *(in_ptr + src_pos + src_pitch + 1);
                Y11 = *(in_ptr + src_pos + src_pitch + 2);
                V1 = *(in_ptr + src_pos + src_pitch + 3);
                break;
            case MODE_YVYU:
                src_pos = (i * src_width + j) << 1;
                Y00 = *(in_ptr + src_pos + 0);
                V0 = *(in_ptr + src_pos + 1);
                Y01 = *(in_ptr + src_pos + 2);
                U0 = *(in_ptr + src_pos + 3);
                Y10 = *(in_ptr + src_pos + src_pitch + 0);
                V1 = *(in_ptr + src_pos + src_pitch + 1);
                Y11 = *(in_ptr + src_pos + src_pitch + 2);
                U1 = *(in_ptr + src_pos + src_pitch + 3);
                break;
            case MODE_UYVY:
                src_pos = (i * src_width + j) << 1;
                U0 = *(in_ptr + src_pos + 0);
                Y00 = *(in_ptr + src_pos + 1);
                V0 = *(in_ptr + src_pos + 2);
                Y01 = *(in_ptr + src_pos + 3);
                U1 = *(in_ptr + src_pos + src_pitch + 0);
                Y10 = *(in_ptr + src_pos + src_pitch + 1);
                V1 = *(in_ptr + src_pos + src_pitch + 2);
                Y11 = *(in_ptr + src_pos + src_pitch + 3);
                break;
            case MODE_VYUY:
                src_pos = (i * src_width + j) << 1;
                V0 = *(in_ptr + src_pos + 0);
                Y00 = *(in_ptr + src_pos + 1);
                U0 = *(in_ptr + src_pos + 2);
                Y01 = *(in_ptr + src_pos + 3);
                V1 = *(in_ptr + src_pos + src_pitch + 0);
                Y10 = *(in_ptr + src_pos + src_pitch + 1);
                U1 = *(in_ptr + src_pos + src_pitch + 2);
                Y11 = *(in_ptr + src_pos + src_pitch + 3);
                break;
            case MODE_UV:
                src_pos = i * src_width + j;
                Y00 = *(in_ptr + src_pos + 0);
                Y01 = *(in_ptr + src_pos + 1);
                Y10 = *(in_ptr + src_pos + src_pitch + 0);
                Y11 = *(in_ptr + src_pos + src_pitch + 1);
                U0 = *(in_uv_ptr + src_pos + 0);
                V0 = *(in_uv_ptr + src_pos + 1);
                U1 = *(in_uv_ptr + src_pos + src_pitch + 0);
                V1 = *(in_uv_ptr + src_pos + src_pitch + 1);
                break;
            case MODE_VU:
                src_pos = i * src_width + j;
                Y00 = *(in_ptr + src_pos + 0);
                Y01 = *(in_ptr + src_pos + 1);
                Y10 = *(in_ptr + src_pos + src_pitch + 0);
                Y11 = *(in_ptr + src_pos + src_pitch + 1);
                V0 = *(in_uv_ptr + src_pos + 0);
                U0 = *(in_uv_ptr + src_pos + 1);
                V1 = *(in_uv_ptr + src_pos + src_pitch + 0);
                U1 = *(in_uv_ptr + src_pos + src_pitch + 1);
                break;
            default:
                return AIC_ENOEXEC;
            }

            /* for binning */
            Y0 = (Y00 + Y01 + Y10 + Y11 + 2) >> 2;
            U = (U0 + U1 + 1) >> 1;
            V = (V0 + V1 + 1) >> 1;

            int x_pos = j >> 1;

            /* one plane mode */
            if (yuyv_out_mode < MODE_Y) {
                *dst_y = Y0;
                dst_y += 2;
                if ((x_pos & 0x1) == 0) {
                    *dst_u = U;
                } else {
                    *dst_u = (*dst_u + U + 1) >> 1;
                    dst_u += 4;
                }

                    if ((x_pos & 0x1) == 0) {
                    *dst_v = V;
                } else {
                    *dst_v = (*dst_v + V + 1) >> 1;
                    dst_v += 4;
                }
            } else if (yuyv_out_mode == MODE_Y) {
                *dst_y = Y0;
                dst_y++;
            } else {
                *dst_y++ = Y0;
                if ((x_pos & 0x1) == 0) {
                    *dst_u = U;
                }
                else {
                    *dst_u = (*dst_u + U + 1) >> 1;
                    dst_u += 2;
                }
                if ((x_pos & 0x1) == 0) {
                    *dst_v = V;
                }
                else {
                    *dst_v = (*dst_v + V + 1) >> 1;
                    dst_v += 2;
                }
            }
        }
    }

    return AIC_SUCCESS;
}

/*
 * src_ptr: src image data buffer, if 2plane, it's y plane buffer.
 * src_uv_ptr: if 2plane, it's y plane buffer; if 1plane, it's invalid.
 * dst_ptr: dst image data buffer
 * width: image width
 * height: image height
 * yuv_in_mode: yuv422 input mode 0: YUYV, 1: YVYU, 2: UYVY, 3: VYUY, 4: Y, 5: UV, 6:VU
 * scl_mode: 0: no scaler, 1: bilinear 2: bining
 * yuyv_out_mode: yuv422 output format, 0: YUYV, 1: YVYU, 2: UYVY, 3: VYUY, 4: Y, 5: UV, 6:VU */
int yuv422_scaler(unsigned char* src_ptr, unsigned char* src_uv_ptr,
                  int src_width, int src_height,
                  unsigned char* dst_ptr, unsigned char* dst_uv_ptr,
                  int dst_width, int dst_height,
                  enum YUYV_MODE yuyv_in_mode, enum SCL_MODE scl_mode,
                  enum YUYV_MODE yuyv_out_mode)
{
    unsigned char* in_ptr = AIC_NULL;
    unsigned char* in_uv_ptr = AIC_NULL;
    unsigned char* out_ptr = AIC_NULL;
    unsigned char* out_uv_ptr = AIC_NULL;

    /* for one plane */
    int src_pitch = src_width << 1;

    /* for two plane and only Y */
    if ((yuyv_in_mode == MODE_UV) || (yuyv_in_mode == MODE_VU)) {
        src_pitch = src_width;
    }

    in_ptr = src_ptr;
    in_uv_ptr = src_uv_ptr;
    out_ptr = dst_ptr;
    out_uv_ptr = dst_uv_ptr;

    /* for bilinear */
    if (scl_mode == BILINEAR_SCALER) {
        yuv422_scaling_by_bilinear(in_ptr, in_uv_ptr, src_width, src_height,
            out_ptr, out_uv_ptr, dst_width, dst_height, yuyv_in_mode, yuyv_out_mode);
    }
    else if (scl_mode == BINNING) { /* for binning */
        yuv422_scaling_by_bining(in_ptr, in_uv_ptr, src_width, src_height,
            out_ptr, out_uv_ptr, yuyv_in_mode, yuyv_out_mode);
    }

    return AIC_SUCCESS;
}

static int32_t yuv420_scaling_by_bilinear(unsigned char* p_src_y, unsigned char* p_src_uv, uint32_t src_width, uint32_t src_height,
    unsigned char* p_dst_y, unsigned char* p_dst_uv, uint32_t dst_width, uint32_t dst_height, enum YUYV_MODE uv_mode)
{
    uint32_t y_f, x_f;
    uint32_t x_frac, y_frac;
    uint32_t y_o, x_o;
    uint32_t y_i, x_i;
    uint8_t Y00, Y01, Y10, Y11, U00, U01, U10, U11, V00, V01, V10, V11;
    int32_t U, V;
    uint32_t pixh0, pixh1;
    uint32_t x_step = (src_width << FIX_BITS) / dst_width;
    uint32_t y_step = (src_height << FIX_BITS) / dst_height;
    uint8_t* dst_y = NULL;
    uint8_t* dst_u = NULL;
    uint8_t* dst_v = NULL;

    int src_pos = 0;
    int src_pitch = src_width;

    dst_y = p_dst_y;

    if (MODE_UV == uv_mode) {
        dst_u = (uint8_t*)p_dst_uv;
        dst_v = (uint8_t*)p_dst_uv + 1;
    }
    else if (MODE_VU == uv_mode) {
        dst_v = (uint8_t*)p_dst_uv;
        dst_u = (uint8_t*)p_dst_uv + 1;
    }

    for (y_o = 0; y_o < dst_height; y_o++) {
        for (x_o = 0; x_o < dst_width; x_o++) {
            y_f = y_o * y_step;
            x_f = x_o * x_step;

            y_i = y_f >> FIX_BITS;
            x_i = x_f >> FIX_BITS;
            y_frac = y_f & ((1 << FIX_BITS) - 1);
            x_frac = x_f & ((1 << FIX_BITS) - 1);

            src_pos = y_i * src_width + x_i;
            Y00 = *(p_src_y + src_pos + 0);
            Y01 = *(p_src_y + src_pos + 1);
            Y10 = *(p_src_y + src_pos + src_pitch + 0);
            Y11 = *(p_src_y + src_pos + src_pitch + 1);

            /* for bilinear */
            pixh0 = argb_linear_avg(Y00, Y01, x_frac);
            pixh1 = argb_linear_avg(Y10, Y11, x_frac);
            *dst_y++ = argb_linear_avg(pixh0, pixh1, y_frac);
        }
    }

    if (uv_mode == MODE_Y) {
        return AIC_SUCCESS;
    }

    for (y_o = 0; y_o < dst_height / 2; y_o++) {
        for (x_o = 0; x_o < dst_width / 2; x_o++) {
            y_f = y_o * y_step;
            x_f = x_o * x_step;

            y_i = y_f >> FIX_BITS;
            x_i = x_f >> FIX_BITS;
            y_frac = y_f & ((1 << FIX_BITS) - 1);
            x_frac = x_f & ((1 << FIX_BITS) - 1);

            switch (uv_mode) {
            case MODE_UV:
                src_pos = y_i * src_width + x_i * 2;
                U00 = *(p_src_uv + src_pos + 0);
                V00 = *(p_src_uv + src_pos + 1);
                U01 = *(p_src_uv + src_pos + 2);
                V01 = *(p_src_uv + src_pos + 3);
                U10 = *(p_src_uv + src_pos + src_pitch + 0);
                V10 = *(p_src_uv + src_pos + src_pitch + 1);
                U11 = *(p_src_uv + src_pos + src_pitch + 2);
                V11 = *(p_src_uv + src_pos + src_pitch + 3);
                break;
            case MODE_VU:
                src_pos = y_i * src_width + x_i * 2;
                V00 = *(p_src_uv + src_pos + 0);
                U00 = *(p_src_uv + src_pos + 1);
                V01 = *(p_src_uv + src_pos + 2);
                U01 = *(p_src_uv + src_pos + 3);
                V10 = *(p_src_uv + src_pos + src_pitch + 0);
                U10 = *(p_src_uv + src_pos + src_pitch + 1);
                V11 = *(p_src_uv + src_pos + src_pitch + 2);
                U11 = *(p_src_uv + src_pos + src_pitch + 3);
                break;
            default:
                return AIC_ENOEXEC;
            }

            pixh0 = argb_linear_avg(U00, U01, x_frac);
            pixh1 = argb_linear_avg(U10, U11, x_frac);
            U = argb_linear_avg(pixh0, pixh1, y_frac);

            pixh0 = argb_linear_avg(V00, V01, x_frac);
            pixh1 = argb_linear_avg(V10, V11, x_frac);
            V = argb_linear_avg(pixh0, pixh1, y_frac);

            *dst_u = U;
            dst_u += 2;
            *dst_v = V;
            dst_v += 2;
        }
    }

    return 0;
}

static int32_t yuv420_scaling_by_bining(unsigned char* p_src_y, unsigned char* p_src_uv, uint32_t src_width, uint32_t src_height,
    unsigned char* p_dst_y, unsigned char* p_dst_uv, enum YUYV_MODE uv_mode)
{
    uint8_t* dst_y = AIC_NULL;
    uint8_t* dst_u = AIC_NULL;
    uint8_t* dst_v = AIC_NULL;
    int Y00, Y01, Y10, Y11, U, V, U0, U1, U2, U3, V0, V1, V2, V3;
    unsigned char* in_ptr = AIC_NULL;
    unsigned char* in_uv_ptr = AIC_NULL;
    int i, j;
    int src_pos = 0;
    int src_pitch = src_width;

    in_ptr = p_src_y;
    in_uv_ptr = p_src_uv;

    dst_y = p_dst_y;

    if (MODE_UV == uv_mode) {
        dst_u = (uint8_t*)p_dst_uv;
        dst_v = (uint8_t*)p_dst_uv + 1;
    }
    else if (MODE_VU == uv_mode) {
        dst_v = (uint8_t*)p_dst_uv;
        dst_u = (uint8_t*)p_dst_uv + 1;
    }

    for (i = 0; i < src_height; i += 2) {
        for (j = 0; j < src_width; j += 2) {
            src_pos = i * src_width + j;
            Y00 = *(in_ptr + src_pos + 0);
            Y01 = *(in_ptr + src_pos + 1);
            Y10 = *(in_ptr + src_pos + src_pitch + 0);
            Y11 = *(in_ptr + src_pos + src_pitch + 1);

            /* for binning */
            *dst_y++ = (Y00 + Y01 + Y10 + Y11 + 2) >> 2;
        }
    }

    if (uv_mode == MODE_Y) {
        return AIC_SUCCESS;
    }

    for (i = 0; i < src_height / 2; i += 2) {
        for (j = 0; j < src_width / 2; j += 2) {
            switch (uv_mode) {
            case MODE_UV:
                src_pos = i * src_width + j * 2;
                U0 = *(in_uv_ptr + src_pos + 0);
                V0 = *(in_uv_ptr + src_pos + 1);
                U1 = *(in_uv_ptr + src_pos + 2);
                V1 = *(in_uv_ptr + src_pos + 3);
                U2 = *(in_uv_ptr + src_pos + src_pitch + 0);
                V2 = *(in_uv_ptr + src_pos + src_pitch + 1);
                U3 = *(in_uv_ptr + src_pos + src_pitch + 2);
                V3 = *(in_uv_ptr + src_pos + src_pitch + 3);
                break;
            case MODE_VU:
                src_pos = i * src_width + j * 2;
                V0 = *(in_uv_ptr + src_pos + 0);
                U0 = *(in_uv_ptr + src_pos + 1);
                V1 = *(in_uv_ptr + src_pos + 2);
                U1 = *(in_uv_ptr + src_pos + 3);
                V2 = *(in_uv_ptr + src_pos + src_pitch + 0);
                U2 = *(in_uv_ptr + src_pos + src_pitch + 1);
                V3 = *(in_uv_ptr + src_pos + src_pitch + 2);
                U3 = *(in_uv_ptr + src_pos + src_pitch + 3);
                break;
            default:
                return AIC_ENOTSUP;
            }

            //for binning
            U = (U0 + U1 + U2 + U3 + 2) >> 2;
            V = (V0 + V1 + V2 + V3 + 2) >> 2;

            *dst_u = U;
            dst_u += 2;
            *dst_v = V;
            dst_v += 2;
        }
    }

    return AIC_SUCCESS;
}

/*
 * src_ptr: src image data buffer, if 2plane, it's y plane buffer.
 * src_uv_ptr: if 2plane, it's y plane buffer; if 1plane, it's invalid.
 * dst_ptr: dst image data buffer
 * width: image width
 * height: image height
 * uv_mode: 0: YUYV, 1: YVYU, 2: UYVY, 3: VYUY, 4: Y, 5: UV, 6:VU
 * scl_mode: 0: no scaler, 1: bilinear 2: bining */
int yuv420_scaler(unsigned char* src_ptr, unsigned char* src_uv_ptr,
                  int src_width, int src_height,
                  unsigned char* dst_ptr, unsigned char* dst_uv_ptr,
                  int dst_width, int dst_height,
                  enum YUYV_MODE uv_mode, enum SCL_MODE scl_mode)
{
    unsigned char* in_ptr = AIC_NULL;
    unsigned char* in_uv_ptr = AIC_NULL;
    unsigned char* out_ptr = AIC_NULL;
    unsigned char* out_uv_ptr = AIC_NULL;

    in_ptr = src_ptr;
    in_uv_ptr = src_uv_ptr;
    out_ptr = dst_ptr;
    out_uv_ptr = dst_uv_ptr;

    /* for bilinear */
    if (scl_mode == BILINEAR_SCALER) {
        yuv420_scaling_by_bilinear(in_ptr, in_uv_ptr, src_width, src_height,
            out_ptr, out_uv_ptr, dst_width, dst_height, uv_mode);
    }
    else if (scl_mode == BINNING) { /* for binning */
        yuv420_scaling_by_bining(in_ptr, in_uv_ptr, src_width, src_height,
            out_ptr, out_uv_ptr, uv_mode);
    }

    return AIC_SUCCESS;
}

/* convert yuv420 y_uv to yuv420 y_u_v */
int nv12_to_i420(unsigned char *psrc, unsigned char *pdst, int width, int height)
{
    int i, y_cnt;
    unsigned char *pv, *puv;

    y_cnt = width * height;
    /* copy y */
    for (i = 0; i < y_cnt; i++)
        *pdst++ = *psrc++;

    pv = pdst + y_cnt / 4;
    puv = psrc;

    /* split uv plane to u plane and v plane */
    for (i = 0; i < y_cnt / 2; i++) {
        if (i % 2 == 0) {
            *pdst++ = *psrc;
        } else {
            *pv++ = *psrc;
        }
        psrc++;
    }

    return AIC_SUCCESS;
}
