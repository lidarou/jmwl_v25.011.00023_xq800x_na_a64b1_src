/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA BITOP Interface.
 */

#ifndef __AOS_BITOP_H__
#define __AOS_BITOP_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
    extern "C" {
#endif

#include <aic_osa.h>

/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/
#define MASK(_w) (((_w)>31) ? 0xffffffff : (0x1u << (_w)) - 1 )

/* _uc is the pointer to the byte stream from which bits to be read */
/* _s is the offset in bits from MSB of the byte stream _uc  */
/* _w is the number of bits to be read  */

#define GET_BITS_1_8(_uc, _s, _w) /*lint -e506*/(                    \
    (((_s)+(_w)>8) ?( *(_uc)) << ((_s)+(_w)-8) |                     \
     (*((_uc)+1)) >> (16-(_s)-(_w)) :                                \
    ( ( *(_uc)) << (_s)) >> (8-(_w)))                                \
    & MASK(_w) )

#define GET_BITS_9_16(_uc, _s, _w) (                                 \
   ((_s+_w>16) ? *_uc << (_s+_w-8) | *(_uc+1) << (_s+_w-16) |        \
    *(_uc+2) >> (24-_s-_w) :                                         \
        *_uc << (_s+_w-8) | *(_uc+1) >> (16-_s-_w))                  \
    & MASK(_w))

#define GET_BITS_17_24(_uc, _s, _w) (                                \
    (((_s)+(_w)>24) ? *(_uc) << ((_s)+(_w)-8) |                      \
     *((_uc)+1) << ((_s)+(_w)-16) |                                  \
     *((_uc)+2) << ((_s)+(_w)-24) | *((_uc)+3) >> (32-(_s)-(_w)) :   \
        *(_uc) << ((_s)+(_w)-8) | *((_uc)+1) << ((_s)+(_w)-16) |     \
        *((_uc)+2) >> (24-(_s)-(_w)))                                \
    & MASK(_w))

#define GET_BITS_25_32(_uc, _s, _w) (                                \
    (((_s)+(_w)>32) ? *(_uc) << ((_s)+(_w)-8) |                      \
     *((_uc)+1) << ((_s)+(_w)-16) |                                  \
     *((_uc)+2) << ((_s)+(_w)-24) | *((_uc)+3) << ((_s)+(_w)-32) |   \
     *((_uc)+4) >> (40-(_s)-(_w)) : *(_uc) << ((_s)+(_w)-8) |        \
     *((_uc)+1) << ((_s)+(_w)-16) | *((_uc)+2) << ((_s)+(_w)-24) |   \
     *((_uc)+3) >> (32-(_s)-(_w))) & MASK(_w))

#define GET_BITS(_uc, _s, _w) /*lint -e506*/(                        \
    ((_w)>8) ? (((_w)>16) ? (((_w)>24) ?                             \
        GET_BITS_25_32((_uc), (_s), (_w)) :                          \
        GET_BITS_17_24((_uc), (_s), (_w))) :                         \
        GET_BITS_9_16((_uc), (_s), (_w))) :                          \
        GET_BITS_1_8((_uc), (_s), (_w))                              \
    )

/* _uc is the pointer to the byte stream to which bits to be written */
/* _s is the offset in bits from MSB of the byte stream _uc  */
/* _w is the number of bits to be written  */
/* _i is the value to be written  */

#define PUT_BITS_1_8(_uc, _s, _w, _i)                                \
    if ((_s)+(_w)>8) {                                               \
        *(_uc)     &= ~ MASK(8-(_s));                                \
        *(_uc)     |= ((_i) & MASK((_w))) >> ((_s)+(_w)- 8);         \
        *((_uc)+1)   = (*((_uc)+1) & MASK(16-(_s)-(_w))) |           \
        (_i) << (16-(_s)-(_w));                                      \
    } else {                                                         \
        *(_uc)     &= ~ (MASK((_w)) << ( 8-(_w)-(_s)));              \
        *(_uc)     |= ((_i) & MASK((_w))) << ( 8-(_w)-(_s));         \
    }

