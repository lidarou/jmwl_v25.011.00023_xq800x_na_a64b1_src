/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is JF video call audio adapt file, all audio API needed by
 *  JF engine are defined here.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x025573
#define LOG_MODULE    AMOD_VENDOR

#include <string.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <aic_errno.h>
#include <aos_mem.h>
#include <aic_init.h>
#include <aic_cmd_api.h>

#include <timer_inc.h>
#include <efuse.h>
#include <audio_system.h>

#include "jc.h"
#include "mme.h"
#include "rtp.h"

/* #define CONFIG_JF_VCALL_AUDIO_DUMP */
#define CONFIG_JF_VCALL_AUDIO_TEST
#define CONFIG_JF_VCALL_TEST

#define MAX_CNT         4
#define AMR_FRM_LEN     64

static ACLIENT ul_handle = 0;
static ACLIENT dl_handle = 0;
static char *mme_buf;
static unsigned short amr_seq_num = 0;

/** MME_系列接口是低层的媒体设备操作层
 * 与 jrtc 的主要交互是基于标准 RTP 包格式:
 * MME_ReadAudioRTP, MME_WriteAudioRTP;
 * MME_ReadVideoRTP, MME_WriteVideoRTP;
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
const char* MME_AudioCodecName(void)
{
    return "AMR-WB/16000";
}

/** 打开音频资源
 * @param[in] payloadType  RTP包中的Payload Type值
 * @param[in] sendCodecIndex 指定编码器序号
 * @param[in] recvCodecIndex 指定解码器序号
 * @remarks
 *  序号是指在 MME_AudioCodecName() 中的序号，从0开始
 * @return 成功返回 0
 */
int MME_StartAudio(int payloadType, int sendCodecIndex, int recvCodecIndex)
{
    int ret;

    if (sendCodecIndex != 0 || recvCodecIndex !=0) {
        aicx_error(0x0c7626, "dd", "invalid codec index, send=%d, recv=%d\n",
                   sendCodecIndex, recvCodecIndex);
        ret = AIC_EPERM;

        goto err_proc;
    }

    if (mme_buf || ul_handle || dl_handle) {
        aicx_error(0x155c18, "", "MME audio not stop\n");

        return AIC_EPIPE;
    }
    mme_buf = aos_malloc(AMR_FRM_LEN * MAX_CNT);
    if (!mme_buf) {
        aicx_error(0x0ccfab, "", "mme start audio failed, no mem\n");
        ret = AIC_ENOMEM;

        goto err_proc;
    }

    ul_handle = audio_open(ASTREAM_TYPE_VOIP_UL, APLAY_TYPE_STREAM,
                           AUDIO_INPUT_FLAG_COMPRESS, 16000, 8,
                           ASTREAM_FORMAT_AMR_WB, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_RECORD,
                           ASTREAM_SOURCE_DEFAULT);
    if (!ul_handle) {
        aicx_error(0x19e9bd, "", "mme ul open failed\n");
        ret = AIC_EIO;

        goto err_proc;
    }

    ret = audio_play_stream(ul_handle, 0, 0);
    if (ret) {
        audio_close(ul_handle);
        ul_handle = 0;
        aicx_error(0x15d18a, "d", "mme ul play failed, ret=%d\n", ret);

        goto err_proc;
    }

    dl_handle = audio_open(ASTREAM_TYPE_VOIP, APLAY_TYPE_STREAM,
                           AUDIO_OUTPUT_FLAG_COMPRESS, 16000, 0,
                           ASTREAM_FORMAT_AMR_WB, ASTREAM_CHN_MONO,
                           ASTREAM_DIR_PLAY,
                           ASTREAM_SOURCE_DEFAULT);
    if (!dl_handle) {
        aicx_error(0x1a1def, "", "mme dl open failed\n");
        ret = AIC_EIO;

        goto err_proc;
    }

    ret = audio_play_stream(dl_handle, 0, 0);
    if (ret) {
        audio_close(dl_handle);
        dl_handle = 0;
        aicx_error(0x03bec5, "d", "mme dl play failed, ret=%d\n", ret);

        goto err_proc;
    }

    amr_seq_num = 0;
#ifdef CONFIG_JF_VCALL_AUDIO_DUMP
    audio_data("#!AMR-WB\n", 9, 0);
    audio_data("#!AMR-WB\n", 9, 1);
#endif

    return 0;

err_proc:
    if (mme_buf) {
        aos_free(mme_buf);
        mme_buf = 0;
    }
    return ret;
}

