#ifndef __VERSION_H__
#define __VERSION_H__

#pragma pack(push,1)

typedef struct _VERSION{
    unsigned char sys[2];    /* sys */
    unsigned char base[48];    /* base version */
    unsigned char time[32];    /* compiler time */
    unsigned char diff;    /* diff flag */
    unsigned char reserved[1];    /* 4byte alignment */
}VERSION;

#define VERSION_STRUCT_SIZE    84

#pragma pack(pop)

#endif /* __VERSION_H__ */
