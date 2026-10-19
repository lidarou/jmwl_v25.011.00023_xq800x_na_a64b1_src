/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the bit-related common operatons.
 */

#ifndef __AIC_BITMAP_H__
#define __AIC_BITMAP_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_math.h>

/* bit define */
#define BIT(n)    (1UL << (n))

#define BIT_0     (0x00000001UL)
#define BIT_1     (0x00000002UL)
#define BIT_2     (0x00000004UL)
#define BIT_3     (0x00000008UL)
#define BIT_4     (0x00000010UL)
#define BIT_5     (0x00000020UL)
#define BIT_6     (0x00000040UL)
#define BIT_7     (0x00000080UL)
#define BIT_8     (0x00000100UL)
#define BIT_9     (0x00000200UL)
#define BIT_10    (0x00000400UL)
#define BIT_11    (0x00000800UL)
#define BIT_12    (0x00001000UL)
#define BIT_13    (0x00002000UL)
#define BIT_14    (0x00004000UL)
#define BIT_15    (0x00008000UL)
#define BIT_16    (0x00010000UL)
#define BIT_17    (0x00020000UL)
#define BIT_18    (0x00040000UL)
#define BIT_19    (0x00080000UL)
#define BIT_20    (0x00100000UL)
#define BIT_21    (0x00200000UL)
#define BIT_22    (0x00400000UL)
#define BIT_23    (0x00800000UL)
#define BIT_24    (0x01000000UL)
#define BIT_25    (0x02000000UL)
#define BIT_26    (0x04000000UL)
#define BIT_27    (0x08000000UL)
#define BIT_28    (0x10000000UL)
#define BIT_29    (0x20000000UL)
#define BIT_30    (0x40000000UL)
#define BIT_31    (0x80000000UL)

/* Number of bits in a long int. */
#define BITS_PER_BYTE    8
#define BITS_PER_LONG    (BITS_PER_BYTE * sizeof(long))

#define BIT_MASK(n)      (BIT(n) - 1)
#define BIT_WORD(n)      ((n) / BITS_PER_LONG)

/**
 * aic_find_first_bit - find first set bit.
 *
 * @word: unsigned long word.
 *
 * return: if find return the first bit index, otherwise return BITS_PER_LONG.
 */
static inline unsigned long aic_find_first_bit(unsigned long word)
{
    unsigned long index = 0, offset;
    unsigned long mask = ~0UL;
    unsigned long tmp = word;

    if (!tmp)
        return BITS_PER_LONG;

    /* Binary search set bit */
    for (offset = BITS_PER_LONG >> 1; offset > 0; offset >>= 1) {
        mask >>= offset;
        if (!(tmp & mask)) {
            index += offset;
            tmp >>= offset;
        }
    }

    return index;
}

/**
 * aic_find_first_bit - find first zero bit.
 *
 * @word: unsigned long word.
 *
 * return: return the first bit index if found, otherwise return BITS_PER_LONG.
 */
#define aic_find_first_z_bit(x)     aic_find_first_bit(~(x))

/**
 * aic_find_last_bit - find last set bit.
 *
 * @word: unsigned long word.
 *
 * return: if find return the last bit index, otherwise return BITS_PER_LONG.
 */
static inline unsigned long aic_find_last_bit(unsigned long word)
{
    unsigned long index = (BITS_PER_LONG - 1), offset;
    unsigned long mask = ~0UL;
    unsigned long tmp = word;

    if (!tmp)
        return BITS_PER_LONG;

    /* Binary search set bit */
    for (offset = BITS_PER_LONG >> 1; offset > 0; offset >>= 1) {
        mask <<= offset;
        if (!(tmp & mask)) {
            index -= offset;
            tmp <<= offset;
        }
    }

    return index;
}

/**
 * hweightN - returns the hamming weight of a N-bit word
 * @x: the word to weigh
 *
 * The Hamming Weight of a number is the total number of bits set in it.
 */
static inline unsigned int __sw_hweight32(unsigned int w)
{
    unsigned int res = w - ((w >> 1) & 0x55555555);
    res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
    res = (res + (res >> 4)) & 0x0F0F0F0F;
    res = res + (res >> 8);
    return (res + (res >> 16)) & 0x000000FF;
}

static inline unsigned int __sw_hweight16(unsigned int w)
{
    unsigned int res = w - ((w >> 1) & 0x5555);
    res = (res & 0x3333) + ((res >> 2) & 0x3333);
    res = (res + (res >> 4)) & 0x0F0F;
    return (res + (res >> 8)) & 0x00FF;
}

static inline unsigned int __sw_hweight8(unsigned int w)
{
    unsigned int res = w - ((w >> 1) & 0x55);
    res = (res & 0x33) + ((res >> 2) & 0x33);
    return (res + (res >> 4)) & 0x0F;
}

