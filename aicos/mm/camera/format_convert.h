/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __FORMAT_CONVERT_H__
#define __FORMAT_CONVERT_H__

enum YUYV_MODE {
    MODE_YUYV = 0,
    MODE_YVYU,
    MODE_UYVY,
    MODE_VYUY,
    MODE_Y,
    MODE_UV,
    MODE_VU
};

enum ROT_MODE {
    ROT_MODE_NONE = 0,
    ROT_MODE_ANTI_CW90,
    ROT_MODE_ANTI_CW270
};

/*
 * NO_SCALER: no scaling
 * BILINEAR_SCALER: arbitrary scaling
 * BINNING: 1/2 fixed scaling and scaler algorithm is binning
 */
enum SCL_MODE {
    NO_SCALER = 0,
    BILINEAR_SCALER,
    BINNING,
};

int32_t bilinear_scaling_for_yuv422(unsigned char* p_src_y, unsigned char* p_src_uv,
                                    uint32_t src_width, uint32_t src_height,
                                    unsigned short* p_dst, uint32_t dst_width, uint32_t dst_height,
                                    enum YUYV_MODE uv_mode, enum ROT_MODE rot_mode);

int32_t bilinear_scaling_for_yuv420(unsigned char* p_src_y, unsigned char* p_src_uv,
                                    uint32_t src_width, uint32_t src_height,
                                    unsigned short* p_dst, uint32_t dst_width, uint32_t dst_height,
                                    enum YUYV_MODE uv_mode, enum ROT_MODE rot_mode);

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
                     enum ROT_MODE rot_mode);

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
                     enum YUYV_MODE uv_mode, enum SCL_MODE scl_mode, enum ROT_MODE rot_mode);

int32_t yuv422_yuyv_to_yuv420_y_uv(uint8_t* p_src, uint8_t* p_dst, uint16_t width, uint16_t height);
int32_t yuv422_y_uv_to_yuv420_y_uv(uint8_t* p_src, uint8_t* p_dst, uint16_t width, uint16_t height);
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
                  enum YUYV_MODE yuyv_out_mode);
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
                  enum YUYV_MODE yuv_mode, enum SCL_MODE scl_mode);

int nv12_to_i420(unsigned char *psrc, unsigned char *pdst, int width, int height);
#endif /* __FORMAT_CONVERT_H__ */

