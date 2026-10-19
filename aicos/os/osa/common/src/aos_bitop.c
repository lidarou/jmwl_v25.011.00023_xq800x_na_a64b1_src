/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA BITOP function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x175334
#define LOG_MODULE    AMOD_AOS

#include "aos_bitop.h"

/*****************************************************************************
* Function: mask
* Purpose:  calculate the mask with width '1's;.
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* width                 u32         In             width of the mask
* Return:  the mask
* Note:    <the limitations to use this function or other comments>
*******************************************************************************/
u32 mask(u32 width)
{
    return MASK(width);
}

/*****************************************************************************
* Function: gen_get_bits_1_8
* Purpose:  This function is used to get bits form pack
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* unpack_info     unpack_info_struct *  In/Out        the pointer to unpack_info_struct
* width           u32                In            widht of the bit to get
*
* Return:  value that unpack from the pack
* Note:    width value range[1, 8]
*******************************************************************************/
u32 gen_get_bits_1_8(unpack_info_struct *unpack_info, u32 width)
{
    u32 get_value;

    /*check the input argument validity*/
    if ((width > 8) || unpack_info == NULL)
        return 0x00;

    /*calculate the left frame*/
    unpack_info->fr_size -= (unpack_info->bits%8 + width)/8;
    if (unpack_info->fr_size < 0) {
        /*there not enough bits to unpack*/
        unpack_info->fr_size += (unpack_info->bits%8 + width)/8;
        return 0x00;
    }

    get_value = GET_BITS_1_8(unpack_info->frame, unpack_info->bits%8, width);
    unpack_info->frame += (unpack_info->bits%8 + width)/8;
    unpack_info->bits += width;
    return get_value;
}

/*****************************************************************************
* Function: get_bits_1_8
* Purpose:  This function is get bits from bit string
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr                 u8*             In          the pointer to bit stream
* offset               u32             In          offset bits from addr,start bit to get value
* width                u32             In          width of bit that to get
* Return:  the value that get from the bit stream
* Note:    width range [1, 8]
*******************************************************************************/
u32 get_bits_1_8(u8 *addr, u32 offset, u32 width)
{
    /*check the input argument validity*/
    if ((width > 8) || (width == 0) || (addr == NULL) || (offset > 7))
        return 0x00;

    return GET_BITS_1_8(addr, offset, width);
}

/*****************************************************************************
* Function: gen_get_bits_9_16
* Purpose:  This function is used to get bits form pack
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* unpack_info     unpack_info_struct *  In/Out        the pointer to unpack_info_struct
* width           u32                In            widht of the bit to get
*
* Return:  value that unpack from the pack
* Note:    width value range[9, 16]
*******************************************************************************/
u32 gen_get_bits_9_16(unpack_info_struct *unpack_info, u32 width)
{
    u32 get_value;

    /*check the input argument validity*/
    if ((width > 16) || (width < 9) || unpack_info == NULL)
        return 0x00;

    /*reset the frame size*/
    unpack_info->fr_size -= (unpack_info->bits%8 + width)/8;
    if (unpack_info->fr_size < 0) {
        /*no more bits in this pack*/
        unpack_info->fr_size += (unpack_info->bits%8 + width)/8;
        return 0x00;
    }

    get_value = GET_BITS_9_16(unpack_info->frame, (unpack_info->bits%8), width);
    unpack_info->frame += (unpack_info->bits%8 + width)/8;
    unpack_info->bits += width;

    return get_value;
}

/*****************************************************************************
* Function: get_bits_9_16
* Purpose:  This function is get bits from bit string
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr                 u8*           In          the pointer to bit stream
* offset               u32           In          offset bits from addr,start bit to get value
* width                u32           In          width of bit that to get
* Return:  the value that get from the bit stream
* Note:    width range [9, 16]
*******************************************************************************/
u32 get_bits_9_16(u8 *addr, u32 offset, u32 width)
{
    /*check the input argument validity*/
    if ((width > 16) || (width < 9) || (addr == NULL) || (offset > 7))
        return 0x00;

    return GET_BITS_9_16(addr, offset, width);
}

