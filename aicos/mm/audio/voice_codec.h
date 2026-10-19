/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is voice codec header file.
 *
 *****************************************************************************/

#ifndef __VOICE_CODEC_H__
#define __VOICE_CODEC_H__

typedef enum {
    VOICE_AMRWB_DEFAULT = 0,
    VOICE_AMRWB_ITU = 1,
    VOICE_AMRWB_RFC3267 = 2,
    VOICE_AMRWB_TMAX
}VOICE_AMRWB_FRAMETYPE;

enum {
    VOICE_CMD_GET_FORMAT = 1,
    VOICE_CMD_GET_CHANNELS,
    VOICE_CMD_GET_SAMPLERATE,
    VOICE_CMD_SET_FRAMETYPE,
    VOICE_CMD_SET_MODE,
    VOICE_CMD_SET_DTX,
};

typedef enum {
    VOICE_AMRWB_MDNONE = -1,    /*!< Invalid mode */
    VOICE_AMRWB_MD66 = 0,       /*!< 6.60kbps   */
    VOICE_AMRWB_MD885 = 1,      /*!< 8.85kbps   */
    VOICE_AMRWB_MD1265 = 2,     /*!< 12.65kbps  */
    VOICE_AMRWB_MD1425 = 3,     /*!< 14.25kbps  */
    VOICE_AMRWB_MD1585 = 4,     /*!< 15.85bps   */
    VOICE_AMRWB_MD1825 = 5,     /*!< 18.25bps   */
    VOICE_AMRWB_MD1985 = 6,     /*!< 19.85kbps  */
    VOICE_AMRWB_MD2305 = 7,     /*!< 23.05kbps  */
    VOICE_AMRWB_MD2385 = 8,     /*!< 23.85kbps> */
    VOICE_AMRWB_N_MODES = 9,	/*!< Invalid mode */
    VOICE_AMRWB_MODE_MAX
}VOICE_AMRWBMODE;

void *voice_codec_open(struct codec_info *info);
int voice_codec_close(void *handle);
int voice_codec_dec(void *handle, u32 fmt, u8 *inbuf, u32 in_size,
                    u8 *outbuf, u32 *out_size);
int voice_codec_enc(void *handle, u32 fmt, u8 *inbuf,
                    u32 in_size, u8 *outbuf, u32 *out_size);
int voice_codec_set_mode(void *handle, u32 fmt, u32 mode);

#endif

