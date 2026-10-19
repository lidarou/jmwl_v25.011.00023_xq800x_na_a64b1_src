#ifndef __NV_PARAM_TYPE_SETTINGS_DEMO_H__
#define __NV_PARAM_TYPE_SETTINGS_DEMO_H__

#pragma pack(push,1)

typedef struct _STRUCT{
    unsigned char item1;    /* item1 */
    unsigned short item2;    /* item2 */
    unsigned int item3;    /* item3 */
}STRUCT;

typedef struct _NV_PARAM_TYPE_T{
    unsigned char string[6];    /* string, strlen(string) bytes */
    unsigned char string_length[50];    /* string use fixed length, fixed length bytes */
    unsigned char unsigned_char;    /* unsigned char 1byte */
    unsigned char hex_unsigned_char;    /* unsigned char 1byte */
    char char;    /* char 1byte */
    signed char signed_char;    /* signed char 1byte */
    signed char negative_signed_char;    /* negative signed char 1byte */
    unsigned short unsigned_short;    /* unsigned short 2byte */
    unsigned short hex_unsigned_short;    /* unsigned short 2byte */
    short short;    /* short 2byte */
    short negative_short;    /* negative short 2byte */
    unsigned int unsigned_int;    /* unsigned int 4byte */
    unsigned int hex_unsigned_int;    /* unsigned int 4byte */
    int int;    /* int 4byte */
    int negative_int;    /* negative int 4byte */
    bool bool;    /* bool 1byte */
    long long;    /* long 4byte */
    long negative_long;    /* negative long 4byte */
    unsigned long unsigned_long;    /* unsigned long 4byte */
    long long long_long;    /* long long 8byte */
    long long negative_long_long;    /* negative long long 8byte */
    unsigned long long unsigned_long_long;    /* unsigned long long 8byte */
    float float;    /* float 4byte */
    double double;    /* double 8byte */
    char[] char[];    /* char[] not support, please use string. */
    STRUCT struct;
    signed char array[3];    /* array */
    unsigned char reserved[1];    /* 4byte alignment */
}NV_PARAM_TYPE_T;

#define NV_PARAM_TYPE_SETTINGS_DEMO_STRUCT_SIZE    156

#pragma pack(pop)

#endif /* __NV_PARAM_TYPE_SETTINGS_DEMO_H__ */