#if 0
static inline unsigned long __sw_hweight64(unsigned long long w)
{
#if (BITS_PER_LONG == 32)
    return __sw_hweight32((unsigned int)(w >> 32)) +
           __sw_hweight32((unsigned int)w);
#elif (BITS_PER_LONG == 64)
    unsigned long long res = w - ((w >> 1) & 0x555555555555555555555555ul);
    res = (res & 0x3333333333333333ul) + ((res >> 2) & 0x3333333333333333ul);
    res = (res + (res >> 4)) & 0x0F0F0F0F0F0F0F0Ful;
    res = res + (res >> 8);
    res = res + (res >> 16);
    return (res + (res >> 32)) & 0x00000000000000FFul;
#endif
}
#endif

#define hweight8(w)   __sw_hweight8(w)
#define hweight16(w)  __sw_hweight16(w)
#define hweight32(w)  __sw_hweight32(w)
/* #define hweight64(w)  __sw_hweight64(w) */

/**
 * BIT_WORD_ALIGN - get the word align bit
 * it is equivalent to (offset/BITS_PER_LONG * BITS_PER_LONG).
*/
#define BIT_WORD_ALIGN(offset)      ((offset) & ~(BITS_PER_LONG - 1))
#define BIT_WORD_OFFSET(offset)     ((offset) % BITS_PER_LONG)


/**
 * GENMASK - a contiguous bitmask starting at bit position l
 *           and ending at position h.
 *
 * @l: low bit positon.
 * @h: high bit positon.
 */
#define GENMASK(l, h) \
    (((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

/* aic bit op define */
#define AIC_BIT_MASK(n)             (BIT(n) - 1)
#define AIC_SET_BIT(var, bit)       (var) = ((var) | BIT(bit))
#define AIC_CLR_BIT(var, bit)       (var) = ((var) & ~BIT(bit))
#define AIC_SET_BITS(var, l, h)     (var) = ((var) | (GENMASK(l, h)))
#define AIC_CLR_BITS(var, l, h)     (var) = ((var) & ~(GENMASK(l, h)))
#define AIC_SET_MASK(var, mask)     (var) = ((var) | (mask))
#define AIC_CLR_MASK(var, mask)     (var) = ((var) & ~(mask))

/**
 * aic_bitmap_get - get a bit area.
 *
 * @map: bitmap.
 * @start: the start bit
 * @nbit: continuous bit numbers, nbit < BITS_PER_LONG.
 *
 * return: consider the continuous nr bit region as an unsigned integer,
 *         return the value of the integer.
 */
unsigned long aic_bitmap_get_value(unsigned long *map, int start, int nr);

/**
 * aic_bitmap_get - set a bit area. consider the continuous nr bit
 * region as an unsigned integer, set the value of the integer.
 *
 * @map: bitmap.
 * @start: the start bit
 * @nbit: continuous bit numbers, nbit < BITS_PER_LONG.
 */
void aic_bitmap_set_value(unsigned long *map, int start, int nr,
                          unsigned long value);

/**
 * aic_bitmap_set - set a bit area.
 *
 * @map: bitmap.
 * @size: the bitmap size.
 * @start: the start bit
 * @nbit: continuous bit numbers.
 */
void aic_bitmap_set(unsigned long *map, int size, int start, int nr);

/**
 * aic_bitmap_clear - clear a bit area.
 *
 * @map: bitmap.
 * @size: the bitmap size.
 * @start: the start bit
 * @nbit: continuous bit numbers.
 */
void aic_bitmap_clear(unsigned long *map, int size, int start, int nr);

/**
 * aic_bitmap_find_next_bit - find next set bit.
 *
 * @map: bitmap.
 * @size: the bitmap size.
 * @start: the start bit
 *
 * @return: if can't find, return size.
 */
unsigned long aic_bitmap_find_next_bit(const unsigned long *addr,
                                       unsigned long size,
                                       unsigned long start);

unsigned long aic_bitmap_find_next_z_bit(const unsigned long *addr,
                                         unsigned long size,
                                         unsigned long start);

/**
 * aic_find_first_z_bitmap_area - find the next continuous zero bit area.
 *
 * @map: bitmap.
 * @size: the bitmap size.
 * @start: the start bit
 *
 * @return: if can't find, return size.
 */
int aic_bitmap_find_next_z_area(unsigned long *map,
                                unsigned long size,
                                unsigned long start,
                                unsigned int nr);

/**
 * aic_find_min_bitmap_area - find the Minimum continuous zero bit area.
 *
 * @map: bitmap.
 * @size: bitmap size.
 * @start: the start serch bit.
 * @nbit: continuous bit numbers.
 */
int aic_bitmap_find_minimum_z_area(unsigned long *map,
                                   unsigned long size,
                                   unsigned long start,
                                   unsigned int nr);

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_BITMAP_H__ */

