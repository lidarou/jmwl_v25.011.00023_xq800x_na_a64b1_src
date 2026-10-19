/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used to gernate tone and play them by using audio API.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c068b
#define LOG_MODULE    AMOD_AUDIO

#include <math.h>
#include <time.h>
#include <aic_type.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <alog_api.h>

#include "audio_util.h"
#include "audio_sink.h"
#include "audio_sink_tone.h"
#include "audio_client.h"
#include "audio_service.h"
#include "audio_system.h"

#ifndef status_t
#define status_t u32
#endif

#ifndef time_t
#define time_t u64
#endif

#ifndef INT_MAX
#define INT_MAX    0x7FFFFFFF
#endif

#ifndef mutex_lock
#define mutex_lock(x)   tx_mutex_get(x, TX_WAIT_FOREVER)
#endif

#ifndef mutex_unlock
#define mutex_unlock    tx_mutex_put
#endif

#define EVENT_DONE       1

#define TONEGEN_MAX_WAVES       3
#define TONEGEN_MAX_SEGMENTS    12
#define TONEGEN_INF             0xFFFFFFFF
#define TONEGEN_GAIN            0.9

#define GEN_AMP     32000
#define S_Q14       14
#define S_Q15       15

#define MAX_WGEN_NUM (TONEGEN_MAX_SEGMENTS * TONEGEN_MAX_WAVES)

enum gen_command {
    WAVEGEN_START,  /* Start/restart wave from phase 0 */
    WAVEGEN_CONT,   /* Continue wave from current phase */
    WAVEGEN_STOP    /* Stop wave on zero crossing */
};

enum tone_state {
    TONE_IDLE,  /* ToneGenerator is being initialized or initialization failed */
    TONE_INIT,  /* ToneGenerator has been successfully initialized and is not playing */
    TONE_STARTING,  /* ToneGenerator is starting playing */
    TONE_PLAYING,   /* ToneGenerator is playing */
    TONE_STOPPING,  /* ToneGenerator is stoping */
    TONE_STOPPED,   /* ToneGenerator is stopped: the AudioTrack will be stopped */
    TONE_RESTARTING /* A start request was received in active state (playing or stopping) */
};

enum regional_tone_type {
    // ANSI supervisory tones
    TONE_ANSI_DIAL = NUM_TONES, // Dial tone: a continuous 350 Hz + 440 Hz tone.
    TONE_ANSI_BUSY,             // Busy tone on:  a 480 Hz + 620 Hz tone repeated in a 500 ms on, 500 ms off cycle.
    TONE_ANSI_CONGESTION,       // Network congestion (reorder) tone on:  a 480 Hz + 620 Hz tone repeated in a 250 ms on, 250 ms off cycle.
    TONE_ANSI_CALL_WAITING,     // Call waiting tone on: 440 Hz, on for 300 ms, 9,7 s off followed by
                                // (440 Hz, on for 100 ms off for 100 ms, on for 100 ms, 9,7s off and repeated as necessary).
    TONE_ANSI_RINGTONE,         // Ring Tone:  a 440 Hz + 480 Hz tone repeated in a 2 s on, 4 s off pattern.
    // JAPAN Supervisory tones
    TONE_JAPAN_DIAL,            // Dial tone: 400Hz, continuous
    TONE_JAPAN_BUSY,            // Busy tone: 400Hz, 500ms ON, 500ms OFF...
    TONE_JAPAN_RADIO_ACK,       // Radio path acknowlegment: 400Hz, 1s ON, 2s OFF...
    // GB Supervisory tones
    TONE_GB_BUSY,               // Busy tone: 400 Hz, 375ms ON, 375ms OFF...
    TONE_GB_CONGESTION,         // Congestion Tone: 400 Hz, 400ms ON, 350ms OFF, 225ms ON, 525ms OFF...
    TONE_GB_RINGTONE,           // Ring Tone: A 400Hz + 450Hz tone repeated in a 0.4s on, 0.2s off, 0.4s on, 2.0s off pattern.
    // AUSTRALIA Supervisory tones
    TONE_AUSTRALIA_DIAL,        // Dial tone: 425 Hz tone modulated with 25 Hz, continuous
    TONE_AUSTRALIA_RINGTONE,    // Ring tone: A 400Hz + 450Hz tone repeated in a 0.4s on, 0.2s off, 0.4s on, 2.0s off pattern.
    TONE_AUSTRALIA_BUSY,        // Busy tone: 425 Hz repeated in a 0.375s on, 0.375s off pattern.
    TONE_AUSTRALIA_CALL_WAITING,// Call waiting tone: 425Hz tone repeated in a 0.2s on, 0.2s off, 0.2s on, 4.4s off pattern.
    TONE_AUSTRALIA_CONGESTION,  // Congestion tone: 425Hz tone repeated in a 0.375s on, 0.375s off pattern
    // SINGAPORE Supervisory tones
    TONE_SG_BUSY,               // Busy tone: 425 Hz, 750ms ON, 750ms OFF...
    TONE_SG_RINGTONE,           // Ring Tone: 425 Hz tone modulated with 24 Hz, 400ms ON, 200ms OFF, 400ms ON, 2s OFF...
    // HONG KONG Supervisory tones
    TONE_HK_BUSY,               // Busy tone: 480 Hz + 620 Hz, 500ms ON, 500ms OFF...
    TONE_HK_RINGTONE,           // Ring Tone: 440 Hz + 480 Hz repeated with pattern 0,4s on, 0,2s off, 0,4s on and 3s off.
    // IRELAND Supervisory tones
    TONE_IE_RINGTONE,           // Ring Tone: A 400Hz + 450Hz tone repeated in a 0.4s on, 0.2s off, 0.4s on, 2.0s off pattern.
    TONE_IE_CALL_WAITING,       // Call waiting tone: 425Hz tone repeated in a 0.18s on, 0.2s off, 0.2s on, 4.5s off pattern.
    NUM_ALTERNATE_TONES
};

struct ToneSegment {
    unsigned int duration;
    unsigned short waveFreq[TONEGEN_MAX_WAVES + 1];
    unsigned short loopCnt;
    unsigned short loopIndx;
};

struct ToneDescriptor {
    struct ToneSegment segments[TONEGEN_MAX_SEGMENTS + 1];
    unsigned long repeatCnt;
    unsigned long repeatSegment;
};

struct WaveGenerator {
    short mA1_Q14;  /* Q14 coefficient */
    /* delay line of full amplitude generator */
    long mS1, mS2;  /* delay line S2 oldest */
    short mS2_0;    /* saved value for reinitialisation */
    short mAmplitude_Q15;  /* Q15 amplitude */
};

struct wgen_key {
    unsigned int freq;
    struct WaveGenerator *wgen;
};

struct ToneGenerator {
    struct ToneDescriptor *sToneDescriptors;

    unsigned int mTotalSmp;
    unsigned int mNextSegSmp;
    /**
     * NOTE: because mTotalSmp, mNextSegSmp are stored on 32 bit, current
     * design will operate properly only if tone duration is less than about
     * 27 Hours(@44100Hz sampling rate). If this time is exceeded,
     * no crash will occur but tone sequence will show a glitch.
     */
    unsigned int mMaxSmp;
    int mDurationMs;

    unsigned short mCurSegment;
    unsigned short mCurCount;
    volatile unsigned short mState;
    unsigned short mRegion;
    struct ToneDescriptor *mpToneDesc;
    struct ToneDescriptor *mpNewToneDesc;

    unsigned short mLoopCounter;

    uint32_t mSamplingRate;
    u32 wait_cnt;
    aos_mutex_ptr mLock;
    aos_event_ptr mEvent;
    float mVolume;

    unsigned int mProcessSize;
    struct timespec mStartTime;

    AHANDLE ahandle;
    aos_thread_id thread;

    struct WaveGenerator WaveGenerator;
    struct wgen_key *wgen_key;
    u32 wgen_key_num;
};
/**
 * Descriptors for all available tones
 * (See ToneDescriptor class declaration for details)
 */
