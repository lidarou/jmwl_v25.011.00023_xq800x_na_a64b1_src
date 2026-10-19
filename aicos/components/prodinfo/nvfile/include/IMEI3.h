#ifndef __IMEI3_H__
#define __IMEI3_H__

#pragma pack(push,1)

typedef struct _IMEI_T{
    unsigned char IMEI[8];    /* imei */
}IMEI_T;


#define IMEI3_STRUCT_SIZE    8

#pragma pack(pop)

#endif /* __IMEI3_H__ */
