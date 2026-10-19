#ifndef __JRTC_H__
#define __JRTC_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * 呼叫流程: 假设 `设备A' 呼叫 `设备B'
 *
 * 启动连接流程
 * - A 需要 "某种推送机制" (长连接,推送方式) 通知到B,
 * - A 打开通信(jrtc_open), 初始状态为 JRTC_OPENED
 * - B 收到 "该种推送通知" 后也打开通信(jrtc_open), 初始状态为 JRTC_OPENED
 * - A 主动激活媒体设备,以便自动接听
 *          jrtc_activate(jc, JRTC_AUDIO|JRTC_CAMERA)
 *
 * 成功建立网络通道
 * - A 和 B 在服务器上对呼后连通, 通话状态都变为 JRTC_JOINED
 * - A 可以开始渲染镜头预览(渲染流程)
 *
 * 被叫流程
 * - B 收到A的音视频数据, 通话状态变为 JRTC_TALKING
 * - B 可以开始渲染对端视频(渲染流程)
 * - B 弹出接听提示, 若用户选择"拒接"则进入挂断流程
 * - B 在界面选择接听后, 也激活媒体设备
 *          jrtc_activate(jc,JRTC_AUDIO|JRTC_CAMERA)
 *
 * 主叫流程
 * - A 在收到音视频数据后, 状态变为JRTC_TALKING
 * - A 也开始渲染对端视频(渲染流程)
 *
 * 挂断流程
 * - A 调用 jrtc_close 主动挂断
 * - B 通话状态将变为 JRTC_LEAVING 或 JRTC_CLOSED
 * - 通过 jrtc_error() 可以获取挂断的原因(出错值)
 * - B 调用 jrtc_close 结束通话
 *
 * 最简函数流程
 * - jrtc_config(...);   // 配置环境,只需调用一次
 * - jc = jrtc_open(...);// 打开通话,返回对象
 * - jrtc_activate(jc,JRTC_AUDIO|JRTC_CAMERA);//激活设备
 * - 在 UI 线程,更新视频或预览,参考下面的`渲染流程'
 * - jrtc_close(jc, JRTC_EBYE); //主动挂断, 并且等待网络线程结束
 *
 * 收到来电中断的处理
 * - jrtc_close(jc, JRTC_EAUDIOCALL);//结束通话,等待网络线程结束
 */

/** 通话状态 */
enum jrtc_state
{
    JRTC_CLOSED=0,/*< 初始状态 */
    JRTC_LEAVING, /*< jrtc_leave 或内部出错后的状态 */
    JRTC_OPENED,  /*< 调用jrtc_open 后的状态 */
    JRTC_JOINING, /*< 加入中的状态 */
    JRTC_JOINED,  /*< 成功加入会话 */
    JRTC_TALKING, /*< 收到媒体后的状态 */
};

/** 通话错误值 */
enum jrtc_error
{
    JRTC_ENIL   = 0,  /* unknown error */
    JRTC_EPERM  = 1,  /* Operation not permitted */
    JRTC_ENONET = 64, /* Machine is not on the network */
    JRTC_EPROTO = 71, /* Protocol error */
    JRTC_EMSGSIZE = 90,  /* Message too long */
    JRTC_ECONNRESET=104, /* Connection reset by peer */
    JRTC_ETIMEDOUT =101, /* Connection timed out */
    JRTC_ECONNREFUSED=111,/* Connection refused or closed */

    JRTC_EOFFLINE = 128, /* Peer offline or over */
    JRTC_EDECLINE = 129, /* Peer decline the call */
    JRTC_EINVOKETIMEOUT = 130, /* IoTUnit invoke timeout */
    JRTC_ECALLSERVERERROR = 131, /* Call server error */
    JRTC_EROOMNOTFND = 132, /* Room or callee not found */
    JRTC_ECALLTIMEOUT = 133, /* Call timeout */
    JRTC_ENOTREG = 134, /* device id not registered */
    JRTC_ELOGIN = 135, /* device id login failed */
    JRTC_ETOKEN_NOTFOUND = 136, /* secret not found */
    JRTC_ETOKEN_INV = 137, /* token invalid */
    JRTC_ETOKEN_EXPIRE = 138, /* token expire */
    JRTC_ECODECINVALID = 139, /* Codec params invalid */

    JRTC_EAUDIOCALL = 252, /* external audio call, don't close audio deivce */
    JRTC_EAUDIO=253,  /* Audio device error */
    JRTC_EVIDEO=254,  /* Video device error */

    JRTC_EBYE = 255,  /* Success, Closed by peer leave */
};

/** 通话中的媒体设备 */
enum jrtc_device
{
    JRTC_SPEAKER    = 1,
    JRTC_MICROPHONE = 2,
    JRTC_CAMERA     = 4,
    JRTC_AUDIO      = 3,
};

enum jrtc_netstat
{
    JRTC_NET_NORMAL  = 0, /* network is normal or unknown */
    JRTC_LOCAL_POOR  = 1,
    JRTC_REMOTE_POOR = 2,
};

/** 数据类型
 * @note
 * |   软件系统   |   平台   |  采集格式  |       编码后格式     |    解码后格式    |
 * | ------------ | -------- | ---------- | -------------------- | ---------------- |
 * | ASR Crane    | 3601     | JRTC_NV12  | JRTC_H263,JRTC_JPEG  | JRTC_I420,JRTC_RGB565 |
 */
enum jrtc_dtype
{
    JRTC_I420 = 0, /*< 图像字节数=W*H*3/2 */
    JRTC_NV12 = 1, /*< 图像字节数=W*H*3/2 */
    JRTC_UYVY = 2, /*< 图像字节数=W*H*2 */
    JRTC_JPEG = 3, /*< 保持兼容性 */
    JRTC_NV16 = 4, /*< 图像字节数=W*H*2 */

    JRTC_RGB565 = 64,

    JRTC_H264 = 96,
    JRTC_H263 = 97,
};

/** 渲染流程:
 * jrtc_image_t* img 代表 jrtc_open 中传入 video 或 camera
 * - 检测当前为通话状态, 否则无须渲染
 *   if (jrtc_state(jc) < JRTC_JOINED) break;
 *
 * - 内部自动更新图像:
 *      if (img->get == img->put) {
 *          '更新图像内容: 即对img中的data, width, height, format 赋值'
 *          img->put++;//增加写计数
 *      }
 *
 * - 上层UI线程(App 界面层)负责绘制图像:
 *      if (img->get != img->put) {
 *          '界面绘制操作: 即使用img中的data, 更新界面控件'
 *          img->get = img->put;//增加读计数
 *      }
 *   在 ASR原生 LittlevGL 的 lv_watch 界面中, 界面绘制操作:
 *      // yuv420_2_rgb565 是内置的格式转换函数
 *      //extern void yuv420_2_rgb565(int width, int height, const unsigned char *src, unsigned short *dst, unsigned semi);
 *      yuv420_2_rgb565(img->width, img->height, img->data, img_dsc->data, img->format);
 *      lv_img_set_src(img, img_dsc);//img, img_dsc 都是界面中的相应UI控件,
 *
 *
 * 在jrtc_open()中用于设置建议值, 通话过程中用于获取图像数据.
 * 例如, 希望接收和发送都是176x144 8帧/秒, 120 kbps 的音视频通话:
 *   static struct jrtc_image_t video, camera;
 *   video.width  = camera.width  = 176;
 *   video.height = camera.height = 144;
 *   video.fps    = camera.fps    = 8;
 *   video.kbps   = camera.kbps   = 120;
 *   //注意,以上仅是建议值, 在最终接通时, 由服务器决定实际值
 *   //目前内部支持JPEG, H263. 最终由配置与对端协商决定。
 *   // H263: 只支持176x144和352x288两种做分辨率
 *   // JPEG: 要求带宽kbps > width * height * 8 * fps / 10000
 *   jc = jrtc_open(roomid, NULL, &video, &camera);
 */
struct jrtc_image_t
{/* 可强转为 jrtc_photo_t 使用 */
    void* data;
    unsigned bytes;
    unsigned short width, height;
    char format;/*< enum jrtc_dtype */
    char fps;
    unsigned short kbps;
    volatile char put, get;
    char reserve[2];
};

/** 当前的版本号 */
#define JRTC_VERSION   "1.7.2"

/** 配置全局参数,只需调用一次.
 * 注意:
 * - appkey 固定长度为24
 * - key 固定长度为16
 * - uid 限制最大字符数为64
 * - tokens 限制最大字符数为128
 *
 * @param[in] appkey 当前APP特有的标识, 需向Juphoon申请获得
 * @param[in] uid 账户
 * @param[in] key 通讯密钥, 必须与appkey匹配, 需向Juphoon申请或注册
 * @param[in] 用于鉴权的tokens,默认为""
 */
void jrtc_config (const char* appkey, const char* uid, const char key[16], const char* tokens);

/** 分配通话资源并启动网络线程
 * 注意:
 * - 对ticket长度和参数roomid都限制最大字符数为64
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
 * @param[in] id            加入一对一会话(对方的uid)或房间(以room:前缀的房间地址)
 * @param[in] ticket        hash值,可通过其他路径获得,默认为NULL,仅用于调试.
 * @param[in,out] video     期望接收的视频尺寸和帧速,码率.  若 NULL,则不接收视频
 * @param[in,out] camera    用于预览图像,建议的发送尺寸和帧速,码率.  若 NULL, 则不使用镜头.
 *                          最终的视频尺寸和镜头的尺寸,都由服务器协商决定.
 * @note
 *  若界面上，检测到video和camera中的width,height都为0, 说明协商不支持视频，自动转为音频通话
 */
struct jrtc_t* jrtc_open (const char* id, const char* ticket, struct jrtc_image_t* video, struct jrtc_image_t* camera);

/** 释放通话资源,并等待网络线程结束.
 *
 * @param[in] err   主动挂断为 JRTC_EBYE
 *                  但当语音电话中断时, 须设置为JRTC_EAUDIOCALL,
 *                  其他未知情况,可设置为 JRTC_ENIL(不修改挂断原因)
 */
void jrtc_close (struct jrtc_t* jc, enum jrtc_error err);

/** 编码前的的视频帧 */
struct jrtc_photo_t
{
    void* data;
    unsigned bytes;
    unsigned short width,height;
    char format;/*< enum jrtc_dtype */
    char reserve[3];
};

/** 动态设置前后镜头
 * @param[in] camera 0为后镜头,1是前镜头
 * @param[in] effect 编码前图像的处理回调
 *                   禁止改变图像尺寸和格式.
 *                   可设置为 NULL
 * @code 
 *   static char* encoding = NULL;
 *   //只镜像发送图像
 *   jrtc_set_camera(jc, 1, [](struct jrtc_photo_t* frame){
 *      auto preview = frame->data;
 *      if (!encoding) encoding = new char[frame->bytes];
 *
 *      assert (frame->format == JRTC_NV12 && "根据平台选择转换函数");
 *      jrtc_nv12_mirror(frame, frame->width, frame->height, encoding);
 *      frame->data = encoding;
 *   });
 *   //只镜像预览图像
 *   jrtc_set_camera(jc, 1, [](struct jrtc_photo_t* frame){
 *      auto preview = frame->data;
 *      if (!encoding) encoding = new char[frame->bytes];
 *      frame->data = memcpy(encoding, preview, frame->bytes);
 *
 *      assert (frame->format == JRTC_NV12 && "根据平台选择转换函数");
 *      jrtc_nv12_mirror(frame, frame->width, frame->height, preview);
 *   });
 *   //同时镜像预览和发送图像
 *   jrtc_set_camera(jc, 1, [](struct jrtc_photo_t* frame){
 *      auto preview = frame->data;
 *      if (!encoding) encoding = new char[frame->bytes];
 *      frame->data = memcpy(encoding, preview, frame->bytes);
 *
 *      assert (frame->format == JRTC_NV12 && "根据平台选择转换函数");
 *      jrtc_nv12_mirror(frame, frame->width, frame->height, preview);
 *      frame->data = preview;
 *   });
 * @endcode
 */
void jrtc_set_camera (struct jrtc_t* jc, int camera,
    void (*effect) (struct jrtc_photo_t* frame));

/** 激活设备, 开启发送能力
 * @param[in] devices jrtc_device 的集合
 */
void jrtc_activate (struct jrtc_t* jc, int devices);

/** 禁活设备, 关闭发送能力
 * @param[in] devices jrtc_device 的集合
 */
void jrtc_deactivate (struct jrtc_t* jc, int devices);

/** 返回通话状态 */
enum jrtc_state jrtc_state  (const struct jrtc_t *jc);

/** 返回最近的出错值 */
enum jrtc_error jrtc_error  (const struct jrtc_t *jc);

/** 返回网络状态
 * @param[out] remote_active_devices  对端的的数据流状态. jrtc_device 的集合
 *                                    允许 NULL.
 */
enum jrtc_netstat jrtc_netstat(struct jrtc_t* jc, int *remote_active_devices);

#ifdef __cplusplus
}
#endif
#endif