/*****************************************************************************
* Function: gen_get_bits_17_24
* Purpose:  This function is used to get bits form pack
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* unpack_info     unpack_info_struct *  In/Out        the pointer to unpack_info_struct
* width           u32                In          widht of the bit to get
*
* Return:  value that unpack from the pack
* Note:    width value range[17, 24]
*******************************************************************************/
u32 gen_get_bits_17_24(unpack_info_struct *unpack_info, u32 width)
{
    u32 get_value;

    /*check the input argument validity*/
    if ((width < 17) || (width > 24) || unpack_info == NULL)
        return 0x00;

    /*reset the frame size*/
    unpack_info->fr_size -= (unpack_info->bits%8 + width)/8;
    if (unpack_info->fr_size < 0) {
        /*no more bits in this pack*/
        unpack_info->fr_size += (unpack_info->bits%8 + width)/8;
        return 0x00;
    }

    get_value = GET_BITS_17_24(unpack_info->frame, unpack_info->bits%8, width);
    unpack_info->frame += (unpack_info->bits%8 + width)/8;
    unpack_info->bits += width;

    return get_value;
}

/*****************************************************************************
* Function: get_bits_17_24
* Purpose:  This function is get bits from bit string
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr                 u8*           In          the pointer to bit stream
* offset               u32           In          offset bits from addr,start bit to get value
* width                u32           In          width of bit that to get
* Return:  the value that get from the bit stream
* Note:    width range [17, 24]
*******************************************************************************/
u32 get_bits_17_24(u8 *addr, u32 offset, u32 width)
{
    /*check the input argument validity*/
    if ((width > 24) || (width < 17) || (addr == NULL) || (offset > 7))
        return 0x00;

    return GET_BITS_17_24(addr, offset, width);
}

/*****************************************************************************
* Function: gen_get_bits_25_32
* Purpose:  This function is used to get bits form pack
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* unpack_info     unpack_info_struct *  In/Out        the pointer to unpack_info_struct
* width           u32                In          widht of the bit to get
*
* Return:  value that unpack from the pack
* Note:    width value range[25, 32]
*******************************************************************************/
u32 gen_get_bits_25_32(unpack_info_struct *unpack_info, u32 width)
{
    u32 get_value;

    /*check the input argument validity*/
    if ((width < 25) || (width > 32) || unpack_info == NULL)
        return 0x00;

    unpack_info->fr_size -= (unpack_info->bits%8 + width)/8;
    if (unpack_info->fr_size < 0) {
        /*no more bits in this pack*/
        unpack_info->fr_size += (unpack_info->bits%8 + width)/8;
        return 0x00;
    }

    get_value = GET_BITS_25_32(unpack_info->frame, unpack_info->bits%8, width);
    unpack_info->frame += (unpack_info->bits%8 + width)/8;
    unpack_info->bits += width;

    return get_value;
}

/*****************************************************************************
* Function: get_bits_25_32
* Purpose:  This function is get bits from bit string
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr                 u8*          In          the pointer to bit stream
* offset               u32           In          offset bits from addr,start bit to get value
* width                u32           In          width of bit that to get
* Return:  the value that get from the bit stream
* Note:    width range [25, 32]
*******************************************************************************/
u32 get_bits_25_32(u8 *addr, u32 offset, u32 width)
{
    /*check the input argument validity*/
    if ((width > 32) || (width < 25) || (addr == NULL) || (offset > 7))
        return 0x00;

    return GET_BITS_25_32(addr, offset, width);
}

/*****************************************************************************
* Function: gen_get_bits
* Purpose:  This function is used to get bits form pack
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* unpack_info     unpack_info_struct *  In/Out        the pointer to unpack_info_struct
* width           u32                In          widht of the bit to get
*
* Return:  value that unpack from the pack
* Note:    width value range[1, 32]
*******************************************************************************/
u32 gen_get_bits(unpack_info_struct *unpack_info,u32 width)
{
    u32 get_value;

    /*check the input argument validity*/
    if ((width < 1) || (width > 32) || unpack_info == NULL)
        return 0x00;

    unpack_info->fr_size -= (unpack_info->bits%8 + width)/8;
    if (unpack_info->fr_size < 0) {
        /*no more bits in this pack*/
        unpack_info->fr_size += (unpack_info->bits%8 + width)/8;
        return 0x00;
    }

    get_value = GET_BITS(unpack_info->frame, unpack_info->bits%8, width);
    unpack_info->frame += (unpack_info->bits%8 + width)/8;
    unpack_info->bits += width;

    return get_value;
}

