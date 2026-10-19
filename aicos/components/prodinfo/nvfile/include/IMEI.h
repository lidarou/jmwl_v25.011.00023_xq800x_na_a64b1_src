#ifndef __IMEI_H__
#define __IMEI_H__

#pragma pack(push,1)

typedef struct _IMEI_T{
    unsigned char IMEI[8];    /* imei */
    unsigned char SVN;    /* software version number, for IMEISV */
    unsigned char reserved[3];    /* 4byte alignment */
}IMEI_T;

#define IMEI_STRUCT_SIZE    12

#pragma pack(pop)

#endif /* __IMEI_H__ */
