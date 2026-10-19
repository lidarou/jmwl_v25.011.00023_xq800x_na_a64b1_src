

/*
 * mp3_mpeg.h
 *
 *  Created on: 2024-6-20
 *      Author:
 * 
 *  2024-6-20:
 *  
 */
#ifndef MPEGAUDIOFRAME_H_
#define MPEGAUDIOFRAME_H_

//  在运行时判断是否是大端

/**
 * 大端字节序，若不定义则是小端字节序
 */
//#define BIG_ENDIAN
// MPEGAudioRet::mErrCode
enum {
    MPEG_AUDIO_OK = 0,MPEG_AUDIO_NEED_MORE = -99
};


/**
 * 代表返回结果的结构体
 */
typedef struct _MPEGAudioRet {
    int             mErrCode;
    int             mNextPos;
} MPEGAudioRet;


/**
 * 代表MPEG音频帧的信息
 */
typedef struct _MPEGAudioFrameInfo {

    // MPEG Audio Frame Header /////////////////////////////////////////////////

    /**
     * MPEG-1.0: 10
     * MPEG-2.0: 20
     * MPEG-2.5: 25
     * invalid : other
     */
    unsigned int    mMPEGVersion    : 6;

    /**
     * Layer I  : 1
     * Layer II : 2
     * Layer III: 3
     * invalid  : other
     */
    unsigned int    mLayer          : 3;

    /**
     * in kbits/s
     */
    unsigned int    mBitrate        : 10;

    /**
     * in Bytes
     */
    unsigned int    mPaddingSize    : 4;


    /**
     * Channel mode
     *
     * Joint Stereo (Stereo) - 0
     * Single channel (Mono) - 1
     * Dual channel (Two mono channels) - 2
     * Stereo - 3
     */
    unsigned int    mChannelMode    : 3;

    /**
     * Mode extension, Only used in Joint Stereo Channel mode.
     * not process
     */
    unsigned int    mExtensionMode  : 3;

    /**
     * Emphasis:
     * The emphasis indication is here to tell the decoder that the file must be 
     * de-emphasized, that means the decoder must 're-equalize' the sound after 
     * a Dolby-like noise suppression. It is rarely used.
     * 
     * 0 - none
     * 1 - 50/15 ms
     * 2 - reserved (invalid)
     * 3 - CCIT J.17
     */
    unsigned int    mEmphasis       : 3;


    /**
     * in Hz
     */
    unsigned int    mSamplerate     : 17;


    /**
     * Protection off: 0
     * Protection on : 1
     */
    unsigned int    mProtection     : 2;


    /**
     * Copyright bit, only informative
     */
    unsigned int    mCopyrightBit   : 2;

    /**
     * Original bit, only informative
     */
    unsigned int    mOriginalBit    : 2;

    /**
     * 边信息大小(Layer III), in Bytes
     */
    unsigned int    mSideInfoSize   : 7;


    /**
     * Samples per frame
     */
    unsigned int    mSamplesPerFrame: 12;

    /**
     * 0 - CBR
     * 1 - CBR(INFO)
     * 2 - VBR(XING)
     * 3 - VBR(VBRI)
     */
    unsigned int    mBitrateType    : 3;

    /**
     * 2 Bytes
     */
    unsigned short int mCRCValue;


    // XING, INFO or VBRI Header /////////////////////////////////////////////////////

    /**
     * mTotalFrames
     */
    unsigned int    mTotalFrames;

    /**
     * mTotalBytes
     */
    unsigned int    mTotalBytes;

    /**
     * Quality indicator
     * 
     * From 0 - worst quality to 100 - best quality
     */
    unsigned short int mQuality;



    // only for VBRI Header ////////////////////////////////////////////////////

    /**
     * Size per table entry in bytes (max 4)
     */
    unsigned short int mEntrySize   : 4;


    /**
     * Number of entries within TOC table
     */
    unsigned short int mEntriesNumInTOCTable;

    /**
     * Scale factor of TOC table entries
     */
    unsigned short int mTOCTableFactor;

    /**
     * VBRI version
     */
    unsigned short int mVBRIVersionID;

    /**
     * Frames per table entry
     */
    unsigned short int mFramesNumPerTable;

    /**
     * VBRI delay
     * 
     * Delay as Big-Endian float.
     */
    unsigned char   mVBRIDelay[2];


    ////////////////////////////////////////////////////////////////////////////

    /**
     * Frame size
     */
    unsigned int    mFrameSize;


} MPEGAudioFrameInfo;


/**
 * 寻找并下一帧MPEG音频帧的信息。
 * 
 * @param buf         数据区
 * @param bufSize     数据区大小
 * @param info[out]   MPEG音频帧的信息，可以为NULL
 * @param firstFrame  是否是寻找和解析第一帧数据
 * 
 * @ret  若MPEGAudioRet.mErrCode为MPEG_AUDIO_OK，则代表解析成功；否则就是需要更多数据。
 *       当解析成功的时候，MPEGAudioRet.mNextPos代表了下一帧的偏移量；否则代表当下一次传
 *       入更多的数据，应该开始解析的偏移量。
 */
MPEGAudioRet findMpegAudioFramePos(unsigned char * buf,
    int         bufSize,
    MPEGAudioFrameInfo * info,
    bool firstFrame);



#endif /* MPEGAUDIOFRAME_H_ */

