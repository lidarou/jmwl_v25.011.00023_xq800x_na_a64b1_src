#ifndef __IMEI1_H__
#define __IMEI1_H__

#pragma pack(push,1)

typedef struct _IMEI_T{
    unsigned char IMEI[8];    /* imei */
    unsigned char SVN;    /* software version number, for IMEISV */
    unsigned char reserved[3];    /* 4byte alignment */
}IMEI_T;

#define IMEI1_STRUCT_SIZE    12

#pragma pack(pop)

#endif /* __IMEI1_H__ */
