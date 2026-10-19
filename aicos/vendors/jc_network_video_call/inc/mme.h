#ifndef __MME_H__
#define __MME_H__
#include "jrtc.h"
#ifdef __cplusplus
extern "C" {
#endif
/** MME_系列接口是低层的媒体设备操作层
 * 与 jrtc 的主要交互是基于标准 RTP 包格式:
 * MME_ReadAudioRTP, MME_WriteAudioRTP;
 * MME_ReadVideoRTP, MME_WriteVideoRTP;
 * 音频回环自测样例:
 * @code
 *  char rtp[1400];
 *  const int rtpHeaderLength = 12;
 *  unsigned short sequenceNumber = 0;
 *  while (1) {
 *      const int rtpLen = MME_ReadAudioRTP(rtp, sizeof(rtp), rtpHeaderLength, 20);
 *      assert(rtpLen >= 0);
 *      if (rtpLen > 0) {
 *          rtp[0] = 0x80;
 *          rtp[1] = 104;
 *          rtp_pack_uint16(rtp+2, sequenceNumber++);
 *          rtp_pack_uint32(rtp+8, 0x6d692aa2);
 *          const int ret = MME_WriteAudioRTP(rtp, rtpHeaderLength, rtpLen);
 *          assert(ret == 0);
 *      }
 *  }
 * @endcode
 * 按第2种读取方式(一帧多次连续读取),视频回环自测样例:
 * @code
 *  const int rtpHeaderLength = 12;
 *  unsigned short sequenceNumber = 0;
 *  while (1) {
 *      MMEImage camera={0}, video={0};
 *      struct rtp_buf_t frame = {0};
 *      unsigned nowMs = jc_time();
 *      frame.size = 1400;
 *      int rtpLen = MME_ReadVideoRTP(&frame, frame.size, rtpHeaderLength, &camera, 80);
 *      assert (rtpLen >= 0);
 *      while (rtpLen > rtpHeaderLength) {
 *          unsigned char* rtp = (unsigned char*)frame.buf;
 *          const char maker = rtp[1]&0x80;
 *          rtp[0] = 0x80;
 *          rtp[1] |= 117;
 *          rtp_pack_uint16(rtp+2, sequenceNumber++);
 *          rtp_pack_uint32(rtp+4, nowMs*90);
 *          rtp_pack_uint32(rtp+8, 0x6d692aa3);
 *          const int ret = MME_WriteVideoRTP(rtp, rtpHeaderLength, rtpLen, maker ? &video : NULL);
 *          assert (ret == 0);
 *          if (maker) break; //一帧结束
 *          rtpLen = MME_ReadVideoRTP(&frame, frame.size, rtpHeaderLength, NULL, 0));
 *          assert (rtpLen >= rtpHeaderLength);
 *      }
 *  }
 * @endcode
 * 按第1种读取方式(一帧单次批量读取),视频回环自测样例:
 * @code
 *  const int rtpHeaderLength = 12;
 *  unsigned short sequenceNumber = 0;
 *  while (1) {
 *      MMEImage camera={0}, video={0};
 *      struct rtp_buf_t frame = {0};
 *      unsigned nowMs = jc_time();
 *      frame.size = 0;
 *      int rtpNum = MME_ReadVideoRTP(&frame, 1400, rtpHeaderLength, &camera, 80);
 *      assert (rtpLen >= 0);
 *      char *rtp = frame.buf;
 *      for (int i = 0; i < rtpNum; rtp += frame.len[i++]) {
 *          rtp[0] = 0x80; // version 2
 *          rtp[1] |= 117;  //payload type
 *          rtp_pack_uint16(rtp+2, sequenceNumber++);
 *          rtp_pack_uint32(rtp+4, nowMs*90);
 *          rtp_pack_uint32(rtp+8, 0x6d692aa3);
 *          MME_WriteVideoRTP(rtp, rtpHeaderLength, frame.len[i], &video);
 *      }
 *  }
 * @endcode
 */

/** 支持的音频编解码名称, 默认只支持单声道
 * @return 返回以逗号分割的多个编解码名称
 * 例如:
 * @code
 *  const char* MME_AudioCodecName() {
 *      return "PCMA/8000,AMR-WB/16000";
 *  }
 * @endcode
 */
const char* MME_AudioCodecName(void);

/** 打开音频资源
 * @param[in] payloadType  RTP包中的Payload Type值
 * @param[in] sendCodecIndex 指定编码器序号
 * @param[in] recvCodecIndex 指定解码器序号
 * @remarks
 *  序号是指在 MME_AudioCodecName() 中的序号，从0开始
 * @return 成功返回 0
 */
int MME_StartAudio(int payloadType, int sendCodecIndex, int recvCodecIndex);

/** 读取 RTP 包
 * @param[in] rtp 将填写的 RTP 包, NULL 时表示需要丢弃该次操作
 * @param[in] rtpBufferSize RTP包的最大缓冲区
 * @param[in] rtpHeaderLength RTP包的头部长度
 * @param[in] timeoutMs 无数据时允许等待的时长. 0时不允许等待立即返回
 * @return 成功返回RTP包的实际长度,无数据返回0
 *
 * @note 只须填写 RTP 包中 TS 和 载荷 部分.
 *  通常打包时长 PTIME_MS = 20, SampleRateKhz = 8KHZ,
 *  _packetSequenceNumber 是该RTP中的最大包序号(从0开始), 则
 *  unsigned ts = PTIME_MS * SampleRateKhz * _packetSequenceNumber;
 *  unsigned char* p = (unsigned char*)rtp;
 *  p[4] = (unsigned char)(ts >> 24);
 *  p[5] = (unsigned char)(ts >> 16);
 *  p[6] = (unsigned char)(ts >> 8);
 *  p[7] = (unsigned char)(ts);
 *
 * @remarks 特别注意
 * 1. _packetSequenceNumber 是递增的, 包括无声或不编码的包。
 *    本质上, 是将唯一的包序号, 转换为对应的时间截, 两者一一对应。
 *
 * 2. 当界面jrtc_deactivate关闭发送时, rtp 为NULL, 表示需要丢弃该次操作
 *    - 音频,直接跳过采集的当前包，返回0
 *      但是, 包的计数要更新的 _packetSequenceNumber
 *    - 视频,直接跳过当前采集图像，返回0
 *      但是, 若是image不为null, 说明界面需要渲染预览，需要填充
 */
int MME_ReadAudioRTP(void* rtp, int rtpBufferSize, int rtpHeaderLength, unsigned timeoutMs);

/** 写入 RTP 包
 * 内部需要将RTP解码，然后扬声器播放
 * @param[in] rtp RTP包
 * @param[in] rtpHeaderLength RTP包的头部长度
 * @param[in] rtpLen RTP包的实际长度
 * @return 成功返回0
 */
int MME_WriteAudioRTP(void*rtp, int rtpHeaderLength, int rtpLen);

/** 动态控制播放是否静音 */
void MME_MutePlayout(char is_mute);

/** 设置是否被外部音频通话中断
 * 在MME_StopAudio()内部有可能需要
 * 针对该中断特殊处理设备.
 */
void MME_OnAudioCall(void);

/** 关闭释放音频资源 */
void MME_StopAudio(void);

typedef struct {
    unsigned short sendWidth;
    unsigned short sendHeight;
    unsigned char  sendFps;
    /* 指定编码器序号, 序号是指在 MME_VideoCodecName() 中的序号，从0开始 */
    signed char  sendCodecIndex;
    unsigned short sendKbps;
    /** 发送的关键帧间隔,毫秒 */
    unsigned short sendGopMs;
    /** 解码的最大宽度 */
    unsigned short recvMaxWidth;
    /** 解码的最大高度 */
    unsigned short recvMaxHeight;
    /** 解码后返回的格式 @see enum jrtc_dtype */
    unsigned char recvFormat;
    /* 指定解码器序号, 序号是指在 MME_VideoCodecName() 中的序号, 从0开始 */
    signed char recvCodecIndex;
#if defined(_JRTC_BUILD_FOR_ESP32S3) || defined(_JRTC_BUILD_FOR_ESP32S2)
    unsigned char format_idx;
#endif
} MMEVideoParam;

/** 支持的视频编解码名称.
 * @return 以逗号分割的多个编码名称
 * 例如: "H263+,H264,JPEG"
 * RTP打包方式说明:
 * 1. JPEG 自定义方式:
 *    最简单，依次分开即可. 按JPEG规范, 因此每帧
 *    首个包载荷头两字节为0xFF,0xD8
 *
 * 2. H263+ 按rfc标准:
 *    原始H263+流前两个字节必定为0x0,0x0.
 *    首个包: 将载荷中该两字节改为0x4, 0x0.
 *    其他剩余包:   载荷中额外添加两字节头0x0, 0x0
 *
 * 3. H264 自定义方式:
 *    原始H264流前4个字节必须为0x0, 0x0, 0x0, 0x1
 *    首个包:       载荷中额外添加两字节头0x4, 0x0.
 *    其他剩余包:   载荷中额外添加两字节头0x0, 0x0
 */
const char* MME_VideoCodecName(void);

/** 打开视频资源 
 * @param[in] param 视频参数
 * @return 成功返回 0
 * @remarks
 *  jrtc_open(...,  video, camera)
 *  这个video, camera 分别作为接收/发送的建议参数, 经过协商，最终转为 param中的值。
 *  外层几乎不用这些值，也不会进行帧速控制，完成由MME_ReadVideoRTP内部实现。
 */
int MME_StartVideo(MMEVideoParam* param);

/** 读取视频RTP包.
 * 须实现两种读取方式:
 * 1. !rtp.size
 *    是一次调用, 就获取预览和该帧的视频编码, 返回一帧连续的媒体RTP包
 *    rtp.buf 是连续的RTP包的首地址, rtp.size 是连续的RTP包的总长度
 *    rtp.len[] 中是每个 RTP 包的长度.
 *    成功返回 RTP包个数
 *    @code
 *      assert(rtp.size == 0 && rtpHeaderLength + 2 <= 16);//假定预留16字节
 *      assert(timeoutMs || image);
 *      ......;//encode_to_h264(_h264Buf, &_h264Len)
 *      rtp->buf = _h264Buf;//_h264Buf之前至少预留rtpHeaderLength+2字节
 *      rtp->len[0] = _h264Len;
 *      rtp->size = _h264Size;//有效缓冲区为[_h264Buf - rtpHeaderLength - 2, _h264Buf + _h264Size)
 *      int num = rtp_split_frame(rtp, rtpBufferSize, rtpHeaderLength + 2);
 *      char *p = rtp->buf;
 *      for (int i=0; i< num; p += rtp->len[i++]) {//设置额外添加的两字节
 *              p[rtpHeaderLength+0] = (i == 0 ? 4 : 0);
 *              p[rtpHeaderLength+1] = 0;
 *              p[1] = 0;
 *      }
 *      p[1] = 0x80; //结尾的Mark位
 *      return num;
 * 2. rtp.size == rtpBufferSize
 *    则多次连续调用,依次读取RTP包.
 *    一帧的首次调用 timeoutMs != 0, 后续 image == NULL && timeoutMs == 0 
 *    一定要设置 Maker 位 rtp.buf[1] = 一帧的最后RTP ? 0x80 : 0x00
 *    成功返回RTP包的实际长度, 不修改 rtp.len, rtp.size
 *    @code
 *      assert(rtp.size == rtpBufferSize && rtpHeaderLength + 2 <= 16);//假定预留16字节
 *      if (timeoutMs || image)  {
 *          ......;//encode_to_h264(_h264Buf, &_h264Len)
 *      }
 *      rtp->buf = _h264Buf - rtpHeaderLength - 2; //_h264Buf之前至少预留rtpHeaderLength+2字节 
 *      rtp.buf[rtpHeaderLength] = (timeoutMs || image) ? 4 : 0; //H263+打包方式的额外两字节
 *      rtp.buf[rtpHeaderLength+1] = 0;
 *      const  int payload = rtpBufferSize - rtpHeaderLength - 2;
 *      if (_h264Len <= payload) {
 *          rtp.buf[1] = 0x80; //结尾的Mark位
 *          return rtpHeaderLength + 2 + _h264Len;
 *      } else {
 *          rtp.buf[1] = 0;
 *          _h264Buf += payload;
 *          _h264Len -=  payload;
 *          return rtpBufferSize;
 *      }
 *    @endcode
 *
 * @param[in,out] rtp  RTP 包读取信息. NULL 时表示需要丢弃该次操作
 * @param[in] rtpBufferSize    RTP包的最大长度
 * @param[in] rtpHeaderLength  RTP包的头部定长
 * @param[out] image 预览图像
 * @param[in] timeoutMs 无数据时允许等待的时长. 0时不允许等待立即返回, >0表示每帧的首次调用
 * @return  方式1返回RTP包个数, 方式2返回RTP包长度, 错误返回-1
 *
 * 多个连续的媒体RTP包
 * struct rtp_buf_t
 * {
 *      void* buf;///<返回的 RTP 包地址.
 *      unsigned len[48];///<方式1时每个 RTP 包的长度
 *      unsigned size;///方式1时多个 RTP 包的总长度
 *      unsigned types;///< 载荷的特征,RTP_PT 的集合
 *      void (*effect)(struct MMEImage* image);///< 编码前的图像特效
 * };
 *
 * @remarks
 * 1. 有关timeoutMs
 *    实际运行时，只有两种值1或0
 *    因为只有一个工作线程(或使用当前线程循环), MME_(数据流），jc_(网络层)都工作在此线程中,
 *    所以MME_Read* 函数要尽快返回，不要阻塞住.
 *    camera采集线程上，只保存最后图(丢帧处理)， 当MME_Read时，若没有就return 0
 *
 * 2. 图像的渲染由上层UI线程(App 界面层)负责绘制图像:
 *    jrtc_image_t *video=malloc(), *camera = malloc();
 *    jrtc_open(..., video, camera);
 *    .....;
 *    if (video->get != video->put) {//图像有更新
 *     .....;//'界面绘制操作: 即使用video中的data, 更新界面控件'
 *     video->get = img->put;//增加读计数
 *    }
 *    因此 image 有可能是NULL的两种情况
 *    - 不是每帧的首次调用
 *    - 界面没有更新读计数
 *
 * 3. 有关 effect的调用
 *   是给上层界面有个修改发送图像的机会的，例如界面想要翻转图像.
 *   内部需要要给它一个调用的机会。
 *   通常要求在编码前，针对采集的镜头图像，调用一次, 以便有机会修改采集的图像
 *   if (rtp && rtp->effect) rtp->effect(&camera);
 */
struct rtp_buf_t;
typedef struct jrtc_photo_t MMEImage;
int MME_ReadVideoRTP(struct rtp_buf_t* rtp, int rtpBufferSize, int rtpHeaderLength, MMEImage*image, unsigned timeoutMs);

/** 写入 RTP 包,尝试解码并返回图像
 * @param[in] rtp RTP包
 * @param[in] rtpHeaderLength RTP包的头部长度
 * @param[in] rtpLen RTP包的实际长度
 * @param[out] image 解码成功返回图像
 * @return 成功返回 0
 */
int MME_WriteVideoRTP(void* rtp, int rtpHeaderLength, int rtpLen, MMEImage* image);

/** 释放视频资源 */
void MME_StopVideo(void);

/** 重置编码器
 * @param[in] sendFps 发送帧数
 * @param[in] sendGopMs 发送关键帧间隔
 * @param[in]  sendKbps 发送码率
 * @return 成功返回 0
 */
int MME_ResetVideo(unsigned char sendFps, unsigned short sendGopMs, unsigned short sendKbps);

/** 反馈网络状态
 * 内部进行可选的优化
 * @param[in] nowMs 当前时刻
 * @param[in] sendLoss 发送丢包率
 * @param[in] sendKbps 发送码率
 * @param[in] rttMS RTT 时长
 * @param[in] recvJitterMs 接收抖动
 * @param[in] recvFPS 接收帧数
 */
void MME_OnVideoStats(unsigned nowMs, char sendLoss, unsigned sendKbps, unsigned rttMs, unsigned recvJitterMs, unsigned recvFPS);

enum {
    MME_CAMERA_BACK = 0,
    MME_CAMERA_FRONT,
};
/** 动态设置前后镜头
 * @param[in] face MME_CAMERA_FRONT 或 MME_CAMERA_BACK 值
 * @return 不支持或成功返回 0
 */
int MME_SetCamera(int face);

#ifdef __cplusplus
}
#endif
#endif