#define PUT_BITS_9_16(_uc, _s, _w, _i)                               \
    *(_uc) &= ~ MASK(8-(_s));                                        \
    if ((_s)+(_w)>16) {                                              \
        *(_uc)     |= ((_i) & MASK((_w))) >> ((_s)+(_w)- 8);         \
        *((_uc)+1)   = (_i) >> ((_s)+(_w)-16);                       \
        *((_uc)+2)   = (*((_uc)+2) & MASK(24-(_s)-(_w))) |           \
        (_i) << (24-(_s)-(_w));                                      \
    } else {                                                         \
        *(_uc)     |= ((_i) & MASK((_w))) >> ((_s)+(_w)- 8);         \
        *((_uc)+1)   = (*((_uc)+1) & MASK(16-(_s)-(_w))) |           \
        (_i) << (16-(_s)-(_w));                                      \
    }

#define PUT_BITS_17_24(_uc, _s, _w, _i)                              \
    *(_uc) &= ~ MASK(8-(_s));                                        \
    if ((_s)+(_w)>24) {                                              \
        *(_uc)     |= ((_i) & MASK((_w))) >> ((_s)+(_w)- 8);         \
        *((_uc)+1)   = (_i) >> ((_s)+(_w)-16);                       \
        *((_uc)+2)   = (_i) >> ((_s)+(_w)-24);                       \
        *((_uc)+3)   = (*((_uc)+3) & MASK(32-(_s)-(_w))) |           \
        (_i) << (32-(_s)-(_w));                                      \
    } else {                                                         \
        *(_uc)     |= ((_i) & MASK((_w))) >> ((_s)+(_w)- 8);         \
        *((_uc)+1)   = (_i) >> ((_s)+(_w)-16);                       \
        *((_uc)+2)   = (*((_uc)+2) & MASK(24-(_s)-(_w))) |           \
        (_i) << (24-(_s)-(_w));                                      \
    }

#define PUT_BITS_25_32(_uc, _s, _w, _i)                              \
    *(_uc) &= ~MASK(8-(_s));                                         \
    if ((_s)+(_w)>32) {                                              \
        *(_uc)     |= ((_i) & MASK((_w))) >> ((_s)+(_w)- 8);         \
        *((_uc)+1)   = (_i) >> ((_s)+(_w)-16);                       \
        *((_uc)+2)   = (_i) >> ((_s)+(_w)-24);                       \
        *((_uc)+3)   = (_i) >> ((_s)+(_w)-32);                       \
        *((_uc)+4)   = (*((_uc)+4) & MASK(40-(_s)-(_w))) |           \
        (_i) << (40-(_s)-(_w));                                      \
    } else {                                                         \
        *(_uc)     |= ((_i) & MASK((_w))) >> ((_s)+(_w)- 8);         \
        *((_uc)+1)   = (_i) >> ((_s)+(_w)-16);                       \
        *((_uc)+2)   = (_i) >> ((_s)+(_w)-24);                       \
        *((_uc)+3)   = (*((_uc)+3) & MASK(32-(_s)-(_w))) |           \
        (_i) << (32-(_s)-(_w));                                      \
    }

#define PUT_BITS(_uc, _s, _w, _i)                                    \
    if ((_w)<9) {                                                    \
        PUT_BITS_1_8(_uc, _s, _w, _i)                                \
    } else if ((_w)<17) {                                            \
        PUT_BITS_9_16(_uc, _s, _w, _i)                               \
    } else if ((_w)<25) {                                            \
        PUT_BITS_17_24(_uc, _s, _w, _i)                              \
    } else {                                                         \
        PUT_BITS_25_32(_uc, _s, _w, _i)                              \
    }

/********************************************************************/
/* This macro insert a given number of bits to a bit position
 * by shifting the existing bits down. */
/* ptr is the pointer to the byte stream
 * to which bits to be inserted.         */
/* _offset is the offset in bytes from MSB of byte stream ptr.  */
/* length is the number of bits to be inserted.                 */
/* _size is the number of bits in the bit stream, after the point
 * of insertion ,which is to be shifted. */
/* _value is the value to be inserted.   */

#define INSERT_BITS(ptr,_offset,length,_size,_value)                 \
{                                                                    \
    u32 temp,value =_value;                                       \
    u16 offset=_offset,size=_size;                                \
    while(length <= size) {                                          \
        temp = GET_BITS(ptr,offset,length);                          \
        PUT_BITS(ptr,offset,length,value);                           \
        ptr = ptr +((offset+length)/8);                              \
        offset = (offset+length)%8;                                  \
        size -=length;                                               \
        value = temp;                                                \
    }                                                                \
    temp = GET_BITS(ptr,offset,size);                                \
    PUT_BITS(ptr,offset,length,value);                               \
    ptr = ptr +(offset+length)/8;                                    \
    offset = (offset+length)%8;                                      \
    PUT_BITS(ptr,offset,size,temp);                                  \
}

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/
typedef struct pack_info_struct {
    u8      *frame;
    u32     bits;
} pack_info_struct;

