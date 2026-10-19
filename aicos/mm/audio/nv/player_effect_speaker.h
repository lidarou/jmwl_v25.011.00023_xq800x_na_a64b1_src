#ifndef __PLAYER_EFFECT_SPEAKER_H__
#define __PLAYER_EFFECT_SPEAKER_H__

#pragma pack(push,1)

typedef struct _SAMPLE_RATE{
    int sample_rate;    /* sample rate */
}SAMPLE_RATE;

typedef struct _DRC_EFFECT{
    short thre;    /* raw threashold */
    float ratio;    /* raw threashold */
    float attack_t;    /* attack time */
    float release_t;    /* release time */
}DRC_EFFECT;

typedef struct _HPF_EFFECT{
    short cutoff;    /* cutoff freq */
}HPF_EFFECT;

typedef struct _BQD1_EFFECT{
    short f_center;    /* frequency of center */
    short Gain;    /* the gain for the frequency of center */
    short Q;    /* the Quarlity for the frequency of center */
}BQD1_EFFECT;

typedef struct _BQD2_EFFECT{
    short f_center;    /* frequency of center */
    short Gain;    /* the gain for the frequency of center */
    short Q;    /* the Quarlity for the frequency of center */
}BQD2_EFFECT;

typedef struct _BQD3_EFFECT{
    short f_center;    /* frequency of center */
    short Gain;    /* the gain for the frequency of center */
    short Q;    /* the Quarlity for the frequency of center */
}BQD3_EFFECT;

typedef struct _DRC_PARAMETER{
    short threashold;    /* threashold */
    short Q16_threashold;    /* Q16_threashold */
    short ratio;    /* ratio */
    short alpha;    /* alpha */
    short beta;    /* beta */
}DRC_PARAMETER;

typedef struct _HPF_PARAMETER{
    short a1;    /* a1 */
    short a2;    /* a2 */
    short b0;    /* b0 */
    short b1;    /* b1 */
    short b2;    /* b2 */
}HPF_PARAMETER;

typedef struct _BQD1_PARAMETER{
    short bqd_type;    /* 0:L-shelf;1:H_shelf;2:Boost; 3:Notch */
    short a1;    /* a1 */
    short a2;    /* a2 */
    short b0;    /* b0 */
    short b1;    /* b1 */
    short b2;    /* b2 */
}BQD1_PARAMETER;

typedef struct _BQD2_PARAMETER{
    short bqd_type;    /* 0:L-shelf;1:H_shelf;2:Boost; 3:Notch */
    short a1;    /* a1 */
    short a2;    /* a2 */
    short b0;    /* b0 */
    short b1;    /* b1 */
    short b2;    /* b2 */
}BQD2_PARAMETER;

typedef struct _BQD3_PARAMETER{
    short bqd_type;    /* 0:L-shelf;1:H_shelf;2:Boost; 3:Notch */
    short a1;    /* a1 */
    short a2;    /* a2 */
    short b0;    /* b0 */
    short b1;    /* b1 */
    short b2;    /* b2 */
}BQD3_PARAMETER;

typedef struct _PLAYER_PARA{
    SAMPLE_RATE sample_rate;
    DRC_EFFECT drc_effect;
    HPF_EFFECT hpf_effect;
    BQD1_EFFECT bqd1_effect;
    BQD2_EFFECT bqd2_effect;
    BQD3_EFFECT bqd3_effect;
}PLAYER_PARA;

typedef struct _PLAYER_EFFECT{
    DRC_PARAMETER drc_parameter;
    HPF_PARAMETER hpf_parameter;
    BQD1_PARAMETER bqd1_parameter;
    BQD2_PARAMETER bqd2_parameter;
    BQD3_PARAMETER bqd3_parameter;
    PLAYER_PARA player_para;
    unsigned char reserved[2];    /* 4byte alignment */
}PLAYER_EFFECT;

#define PLAYER_EFFECT_SPEAKER_STRUCT_SIZE    96

#pragma pack(pop)

#endif /* __PLAYER_EFFECT_SPEAKER_H__ */
