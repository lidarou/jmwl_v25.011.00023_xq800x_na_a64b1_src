#ifndef __IMEI2_H__
#define __IMEI2_H__

#pragma pack(push,1)

typedef struct _IMEI_T{
    unsigned char IMEI[8];    /* imei */
}IMEI_T;


#define IMEI2_STRUCT_SIZE    8

#pragma pack(pop)

#endif /* __IMEI2_H__ */
