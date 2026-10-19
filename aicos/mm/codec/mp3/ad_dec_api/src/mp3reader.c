/*
 * Copyright (C) 2014 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define AIC_THIS_FILE    0x13f12f
#define LOG_MODULE    AMOD_AUDIO

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "mp3reader.h"
#include "codec_mng.h"
#include "audio_client.h"
#include <alog_api.h>

#include "audio_util.h"

#define READER_READ_EOS    (-2)
#define MAX_FRAMES_SIZE     (2304)
#define SEQ_CHECK_SYNC_FRAMES    0
#define READER_SYNC_BUF_SIZE   (1024)

static uint32_t U32_AT(const uint8_t *ptr) {
    return ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];
}

static bool parseHeader(
        uint32_t header, size_t *frame_size,
        uint32_t *out_sampling_rate, uint32_t *out_channels,
        uint32_t *out_bitrate, uint32_t *out_num_samples) {
    *frame_size = 0;

    if (out_sampling_rate) {
        *out_sampling_rate = 0;
    }

    if (out_channels) {
        *out_channels = 0;
    }

    if (out_bitrate) {
        *out_bitrate = 0;
    }

    if (out_num_samples) {
        *out_num_samples = 1152;
    }

    if ((header & 0xffe00000) != 0xffe00000) {
        ALOGE("mp3 err header:0x%x", header);
        return false;
    }

    unsigned version = (header >> 19) & 3;

    if (version == 0x01) {
        ALOGE("mp3 err version:%d", version);
        return false;
    }

    unsigned layer = (header >> 17) & 3;

    if (layer == 0x00) {
        ALOGE("mp3 err layer");
        return false;
    }

    unsigned bitrate_index = (header >> 12) & 0x0f;

    if (bitrate_index == 0 || bitrate_index == 0x0f) {
        // Disallow "free" bitrate.
        ALOGE("mp3 err bitrate_index:%d", bitrate_index);
        return false;
    }

    unsigned sampling_rate_index = (header >> 10) & 3;

    if (sampling_rate_index == 3) {
        ALOGE("mp3 err sampling_rate_index:%d", sampling_rate_index);
        return false;
    }

    static const int kSamplingRateV1[] = { 44100, 48000, 32000 };
    int sampling_rate = kSamplingRateV1[sampling_rate_index];
    if (version == 2 /* V2 */) {
        sampling_rate /= 2;
    } else if (version == 0 /* V2.5 */) {
        sampling_rate /= 4;
    }

    unsigned padding = (header >> 9) & 1;

    if (layer == 3) {
        // layer I

        static const int kBitrateV1[] = {
            32, 64, 96, 128, 160, 192, 224, 256,
            288, 320, 352, 384, 416, 448
        };

        static const int kBitrateV2[] = {
            32, 48, 56, 64, 80, 96, 112, 128,
            144, 160, 176, 192, 224, 256
        };

        int bitrate =
            (version == 3 /* V1 */)
                ? kBitrateV1[bitrate_index - 1]
                : kBitrateV2[bitrate_index - 1];

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }

        *frame_size = (12000 * bitrate / sampling_rate + padding) * 4;

        if (out_num_samples) {
            *out_num_samples = 384;
        }
    } else {
        // layer II or III

        static const int kBitrateV1L2[] = {
            32, 48, 56, 64, 80, 96, 112, 128,
            160, 192, 224, 256, 320, 384
        };

        static const int kBitrateV1L3[] = {
            32, 40, 48, 56, 64, 80, 96, 112,
            128, 160, 192, 224, 256, 320
        };

        static const int kBitrateV2[] = {
            8, 16, 24, 32, 40, 48, 56, 64,
            80, 96, 112, 128, 144, 160
        };

        int bitrate;
        if (version == 3 /* V1 */) {
            bitrate = (layer == 2 /* L2 */)
                ? kBitrateV1L2[bitrate_index - 1]
                : kBitrateV1L3[bitrate_index - 1];

            if (out_num_samples) {
                *out_num_samples = 1152;
            }
        } else {
            // V2 (or 2.5)

            bitrate = kBitrateV2[bitrate_index - 1];
            if (out_num_samples) {
                *out_num_samples = (layer == 1 /* L3 */) ? 576 : 1152;
            }
        }

        if (out_bitrate) {
            *out_bitrate = bitrate;
        }

        if (version == 3 /* V1 */) {
            *frame_size = 144000 * bitrate / sampling_rate + padding;
        } else {
            // V2 or V2.5
            size_t tmp = (layer == 1 /* L3 */) ? 72000 : 144000;
            *frame_size = tmp * bitrate / sampling_rate + padding;
        }
    }

    if (out_sampling_rate) {
        *out_sampling_rate = sampling_rate;
    }

    if (out_channels) {
        int channel_mode = (header >> 6) & 3;

        *out_channels = (channel_mode == 3) ? 1 : 2;
    }

    return true;
}

