/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA BITSTREAM function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d1a0c
#define LOG_MODULE    AMOD_AOS

#include <aos_bitstream.h>

/*****************************************************************************
* Function: bit_stream_init
* Purpose:  sets the underlying byte stream and the
*           current read/write position within the byte stream. Can also
*           be called when a seek to a new position is desired.
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                bit_stream*         In/Out      A bit stream allows bit fields to be read and written to,
*                                                   just like an byte stream allows byte to be read and written
*                                                   to. Reading and writing of bit fields is done to an underlying
*                                                   byte stream, in network byte order.
* byte_stream       u8 *     In          byte stream, used to init the bit stream
* offset            u32        In          offset in bits of the bit stream
* Return:  None
* Note:
*******************************************************************************/
void bit_stream_init(bit_stream *bs, u8 *byte_stream, u32 offset)
{

    bs->byte_stream_start = byte_stream;
    bs->byte_stream = byte_stream + (offset >> 3);
    bs->offset = offset & 0x7;

#ifdef REAL_DISTANCE
    bs->offset_start = offset;
#endif

    return ;
}

/*****************************************************************************
* Function: bit_stream_read
* Purpose:  Read specified number of bits from current position and return it as
*           a u32
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                bit_stream *        In/Out       bit stream to read bit from
* width             u32               In         width of bit to read
*
* Return:  the value ream from the bit stream
* Note:    The bit_stream struct variable is maintained by stream struct utility
*          so do not need adjust offset value
*******************************************************************************/

u32 bit_stream_read(bit_stream *bs, u32 width)
{
    u32 value;

    value = get_bits (bs->byte_stream, bs->offset, width);
    bs->byte_stream += (bs->offset + width) / 8;
    bs->offset = (bs->offset + width) % 8;

    return value;
}

/*****************************************************************************
* Function: bit_stream_write
* Purpose:  Write specified number of bits from current position into the
*           underlying byte stream.
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                bit_stream *         Out          the bit stream write bits to
* width             u32               In           width of bits write to the bit stream
* value             u32               In           value put to the bit stream
*
* Return:  None
* Note:
*******************************************************************************/

void bit_stream_write(bit_stream *bs,u32 width,u32 value)
{
    put_bits(bs->byte_stream, bs->offset, width, value);
    bs->byte_stream += (bs->offset + width) / 8;
    bs->offset = (bs->offset + width) % 8;

    return;
}

/*****************************************************************************
* Function: bit_stream_cur_pos
* Purpose:  Return the number of bits the internal pointer has moved since
*           bit_stream_init
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                 bit_stream *        In          pointer to the bit stream
*
* Return:  the number of bits the internal pointer has moved since bit_stream_init
* Note:
*******************************************************************************/
u32 bit_stream_cur_pos(bit_stream *bs)
{
#ifdef REAL_DISTANCE
    if ((bs->offset>=bs->offset_start))
        return ((unsigned int)(((bs->byte_stream-bs->byte_stream_start) << 3) +
                bs->offset-bs->offset_start));
    else
        return ((unsigned int)(((bs->byte_stream-bs->byte_stream_start - 1) << 3) +
                bs->offset + 8 - bs->offset_start));
#else
    return ((unsigned int)(((bs->byte_stream-bs->byte_stream_start) << 3) +
            bs->offset));
#endif
}

/*****************************************************************************
* Function: bit_stream_move_back
* Purpose:  Move pointer back by given number of bits
*
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                 bit_stream *        In          pointer to the bit stream
* bits               u32              In          number of bits to move back
*
* Return:  None
* Note:
*******************************************************************************/
void bit_stream_move_back(bit_stream *bs, u32 bits)
{
    bs->byte_stream -= (bits>>3);
    bits &= 0x7;

    if (bits > bs->offset) {
        bs->offset += 8;
        bs->byte_stream--;
    }
    bs->offset -= bits;

    return ;
}

/*****************************************************************************
* Function: bit_stream_move_fwd
* Purpose:  Move pointer forward by given number of bits
*
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                 bit_stream *        In          pointer to the bit stream
* bits               u32        In          number of bits to move forward
*
* Return:  None
* Note:
*******************************************************************************/
void bit_stream_move_fwd(bit_stream *bs, u32 bits)
{
    bs->byte_stream += ((bits+bs->offset) >> 3);
    bs->offset = (bits+bs->offset) & 0x7;

    return ;
}

/*****************************************************************************
* Function: byte_stream_bit_copy
* Purpose:  Copy bits from one byte stream to another.
*           Byte stream is incremented as read operation is done
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* s                    u8 *          In          source byte stream
* s_offset             u32           In          bit head offset the first byte
* length               u32           In          length of bits to copy
* d                    u8 *          Out         dest byts stream
* d_offset             u32           In          bit head offset the first byte
*
* Return:  None
* Note:    The input offset may exceed 8 bits,so this function need adjust the value
*          and the copy width may exceed than 32 bits
*******************************************************************************/

