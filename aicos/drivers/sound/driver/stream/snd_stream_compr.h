
#ifndef __SND_STREAM_COMPR_H__
#define __SND_STREAM_COMPR_H__

#define SND_STREAM_ID_PCM           0
#define SND_STREAM_ID_COMPR         1
#define SND_STREAM_ID_REC           2
#define SND_STREAM_ID_VOICE_DL      3
#define SND_STREAM_ID_VOICE_UL      4
#define SND_STREAM_ID_VOIP_DL       5
#define SND_STREAM_ID_VOIP_UL       6
#define SND_STREAM_ID_HWCODEC_DL    7
#define SND_STREAM_ID_HWCODEC_UL    8

#define SND_STREAM_CMD_OPEN         1
#define SND_STREAM_CMD_SET_PARAM    2
#define SND_STREAM_CMD_START        3
#define SND_STREAM_CMD_PAUSE        4
#define SND_STREAM_CMD_RESUME       5
#define SND_STREAM_CMD_STOP         6
#define SND_STREAM_CMD_CLOSE        7
#define SND_STREAM_CMD_DRAIN        8
#define SND_STREAM_CMD_GAIN         9

#define SND_FMT_PCM                   (0x00000001)
#define SND_FMT_MP3                   (0x00000002)
#define SND_FMT_AMR                   (0x00000003)
#define SND_FMT_AMRWB                 (0x00000004)
#define SND_FMT_AMRWBPLUS             (0x00000005)
#define SND_FMT_AAC                   (0x00000006)
#define SND_FMT_WMA                   (0x00000007)
#define SND_FMT_REAL                  (0x00000008)
#define SND_FMT_VORBIS                (0x00000009)
#define SND_FMT_FLAC                  (0x0000000A)
#define SND_FMT_IEC61937              (0x0000000B)
#define SND_FMT_G723_1                (0x0000000C)
#define SND_FMT_G729                  (0x0000000D)
#define SND_FMT_BESPOKE               (0x0000000E)

struct snd_stream_cmd_common {
    u32 cmd;
    u32 stream_id;
};

struct snd_stream_cmd_rsp {
    u32 cmd;
    u32 stream_id;
    u32 result;
};

struct snd_stream_cmd_open {
    u32 cmd;
    u32 stream_id;
    u32 flag;
    u32 info_addr;
    u32 ipc_chn;
    u32 ipc_buf_addr;
    u32 reserve;
};

struct snd_stream_cmd_param {
    u32 cmd;
    u32 stream_id;
    u32 format;
    u32 sample_rate;
    u32 bit_rate;
    u32 chn_cnt;
    u32 bit_wid;
    u32 mm_voip;
    /* TBD: stream info */
};

struct snd_stream_cmd_start {
    u32 cmd;
    u32 stream_id;
};

struct snd_stream_cmd_pause {
    u32 cmd;
    u32 stream_id;
};

struct snd_stream_cmd_resume {
    u32 cmd;
    u32 stream_id;
};

struct snd_stream_cmd_stop {
    u32 cmd;
    u32 stream_id;
};

struct snd_stream_cmd_close {
    u32 cmd;
    u32 stream_id;
};

struct snd_stream_cmd_drain {
    u32 cmd;
    u32 stream_id;
    u32 total_size;
    u32 session_id;
};

struct snd_stream_cmd_drain_rsp {
    u32 session_id;
    u32 cmd;
    u32 stream_id;
    u32 result;
};

struct snd_stream_cmd_gain {
    u32 cmd;
    u32 stream_id;
    u32 gain;
};

union snd_stream_cmd {
    struct snd_stream_cmd_common common;
    struct snd_stream_cmd_open   open;
    struct snd_stream_cmd_param  param;
    struct snd_stream_cmd_start  start;
    struct snd_stream_cmd_pause  pause;
    struct snd_stream_cmd_resume resume;
    struct snd_stream_cmd_stop   stop;
    struct snd_stream_cmd_close  close;
    struct snd_stream_cmd_drain  drain;
    struct snd_stream_cmd_gain   gain;
};

#endif /* __SND_STREAM_COMPR_H__ */