// Mask to extract the version, layer, sampling rate parts of the MP3 header,
// which should be same for all MP3 frames.
static const uint32_t kMask = 0xfffe0c00;

static int sourceReadAt(u32 priv, struct ad_mp3reader_ops *ops, int64_t offset, void *data, size_t size, size_t *out_size)
{
    u32 tmp;
    int ret;

    *out_size = 0;
    ret = ops->seek_func(priv, offset, CODEC_SEEK_BEGIN);
    if (ret < 0) {
        return -1;
    } else {
       ret = ops->read_func(priv, data, size, &tmp);
       if(ret != 0 && ret != ASTREAM_DATA_EOS) {
            return -1;
       }
       *out_size = tmp;

       return ret;
    }
}

static int read_seq_wrapper(u32 priv, struct ad_mp3reader_ops *ops,void *data,  u32 size, u32 *real_size)
{
    u32 total_size = 0;
    u32 tmp;
    int ret;

    while(total_size < (size)) {
        u32 readsize = (size) - total_size;
        ret = ops->read_func(priv, (uint8_t *)data + total_size, readsize, &tmp);
        if(ret != 0 && ret != ASTREAM_DATA_EOS) {
            ALOGE("sourceReadSeq read error ret:%d", ret);
            return -1;
        }
        total_size += tmp;
        *real_size = total_size;
        if(ret == ASTREAM_DATA_EOS)
            return ASTREAM_DATA_EOS;
    }
    *real_size = total_size;

    return 0;
}

static int sourceReadSeq(void *obj, struct ad_mp3reader_ops *ops, void *data, size_t size, size_t *out_size)
{
    struct Mp3Reader *reader = (struct Mp3Reader *)obj;
    u32 tmp, priv;
    int ret = 0;

    *out_size = 0;
    priv = reader->priv;
    if (reader->back_buf_size > reader->back_buf_offset) {
        u32 back_num = reader->back_buf_size - reader->back_buf_offset;
        u8 back_suff = back_num >= size ? 1 : 0;
        u32 copy_size = back_suff ? size : back_num;

        memcpy(data, reader->back_buf + reader->back_buf_offset, copy_size);
        reader->back_buf_offset += copy_size;
        if(back_suff) {
            *out_size = size;
            ALOGV("back buf valid");
        } else {
            ret = read_seq_wrapper(priv, ops, (uint8_t *)data + copy_size, (size - copy_size), &tmp);
            if(ret != 0 && ret != ASTREAM_DATA_EOS)
                return -1;
            reader->back_buf_offset = reader->back_buf_size = 0;
            ALOGV("read empty back buf");
            *out_size = tmp + copy_size;
        }
    } else {
        ALOGV("back buf not valid");
        ret = read_seq_wrapper(priv, ops, (uint8_t *)data, size, &tmp);
        if(ret != 0 && ret != ASTREAM_DATA_EOS) {
            return -1;
        }
        *out_size = tmp;
    }

    return ret;
}

static int writeSeq_backbuf(void *obj, struct ad_mp3reader_ops *ops, void *data, size_t size)
{
    struct Mp3Reader *reader = (struct Mp3Reader *)obj;

    if (reader->back_buf_offset < reader->back_buf_size) {
        /*if back buf not empty, then give back data*/
        assert(size <= reader->back_buf_offset);
        reader->back_buf_offset -= size;
    } else {
        assert(reader->back_buf_size + size <=
               reader->back_buf_totalsize);
        memcpy(reader->back_buf + reader->back_buf_size, data, size);
        reader->back_buf_size += size;
    }

    return 0;
}