typedef struct unpack_info_struct {
    u8      *frame;
    u32     bits;
    s32     fr_size;
    void    *err_hndl;
} unpack_info_struct;


/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/
/*calculate the mask with width '1's;*/
u32 mask(u32 width);

/*This function is used to get bits form pack,bit width:1~8*/
u32 gen_get_bits_1_8(
                          unpack_info_struct *unpack_info,
                          u32        width
                          );

/*This function is get bits from bit string,bit width:1~8*/
u32 get_bits_1_8(
                          u8    *addr,
                          u32 offset,
                          u32 width
                          );

/*This function is used to get bits form pack,bit width:9~16*/
u32 gen_get_bits_9_16(
                          unpack_info_struct *unpack_info,
                          u32        width
                          );

/*This function is get bits from bit string,bit width:9~16*/
extern u32 get_bits_9_16(
                           u8    *addr,
                           u32 offset,
                           u32 width
                           );

/*This function is used to get bits form pack,bit width:17~24*/
u32 gen_get_bits_17_24(
                          unpack_info_struct *unpack_info,
                          u32        width
                          );

/*This function is get bits from bit string,bit width:17~24*/
u32 get_bits_17_24(
                            u8    *addr,
                            u32 offset,
                            u32 width
                            );

/*This function is used to get bits form pack,bit width:25~32*/
u32 gen_get_bits_25_32(
                          unpack_info_struct *unpack_info,
                          u32        width
                          );

/*This function is get bits from bit string,bit width:25~32*/
u32 get_bits_25_32(
                            u8    *addr,
                            u32 offset,
                            u32 width
                            );

/*This function is used to get bits form pack,bit width:1~32*/
u32 gen_get_bits(
                          unpack_info_struct *unpack_info,
                          u32        width
                          );

/*This function is get bits from bit string,bit width:1~32*/
u32 get_bits(
                      u8    *addr,
                      u32 offset,
                      u32 width
                      );

/*This function is used to put bits to the package,bit width:1~8*/
void gen_put_bits_1_8(
                  pack_info_struct *pack_info,
                  u32      width,
                  u32      put_val
                  );

/*This function is used to put bits to the byte stream,bit width:1~8*/
void put_bits_1_8(
                  u8    *addr,
                  u32 offset,
                  u32 width,
                  u32 put_val
                  );

/*This function is used to put bits to the package,bit width:9~16*/
void gen_put_bits_9_16(
                  pack_info_struct *pack_info,
                  u32      width,
                  u32      put_val
                  );

/*This function is used to put bits to the byte stream,bit width:9~16*/
void put_bits_9_16(
                   u8    *addr,
                   u32 offset,
                   u32 width,
                   u32 put_val
                   );

/*This function is used to put bits to the package,bit width:17~24*/
void gen_put_bits_17_24(
                  pack_info_struct *pack_info,
                  u32      width,
                  u32      put_val
                  );

/*This function is used to put bits to the byte stream,bit width:17~24*/
void put_bits_17_24(
                   u8    *addr,
                   u32 offset,
                   u32 width,
                   u32 put_val
                   );

/*This function is used to put bits to the package,bit width:25~32*/
void gen_put_bits_25_32(
                  pack_info_struct *pack_info,
                  u32      width,
                  u32      put_val
                  );

/*This function is used to put bits to the byte stream,bit width:25~32*/
void put_bits_25_32(
                   u8    *addr,
                   u32 offset,
                   u32 width,
                   u32 put_val
                   );

/*This function is used to put bits to the package,bit width:1~32*/
void gen_put_bits(
                  pack_info_struct *pack_info,
                  u32      width,
                  u32      put_val
                  );

/*This function is used to put bits to the byte stream,bit width:1~32*/
void put_bits(
              u8    *addr,
              u32 offset,
              u32 width,
              u32 put_val
              );

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AOS_BITOP_H__ */

