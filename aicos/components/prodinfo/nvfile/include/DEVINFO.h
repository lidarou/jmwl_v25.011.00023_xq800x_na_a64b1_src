#ifndef __DEVINFO_H__
#define __DEVINFO_H__

#pragma pack(push,1)

typedef struct _DEV_T{
    unsigned char DEV[56];    /* device info */
}DEV_T;


#define DEVINFO_STRUCT_SIZE    56

#pragma pack(pop)

#endif /* __DEVINFO_H__ */
