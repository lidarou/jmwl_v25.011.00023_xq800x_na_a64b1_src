#ifndef __D2D_H__
#define __D2D_H__

#pragma pack(push,1)

typedef struct _D2D_T{
    unsigned int d2d_id;    /* d2d id */
    unsigned char pbch_freq_group;    /* PBCH freq group, only for debug Use */
    unsigned char bch_report_period;    /* D2D broadcast report period, unit is second */
    unsigned char Tout;    /* L1 out of sync param, unit is second */
    unsigned short tx_power;    /* L1 tx power, unit is 1/10 dBm */
    unsigned short max_tx_power;    /* L1 max tx power, unit is 1/10 dBm */
    unsigned char reserved[1];    /* 4byte alignment */
}D2D_T;

#define D2D_STRUCT_SIZE    12

#pragma pack(pop)

#endif /* __D2D_H__ */