static BOOL mme_audio_mute;
/** 读取 RTP 包
 * @param[in] rtp 将填写的 RTP 包, NULL 时表示需要丢弃该次操作
 * @param[in] rtpBufferSize RTP包的最大缓冲区
 * @param[in] rtpHeaderLength RTP包的头部长度
 * @param[in] timeoutMs 无数据时允许等待的时长
 * @return 成功返回RTP包的实际长度
 *
 * @note 只须填写 RTP 包中 TS 和 载荷 部分.
 *  通常打包时长 PTIME_MS = 20, SampleRateKhz = 8KHZ,
 *  PacketCount 是积加的打包个数,则
 *  unsigned ts = PTIME_MS * SampleRateKhz * PacketCount;
 *  unsigned char* p = (unsigned char*)rtp;
 *  p[4] = (unsigned char)(ts >> 24);
 *  p[5] = (unsigned char)(ts >> 16);
 *  p[6] = (unsigned char)(ts >> 8);
 *  p[7] = (unsigned char)(ts);
 */
int MME_ReadAudioRTP(void* rtp, int rtpBufferSize, int rtpHeaderLength,
                     unsigned timeoutMs)
{
    int ret, i, len = 0, rd_len, pos;
    char cnt = 0;

    rd_len = audio_read(ul_handle, mme_buf, AMR_FRM_LEN, timeoutMs);
    if (rd_len < AMR_FRM_LEN) {
        aicx_error(0x123d3d, "d", "mme read error, rd_len=%d\n", rd_len);
        return 0;
    }

    cnt = rd_len / AMR_FRM_LEN;
    ret = rtp_pack_amr_init((void *)rtp, rtpHeaderLength, true, true, cnt);
    if (ret < 0) {
        aicx_error(0x11053f, "d", "pack amr init error, ret=%d\n", ret);
        return 0;
    }
    for (i = 0; i < cnt; i++) {
        pos = i * AMR_FRM_LEN;
        if (mme_buf[pos] > AMR_FRM_LEN) {
            aicx_error(0x09e974, "d", "pack amr, bad frame size=%d\n", mme_buf[pos]);
            continue;
        }

        /* clear padding bit */
        mme_buf[pos + 1] &= (~(BIT_7 | BIT_1 | BIT_0));
#ifdef CONFIG_JF_VCALL_AUDIO_DUMP
        audio_data(mme_buf + pos + 1, mme_buf[pos] - 1, 0);
#endif
        len += rtp_pack_amr(rtp, rtpBufferSize, mme_buf + pos + 1,
                            mme_buf[pos] - 1);
    }

    if (len <= 0) {
        aicx_error(0x02f097, "dd", "pack amr error, len=%d, cnt=%d\n", len, cnt);
    }
    {
        unsigned ts = 20 * 16 * amr_seq_num;
        unsigned char *p = (unsigned char*)rtp;
        p[0] = 0x80;
        p[1] = 104;
        rtp_pack_uint16(p + 2, amr_seq_num++);
        p[4] = (unsigned char)(ts >> 24);
        p[5] = (unsigned char)(ts >> 16);
        p[6] = (unsigned char)(ts >> 8);
        p[7] = (unsigned char)(ts);
        rtp_pack_uint32(p + 8, 0x6d692aa2);
    }

    return len;
}

/** 写入 RTP 包
 * 内部需要将RTP解码，然后扬声器播放
 * @param[in] rtp RTP包
 * @param[in] rtpHeaderLength RTP包的头部长度
 * @param[in] rtpLen RTP包的实际长度
 * @return 成功返回0
 */
int MME_WriteAudioRTP(void*rtp, int rtpHeaderLength, int rtpLen)
{
    int ret = 0;
    int i, cnt, len;

    cnt = rtp_unpack_amr_init(rtp, rtpLen, true, true);
    aicx_error(0x0527ce, "d", "unpack amr init, cnt=%d\n", cnt);
    for (i = 0; i < cnt; i++) {
        len = rtp_unpack_amr(rtp, rtpLen, mme_buf + i * AMR_FRM_LEN);
        aicx_error(0x1150d9, "d", "unpack amr len=%d\n", len);
        if (len <= 0) {
            aicx_error(0x04106e, "dd", "unpack amr error, i=%d,len=%d\n", i, len);
            continue;
        }
        /* set WB flag */
        mme_buf[i * AMR_FRM_LEN] |= 0x80;
        ret = audio_write(dl_handle, mme_buf + i * AMR_FRM_LEN,
                          AMR_FRM_LEN, 21);
        if (ret < 0)
            aicx_error(0x15a161, "d", "rtp write error, ret=%d\n", ret);

#ifdef CONFIG_JF_VCALL_AUDIO_DUMP
        audio_data(mme_buf + i * AMR_FRM_LEN, len, 1);
#endif
    }

    return ret;
}

/** 动态控制播放是否静音 */
void MME_MutePlayout(char is_mute)
{
    mme_audio_mute = is_mute;
}

/** 设置是否被外部音频通话中断
 * 在MME_StopAudio()内部有可能需要
 * 针对该中断特殊处理设备.
 */
