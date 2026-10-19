/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA BITSTREAM Interface.
 */

#ifndef __AOS_BITSTREAM_H__
#define __AOS_BITSTREAM_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_osa.h>
#include <aos_bitop.h>

/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/
#define AUTO_TRUE    1
#define AUTO_FALSE   0

/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/
typedef struct bit_stream_st {
    u8    *byte_stream; /* Pointer to underlying byte stream. */
    u8    *byte_stream_start;
    u32   offset;       /* Offset in bits from MSB of byte_stream */
    #ifdef REAL_DISTANCE
    u32   offset_start; /* the init Offset in bits from MSB of byte_stream */
    #endif
} bit_stream;


/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/

/* Initialise Bit stream operations
 *
 *                                offset
 *                                |
 *                                V
 *               /-------------------------------\
 * byte_stream-->|   |   |   |   |   |   |   |   |
 *               |---|---|---|---|---|---|---|---|
 *               |   |   |   |   |   |   |   |   |
 *               |---|---|---|---|---|---|---|---|
 *               |   |   |   |   |   |   |   |   |
 *               |---|---|---|---|---|---|---|---|
 *               | . | . | . | . | . | . | . | . |
 *                 .   .   .   .   .   .   .   .
 *                 .   .   .   .   .   .   .   .
 *
 *  If offset is more than eight then, byte_stream is incremented by
 *  offset/8 and offset is taken to be offset modulo 8.
 */
void bit_stream_init(bit_stream *bs,
                     u8 *byte_stream,
                     u32 offset);

/* Read specified number of bits from current position and return it as
 *  a u32
 *
 *                                        offset (before)
 *                                        |
 *                                        V
 *                       /-------------------------------\
 * byte_stream (before)->|   |   |   |   | 1 | 0 | 0 | 0 |
 *                       |---|---|---|---|---|---|---|---|
 * byte_stream (after) ->| 1 | 0 |   |   |   |   |   |   |
 *                       |---|---|---|---|---|---|---|---|
 *                       | . | . | ^ | . | . | . | . | . |
 *                                 |
 *                                 offset (after)
 *
 *      On the above stream bit_stream_read(bs, 6) will return 34
 */
u32 bit_stream_read(bit_stream *bs,
                    u32 width);

/* Write specified number of bits from current position into the
 * underlying byte stream.
 *
 *              bit_stream_write(bs, 6)
 *
 *                                        offset (before)
 *                                        |
 *                                        V
 *                       /-------------------------------\
 * byte_stream (before)->|   |   |   |   | 1 | 0 | 0 | 0 |
 *                       |---|---|---|---|---|---|---|---|
 * byte_stream (after) ->| 1 | 0 |   |   |   |   |   |   |
 *                       |---|---|---|---|---|---|---|---|
 *                       | . | . | ^ | . | . | . | . | . |
 *                                 |
 *                                 offset (after)
 *
 */
void bit_stream_write(bit_stream *bs,
                      u32 width,
                      u32 value );

/* Return the number of bits the internal pointer has moved since
 * bit_stream_init */
u32 bit_stream_cur_pos(bit_stream *bs);

/* Move pointer back by given number of bits */
void bit_stream_move_back(bit_stream *bs,
                          u32 bits);

void bit_stream_move_fwd(bit_stream *bs,
                         u32 bits );

/* Copy bits from one byte stream to another.
 * Byte stream is incremented as read operation
 * is done */

void byte_stream_bit_copy(u8 *s,
                          u32 s_offset,
                          u32 length,
                          u8 *d,
                          u32 d_offset);

void bit2byte_stream_copy(u8 *s,
                          u32 s_offset,
                          u32 length,
                          u8 *d
                          );

void byte2bit_stream_copy(u8 *s,
                          u32 length,
                          u8 *d,
                          u32 d_offset);

/* Read specified number of bits from the bitstream, starting
 * from a position, width bits before the current position.

 *
 *                                        offset (after)
 *                                        |
 *                                        V
 *                       /-------------------------------\
 * byte_stream (after)-> |   |   |   |   | 1 | 0 | 0 | 0 |
 *                       |---|---|---|---|---|---|---|---|
 * byte_stream (before)->| 1 | 0 |   |   |   |   |   |   |
 *                       |---|---|---|---|---|---|---|---|
 *                       | . | . | ^ | . | . | . | . | . |
 *                                 |
 *                                 offset (before)
 *
 *      On the above stream ,
 *         bit_stream_read_back(bs, 6) will return 34
 */

u32 bit_stream_read_back(bit_stream *bs,
                         u32 width);

/* Write specified number of bits to the bitstream starting
 * from a position width bits before the current position.
 *
 *     bit_stream_write_back(bs, 6, 34) will return
 *
 *                                        offset (after)
 *                                        |
 *                                        V
 *                       /-------------------------------\
 * byte_stream (after) ->|   |   |   |   | 1 | 0 | 0 | 0 |
 *                       |---|---|---|---|---|---|---|---|
 * byte_stream (before)->| 1 | 0 |   |   |   |   |   |   |
 *                       |---|---|---|---|---|---|---|---|
 *                       | . | . | ^ | . | . | . | . | . |
 *                                 |
 *                                 offset (before)
 *
 */
void bit_stream_write_back(bit_stream *bs,
                           u32 width,
                           u32 value );

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif/* __AOS_BITSTREAM_H__ */

