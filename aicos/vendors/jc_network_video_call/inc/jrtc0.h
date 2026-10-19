#ifndef __JRTC_0_H__
#define __JRTC_0_H__
#include "jrtc.h"
#ifdef __cplusplus
extern "C" {
#endif

/** 通话状态
 *               jrtc_alloc              jrtc_schedule               jrtc_schedule
 * JRTC_CLOSED ----------> JRTC_OPENED -------------> JRTC_JOINING -------------> JRTC_JOINED
 *       ^                                                                            |
 *       | jrtc_step                                                                  | jrtc_schedule
 *       |            jrtc_leave 或 内部出错                                          | 收到媒体数据
 * JRTC_LEAVING <---------------------------+------------- JRTC_TALKING <-------------+
 *                                          |                  ^
 *                                          |  jrtc_schedule   |
 *                                          +------------------+
 */

/** 重置服务器地址
 * 注意:
 * - 手工设置新地址,通常只用于测试
 * - srvURL 限制最大字符数为128
 * - srvURL 为空时, 恢复为内置的默认服务器地址
 *
 * @param[in] srvURL 服务器地址
 */
void jrtc_config_server (const char* srvURL);

/** 分配通话资源
 * 注意:
 * - 内部直接引用地址,由外部保证内存的有效性
 * - 内部没有启动网络线程,需要不断的调用如下驱动
 *   jrtc_schedule(jc, nowMs);
 * - 内部设备初始没有激活,后续需要调如下开启
 *   jrtc_activate(jc,JRTC_AUDIO|JRTC_CAMERA)
 *
 * ticket 作为通话ID, 唯一标识每次通话:
 *  - 不能包含JSON需转义的字符
 *  - 每次通话, 要使用不同的 ticket,(随机产生或向服务器申请)
 *  - A 呼叫前,申请获得 ticket
 *  - A 需要通"某种推送机制"(长连接,推送方式) 将 ticket 通知到B
 *  - A 和B 对呼时, 必须使用相同的ticket, 以便服务器精确标识会话
 *
 * @param[in] roomid    加入的房间号(对方账户)
 * @param[in] ticket    用于房间的hash值,可通过其他路径获得,默认为NULL,仅用于调试.
 * @param[in] video     期望接收的视频尺寸和帧速,码率.  若 NULL,则不接收视频
 * @param[in] camera    用于预览图像,建议的发送尺寸和帧速,码率.  若 NULL, 则不使用镜头.
 *                      最终的视频尺寸和镜头的尺寸,都由服务器协商决定.
 */
struct jrtc_t* jrtc_alloc (const char* roomid, const char* ticket, struct jrtc_image_t* video, struct jrtc_image_t* camera);

/** 单步更新通话
 * 要求实时性处理, 务必在独立线程中持续更新,
 * 当返回 JRTC_CLOSED 时, 可以退出该线程.
 * 例如:
 *  启动网络线程([jc]{
 *      do {
 *          state = jrtc_schedule(jc, 当前毫秒);
 *      } while(state != JRTC_CLOSED);
 *  }
 *
 * @param[in] ms        当前毫秒
 */
enum jrtc_state jrtc_schedule (struct jrtc_t* jc, unsigned ms);

/** 主动离开/挂断
 * 注意: 这里只改变状态, 销毁将在jrtc_schedule()中自动处理
 *
 * @param[in] err   主动挂断为 JRTC_EBYE
 *                  但当语音电话中断时, 须设置为JRTC_EAUDIOCALL,
 *                  其他未知情况,可设置为 JRTC_ENIL(不修改挂断原因)
 */
enum jrtc_state jrtc_leave (struct jrtc_t *jc, enum jrtc_error err);

/** 镜像
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转(等价于旋转180度)
 * @param[out] dst  目标内存
 */
void jrtc_i420_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv12_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_uyvy_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv16_mirror(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);

/** 顺时针转90度
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转(等价于转90后水平镜像)
 * @param[out] dst  目标内存
 */
void jrtc_i420_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv12_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_uyvy_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv16_rotate90(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);

/** 顺时针转270度
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转(等价于转270后水平镜像)
 * @param[out] dst  目标内存
 */
void jrtc_i420_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv12_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_uyvy_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);
void jrtc_nv16_rotate270(const struct jrtc_photo_t* src, int dst_w, int dst_h,  char* dst);

/** 裁剪
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转
 * @param[out] dst  目标内存
 */
void jrtc_i420_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv12_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_uyvy_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv16_copy(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);

/** 转换为 RGB565
 * @param[in] dst_w 目标宽度, 会自动裁剪,但不缩放
 * @param[in] dst_h 目标高度, 负数时会先上下翻转
 * @param[out] dst  目标内存, 输出长度为 dst_w*dst_h*sizeof(short)
 */
void jrtc_i420_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv12_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_uyvy_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);
void jrtc_nv16_torgb565(const struct jrtc_photo_t* src, int dst_w, int dst_h, char* dst);

/** 可优化配置的参数 @{ */
extern unsigned JRTC_SESSION_TIMEOUT; /*< 建立通话总的超时阀值,默认 300 秒*/
extern unsigned JRTC_REPLY_TIMEOUT; /*< 建立通话中等待服务器应答的超时阀值,默认120 秒 */
extern unsigned JRTC_MEDIA_TIMEOUT; /*< 接收媒体数据的超时阀值, 默认 20 秒 */
extern unsigned JRTC_MAX_RTP_SIZE; /*< 发送的最大 RTP 包阀值, 默认1200,不能大于1500 */
extern unsigned JRTC_AUDIO_RED_LOSS;/*< 开启音频冗余编码的丢包率阀值, 默认 10 % */
extern unsigned JRTC_VIDEO_FEC_LOSS; /*< 开启视频冗余编码的丢包率阀值, 默认 5 % */
extern unsigned JRTC_VIDEO_FEC_MAX_RTT;/*< 开启视频冗余编码的RTT上限, 默认 3000 ms, 0 则强制关闭FEC */
extern unsigned JRTC_VIDEO_FEC_MAX_BW_RATIO; /*< 开启视频冗余编码的额外的码率占比, 默认 80 %, 0 则强制关闭FEC */
extern unsigned JRTC_VIDEO_INIT_GOP; /*< 初始默认的视频关键帧间隔, 默认2000 ms */
extern unsigned JRTC_TASK_STACK_KB; /*< 内部线程的栈大小,与平台相关 */
extern unsigned JRTC_TASK_PRIORITY; /*< 内部线程的优先级,与平台相关 */
/** @} */

/** 可辅助调试的全局值 @{ */
extern char  _jrtc_errstr[256]; /* 最近错误描述 */
extern const char* _jrtc_version; /* 当前版本号 */
/* 以下是jrtc_config(...)中的入参值 */
extern char _jrtc_appkey[25];
extern char _jrtc_uid[65];
extern char _jrtc_srvURL[129];
extern char _jrtc_aeskey[17];
extern char _jrtc_tokens[129];
/** @} */

extern unsigned jc_time(void);
#ifdef __cplusplus
}
#endif
#endif