void MME_OnAudioCall(void)
{
}

/** 关闭释放音频资源 */
void MME_StopAudio(void)
{
    if (ul_handle) {
        audio_stop(ul_handle);
        audio_close(ul_handle);
        ul_handle = 0;
    }
    if (dl_handle) {
        audio_stop(dl_handle);
        audio_close(dl_handle);
        dl_handle = 0;
    }
    if (mme_buf) {
        aos_free(mme_buf);
        mme_buf = 0;
    }
}

#ifdef CONFIG_JF_VCALL_AUDIO_TEST
static aos_thread_id vcall_test_th;
static u8 vcall_test_state;
static int rtp_test_buf[1400 / 4];

void vcall_test_thread_loop(void *data)
{
    const int rtpHeaderLength = 12;
    char *rtp = (char *)rtp_test_buf;

    MME_StartAudio(0, 0, 0);
    while (vcall_test_state == 1) {
        int rtpLen = MME_ReadAudioRTP(rtp, sizeof(rtp_test_buf),
                                      rtpHeaderLength, 80);
        if (rtpLen < 0)
            aicx_error(0x166c6a, "d", "**ERR ReadAudioRTP %d\r\n", rtpLen);
        if (rtpLen > 0) {
            //rtp[0] = 0x80;
            //rtp[1] = 104;
            //rtp_pack_uint16(rtp+2, amr_seq_num++);
            //rtp_pack_uint32(rtp+8, 0x6d692aa2);
            rtpLen = MME_WriteAudioRTP(rtp, rtpHeaderLength, rtpLen);
            if (rtpLen <= 0)
                aicx_error(0x05c4e3, "d", "**ERR WriteAudioRTP %d\r\n", rtpLen);
        }
    }
    MME_StopAudio();
    vcall_test_state = 0;
}

int vcall_test_main(int argc, char **argv)
{
    if (vcall_test_state != 0) {
        CONS_OUT("Active or stopping!\n");
        return 0;
    }
    vcall_test_state = 1;

    if (vcall_test_th)
        aos_delete_thread(vcall_test_th);
    vcall_test_th = aos_create_thread("vcall test",
                                     vcall_test_thread_loop,
                                     NULL,
                                     1024, 0,
                                     NULL,
                                     20,
                                     AOS_AUTO_START);
    return 0;
}
CONSOLE_EXPORT_CMD(vaudio_start, vcall_test_main, vaudio start test);

int vcall_test_stop(int argc, char **argv)
{
    if (vcall_test_state == 1)
        vcall_test_state = 2;
    else if (vcall_test_state != 2)
        vcall_test_state = 0;

    return 0;
}
CONSOLE_EXPORT_CMD(vaudio_stop, vcall_test_stop, vaudio stop test);

#endif

#ifdef CONFIG_JF_VCALL_TEST
#include <jrtc.h>

struct jrtc_t;

#define VCALL_APPKEY    "41eac708f51ce97953db5097"
#define VCALL_KEY       "1E34828L0426q639"
#define VCALL_ID        0

static struct jrtc_t *jc;
struct jrtc_image_t video;
struct jrtc_image_t camera;

int vcall_start(int argc, char **argv)
{
    char *str_self_id, *str_remote_id;
    static struct jrtc_image_t video, camera;

    if (argc != 2) {
        CONS_OUT("error id!\n");
        return 0;
    }

    if (jc) {
        CONS_OUT("already start!\n");
        return 0;
    }
    str_self_id = argv[0];
    str_remote_id = argv[1];


   video.width  = camera.width  = 240;
   video.height = camera.height = 320;
   video.fps    = camera.fps    = 8;
   video.kbps   = camera.kbps   = 120;
   //注意,以上仅是建议值, 在最终接通时, 由服务器决定实际值
   //目前内部支持JPEG, H263. 最终由配置与对端协商决定。
   // H263: 只支持176x144和352x288两种做分辨率
   // JPEG: 要求带宽kbps > width * height * 8 * fps / 10000

    jrtc_config(VCALL_APPKEY, str_self_id, VCALL_KEY, "");
    jc = jrtc_open(str_remote_id, NULL, &video, &camera);
    jrtc_activate(jc, JRTC_AUDIO | JRTC_CAMERA);

    return jrtc_error(jc);
}
CONSOLE_EXPORT_CMD(vcall_start, vcall_start, vcall_start test);

int vcall_stop(int argc, char **argv)
{
    if (jc) {
        jrtc_deactivate(jc, JRTC_AUDIO | JRTC_CAMERA);
        jrtc_close(jc, JRTC_EBYE);
        jc = 0;
    }
    return 0;
}
CONSOLE_EXPORT_CMD(vcall_stop, vcall_stop, vcall_stop test);
#endif