void byte_stream_bit_copy(u8 *s, u32 s_offset, u32 length, u8 *d, u32 d_offset)
{
    u32 value_tmp;
    u8 *src_tmp;
    u32 s_offset_tmp;
    u8 *dst_tmp;
    u32 d_offset_tmp;

    /* adjust input value */
    src_tmp = s + (s_offset / 8);
    s_offset_tmp = s_offset % 8;

    dst_tmp = d + (d_offset / 8);
    d_offset_tmp = d_offset % 8;

    while (length > 32) {
        /* read data from src */
        value_tmp = get_bits(src_tmp, s_offset_tmp, 32);
        src_tmp += 4;

        put_bits(dst_tmp, d_offset_tmp, 32, value_tmp);
        dst_tmp += 4;

        length -= 32;
    }

    if (length != 0) {
        value_tmp = get_bits(src_tmp, s_offset_tmp, length);
        put_bits(dst_tmp, d_offset_tmp, length, value_tmp);
    }

    return;
}

/*****************************************************************************
* Function: bit2byte_stream_copy
* Purpose:  copy bit stream to other byte stream
*
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* s                     u8 *         In          source byte stream
* s_offset              u32          In          bit head offset the first byte
* length                u32          In          length of bits to copy
* d                     u8 *         Out         dest byts stream
*
* Return:  None
* Note:
*******************************************************************************/

void bit2byte_stream_copy(u8 *s, u32 s_offset, u32 length, u8 *d)
{
    u32 value_tmp;
    u8 *src_tmp;
    u32 s_offset_tmp;
    u8 *dst_tmp;

    /* adjust input value */
    src_tmp = s + (s_offset / 8);
    s_offset_tmp = s_offset % 8;
    dst_tmp = d;

    while (length > 32) {
        value_tmp = get_bits(src_tmp, s_offset_tmp, 32);
        src_tmp += 4;

        put_bits(dst_tmp, 0, 32, value_tmp);
        dst_tmp += 4;
        length -= 32;
    }

    if (length != 0) {
        value_tmp = get_bits(src_tmp, s_offset_tmp, length);
        put_bits(dst_tmp, 0, length, value_tmp);
    }
}

/*****************************************************************************
* Function: byte2bit_stream_copy
* Purpose:  copy byte stream to bit stream
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* s                 u8 *             In          source byte stream
* s_offset          u32              In          bit head offset the first byte
* length            u32              In          length of bits to copy
* d                 u8 *             Out         dest byts stream
*
* Return:  None
* Note:
*******************************************************************************/

void byte2bit_stream_copy(u8 *s, u32 length, u8 *d, u32 d_offset)
{
    u32 value_tmp;
    u8 *src_tmp;
    u32 d_offset_tmp;
    u8 *dst_tmp;

    src_tmp = s;
    dst_tmp = d + (d_offset / 8);
    d_offset_tmp = d_offset % 8;

    while (length > 32) {
        value_tmp = get_bits(src_tmp, 0, 32);
        src_tmp += 4;

        put_bits(dst_tmp, d_offset_tmp, 32, value_tmp);
        dst_tmp += 4;
        length -= 32;
    }

    if (length != 0) {
        value_tmp = get_bits(src_tmp, 0, length);
        put_bits(dst_tmp, d_offset_tmp, length, value_tmp);
    }
}

/*****************************************************************************
* Function: bit_stream_read_back
* Purpose:  Read specified number of bits from the bitstream, starting
*           from a position, width bits before the current position.
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                 bit_stream *       In/Out      pointer to the bit stream
* width              u32             In         width of bits to read
*
* Return:  the value read from the bit stream
* Note:
*******************************************************************************/

u32 bit_stream_read_back(bit_stream *bs, u32 width)
{
    u32 val;
    bit_stream_move_back(bs, width);
    val = bit_stream_read(bs, width);
    bit_stream_move_back(bs, width);

    return val;
}

/*****************************************************************************
* Function: bit_stream_write_back
* Purpose:  Write specified number of bits to the bitstream starting
*           from a position width bits before the current position.
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* bs                 bit_stream *       In/Out      pointer to the bit stream
* width              u32             In         width of bits to read
* value              u32             In         value put to the bit stream
*
* Return:  None
* Note:
*******************************************************************************/

void bit_stream_write_back(bit_stream *bs, u32 width, u32 value)
{
    bit_stream_move_back(bs, width);
    bit_stream_write(bs, width, value);
    bit_stream_move_back(bs, width);
}