/*****************************************************************************
* Function: get_bits
* Purpose:  This function is get bits from bit string
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr                 u8*            In          the pointer to bit stream
* offset               u32            In          offset bits from addr,start bit to get value
* width                u32            In          width of bit that to get
* Return:  the value that get from the bit stream
* Note:    width range [1, 32]
*******************************************************************************/
u32 get_bits(u8 *addr, u32 offset, u32 width)
{
    /*check the input argument validity*/
    if ((width > 32) || (width < 1) || (addr == NULL) || (offset > 7))
        return 0x00;

    return GET_BITS(addr, offset, width);
}

/*****************************************************************************
* Function: gen_put_bits_1_8
* Purpose:  This function is used to put bits to the package
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* pack_info        pack_info_struct *   In/Out        the pointer to data pack
* width            u32               In          the pointer to timer message
* put_val          u32             In          value put to the package
* Return:  None
* Note:    The width range [1, 8]
*******************************************************************************/
void gen_put_bits_1_8(pack_info_struct *pack_info, u32 width, u32 put_val)
{
    s32 para;

    /*check the input argument validity*/
    if ((pack_info->frame == NULL) || (width < 1) || (width > 8))
        return;

    para = pack_info->bits%8;
    PUT_BITS_1_8(pack_info->frame, para, (s32)width, put_val);
    pack_info->frame += (pack_info->bits%8 + width)/8;
    pack_info->bits += width;
}

/*****************************************************************************
* Function: put_bits_1_8
* Purpose:  This function is used to put bits to the byte stream
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr               u8 *            In/Out        the pointer to the byte stream to which bits to be written
* offset             u32             In          the offset in bits from MSB of the byte stream
* width              u32             In          the number of bits to be written
* put_val            u32             In          the value to be written
* Return:  None
* Note:    width range:[1, 8]
*          offset range: [0, 7]
*******************************************************************************/
void put_bits_1_8(u8 *addr, u32 offset, u32 width, u32 put_val)
{
    /*check the input argument validity*/
    if ((width > 8) || (width < 1) || (addr == NULL) || (offset > 7))
        return ;

    PUT_BITS_1_8(addr, (s32)offset, (s32)width, put_val);
}

/*****************************************************************************
* Function: gen_put_bits_9_16
* Purpose:  This function is used to put bits to the package
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* pack_info        pack_info_struct *   In/Out        the pointer to data pack
* width            u32               In          the pointer to timer message
* put_val          u32               In          value put to the package
* Return:  None
* Note:    The width range [9, 16]
*******************************************************************************/
void gen_put_bits_9_16(pack_info_struct *pack_info, u32 width, u32 put_val)
{
    s32 para;

    /*check the input argument validity*/
    if ((pack_info->frame == NULL) || (width < 9) || (width > 16))
        return;

    para = pack_info->bits%8;
    PUT_BITS_9_16(pack_info->frame, para , (s32)width, put_val);
    pack_info->frame += (pack_info->bits%8 + width)/8;
    pack_info->bits += width;
}

/*****************************************************************************
* Function: put_bits_9_16
* Purpose:  This function is used to put bits to the byte stream
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr               u8 *            In/Out          the pointer to the byte stream to which bits to be written
* offset             u32             In              the offset in bits from MSB of the byte stream
* width              u32             In              the number of bits to be written
* put_val            u32             In              the value to be written
* Return:  None
* Note:    width range:[9, 16]
*          offset range: [0, 7]
*******************************************************************************/
void put_bits_9_16(u8 *addr, u32 offset, u32 width, u32 put_val)
{
    /*check the input argument validity*/
    if ((width > 16) || (width < 9) || (addr == NULL) || (offset > 7))
        return ;

    PUT_BITS_9_16(addr, (s32)offset, (s32)width, put_val);
}

/*****************************************************************************
* Function: gen_put_bits_17_24
* Purpose:  This function is used to put bits to the package
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* pack_info        pack_info_struct *   In/Out          the pointer to data pack
* width            u32               In              the pointer to timer message
* put_val          u32               In              value put to the package
* Return:  None
* Note:    The width range [17, 24]
*******************************************************************************/
void gen_put_bits_17_24(pack_info_struct *pack_info, u32 width, u32 put_val)
{
    s32 para;

    /*check the input argument validity*/
    if ((pack_info->frame == NULL) || (width < 17) || (width > 24))
        return;

    para = pack_info->bits%8;
    PUT_BITS_17_24(pack_info->frame, para, (s32)width, put_val);
    pack_info->frame += (pack_info->bits%8 + width)/8;
    pack_info->bits += width;
}