#if 0
static int resync_seq(void *obj, struct ad_mp3reader_ops *ops,
                       uint32_t *out_header, uint32_t inlength)
{
    bool valid = false;
    struct Mp3Reader *reader = (struct Mp3Reader *)obj;
    const int32_t kMaxReadBytes = reader->sync_buf_size;
    const int32_t kMaxBytesChecked = 128 * 1024;
    uint8_t* buf = reader->sync_buf;
    ssize_t bytesToRead = kMaxReadBytes;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;
    int64_t pos = 0;
    size_t out_size;
    size_t frame_size = 0;
    int ret;

    do {
        if (pos >= kMaxBytesChecked) {
            // Don't scan forever.
            break;
        }

        if (remainingBytes < 4) {
            if (reachEOS) {
                break;
            } else {
                memcpy(buf, tmp, remainingBytes);
                bytesToRead = kMaxReadBytes - remainingBytes;

                /*
                 * The next read position should start from the end of
                 * the last buffer, and thus should include the remaining
                 * bytes in the buffer.
                 */
                ret = sourceReadSeq(obj, ops,
                                               buf + remainingBytes,
                                               bytesToRead, &totalBytesRead);

                if (ret) {
                    if(ret == ASTREAM_DATA_EOS)
                        reachEOS = 1;
                    else
                        break;
                }
                remainingBytes += totalBytesRead;
                tmp = buf;
                continue;
            }
        }

        uint32_t header = U32_AT(tmp);

        uint32_t sample_rate, num_channels, bitrate;
        if (!parseHeader(
                    header, &frame_size,
                    &sample_rate, &num_channels, &bitrate, NULL)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        // We found what looks like a valid frame,
        valid = true;
    } while (!valid);

    if (valid) {
        int32_t need_read;
        uint32_t offset = tmp - buf;
        need_read = frame_size >= (kMaxReadBytes - offset) ?
                    frame_size - (kMaxReadBytes - offset) :
                    -1;
        if (need_read < 0) {
            assert(frame_size <= inlength);
            memcpy(out_header, buf + offset, frame_size);
            writeSeq_backbuf(obj, ops,
                             buf + offset + frame_size,
                             (kMaxReadBytes - offset - frame_size));
        } else {
            int ret;
            assert((kMaxReadBytes - offset + need_read) <= inlength);
            memcpy(out_header, buf + (offset), kMaxReadBytes - offset);
            ret = sourceReadSeq(obj, ops,
                          (uint8_t *)out_header + (kMaxReadBytes - offset),
                          need_read, &out_size);
            if(!ret || (ret == ASTREAM_DATA_EOS)) {
                if(ret == ASTREAM_DATA_EOS)
                    reachEOS = 1;
            }
            if(out_size != need_read)
                valid = 0;
        }
    }
    if (reachEOS) {
        return ASTREAM_DATA_EOS;
    }
    if(valid)
        return 0;

    return -1;

}

#else

static int resync_seq(void *obj, struct ad_mp3reader_ops *ops,
                       uint32_t *out_header, uint32_t inlength)
{
    bool valid = false;
    const int32_t kMaxReadBytes = READER_SYNC_BUF_SIZE + MAX_FRAMES_SIZE * SEQ_CHECK_SYNC_FRAMES;
    const int32_t kMaxBytesChecked = 128 * 1024;
    uint8_t* buf = NULL;
    ssize_t bytesToRead = kMaxReadBytes;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;
    int64_t pos = 0;
    size_t frame_size = 0;
    int ret;
    u32 check_offset;
    u32 remainMinsize = (MAX_FRAMES_SIZE * SEQ_CHECK_SYNC_FRAMES) > 4 ?
                        MAX_FRAMES_SIZE * SEQ_CHECK_SYNC_FRAMES : 4;

    buf = audio_malloc(kMaxReadBytes);
    if (!buf) {
        ALOGE("no enought memory to alloc sync temp buf");
        return -1;
    }
    do {
        if (pos >= kMaxBytesChecked) {
            // Don't scan forever.
            break;
        }

        if (remainingBytes < remainMinsize) {
            if (reachEOS) {
                break;
            } else {
                memcpy(buf, tmp, remainingBytes);
                bytesToRead = kMaxReadBytes - remainingBytes;

                /*
                 * The next read position should start from the end of
                 * the last buffer, and thus should include the remaining
                 * bytes in the buffer.
                 */
                ret = sourceReadSeq(obj, ops,
                                    buf + remainingBytes,
                                    bytesToRead, &totalBytesRead);

                if (ret) {
                    if(ret == ASTREAM_DATA_EOS)
                        reachEOS = 1;
                    else
                        break;
                }
                remainingBytes += totalBytesRead;
                tmp = buf;
                continue;
            }
        }

        uint32_t header = U32_AT(tmp);

        uint32_t sample_rate, num_channels, bitrate;
        if (!parseHeader(
                    header, &frame_size,
                    &sample_rate, &num_channels, &bitrate, NULL)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }
        ALOGI("look like find valid frame");
        // We found what looks like a valid frame,
        valid = true;
        check_offset = tmp - buf;
        u32 test_frame_offset = check_offset + frame_size;

        for (int j = 0; j < SEQ_CHECK_SYNC_FRAMES; ++j) {
            AIC_ASSERT(test_frame_offset <= (kMaxReadBytes - 4));
            u32 test_header = U32_AT(buf + test_frame_offset);

            if ((test_header & kMask) != (header & kMask)) {
                valid = false;
                break;
            }

            size_t test_frame_size;
            if (!parseHeader(test_header, &test_frame_size,
                             NULL, NULL, NULL, NULL)) {
                valid = false;
                break;
            }
            ALOGI("test offset:%d, header:%x, base_header:%x", test_frame_offset, test_header, header);
            test_frame_offset += test_frame_size;
        }

    } while (!valid);

    if (valid) {
        writeSeq_backbuf(obj, ops, tmp, remainingBytes);
    }

    if (buf)
        audio_free(buf);

    if (reachEOS) {
        if (!valid) {
            ALOGI("resync_seq reachEOS, but not valid");
            return ASTREAM_DATA_EOS;
        }
        ALOGI("resync_seq reachEOS, but valid");
    }
    if(valid)
        return 0;

    return -1;

}

#endif
// Resync to next valid MP3 frame in the file.
static int resync(
        void *obj, struct ad_mp3reader_ops *ops,
        uint32_t match_header,
        int64_t *inout_pos, uint32_t *out_header) {
    struct Mp3Reader *reader = (struct Mp3Reader *)obj;
    ssize_t out_size = 0;
    u32 priv = reader->priv;

    if (*inout_pos == 0) {
        // Skip an optional ID3 header if syncing at the very beginning
        // of the datasource.

        for (;;) {
            int retVal;
            uint8_t id3header[10];

            retVal = sourceReadAt(priv, ops, *inout_pos, id3header,
                                      sizeof(id3header), &out_size);
            if(retVal == ASTREAM_DATA_EOS) {
                ALOGE("read id3header eos");
                return ASTREAM_DATA_EOS;
            }
            if (out_size < (ssize_t)sizeof(id3header)) {
                // If we can't even read these 10 bytes, we might as well bail
                // out, even if there _were_ 10 bytes of valid mp3 audio data...
                ALOGE("read id3header failed");
                return -1;
            }

            if (memcmp("ID3", id3header, 3)) {
                break;
            }

            // Skip the ID3v2 header.

            size_t len =
                ((id3header[6] & 0x7f) << 21)
                | ((id3header[7] & 0x7f) << 14)
                | ((id3header[8] & 0x7f) << 7)
                | (id3header[9] & 0x7f);

            len += 10;
            *inout_pos += len;
        }

    }

    int64_t pos = *inout_pos;

    bool valid = false;

    const int32_t kMaxReadBytes = reader->sync_buf_size;
    const int32_t kMaxBytesChecked = 128 * 1024;
    uint8_t *buf = reader->sync_buf;
    ssize_t bytesToRead = kMaxReadBytes;
    ssize_t totalBytesRead = 0;
    ssize_t remainingBytes = 0;
    bool reachEOS = false;
    uint8_t *tmp = buf;

    do {
        if (pos >= *inout_pos + kMaxBytesChecked) {
            // Don't scan forever.
            break;
        }

        if (remainingBytes < 4) {
            if (reachEOS) {
                break;
            } else {
                memcpy(buf, tmp, remainingBytes);
                bytesToRead = kMaxReadBytes - remainingBytes;

                /*
                 * The next read position should start from the end of
                 * the last buffer, and thus should include the remaining
                 * bytes in the buffer.
                 */
                 int retVal;
                retVal = sourceReadAt(priv, ops, pos + remainingBytes,
                                              buf + remainingBytes, bytesToRead, &out_size);

                if (retVal) {
                    if(retVal != ASTREAM_DATA_EOS)
                        break;
                }
                totalBytesRead = out_size;
                reachEOS = (retVal == ASTREAM_DATA_EOS) ? 1 : 0;//(totalBytesRead != bytesToRead);
                remainingBytes += totalBytesRead;
                tmp = buf;
                continue;
            }
        }

        uint32_t header = U32_AT(tmp);

        if (match_header != 0 && (header & kMask) != (match_header & kMask)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        size_t frame_size;
        uint32_t sample_rate, num_channels, bitrate;
        if (!parseHeader(
                    header, &frame_size,
                    &sample_rate, &num_channels, &bitrate, NULL)) {
            ++pos;
            ++tmp;
            --remainingBytes;
            continue;
        }

        // We found what looks like a valid frame,
        // now find its successors.

        int64_t test_pos = pos + frame_size;

        valid = true;
        const int FRAME_MATCH_REQUIRED = 3;
        for (int j = 0; j < FRAME_MATCH_REQUIRED; ++j) {
            uint8_t tmp[4];
            ssize_t retval;

            retval = sourceReadAt(priv, ops, test_pos, tmp, sizeof(tmp), &out_size);
            if(retval) {
                if(retval != ASTREAM_DATA_EOS) {
                    valid = false;
                    break;
                }
                reachEOS = 1;
            }
            if (out_size < (ssize_t)sizeof(tmp)) {
                valid = false;
                break;
            }

            uint32_t test_header = U32_AT(tmp);

            if ((test_header & kMask) != (header & kMask)) {
                valid = false;
                break;
            }

            size_t test_frame_size;
            if (!parseHeader(test_header, &test_frame_size,
                             NULL, NULL, NULL, NULL)) {
                valid = false;
                break;
            }

            test_pos += test_frame_size;
        }

        if (valid) {
            *inout_pos = pos;

            if (out_header != NULL) {
                *out_header = header;
            }
        }

        ++pos;
        ++tmp;
        --remainingBytes;
    } while (!valid);

    if (reachEOS) {
        if (!valid) {
            ALOGI("resync reachEOS, but not valid");
            return ASTREAM_DATA_EOS;
        }
        ALOGI("resync reachEOS, but valid");
    }

    return valid == true ? 0 : -1;
}

// Initialize the MP3 reader.
void *ad_mp3reader_init(u32 priv, struct ad_mp3reader_ops *ops,
                        struct reader_extra_data *extra_info) {

    struct Mp3Reader *reader = malloc(sizeof(struct Mp3Reader));//obj;
    int64_t pos = 0;
    uint32_t header;
    int ret;

    if (!reader) {
        goto _alloc_error;
    }
    memset(reader, 0 , sizeof(struct Mp3Reader));
    reader->priv = priv;
    reader->ops.read_func = ops->read_func;
    reader->ops.seek_func = ops->seek_func;

    if (reader->ops.seek_func) {
        reader->sync_buf = malloc(READER_SYNC_BUF_SIZE);
        if(!reader->sync_buf) {
            goto _alloc_syncbuf_error;
        }
        reader->sync_buf_size = READER_SYNC_BUF_SIZE;
        ret = resync(reader, &reader->ops, 0, &pos, &header);
        if (ret) {
            ALOGE("ad_mp3reader_init resync error!");
            goto _resync_error;
        }
        reader->mFixedHeader = header;
    }
    else {
        reader->back_buf = malloc(READER_SYNC_BUF_SIZE+ MAX_FRAMES_SIZE * SEQ_CHECK_SYNC_FRAMES + extra_info->size);
        if(!reader->back_buf) {
            goto _alloc_backbuf_error;
        }
        reader->back_buf_totalsize = READER_SYNC_BUF_SIZE + MAX_FRAMES_SIZE * SEQ_CHECK_SYNC_FRAMES + extra_info->size;
        reader->back_buf_size = 0;
        writeSeq_backbuf(reader, &reader->ops,
                         extra_info->data, extra_info->size);

    }

    return reader;


_resync_error:
    free(reader->sync_buf);
_alloc_syncbuf_error:
_alloc_backbuf_error:
    free(reader);
_alloc_error:
    return NULL;
}

// Get the next valid MP3 frame.
/*return value: 0 is OK
                ASTREAM_DATA_EOS  eos
                other value failed*/
int ad_mp3reader_getframe(void *obj, void *buffer, uint32_t insize, uint32_t *size) {

    struct Mp3Reader *reader = (struct Mp3Reader *)obj;
    size_t frame_size;
    u8 eos_flag = 0;
    uint32_t bitrate;
    uint32_t num_samples;
    uint32_t sample_rate;
    uint8_t seek_suport = reader->ops.seek_func ? 1 : 0;
    ssize_t n;
    int ret = 0;

    *size = 0;

    for (;;) {
        ssize_t n;
        if (seek_suport) {
            ret = sourceReadAt(reader->priv, &reader->ops, reader->mCurrentPos, buffer, 4, &n);
            if(ret == ASTREAM_DATA_EOS)
                eos_flag = 1;
        }
        else {
            /*check whether back buf valid*/
           ret = sourceReadSeq(reader, &reader->ops,
                               buffer, 4, &n);
           if(ret == ASTREAM_DATA_EOS)
               eos_flag = 1;
        }
        /*if eos, do not get return EOS*/
        if (eos_flag) {
            ALOGI("eos flag when read for header bytes");
            return ASTREAM_DATA_EOS;
        }
        if (ret) {
            ALOGI("eos flag when read for header bytes failed");
            return -1;
        }

        uint32_t header = U32_AT((const uint8_t *)buffer);

        /*if seek support check may be more strictly, if not seek_suport, don't check fix header*/
        if ((((header & kMask) == (reader->mFixedHeader & kMask)) || (!seek_suport))
            && parseHeader(
                header, &frame_size, &sample_rate, NULL /*out_channels*/,
                &bitrate, &num_samples)) {
            ALOGV("mp3reader get one frame header!");
            break;
        }

        // Lost sync.
        if(seek_suport) {
            int64_t pos = reader->mCurrentPos;
            ALOGI("resync enter cur pos:%d", reader->mCurrentPos);
            ret = resync(reader, &reader->ops, reader->mFixedHeader, &pos, NULL);
            if (ret) {
                // Unable to resync. Signalling end of stream.
                ALOGW("resync error return eos as replacement ret:%d!", ret);
                return ASTREAM_DATA_EOS;
            }
            reader->mCurrentPos = pos;
        } else {
            ALOGI("resync_seq enter");
            ret = resync_seq(reader, &reader->ops, buffer, insize);
            if(ret) {
                //Unabld to resync. Signalling end of stream.
                ALOGW("resync_seq error return eos as replacement ret:%d!", ret);
                return ASTREAM_DATA_EOS;
            }
        }
        // Try again with the new position.
    }
    assert(frame_size <= insize);
    if (seek_suport) {
        ret = sourceReadAt(reader->priv, &reader->ops, reader->mCurrentPos, buffer, frame_size, &n);
        eos_flag = (ret == ASTREAM_DATA_EOS) ? 1 : 0;
    }
    else {
        ret = sourceReadSeq(reader, &reader->ops, (u8 *)buffer + 4, frame_size - 4, &n);
        eos_flag  = (ret == ASTREAM_DATA_EOS) ? 1 : 0;
        n += 4;
    }
    if(eos_flag) {
        if (n == frame_size)
            *size = n;
        ALOGI("mp3reader eos out size:%d, n is:%d", *size, n);
        return ASTREAM_DATA_EOS;
    }
    if (n < (ssize_t)frame_size) {
        ALOGE("mp3reader read frame error, readsize:%d, frame_size:%d", n, frame_size);
        return -1;
    }
    *size = frame_size;
    reader->mCurrentPos += frame_size;
    ALOGV("mp3reader get ok size:%d", *size);
    return 0;
}

// Close the MP3 reader.
void ad_mp3reader_deinit(void *obj) {
    if(obj) {
        struct Mp3Reader *reader = (struct Mp3Reader *) obj;
        if (reader->sync_buf)
            free(reader->sync_buf);
        if (reader->back_buf)
            free(reader->back_buf);
        free(obj);
    }
}


