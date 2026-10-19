/*****************************************************************************
 * Tencent is pleased to support the open source community by making IoT Video available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 *is distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 *or implied. See the License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file    iv_voip.h
 * @brief   Description audio and video function of sdk
 * @version v1.0.0
 *
 *****************************************************************************/

#ifndef __IV_VOIP_H__
#define __IV_VOIP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>
#include <stdint.h>

#include "iv_cm.h"

#include "wmpf/error.h"
#include "wmpf/wxvoip_os_impl.h"

typedef enum voip_wxa_type {
  VOIP_WXA_TYPE_RELEASE = 0,  // 小程序正式版
  VOIP_WXA_TYPE_DEBUG = 1,    // 小程序开发版
  VOIP_WXA_TYPE_DEMO = 2,     // 小程序体验版
} voip_wxa_type_t;

/* 暂不支持 发H264,收H265 组合*/
typedef struct voip_video_info_s {
  iv_cm_venc_type_e send_v_enc;          // 设备上行发送视频编码格式
  iv_cm_venc_type_e recv_v_enc;          // 设备下行接收视频编码格式
  iv_cm_pixel_type_e recv_pixel;         // 设备下行接收视频分辨率
} voip_video_info_s;

/**
 * @brief 初始化voip功能
 * 
 * @param hal hal接口
 * @param type 小程序类型
 * @param data_path voip 自动生成的设置文件存储路径
 * @param model_id voip model_id
 * @param device_id voip device_id
 * @param wxa_appid voip 微信小程序 wxa_appid
 * @param ticket voip 微信小程序绑定 ticket
 * @return int 0正常，其他值异常
 */
int iv_avt_voip_init(wxvoip_os_impl_t *hal, voip_wxa_type_t type,
                     const char *data_path, const char *model_id,
                     const char *device_id, const char *wxa_appid, const char *ticket);

/**
 * @brief 退出voip功能
 * 
 */
void iv_avt_voip_exit();

/**
 * @brief voip呼叫
 * 
 * @param type 呼叫类型，音视频，或仅音频
 * @param open_id voip open_id
 * @param device_id voip device_id
 * @param model_id voip model_id
 * @param wxa_appid voip 微信小程序 wxa_appid
 * @param v_info 设置希望接收到的视频数据格式，仅当呼叫类型为音视频时生效
 * @param caller_camera_switch 主叫端摄像头开关，0关闭，1开启，如果设备端不具备摄像头或不需要开启摄像头，请设置为关闭
 * @param callee_camera_switch 被叫端摄像头开关，0关闭，1开启，如果设备端不具备屏幕或不需要查看微信用户的摄像头内容，请设置为关闭
 * @return int 0正常，其他值异常
 */
int iv_avt_voip_call(iv_cm_stream_type_e type, const char *open_id, const char *device_id,
                     const char *model_id, const char *wxa_appid, voip_video_info_s v_info,
                     uint32_t caller_camera_switch, uint32_t callee_camera_switch);

/**
 * @brief 呼叫挂断
 * 
 * @return int 0正常，其他值异常
 */
int iv_avt_voip_hang_up();

/**
 * @brief 是否占线
 * 
 * @return int 0无占线，1占线
 */
int iv_avt_voip_is_busy();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __IV_AV_H__ */