/*****************************************************************************
* Function: put_bits_17_24
* Purpose:  This function is used to put bits to the byte stream
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr               u8 *            In/Out          the pointer to the byte stream to which bits to be written
* offset             u32             In              the offset in bits from MSB of the byte stream
* width              u32             In              the number of bits to be written
* put_val            u32             In              the value to be written
* Return:  None
* Note:    width range:[17, 24]
*          offset range: [0, 7]
*******************************************************************************/
void put_bits_17_24(u8 *addr, u32 offset, u32 width, u32 put_val)
{
    /*check the input argument validity*/
    if ((width > 24) || (width < 17) || (addr == NULL) || (offset > 7))
        return ;

    PUT_BITS_17_24(addr, (s32)offset, (s32)width, put_val);
}

/*****************************************************************************
* Function: gen_put_bits_25_32
* Purpose:  This function is used to put bits to the package
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* pack_info        pack_info_struct *   In/Out          the pointer to data pack
* width            u32               In              the pointer to timer message
* put_val          u32               In              value put to the package
* Return:  None
* Note:    The width range [25, 32]
*******************************************************************************/
void gen_put_bits_25_32(pack_info_struct *pack_info, u32 width, u32 put_val)
{
    s32 para;

    /*check the input argument validity*/
    if ((pack_info->frame == NULL) || (width < 25) || (width > 32))
        return;

    para = pack_info->bits%8;
    PUT_BITS_25_32(pack_info->frame, para, (s32)width, put_val);
    pack_info->frame += (pack_info->bits%8 + width)/8;
    pack_info->bits += width;
}

/*****************************************************************************
* Function: put_bits_25_32
* Purpose:  This function is used to put bits to the byte stream
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr               u8 *            In/Out          the pointer to the byte stream to which bits to be written
* offset             u32             In              the offset in bits from MSB of the byte stream
* width              u32             In              the number of bits to be written
* put_val            u32             In              the value to be written
* Return:  None
* Note:    width range:[25, 32]
*          offset range: [0, 7]
*******************************************************************************/
void put_bits_25_32(u8 *addr, u32 offset, u32 width, u32 put_val)
{
    /*check the input argument validity*/
    if ((width > 32) || (width < 25) || (addr == NULL) || (offset > 7))
        return ;

    PUT_BITS_25_32(addr, (s32)offset, (s32)width, put_val);
}

/*****************************************************************************
* Function: gen_put_bits
* Purpose:  This function is used to put bits to the package
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* pack_info        pack_info_struct *   In/Out        the pointer to data pack
* width            u32               In          the pointer to timer message
* put_val          u32               In          value put to the package
* Return:  None
* Note:    The width range [1, 32]
*******************************************************************************/
void gen_put_bits(pack_info_struct *pack_info, u32 width, u32 put_val)
{
    s32 para;

    /*check the input argument validity*/
    if ((pack_info->frame == NULL) || (width < 1) || (width > 32))
        return;

    para =  pack_info->bits%8;
    PUT_BITS(pack_info->frame,para, (s32)width, put_val);
    pack_info->frame += (pack_info->bits%8 + width)/8;
    pack_info->bits += width;
}

/*****************************************************************************
* Function: put_bits
* Purpose:  This function is used to put bits to the byte stream
* Relation: NA
* Params:
*
*   Name                Type            In/Out          Description
* --------              ----            ------          -----------
* addr               u8 *            In/Out          the pointer to the byte stream to which bits to be written
* offset             u32             In              the offset in bits from MSB of the byte stream
* width              u32             In              the number of bits to be written
* put_val            u32             In              the value to be written
* Return:  None
* Note:    width range:[1, 32]
*          offset range: [0, 7]
*******************************************************************************/
void put_bits(u8 *addr, u32 offset, u32 width, u32 put_val)
{
    /*check the input argument validity*/
    if ((width > 32) || (width < 1) || (addr == NULL) || (offset > 7))
        return ;

    PUT_BITS(addr, (s32)offset, (s32)width, put_val);
}

