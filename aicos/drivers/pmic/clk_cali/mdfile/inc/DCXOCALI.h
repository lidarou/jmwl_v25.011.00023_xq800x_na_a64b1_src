#ifndef __DCXOCALI_H__
#define __DCXOCALI_H__

#pragma pack(push,1)

typedef struct _AFCCAL{
    unsigned short CalFreq;    /* 1 Bit:100KHz */
    unsigned short CT;    /* Coarse tune */
    unsigned short AFCCal;    /* afc cali */
    unsigned short HPM_32K;    /* div_dcoeff_hp */
    unsigned short LPM_32K;    /* div_dcoeff_lp */
}AFCCAL;

typedef struct _DCXO_CALI{
    AFCCAL afccal;
    unsigned short AfcCalFlag;    /* Cal=1,default=0 */
    unsigned short CalToolVersion;    /* ToolVersion */
    unsigned char Ct_Lp;    /* ct_lp */
    unsigned char reserved[1];    /* 4byte alignment */
}DCXO_CALI;

#define DCXOCALI_STRUCT_SIZE    16

#pragma pack(pop)

#endif /* __DCXOCALI_H__ */
