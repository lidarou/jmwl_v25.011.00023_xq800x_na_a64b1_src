#ifndef __CM_AT_LBS_H__
#define __CM_AT_LBS_H__

#pragma pack(push,1)

typedef struct _LBS_T{
    unsigned char METHOD;    /* METHOD */
    unsigned char PRECISION;    /* PRECISION */
    unsigned char NEARBTSEN;    /* NEARBTSEN */
    unsigned char COORDINATE;    /* COORDINATE */
    unsigned char PID[20];    /* PID */
}LBS_T;


#define CM_AT_LBS_STRUCT_SIZE    24

#pragma pack(pop)

#endif /* __CM_AT_LBS_H__ */