const struct ToneDescriptor gToneDescriptors[] = {
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1336, 941, 0 }, 0, 0},
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_0
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1209, 697, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_1
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1336, 697, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_2
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1477, 697, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_3
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1209, 770, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_4
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1336, 770, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_5
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1477, 770, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_6
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1209, 852, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_7
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1336, 852, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_8
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1477, 852, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_9
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1209, 941, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_S
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1477, 941, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_P
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1633, 697, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_A
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1633, 770, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_B
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1633, 852, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_C
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1633, 941, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_DTMF_D
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SUP_DIAL
    { .segments = { { .duration = 500 , .waveFreq = { 425, 0 }, 0, 0},
                    { .duration = 500, .waveFreq = { 0 }, 0, 0},
                       { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SUP_BUSY
    { .segments = { { .duration = 200, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SUP_CONGESTION
    { .segments = { { .duration = 200, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_SUP_RADIO_ACK
    { .segments = { { .duration = 200, .waveFreq = { 425, 0 }, 0, 0},
                    { .duration = 200, .waveFreq = { 0 }, 0, 0},
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 2,
      .repeatSegment = 0 },                  // TONE_SUP_RADIO_NOTAVAIL
    { .segments = { { .duration = 330, .waveFreq = { 950, 1400, 1800, 0 }, 0, 0},
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0},
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SUP_ERROR
    { .segments = { { .duration = 200, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 600, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SUP_CALL_WAITING
    { .segments = { { .duration = 1000, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SUP_RINGTONE
    { .segments = { { .duration = 40, .waveFreq = { 400, 1200, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_PROP_BEEP
    { .segments = { { .duration = 100, .waveFreq = { 1200, 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 0 }, 0, 0  },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 1,
      .repeatSegment = 0 },                  // TONE_PROP_ACK
    { .segments = { { .duration = 400, .waveFreq = { 300, 400, 500, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_PROP_NACK
    { .segments = { { .duration = 200, .waveFreq = { 400, 1200, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_PROP_PROMPT
#if 0
    { .segments = { { .duration = 40, .waveFreq = { 400, 1200, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 40, .waveFreq = { 400, 1200, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_PROP_BEEP2
#else
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 1000, 0, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_PROP_BEEP2
#endif
    { .segments = { { .duration = 250, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 620, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SUP_INTERCEPT
    { .segments = { { .duration = 250, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 620, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 7,
      .repeatSegment = 0 },                  // TONE_SUP_INTERCEPT_ABBREV
    { .segments = { { .duration = 250, .waveFreq = { 480, 620, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 7,
      .repeatSegment = 0 },                  // TONE_SUP_CONGESTION_ABBREV
    { .segments = { { .duration = 100, .waveFreq = { 350, 440, 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 2,
      .repeatSegment = 0 },                  // TONE_SUP_CONFIRM
    { .segments = { { .duration = 100, .waveFreq = { 480, 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 3,
      .repeatSegment = 0 },                  // TONE_SUP_PIP
    { .segments = {{ .duration = TONEGEN_INF, .waveFreq = { 425, 0 }, 0, 0},
                   { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_CDMA_DIAL_TONE_LITE
    { .segments = { { .duration = 2000, .waveFreq = { 440, 480, 0 }, 0, 0 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_CDMA_NETWORK_USA_RINGBACK
    { .segments = { { .duration = 250, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 620, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt =  TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_CDMA_INTERCEPT
    { .segments = { { .duration = 250, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 620, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt =  0,
      .repeatSegment = 0 },                  // TONE_CDMA_ABBR_INTERCEPT
    { .segments = { { .duration = 250, .waveFreq = { 480, 620, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_CDMA_REORDER
    { .segments = { { .duration = 250, .waveFreq = { 480, 620, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 7,
      .repeatSegment = 0 },                  // TONE_CDMA_ABBR_REORDER
    { .segments = { { .duration = 500, .waveFreq = { 480, 620, 0 }, 0, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_CDMA_NETWORK_BUSY
    { .segments = { { .duration = 100, .waveFreq = { 350, 440, 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 2,
      .repeatSegment = 0 },                  // TONE_CDMA_CONFIRM
    { .segments = { { .duration = 500, .waveFreq = { 660, 1000, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_CDMA_ANSWER
    { .segments = { { .duration = 300, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_CDMA_NETWORK_CALLWAITING
    { .segments = { { .duration = 100, .waveFreq = { 480, 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 3,
      .repeatSegment = 0 },                  // TONE_CDMA_PIP

    { .segments = { { .duration = 32, .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 64, .waveFreq = { 2556, 0}, 19, 0},
                    { .duration = 32, .waveFreq = { 2091, 0}, 0, 0},
                    { .duration = 48, .waveFreq = { 2556, 0}, 0, 0},
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0},
                    { .duration = 0,  .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = 0,
      .repeatSegment = 0 },                 // TONE_CDMA_CALL_SIGNAL_ISDN_NORMAL
    { .segments = { { .duration = 32, .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 64, .waveFreq = { 2556, 0}, 7, 0 },
                    { .duration = 32, .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 400, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 32,  .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 64,  .waveFreq = { 2556, 0}, 7, 4 },
                    { .duration = 32,  .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0,    .waveFreq = { 0 }, 0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },             // TONE_CDMA_CALL_SIGNAL_ISDN_INTERGROUP
    { .segments = { { .duration = 32, .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 64, .waveFreq = { 2556, 0}, 3, 0 },
                    { .duration = 16, .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 },     0, 0 },
                    { .duration = 32, .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 64, .waveFreq = { 2556, 0}, 3, 4 },
                    { .duration = 16, .waveFreq = { 2091, 0}, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 },     0, 0 },
                    { .duration = 0,   .waveFreq = { 0 },     0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                 // TONE_CDMA_CALL_SIGNAL_ISDN_SP_PRI
    { .segments = { { .duration = 0,  .waveFreq = { 0 }, 0, 0} },
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_CDMA_CALL_SIGNAL_ISDN_PAT3
    { .segments = { { .duration = 32, .waveFreq = { 2091, 0 }, 0, 0 },
                    { .duration = 64, .waveFreq = { 2556, 0 }, 4, 0 },
                    { .duration = 20, .waveFreq = { 2091, 0 }, 0, 0 },
                    { .duration = 0,  .waveFreq = { 0 }      , 0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },              // TONE_CDMA_CALL_SIGNAL_ISDN_PING_RING
    { .segments = { { .duration = 0,  .waveFreq = { 0 }, 0, 0} },
      .repeatCnt = 0,
      .repeatSegment = 0 },                   // TONE_CDMA_CALL_SIGNAL_ISDN_PAT5
    { .segments = { { .duration = 0,  .waveFreq = { 0 }, 0, 0} },
      .repeatCnt = 0,
      .repeatSegment = 0 },                   // TONE_CDMA_CALL_SIGNAL_ISDN_PAT6
    { .segments = { { .duration = 0,  .waveFreq = { 0 }, 0, 0} },
      .repeatCnt = 0,
      .repeatSegment = 0 },                   // TONE_CDMA_CALL_SIGNAL_ISDN_PAT7

    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 39, 0 },
                    { .duration = 4000, .waveFreq = { 0 },     0, 0 },
                    { .duration = 0,    .waveFreq = { 0 },     0, 0 } },
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_L
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 39, 0 },
                    { .duration = 4000, .waveFreq = { 0 },     0, 0 },
                    { .duration = 0,    .waveFreq = { 0 },     0, 0 } },
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_L
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 39, 0 },
                    { .duration = 4000, .waveFreq = { 0 },     0, 0 },
                    { .duration = 0,    .waveFreq = { 0 },     0, 0 } },
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_L
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 15, 0 },
                    { .duration = 400, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 } },
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_SS
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 15, 0 },
                    { .duration = 400, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_SS
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 15, 0 },
                    { .duration = 400, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_SS
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 15, 6 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_SSL
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 15, 6 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_SSL
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 15, 6 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_SSL
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                      { .duration = 25, .waveFreq = { 4000, 0 }, 19, 0 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 19, 3 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_SS_2
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 19, 0 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 19, 3 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_SS_2
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 19, 0 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 19, 3 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_SS_2
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 9, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 19, 3 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 9, 6 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_SLS
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 9, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 19, 3 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 9, 6 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_SLS
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 9, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 19, 3 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 9, 6 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_SLS
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 9, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 9, 3 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 9, 6 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 9, 9 },
                    { .duration = 2500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_S_X4
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 9, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 9, 3 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 9, 6 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 9, 9 },
                    { .duration = 2500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_S_X4
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 9, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 9, 3 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 9, 6 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 9, 9 },
                    { .duration = 2500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_S_X4
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 19, 0 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_PBX_L
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 19, 0 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_PBX_L
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 19, 0 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_PBX_L
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 3 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_PBX_SS
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 3 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_PBX_SS
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 3 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_PBX_SS
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 15, 6 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_PBX_SSL
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 15, 6 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_PBX_SSL
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 15, 6 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_PBX_SSL
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 15, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 6 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_PBX_SLS
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 15, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 6 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_PBX_SLS
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 15, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 6 },
                    { .duration = 1000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
       .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_LOW_PBX_SLS
    { .segments = { { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 6 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 3700, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 4000, 0 }, 7, 9 },
                    { .duration = 800, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_HIGH_PBX_S_X4
    { .segments = { { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 6 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2600, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 2900, 0 }, 7, 9 },
                    { .duration = 800, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                   // TONE_CDMA_MED_PBX_S_X4
    { .segments = { { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 3 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 6 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1300, 0 }, 0, 0 },
                    { .duration = 25, .waveFreq = { 1450, 0 }, 7, 9 },
                    { .duration = 800, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                // TONE_CDMA_LOW_PBX_S_X4

    { .segments = { { .duration = 62, .waveFreq = { 1109, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 784, 0 },  0, 0 },
                    { .duration = 62, .waveFreq = { 740, 0 },  0, 0 },
                    { .duration = 62, .waveFreq = { 622, 0 },  0, 0 },
                    { .duration = 62, .waveFreq = { 1109, 0 }, 0, 0 },
                    { .duration = 0,  .waveFreq = { 0 },       0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                // TONE_CDMA_ALERT_NETWORK_LITE
    { .segments = { { .duration = 62, .waveFreq = { 1245, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 659, 0 },  2, 0 },
                    { .duration = 62, .waveFreq = { 1245, 0 }, 0, 0 },
                    { .duration = 0,  .waveFreq = { 0 },       0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                // TONE_CDMA_ALERT_AUTOREDIAL_LITE
    { .segments = { { .duration = 400, .waveFreq = { 1150, 770, 0 }, 0, 0 },
                    { .duration = 0,   .waveFreq = { 0 },            0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                // TONE_CDMA_ONE_MIN_BEEP
    { .segments = { { .duration = 120, .waveFreq = { 941, 1477, 0 }, 0, 0 },
                    { .duration = 0,   .waveFreq = { 0 },            0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                // TONE_CDMA_KEYPAD_VOLUME_KEY_LITE
    { .segments = { { .duration = 375, .waveFreq = { 587, 0 }, 0, 0 },
                    { .duration = 125, .waveFreq = { 1175, 0 }, 0, 0 },
                    { .duration = 0,   .waveFreq = { 0 },       0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                // TONE_CDMA_PRESSHOLDKEY_LITE
    { .segments = { { .duration = 62, .waveFreq = { 587, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 784, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 831, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 784, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 1109, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 784, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 831, 0 }, 0, 0 },
                    { .duration = 62, .waveFreq = { 784, 0 }, 0, 0 },
                    { .duration = 0,  .waveFreq = { 0 },      0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                 // TONE_CDMA_ALERT_INCALL_LITE
    { .segments = { { .duration = 125, .waveFreq = { 941, 0 }, 0, 0 },
                    { .duration = 10,  .waveFreq = { 0 },      2, 0 },
                    { .duration = 4990, .waveFreq = { 0 },     0, 0 },
                    { .duration = 0,    .waveFreq = { 0 },     0, 0 } },
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                // TONE_CDMA_EMERGENCY_RINGBACK
    { .segments = { { .duration = 125, .waveFreq = { 1319, 0 }, 0, 0 },
                    { .duration = 125, .waveFreq = { 0 },       0, 0 },
                    { .duration = 0,   .waveFreq = { 0 },       0, 0 } },
      .repeatCnt = 2,
      .repeatSegment = 0 },                // TONE_CDMA_ALERT_CALL_GUARD
    { .segments = { { .duration = 125, .waveFreq = { 1047, 0 }, 0, 0 },
                    { .duration = 125, .waveFreq = { 370,  0 }, 0, 0 },
                    { .duration = 0,   .waveFreq = { 0 },       0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                // TONE_CDMA_SOFT_ERROR_LITE
    { .segments = { { .duration = 125, .waveFreq = { 1480, 0 }, 0, 0 },
                    { .duration = 125, .waveFreq = { 1397, 0 }, 0, 0 },
                    { .duration = 125, .waveFreq = { 784, 0 },  0, 0 },
                    { .duration = 0,   .waveFreq = { 0 },       0, 0 } },
      .repeatCnt = 0,
      .repeatSegment = 0 },                // TONE_CDMA_CALLDROP_LITE

    { .segments = { { .duration = 500, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_CDMA_NETWORK_BUSY_ONE_SHOT
    { .segments = { { .duration = 400, .waveFreq = { 1150, 770 }, 0, 0 },
                    { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_CDMA_ABBR_ALERT
      { .segments = { { .duration = 0, .waveFreq = { 0 }, 0, 0 }},
      .repeatCnt = 0,
      .repeatSegment = 0 },                  // TONE_CDMA_SIGNAL_OFF

    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 350, 440, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_ANSI_DIAL
    { .segments = { { .duration = 500, .waveFreq = { 480, 620, 0 }, 0, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_ANSI_BUSY
    { .segments = { { .duration = 250, .waveFreq = { 480, 620, 0 }, 0, 0 },
                    { .duration = 250, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_ANSI_CONGESTION
    { .segments = { { .duration = 300, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 9700, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 100, .waveFreq = { 440, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 1 },                  // TONE_ANSI_CALL_WAITING
    { .segments = { { .duration = 2000, .waveFreq = { 440, 480, 0 }, 0, 0 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_ANSI_RINGTONE
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 400, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_JAPAN_DIAL
    { .segments = { { .duration = 500, .waveFreq = { 400, 0 }, 0, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_JAPAN_BUSY
    { .segments = { { .duration = 1000, .waveFreq = { 400, 0 }, 0, 0 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_JAPAN_RADIO_ACK
    { .segments = { { .duration = 375, .waveFreq = { 400, 0 }, 0, 0 },
                    { .duration = 375, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_GB_BUSY
    { .segments = { { .duration = 400, .waveFreq = { 400, 0 }, 0, 0 },
                    { .duration = 350, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 225, .waveFreq = { 400, 0 }, 0, 0 },
                    { .duration = 525, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_GB_CONGESTION
    { .segments = { { .duration = 1000, .waveFreq = { 400, 450, 0 }, 0, 0 },
                    { .duration = 4000, .waveFreq = { 0 }, 0, 0},
                    { .duration = 0, .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_GB_RINGTONE
    { .segments = { { .duration = TONEGEN_INF, .waveFreq = { 400, 425, 450, 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_AUSTRALIA_DIAL
    { .segments = { { .duration = 400, .waveFreq = { 400, 450, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 400, .waveFreq = { 400, 450, 0 }, 0, 0 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0},
                    { .duration = 0, .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_AUSTRALIA_RINGTONE
    { .segments = { { .duration = 375, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 375, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_AUSTRALIA_BUSY
    { .segments = { { .duration = 200, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 4400, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_AUSTRALIA_CALL_WAITING
    { .segments = { { .duration = 375, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 375, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_AUSTRALIA_CONGESTION
    { .segments = { { .duration = 750, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 750, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SG_BUSY
    { .segments = { { .duration = 400, .waveFreq = { 401, 425, 449, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 400, .waveFreq = { 401, 425, 449, 0 }, 0, 0 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_SG_RINGTONE
    { .segments = { { .duration = 500, .waveFreq = { 480, 620, 0 }, 0, 0 },
                    { .duration = 500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_HK_BUSY
    { .segments = { { .duration = 400, .waveFreq = { 440, 480, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 400, .waveFreq = { 440, 480, 0 }, 0, 0 },
                    { .duration = 3000, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_HK_RINGTONE
    { .segments = { { .duration = 400, .waveFreq = { 400, 450, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 400, .waveFreq = { 400, 450, 0 }, 0, 0 },
                    { .duration = 2000, .waveFreq = { 0 }, 0, 0},
                    { .duration = 0, .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_IE_RINGTONE
    { .segments = { { .duration = 180, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 200, .waveFreq = { 425, 0 }, 0, 0 },
                    { .duration = 4500, .waveFreq = { 0 }, 0, 0 },
                    { .duration = 0 , .waveFreq = { 0 }, 0, 0}},
      .repeatCnt = TONEGEN_INF,
      .repeatSegment = 0 },                  // TONE_IE_CALL_WAITING
};

/**
 * Used by getToneForRegion() to convert user specified supervisory tone type
 * to actual tone for current region.
 */
const unsigned char sToneMappingTable[TONE_REGION_MAX - 1][NUM_SUP_TONES] = {
    {   // ANSI
        TONE_ANSI_DIAL,             // TONE_SUP_DIAL
        TONE_ANSI_BUSY,             // TONE_SUP_BUSY
        TONE_ANSI_CONGESTION,       // TONE_SUP_CONGESTION
        TONE_SUP_RADIO_ACK,         // TONE_SUP_RADIO_ACK
        TONE_SUP_RADIO_NOTAVAIL,    // TONE_SUP_RADIO_NOTAVAIL
        TONE_SUP_ERROR,             // TONE_SUP_ERROR
        TONE_ANSI_CALL_WAITING,     // TONE_SUP_CALL_WAITING
        TONE_ANSI_RINGTONE          // TONE_SUP_RINGTONE
    },
    {   // JAPAN
        TONE_JAPAN_DIAL,             // TONE_SUP_DIAL
        TONE_JAPAN_BUSY,             // TONE_SUP_BUSY
        TONE_SUP_CONGESTION,         // TONE_SUP_CONGESTION
        TONE_JAPAN_RADIO_ACK,        // TONE_SUP_RADIO_ACK
        TONE_SUP_RADIO_NOTAVAIL,     // TONE_SUP_RADIO_NOTAVAIL
        TONE_SUP_ERROR,              // TONE_SUP_ERROR
        TONE_SUP_CALL_WAITING,       // TONE_SUP_CALL_WAITING
        TONE_SUP_RINGTONE            // TONE_SUP_RINGTONE
    },
    {   // GB
        TONE_ANSI_DIAL,              // TONE_SUP_DIAL
        TONE_GB_BUSY,                // TONE_SUP_BUSY
        TONE_GB_CONGESTION,          // TONE_SUP_CONGESTION
        TONE_SUP_RADIO_ACK,          // TONE_SUP_RADIO_ACK
        TONE_SUP_RADIO_NOTAVAIL,     // TONE_SUP_RADIO_NOTAVAIL
        TONE_SUP_ERROR,              // TONE_SUP_ERROR
        TONE_SUP_CALL_WAITING,       // TONE_SUP_CALL_WAITING
        TONE_SUP_RINGTONE            // TONE_SUP_RINGTONE
    },
    {   // AUSTRALIA
        TONE_AUSTRALIA_DIAL,        // TONE_SUP_DIAL
        TONE_AUSTRALIA_BUSY,        // TONE_SUP_BUSY
        TONE_AUSTRALIA_CONGESTION,  // TONE_SUP_CONGESTION
        TONE_SUP_RADIO_ACK,         // TONE_SUP_RADIO_ACK
        TONE_SUP_RADIO_NOTAVAIL,    // TONE_SUP_RADIO_NOTAVAIL
        TONE_SUP_ERROR,             // TONE_SUP_ERROR
        TONE_AUSTRALIA_CALL_WAITING,// TONE_SUP_CALL_WAITING
        TONE_AUSTRALIA_RINGTONE     // TONE_SUP_RINGTONE
    },
    {   // SINGAPORE
        TONE_SUP_DIAL,               // TONE_SUP_DIAL
        TONE_SG_BUSY,                // TONE_SUP_BUSY
        TONE_SUP_CONGESTION,         // TONE_SUP_CONGESTION
        TONE_SUP_RADIO_ACK,          // TONE_SUP_RADIO_ACK
        TONE_SUP_RADIO_NOTAVAIL,     // TONE_SUP_RADIO_NOTAVAIL
        TONE_SUP_ERROR,              // TONE_SUP_ERROR
        TONE_SUP_CALL_WAITING,       // TONE_SUP_CALL_WAITING
        TONE_SG_RINGTONE             // TONE_SUP_RINGTONE
    },
    {   // HONGKONG
        TONE_SUP_DIAL,               // TONE_SUP_DIAL
        TONE_HK_BUSY,                // TONE_SUP_BUSY
        TONE_SUP_CONGESTION,         // TONE_SUP_CONGESTION
        TONE_SUP_RADIO_ACK,          // TONE_SUP_RADIO_ACK
        TONE_SUP_RADIO_NOTAVAIL,     // TONE_SUP_RADIO_NOTAVAIL
        TONE_SUP_ERROR,              // TONE_SUP_ERROR
        TONE_SUP_CALL_WAITING,       // TONE_SUP_CALL_WAITING
        TONE_HK_RINGTONE             // TONE_SUP_RINGTONE
    },
    {   // IRELAND
        TONE_SUP_DIAL,               // TONE_SUP_DIAL
        TONE_SUP_BUSY,               // TONE_SUP_BUSY
        TONE_SUP_CONGESTION,         // TONE_SUP_CONGESTION
        TONE_SUP_RADIO_ACK,          // TONE_SUP_RADIO_ACK
        TONE_SUP_RADIO_NOTAVAIL,     // TONE_SUP_RADIO_NOTAVAIL
        TONE_SUP_ERROR,              // TONE_SUP_ERROR
        TONE_IE_CALL_WAITING,        // TONE_SUP_CALL_WAITING
        TONE_IE_RINGTONE             // TONE_SUP_RINGTONE
    }
};

struct ToneGenerator g_tone_gen;

static bool prepareWave(struct ToneGenerator *p_tone_gen);
static void clearWaveGens(void);
static struct WaveGenerator *WaveGenerator_create(uint32_t samplingRate,
       unsigned short frequency, float volume);
static void WaveGenerator_destroy(struct WaveGenerator *p_wave_gen);

static inline struct ToneGenerator *get_tone_gen(void)
{
    return &g_tone_gen;
}

static u32 tone_wait(u32 event, u32 time)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();
    u32 actual_flags;
    u32 ret;

    p_tone_gen->wait_cnt++;
    mutex_unlock(p_tone_gen->mLock);
    ret = aos_get_event(p_tone_gen->mEvent, event, AOS_OR_CLEAR,
                        &actual_flags, time);
    mutex_lock(p_tone_gen->mLock);

    return ret;
}

static u32 tone_wakeup(u32 event)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    if (p_tone_gen->wait_cnt) {
        aos_set_event(p_tone_gen->mEvent, event, AOS_OR);
        p_tone_gen->wait_cnt = 0;
    }

    return 0;
}

static void wgen_key_add(unsigned int freq, struct WaveGenerator *wgen)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();
    int index = p_tone_gen->wgen_key_num;

    if (index < MAX_WGEN_NUM) {
        p_tone_gen->wgen_key[index].freq = freq;
        p_tone_gen->wgen_key[index].wgen = wgen;
        p_tone_gen->wgen_key_num++;
    }
}

static struct WaveGenerator *wgen_key_get(unsigned int freq)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();
    int i;

    for (i = 0; i < p_tone_gen->wgen_key_num; i++) {
        if (p_tone_gen->wgen_key[i].freq == freq)
            return (struct WaveGenerator *)p_tone_gen->wgen_key[i].wgen;
    }
    return NULL;
}

#if 0
static struct WaveGenerator *wgen_key_get_by_index(int index)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    if (index < p_tone_gen->wgen_key_num)
        return (struct WaveGenerator *)p_tone_gen->wgen_key[index]->wgen;
    else
        return NULL;
}

static int wgen_key_get_num(struct vector_array *arr)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    return p_tone_gen->wgen_key_num;
}
#endif

static void wgen_key_init(void)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    if (!p_tone_gen->wgen_key) {
        p_tone_gen->wgen_key_num = 0;
        p_tone_gen->wgen_key = (void *)audio_malloc(MAX_WGEN_NUM *
                               sizeof(struct wgen_key));
        if (p_tone_gen->wgen_key)
            memset(p_tone_gen->wgen_key, 0,
                   MAX_WGEN_NUM * sizeof(struct wgen_key));
    }
}

static void wgen_key_deinit(void)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();
    int i;

    for (i = 0; i < p_tone_gen->wgen_key_num; i++) {
        p_tone_gen->wgen_key[i].freq = 0;
        if (p_tone_gen->wgen_key[i].wgen) {
            WaveGenerator_destroy(p_tone_gen->wgen_key[i].wgen);
            p_tone_gen->wgen_key[i].wgen = 0;
        }
    }
    p_tone_gen->wgen_key_num = 0;
}

unsigned int systemTime(void)
{
    /* us */
    //return tx_time_get() * 1000;
    return 0;
}

static int tone_gettime(struct timespec *time)
{
    /* TBD */
    return -1;
}

/*******************************************************************************
 *
 *    Method:        numWaves()
 *
 *    Description:    Count number of sine waves needed to generate
 *                    a tone segment (e.g 2 for DTMF).
 *
 *    Input:
 *        segmentIdx        tone segment index
 *
 *    Output:
 *        returned value:    nummber of sine waves
 *
*******************************************************************************/
static unsigned int numWaves(unsigned int segmentIdx)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();
    unsigned int lCnt = 0;

    if (p_tone_gen->mpToneDesc->segments[segmentIdx].duration) {
        while (p_tone_gen->mpToneDesc->segments[segmentIdx].waveFreq[lCnt]) {
            lCnt++;
        }
        lCnt++;
    }

    return lCnt;
}

/*******************************************************************************
 *
 *    Method:        getSamples()
 *
 *    Description:    Generates count samples of a sine wave and accumulates
 *        result in outBuffer.
 *
 *    Input:
 *        outBuffer:      Output buffer where to accumulate samples.
 *        count:          number of samples to produce.
 *        command:        special action requested (see enum gen_command).
 *
 *    Output:
 *        none
 *
*******************************************************************************/
static void getSamples(struct WaveGenerator *p_wave_gen, short *outBuffer,
                       unsigned int count, unsigned int command)
{
    long lS1, lS2;
    long lA1, lAmplitude;
    long Sample;

    if (command == WAVEGEN_START) {
        lS1 = (long)0;
        lS2 = (long)p_wave_gen->mS2_0;
    } else {
        lS1 = p_wave_gen->mS1;
        lS2 = p_wave_gen->mS2;
    }
    lA1 = (long)p_wave_gen->mA1_Q14;
    lAmplitude = (long)p_wave_gen->mAmplitude_Q15;

    if (command == WAVEGEN_STOP) {
        lAmplitude <<= 16;
        if (count == 0) {
            return;
        }
        long dec = lAmplitude/count;
        /* loop generation */
        while (count) {
            count--;
            Sample = ((lA1 * lS1) >> S_Q14) - lS2;
            /* shift delay */
            lS2 = lS1;
            lS1 = Sample;
            Sample = ((lAmplitude>>16) * Sample) >> S_Q15;
            /* put result in buffer */
            *(outBuffer++) += (short)Sample;
            lAmplitude -= dec;
        }
    } else {
        /* loop generation */
        while (count) {
            count--;
            Sample = ((lA1 * lS1) >> S_Q14) - lS2;
            /* shift delay */
            lS2 = lS1;
            lS1 = Sample;
            Sample = (lAmplitude * Sample) >> S_Q15;
            /* put result in buffer */
            *(outBuffer++) += (short)Sample;
        }
    }

    /* save status */
    p_wave_gen->mS1 = lS1;
    p_wave_gen->mS2 = lS2;
}

static int audioCallback(u32 obj, int event, char *buffer, u32 *gen_size)
{
    struct ToneGenerator *p_tone_gen = (struct ToneGenerator *)obj;
    short *lpOut = (short *)buffer;
    u32 size = *gen_size;
    unsigned int lNumSmp = size/sizeof(short);
    const struct ToneDescriptor *lpToneDesc = p_tone_gen->mpToneDesc;

    if (event != AUDIO_CLIENT_CMD_GET_MORE)
        return 0;

    if (size == 0)
        return 0;

    memset(lpOut, 0, size);

    while (lNumSmp) {
        unsigned int lReqSmp = lNumSmp < p_tone_gen->mProcessSize * 2 ?
            lNumSmp : p_tone_gen->mProcessSize;
        unsigned int lGenSmp;
        unsigned int lWaveCmd = WAVEGEN_CONT;
        bool lSignal = FALSE;

        mutex_lock(p_tone_gen->mLock);

        /**
         * Update pcm frame count and end time
         * (current time at the end of this process)
         */
        p_tone_gen->mTotalSmp += lReqSmp;

        /* Update tone gen state machine and select wave gen command */
        switch (p_tone_gen->mState) {
        case TONE_PLAYING:
            lWaveCmd = WAVEGEN_CONT;
            break;
        case TONE_STARTING:
            AIDXD(0x073399, "", "Starting Cbk");
            lWaveCmd = WAVEGEN_START;
            break;
        case TONE_STOPPING:
        case TONE_RESTARTING:
            AIDXD(0x084668, "", "Stop/restart Cbk");
            lWaveCmd = WAVEGEN_STOP;

            /* forced to skip state machine management below */
            p_tone_gen->mNextSegSmp = TONEGEN_INF;
            break;
        case TONE_STOPPED:
            AIDXD(0x05de7e, "", "Stopped Cbk");
            goto audioCallback_EndLoop;
        default:
            AIDXD(0x00ee2b, "", "Extra Cbk");
            goto audioCallback_EndLoop;
        }

        AIDXD(0x0c5251, "dd", "mTotalSmp=%d,mMaxSmp=%d\n",
              p_tone_gen->mTotalSmp, p_tone_gen->mMaxSmp);
        /* Exit if tone sequence is over */
        if (lpToneDesc->segments[p_tone_gen->mCurSegment].duration == 0 ||
            p_tone_gen->mTotalSmp > p_tone_gen->mMaxSmp) {
            if (p_tone_gen->mState == TONE_PLAYING) {
                p_tone_gen->mState = TONE_STOPPING;
            }
            if (lpToneDesc->segments[p_tone_gen->mCurSegment].duration == 0) {
                goto audioCallback_EndLoop;
            }
            /* fade out before stopping if maximum duration reached */
            lWaveCmd = WAVEGEN_STOP;
            /* forced to skip state machine management below */
            p_tone_gen->mNextSegSmp = TONEGEN_INF;
        }

        if (p_tone_gen->mTotalSmp > p_tone_gen->mNextSegSmp) {
            /* Time to go to next sequence segment */

            AIDXD(0x107d81, "d", "End Segment, time:%d", (unsigned int)(systemTime()/1000000));

            lGenSmp = lReqSmp;

            /* If segment,  ON -> OFF transition : ramp volume down */
            if (lpToneDesc->segments[p_tone_gen->mCurSegment].waveFreq[0] != 0) {
                lWaveCmd = WAVEGEN_STOP;
                unsigned int lFreqIdx = 0;
                unsigned short lFrequency = lpToneDesc->segments[p_tone_gen->mCurSegment].waveFreq[lFreqIdx];

                while (lFrequency != 0) {
                    struct WaveGenerator *lpWaveGen = wgen_key_get(lFrequency);

                    getSamples(lpWaveGen, lpOut, lGenSmp, lWaveCmd);
                    lFrequency = lpToneDesc->segments[p_tone_gen->mCurSegment].waveFreq[++lFreqIdx];
                }
                AIDXD(0x0ca4b5, "dd", "ON->OFF, lGenSmp: %d, lReqSmp: %d", lGenSmp, lReqSmp);
            }

            /* check if we need to loop and loop for the reqd times */
            if (lpToneDesc->segments[p_tone_gen->mCurSegment].loopCnt) {
                if (p_tone_gen->mLoopCounter <
                    lpToneDesc->segments[p_tone_gen->mCurSegment].loopCnt) {
                    AIDXD(0x14f664, "ddd", "in if loop loopCnt(%d) loopctr(%d), CurSeg(%d)",
                          lpToneDesc->segments[p_tone_gen->mCurSegment].loopCnt,
                          p_tone_gen->mLoopCounter,
                          p_tone_gen->mCurSegment);
                    p_tone_gen->mCurSegment =
                        lpToneDesc->segments[p_tone_gen->mCurSegment].loopIndx;
                    ++p_tone_gen->mLoopCounter;
                } else {
                    /* completed loop. go to next segment */
                    p_tone_gen->mLoopCounter = 0;
                    p_tone_gen->mCurSegment++;
                    AIDXD(0x0ba813, "ddd", "in else loop loopCnt(%d) loopctr(%d), CurSeg(%d)",
                          lpToneDesc->segments[p_tone_gen->mCurSegment].loopCnt,
                          p_tone_gen->mLoopCounter,
                          p_tone_gen->mCurSegment);
                }
            } else {
                p_tone_gen->mCurSegment++;
                AIDXD(0x09a500, "ddd", "Goto next seg loopCnt(%d) loopctr(%d), CurSeg(%d)",
                      lpToneDesc->segments[p_tone_gen->mCurSegment].loopCnt,
                      p_tone_gen->mLoopCounter,
                      p_tone_gen->mCurSegment);

            }

            /* Handle loop if last segment reached */
            if (lpToneDesc->segments[p_tone_gen->mCurSegment].duration == 0) {
                AIDXD(0x0f4df1, "d", "Last Seg: %d", p_tone_gen->mCurSegment);

                /**
                 * Pre increment loop count and restart if total
                 * count not reached. Stop sequence otherwise
                 */
                if (++p_tone_gen->mCurCount <= lpToneDesc->repeatCnt) {
                    AIDXD(0x17f82a, "d", "Repeating Count: %d", p_tone_gen->mCurCount);

                    p_tone_gen->mCurSegment = lpToneDesc->repeatSegment;
                    if (lpToneDesc->segments[lpToneDesc->repeatSegment].waveFreq[0] != 0) {
                        lWaveCmd = WAVEGEN_START;
                    }

                    AIDXD(0x06307b, "dd", "New segment %d, Next Time: %d",
                          p_tone_gen->mCurSegment,
                          (p_tone_gen->mNextSegSmp*1000) / p_tone_gen->mSamplingRate);

                } else {
                    lGenSmp = 0;
                    AIDXD(0x18a85b, "d", "End repeat, time: %d",
                          (unsigned int)(systemTime() / 1000000));
                }
            } else {
                AIDXD(0x06307b, "dd", "New segment %d, Next Time: %d", p_tone_gen->mCurSegment,
                      (p_tone_gen->mNextSegSmp*1000) / p_tone_gen->mSamplingRate);
                if (lpToneDesc->segments[p_tone_gen->mCurSegment].waveFreq[0] != 0) {
                    /**
                     * If next segment is not silent,
                     * OFF -> ON transition : reset wave generator
                     */
                    lWaveCmd = WAVEGEN_START;

                    AIDXD(0x07024f, "dd", "OFF->ON, lGenSmp: %d, lReqSmp: %d",
                          lGenSmp, lReqSmp);
                } else {
                    lGenSmp = 0;
                }
            }

            /**
             * Update next segment transition position.
             * No harm to do it also for last segment as
             * p_tone_gen->mNextSegSmp won't be used any more
             */
            p_tone_gen->mNextSegSmp +=
                (lpToneDesc->segments[p_tone_gen->mCurSegment].duration *
                p_tone_gen->mSamplingRate) / 1000;

        } else {
            /* Inside a segment keep tone ON or OFF */
            if (lpToneDesc->segments[p_tone_gen->mCurSegment].waveFreq[0] == 0) {
                /* If odd segment, tone is currently OFF */
                lGenSmp = 0;
            } else {
                /* If event segment, tone is currently ON */
                lGenSmp = lReqSmp;
            }
        }

        if (lGenSmp) {
            /**
             * If samples must be generated,
             * call all active wave generators and acumulate waves in lpOut
             */
            unsigned int lFreqIdx = 0;
            unsigned short lFrequency =
                lpToneDesc->segments[p_tone_gen->mCurSegment].waveFreq[lFreqIdx];

            while (lFrequency != 0) {
                struct WaveGenerator *lpWaveGen = wgen_key_get(lFrequency);

                if (lpWaveGen)
                    getSamples(lpWaveGen, lpOut, lGenSmp, lWaveCmd);
                lFrequency = lpToneDesc->segments[p_tone_gen->mCurSegment].waveFreq[++lFreqIdx];
            }
        }

        lNumSmp -= lReqSmp;
        lpOut += lReqSmp;

audioCallback_EndLoop:

        switch (p_tone_gen->mState) {
        case TONE_RESTARTING:
            AIDXD(0x0f97d1, "", "Cbk restarting track");
            if (prepareWave(p_tone_gen)) {
                p_tone_gen->mState = TONE_STARTING;
                if (tone_gettime(&p_tone_gen->mStartTime) != 0) {
                    p_tone_gen->mStartTime.tv_sec = 0;
                }
                /**
                 * must reload lpToneDesc as prepareWave()
                 * may change mpToneDesc
                 */
                lpToneDesc = p_tone_gen->mpToneDesc;
            } else {
                AIDXW(0x025b23, "", "Cbk restarting prepareWave() failed");
                p_tone_gen->mState = TONE_IDLE;
                /* Force loop exit */
                lNumSmp = 0;
            }
            lSignal = TRUE;
            break;
        case TONE_STOPPING:
            AIDXD(0x011e71, "", "Cbk Stopping");
            p_tone_gen->mState = TONE_STOPPED;
            /* Force loop exit */
            lNumSmp = 0;
            break;
        case TONE_STOPPED:
            AIDXD(0x05b5fa, "", "Cbk Stopped track");
            p_tone_gen->mState = TONE_INIT;

            /* Force loop exit */
            lNumSmp = 0;
            *gen_size = 0;
            lSignal = TRUE;
            break;
        case TONE_STARTING:
            AIDXD(0x174b65, "", "Cbk starting track");
            p_tone_gen->mState = TONE_PLAYING;
            /* do not need signal */
            /* lSignal = TRUE; */
            break;
        case TONE_PLAYING:
            AIDXD(0x110d07, "", "Cbk playing");
            break;
        default:
            /* Force loop exit */
            lNumSmp = 0;
            *gen_size = 0;
            break;
        }

        if (lSignal) {
            tone_wakeup(EVENT_DONE);
        }
        mutex_unlock(p_tone_gen->mLock);
    }

    if (p_tone_gen->mState == TONE_INIT)
        return ASTREAM_DATA_EOS;

    return 0;
}

/*******************************************************************************
 *
 *    Method:        prepareWave()
 *
 *    Description:    Prepare wave generators and reset tone sequencer state
 *                machine. mpNewToneDesc must have been initialized
 *                before calling this function.
 *    Input:
 *        none
 *
 *    Output:
 *        returned value:   TRUE if wave generators have been created,
 *                          FALSE otherwise
 *
*******************************************************************************/
static bool prepareWave(struct ToneGenerator *p_tone_gen)
{
    unsigned int segmentIdx = 0;

    if (p_tone_gen->mpNewToneDesc == NULL) {
        return FALSE;
    }

    /* Remove existing wave generators if any */
    clearWaveGens();

    p_tone_gen->mpToneDesc = p_tone_gen->mpNewToneDesc;

    if (p_tone_gen->mDurationMs == -1) {
        p_tone_gen->mMaxSmp = TONEGEN_INF;
    } else {
        if (p_tone_gen->mDurationMs >
                (int)(TONEGEN_INF / p_tone_gen->mSamplingRate)) {
            p_tone_gen->mMaxSmp =
                (p_tone_gen->mDurationMs / 1000) * p_tone_gen->mSamplingRate;
        } else {
            p_tone_gen->mMaxSmp =
                (p_tone_gen->mDurationMs * p_tone_gen->mSamplingRate) / 1000;
        }
        AIDXD(0x08841c, "d", "prepareWave, duration limited to %d ms",
              p_tone_gen->mDurationMs);
    }

    while (p_tone_gen->mpToneDesc->segments[segmentIdx].duration) {
        /* Get total number of sine waves: needed to adapt sine wave gain. */
        unsigned int lNumWaves = numWaves(segmentIdx);
        unsigned int freqIdx = 0;
        unsigned int frequency =
            p_tone_gen->mpToneDesc->segments[segmentIdx].waveFreq[freqIdx];
        if (lNumWaves == 0)
            continue;
        while (frequency) {
            /**
             * Instantiate a wave generator
             * if  ot already done for this frequency.
             */
            if (wgen_key_get(frequency) == NULL) {
                struct WaveGenerator *lpWaveGen =
                    WaveGenerator_create(p_tone_gen->mSamplingRate, frequency,
                                         TONEGEN_GAIN / lNumWaves);
                wgen_key_add(frequency, lpWaveGen);
            }
            frequency = p_tone_gen->mpNewToneDesc->segments[segmentIdx].waveFreq[++freqIdx];
        }
        segmentIdx++;
    }

    /* Initialize tone sequencer */
    p_tone_gen->mTotalSmp = 0;
    p_tone_gen->mCurSegment = 0;
    p_tone_gen->mCurCount = 0;
    p_tone_gen->mLoopCounter = 0;
    if (p_tone_gen->mpToneDesc->segments[0].duration == TONEGEN_INF) {
        p_tone_gen->mNextSegSmp = TONEGEN_INF;
    } else{
        p_tone_gen->mNextSegSmp = p_tone_gen->mpToneDesc->segments[0].duration *
                                    p_tone_gen->mSamplingRate / 1000;
    }

    return TRUE;
}

/*******************************************************************************
 *
 *    Method:        clearWaveGens()
 *
 *    Description:    Removes all wave generators.
 *
 *    Input:
 *        none
 *
 *    Output:
 *        none
 *
*******************************************************************************/
static void clearWaveGens() {
    AIDXD(0x111a66, "", "Clearing mWaveGens:");
    wgen_key_deinit();
}

/*******************************************************************************
 *
 *    Method:       getToneForRegion()
 *
 *    Description:  Get correct ringtone type according to current region.
 *      The corrected ring tone type is the tone descriptor index
 *      in sToneDescriptors[].
 *
 *    Input:
 *        none
 *
 *    Output:
 *        none
 *
*******************************************************************************/
static enum tone_type getToneForRegion(enum tone_type toneType)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    enum tone_type regionTone;

    if (p_tone_gen->mRegion == TONE_REGION_CEPT || toneType < FIRST_SUP_TONE ||
        toneType > LAST_SUP_TONE) {
        regionTone = toneType;
    } else {
        regionTone = (enum tone_type)
            sToneMappingTable[p_tone_gen->mRegion][toneType - FIRST_SUP_TONE];
    }

    AIDXD(0x075882, "ddd", "getToneForRegion, tone %d, region %d, regionTone %d",
          toneType, p_tone_gen->mRegion, regionTone);

    return regionTone;
}

/*******************************************************************************
 *
 *    Method:        WaveGenerator::WaveGenerator()
 *
 *    Description:    Constructor.
 *
 *    Input:
 *        samplingRate:    Output sampling rate in Hz
 *        frequency:       Frequency of the sine wave to generate in Hz
 *        volume:          volume (0.0 to 1.0)
 *
 *    Output:
 *        none
 *
*******************************************************************************/
static struct WaveGenerator *WaveGenerator_create(uint32_t samplingRate,
                   unsigned short frequency, float volume)
{
    double d0;
    /* frequency / samplingRate */
    double F_div_Fs;

    struct WaveGenerator *p_wave_gen;
    p_wave_gen = (struct WaveGenerator *)
                    audio_malloc(sizeof(struct WaveGenerator));
    if (!p_wave_gen) {
        AIDXE(0x143530, "", "WaveGenerator_create no memory!\n");
        return 0;
    }

    F_div_Fs = frequency / (double)samplingRate;
    d0 = - (float)GEN_AMP * sin(2 * M_PI * F_div_Fs);
    p_wave_gen->mS2_0 = (short)d0;
    p_wave_gen->mS1 = 0;
    p_wave_gen->mS2 = p_wave_gen->mS2_0;

    p_wave_gen->mAmplitude_Q15 = (short)(32767. * 32767. * volume / GEN_AMP);
    /* take some margin for amplitude fluctuation */
    if (p_wave_gen->mAmplitude_Q15 > 32500)
        p_wave_gen->mAmplitude_Q15 = 32500;

    /* Q14*2*cos() */
    d0 = 32768.0 * cos(2 * M_PI * F_div_Fs);
    if (d0 > 32767)
        d0 = 32767;
    p_wave_gen->mA1_Q14 = (short) d0;

    AIDXD(0x112c1d, "ddd", "WaveGenerator init, mA1_Q14: %d, mS2_0: %d, mAmplitude_Q15: %d",
          p_wave_gen->mA1_Q14, p_wave_gen->mS2_0, p_wave_gen->mAmplitude_Q15);

    return p_wave_gen;
}

/*******************************************************************************
 *
 *    Method:        WaveGenerator::~WaveGenerator()
 *
 *    Description:    Destructor.
 *
 *    Input:
 *        none
 *
 *    Output:
 *        none
 *
*******************************************************************************/
static void WaveGenerator_destroy(struct WaveGenerator *p_wave_gen)
{
    if (p_wave_gen)
        audio_free(p_wave_gen);
}

static bool init_audio_stream(void)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    if (!p_tone_gen->ahandle)
        p_tone_gen->ahandle = 1;/* TODO */

    if (p_tone_gen->ahandle)
        p_tone_gen->mState = TONE_INIT;

    return !!p_tone_gen->ahandle;
}

static u32 tone_open(enum astream_format format, void *msg)
{
    return (u32)get_tone_gen();
}

static int tone_start(u32 obj, void *aplay_data)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();
    struct audio_play_sub_tone *aplay_tone =
        (struct audio_play_sub_tone *)aplay_data;
    enum tone_type toneType;
    int durationMs;
    bool lResult = AIC_EPERM;
    status_t lStatus;

    AENTRY;

    toneType = aplay_tone->tone_id;
    durationMs = aplay_tone->duation;

    if (toneType >= NUM_TONES)
        return lResult;

    toneType = getToneForRegion(toneType);
    if (toneType == TONE_CDMA_SIGNAL_OFF) {
        return AIC_SUCCESS;
    }

    if (p_tone_gen->mState == TONE_IDLE) {
        AIDXD(0x16f79b, "", "startTone: try to re-init AudioTrack");
        if (!init_audio_stream()) {
            return lResult;
        }
    }

    AIDXD(0x0fa0b4, "d", "startTone toneType %d", toneType);

    mutex_lock(p_tone_gen->mLock);

    /* Get descriptor for requested tone */
    p_tone_gen->mpNewToneDesc = &p_tone_gen->sToneDescriptors[toneType];

    p_tone_gen->mDurationMs = durationMs;

    if (p_tone_gen->mState == TONE_STOPPED) {
        AIDXD(0x0977d4, "", "Start waiting for previous tone to stop");
        /* lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3)); */
        lStatus = tone_wait(EVENT_DONE, 3000);
        if (lStatus != AOS_SUCCESS) {
            AIDXE(0x0fda32, "d", "--- start wait for stop timed out, status %d", lStatus);
            p_tone_gen->mState = TONE_IDLE;
            mutex_unlock(p_tone_gen->mLock);
            return lResult;
        }
    }

    if (p_tone_gen->mState == TONE_INIT) {
        if (prepareWave(p_tone_gen)) {
            AIDXD(0x08484e, "d", "Immediate start, time %d",
                  (unsigned int)(systemTime() / 1000000));
            lResult = AIC_SUCCESS;
            p_tone_gen->mState = TONE_STARTING;
            if (tone_gettime(&p_tone_gen->mStartTime) != 0) {
                p_tone_gen->mStartTime.tv_sec = 0;
            }
            mutex_unlock(p_tone_gen->mLock);
            /* mpAudioTrack->start(); */
            mutex_lock(p_tone_gen->mLock);
            if (p_tone_gen->mState == TONE_STARTING) {
                AIDXD(0x1306d8, "", "Wait for start callback");
                /* lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3)); */
                /* lStatus = tone_wait(EVENT_DONE, 3000); */
                lStatus = AOS_SUCCESS;
                if (lStatus != AOS_SUCCESS) {
                    AIDXE(0x0eaae5, "d", "--- Immediate start timed out, status %d", lStatus);
                    p_tone_gen->mState = TONE_IDLE;
                    lResult = AIC_EPERM;
                }
            }
        } else {
            p_tone_gen->mState = TONE_IDLE;
        }
    } else {
        AIDXD(0x0e815e, "", "Delayed start");
        p_tone_gen->mState = TONE_RESTARTING;
        p_tone_gen->mStartTime.tv_sec = 0;
        /* lStatus = mWaitCbkCond.waitRelative(mLock, seconds(3)); */
        lStatus = tone_wait(EVENT_DONE, 3000);
        if (lStatus == AOS_SUCCESS) {
            if (p_tone_gen->mState != TONE_IDLE) {
                lResult = AIC_SUCCESS;
            }
            AIDXD(0x10e91c, "", "cond received");
        } else {
            AIDXE(0x14e5cb, "d", "--- Delayed start timed out, status %d", lStatus);
            p_tone_gen->mState = TONE_IDLE;
        }
    }
    mutex_unlock(p_tone_gen->mLock);

    if (!lResult)
        ALOGV("Tone started, time %d", (systemTime() / 1000000));
    else
        AIDXW(0x065f59, "d", "Tone start failed, time %d", (systemTime() / 1000000));

    return lResult;
}

static int tone_stop(u32 obj)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    AIDXD(0x031c2b, "", "tone_stop");

    mutex_lock(p_tone_gen->mLock);
    if (p_tone_gen->mState != TONE_IDLE && p_tone_gen->mState != TONE_INIT) {
        if (p_tone_gen->mState == TONE_PLAYING ||
            p_tone_gen->mState == TONE_STARTING ||
            p_tone_gen->mState == TONE_RESTARTING) {

            struct timespec stopTime;
            /**
             * If the start time is valid, make sure that the number of audio
             * samples produced corresponds at least to the time between the
             * start and stop commands.
             * This is needed in case of cold start of the output stream.
             */
            if (p_tone_gen->mStartTime.tv_sec != 0 &&
                    tone_gettime(&stopTime) == 0) {
                time_t sec = stopTime.tv_sec - p_tone_gen->mStartTime.tv_sec;
                long nsec = stopTime.tv_nsec - p_tone_gen->mStartTime.tv_nsec;
                if (nsec < 0) {
                    --sec;
                    nsec += 1000000000;
                }

                if ((sec + 1) > ((long)(INT_MAX / p_tone_gen->mSamplingRate))) {
                    p_tone_gen->mMaxSmp = sec * p_tone_gen->mSamplingRate;
                } else {
                    /**
                     * duration in milliseconds, and
                     * mSamplingRate is always > 1000
                     */
                    sec = sec * 1000 + nsec / 1000000;
                    p_tone_gen->mMaxSmp = (unsigned int)
                        (((int64_t)sec * p_tone_gen->mSamplingRate) / 1000);
                }
                AIDXD(0x0ffa2b, "dd", "tone_stop() forcing mMaxSmp to %d, total for far %d",
                      p_tone_gen->mMaxSmp, p_tone_gen->mTotalSmp);
            } else {
                p_tone_gen->mState = TONE_STOPPING;
            }
        }
        AIDXD(0x0b95c7, "", "waiting cond");
        /* mWaitCbkCond.waitRelative(mLock, seconds(3)); */
        status_t lStatus = AOS_ERROR;//tone_wait(EVENT_DONE, 3000);
        if (lStatus == AOS_SUCCESS) {
            /**
             * If the tone was restarted exit now
             * before calling clearWaveGens();
             */
            /**
            if (p_tone_gen->mState != TONE_INIT) {
                mutex_unlock(p_tone_gen->mLock);
                return 0;
            }
            */
            AIDXD(0x0d8754, "d", "track stop complete, time %d", systemTime() / 1000000);
        } else {
            AIDXE(0x0ad8d8, "", "--- Stop timed out");
            p_tone_gen->mState = TONE_IDLE;
        }
        clearWaveGens();
    }

    mutex_unlock(p_tone_gen->mLock);

    AEXIT;

    return 0;
}

/* do not support multi-instance */
struct audio_client asink_tone = {
    .type = ASTREAM_TYPE_TONE,
    .open = tone_open,
    .start = tone_start,
    .stop = tone_stop,
    .cb = audioCallback
};

static int tone_init(void)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    AIDXD(0x1b86b5, "", "tone_init");

    p_tone_gen->mState = TONE_IDLE;
    p_tone_gen->mVolume = 1.0;
    p_tone_gen->mpToneDesc = NULL;
    p_tone_gen->mpNewToneDesc = NULL;
    p_tone_gen->mSamplingRate = 16000;
    p_tone_gen->sToneDescriptors =
        (struct ToneDescriptor *)&gToneDescriptors[0];

    /* Generate tone by chunks of 20 ms to keep cadencing precision */
    p_tone_gen->mProcessSize = (p_tone_gen->mSamplingRate * 20) / 1000;

    /* set GB default, should get region from some system configure */
    p_tone_gen->mRegion = TONE_REGION_GB;

    wgen_key_init();

    /* create only once */
    p_tone_gen->mEvent = aos_create_event("tone event");
    p_tone_gen->mLock = aos_create_mutex("tone mutex");

    if (init_audio_stream()) {
        AIDXD(0x017860, "d", "ToneGenerator INIT OK, time: %d",
              (unsigned int)(systemTime() / 1000000));
        audio_client_register(&asink_tone);
    } else {
        AIDXE(0x0518c2, "", "!!!ToneGenerator INIT FAILED!!!");
    }

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(tone_init);

int audio_set_tone_region(u32 region_id)
{
    struct ToneGenerator *p_tone_gen = get_tone_gen();

    if (region_id < TONE_REGION_MAX) {
        p_tone_gen->mRegion = region_id;
        return 0;
    }

    return AIC_EPERM;
}

