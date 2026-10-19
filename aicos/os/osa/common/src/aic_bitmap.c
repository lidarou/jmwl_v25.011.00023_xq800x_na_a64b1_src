/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
 /*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aipc module-related data and api.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d14c2
#define LOG_MODULE    AMOD_AOS

#include <aic_assert.h>
#include <aic_bitmap.h>

#define START_MASK(start)   (~0UL << (start))
#define LAST_MASK(end)      BIT_MASK((end))

#define START_ULONG_MASK(start) (~0UL << ((start) % BITS_PER_LONG))
#define LAST_ULONG_MASK(nbits)   BIT_MASK((nbits) % BITS_PER_LONG)

unsigned long aic_bitmap_get_value(unsigned long *map, int start, int nbits)
{
    unsigned long *cur;
    unsigned long value, temp;
    int end_bits;

    AIC_ASSERT(nbits <= BITS_PER_LONG);

    /* get the start word */
    cur = map + BIT_WORD(start);
    start %= BITS_PER_LONG;
    value = (*cur & START_MASK(start));
    value >>= start;

    /* only in one word */
    if (start + nbits <= BITS_PER_LONG)
        return value & LAST_MASK(nbits);

    /* get the end word */
    end_bits = start + nbits - BITS_PER_LONG;
    cur++;
    temp = *cur & LAST_MASK(end_bits);
    value |= (temp << (BITS_PER_LONG - start));

    return value;
}

void aic_bitmap_set_value(unsigned long *map, int start, int nbits,
                          unsigned long value)
{
    unsigned long *cur;
    int end_bits;

    AIC_ASSERT(nbits <= BITS_PER_LONG);
    if (nbits < BITS_PER_LONG)
        AIC_ASSERT(value < BIT(nbits));

    /* arrive start word */
    cur = map + BIT_WORD(start);
    start %= BITS_PER_LONG;

    /* only in one word */
    if (start + nbits <= BITS_PER_LONG) {
        AIC_CLR_BITS(*cur, start, start + nbits - 1);
        *cur |= (value << start);
        return;
    }

    /* set sart word */
    AIC_CLR_BITS(*cur, start, BITS_PER_LONG - 1);
    *cur |= (value << start);

    /* set end word */
    end_bits = start + nbits - BITS_PER_LONG;
    cur++;
    AIC_CLR_BITS(*cur, 0, end_bits - 1);
    *cur |= (value >> (nbits - end_bits) );
}

void aic_bitmap_clear(unsigned long *map, int size, int start, int nbits)
{
    unsigned long *cur;
    int bits_clear, end;

    AIC_ASSERT(start + nbits <= size);

    /* arrive start word */
    cur = map + BIT_WORD(start);
    start %= BITS_PER_LONG;

    /* only one word, clear [start, stat + nbits) */
    if (start + nbits <= BITS_PER_LONG) {
        AIC_CLR_BITS(*cur, start, start + nbits - 1);
        return;
    }

    /* clear start word */
    *cur &=  ~(START_MASK(start));
    bits_clear = BITS_PER_LONG - start;

   /* clear middle word */
    while (nbits - bits_clear >= BITS_PER_LONG) {
        cur++;
        nbits -= BITS_PER_LONG;
        *cur = 0;
    }

    /* clear last word */
    if (nbits) {
        end = start + nbits - BITS_PER_LONG;
        cur++;
        *cur &= ~(LAST_MASK(end));
    }
}

void aic_bitmap_set(unsigned long *map,  int size, int start, int nbits)
{
    unsigned long *cur;
    int bits_set, end;

    AIC_ASSERT(start + nbits <= size);

    /* arrive start word */
    cur = map + BIT_WORD(start);
    start %= BITS_PER_LONG;

    /* only one word, set [start, stat + nbits) */
    if (start + nbits <= BITS_PER_LONG) {
        AIC_SET_BITS(*cur, start, start + nbits - 1);
        return;
    }

    /* set start word */
    *cur |=  START_MASK(start);
    bits_set = BITS_PER_LONG - (start % BITS_PER_LONG);

   /* set middle word */
    while (nbits - bits_set >= BITS_PER_LONG) {
        cur++;
        nbits -= BITS_PER_LONG;
        *cur = ~0UL;
    }

    /* set end word */
    end = start + nbits;
    if (nbits) {
        end = start + nbits - BITS_PER_LONG;
        cur++;
        *cur |= LAST_MASK(end);
    }
}

unsigned long aic_bitmap_find_next_bit(const unsigned long *addr,
                                       unsigned long size,
                                       unsigned long start)
{
    const unsigned long *cur;
    unsigned long serch_bits, pos;
    unsigned long tmp, left_size;

    /* start >= size, end serch, return size */
    if (start >= size)
        return size;

    left_size = size;

    /*  start word */
    cur = addr + BIT_WORD(start);
    pos  = BIT_WORD_OFFSET(start);
    serch_bits = BIT_WORD_ALIGN(start);
    left_size -= serch_bits;

    /* not start with a word align pos, serch in start word. */
    if (pos) {
        tmp = *cur;
        /* clear [0, pos) bit*/
        tmp &= (~0UL << pos);

        /* the start word is also the last word */
        if (left_size <= BITS_PER_LONG)
            goto find_last_word;

        goto find_middle_word;
    }

    /*when left_size >=BITS_PER_LONG, it is not the last word. */
    while (left_size & ~(BITS_PER_LONG - 1)) {
        tmp = *cur;

find_middle_word:
        /* tmp >0, must find in it. */
        if (tmp)
            return serch_bits + aic_find_first_bit(tmp);

        /* move to next word */
        cur++;
        left_size -= BITS_PER_LONG;
        serch_bits += BITS_PER_LONG;
    }

    /* left_size is 0, can't find it. */
    if (!left_size)
        return size;

    /* last word */
    tmp = *cur;
find_last_word:
    /* clear [left_size, BITS_PER_LONG) bit */
    tmp &= (~0UL >> (BITS_PER_LONG - left_size));
    if (!tmp)
        return size;

    return serch_bits + aic_find_first_bit(tmp);
}

unsigned long aic_bitmap_find_next_z_bit(const unsigned long *addr,
                                         unsigned long size,
                                         unsigned long start)
{
    const unsigned long *cur;
    unsigned long serch_bits, pos;
    unsigned long tmp, left_size;

    /* start >= size, end serch, return size */
    if (start >= size)
        return size;

    left_size = size;

    /* arrive start word */
    cur = addr + BIT_WORD(start);
    pos  = BIT_WORD_OFFSET(start);
    serch_bits = BIT_WORD_ALIGN(start);
    left_size -= serch_bits;

    /* not start with a word align pos, serch in start word. */
    if (pos) {
        tmp = *cur;
        /* set (0, pos] bit*/
        tmp |= (~0UL >>  (BITS_PER_LONG - pos));

        /* the start word is also the last word */
        if (left_size < BITS_PER_LONG)
            goto find_z_last_word;

        goto find_z_middle_word;
    }

    /* when left_size >= BITS_PER_LONG, it is not the last word. */
    while (left_size & ~(BITS_PER_LONG - 1)) {
        tmp = *cur;

find_z_middle_word:
        if (~tmp)
            return serch_bits + aic_find_first_z_bit(tmp);

        /* move to next word */
        cur++;
        left_size -= BITS_PER_LONG;
        serch_bits += BITS_PER_LONG;
    }

    /* left_size is 0, can't find it. */
    if (!left_size)
        return size;

    /* last word */
    tmp = *cur;
find_z_last_word:
    /* set (left_size, BITS_PER_LONG] bit */
    tmp |= (~0UL <<  left_size);
    if (tmp == ~0UL)
        return size;

    return serch_bits + aic_find_first_z_bit(tmp);
}

extern int aic_bitmap_find_next_z_area(unsigned long *map,
                                       unsigned long size,
                                       unsigned long start,
                                       unsigned int nbits)
{
    unsigned long index, end, i;

    do {
        /* first, find next zero bit for start. */
        index = aic_bitmap_find_next_z_bit(map, size, start);

        /*
         * second, sech the next nbits bit, if it has non-zero bit.
         * if has, start with the next bit, try again.
         */
        end = index + nbits;
        /* end > size, can't find it, return size. */
        if (end > size)
            return size;

        /* find non-zero bit */
        i = aic_bitmap_find_next_bit(map, end, index);
        /* i < end find non-zero bit, start = i + 1, try again.*/
        if (i < end) {
            start = i + 1;
            continue;
        }
        /* find the first eligible area, break */
        break;
    }while (1);

    return index;
}

extern int aic_bitmap_find_minimum_z_area(unsigned long *map,
                                          unsigned long size,
                                          unsigned long start,
                                          unsigned long nbits)
{
    unsigned long index, end, i, min_bits, min_index, bits, z_start;

    min_bits = size + 1;
    min_index = size;

    do {
        /* first, find next zero bit for start. */
        index = aic_bitmap_find_next_z_bit(map, size, start);

        /*
         * second, sech the next nbits bit, if it has non-zero bit.
         * if has, start with the next bit, try again.
         */
        end = index + nbits;
        /* end > size, over the serch. */
        if (end > size)
            break;

        /* find non-zero bit */
        i = aic_bitmap_find_next_bit(map, end, index);
        /* i < end find non-zero bit, start = i + 1, try again.*/
        if (i < end) {
            start = i + 1;
            continue;
        }

        /* find an eligible area,  get the next start serch bit. */
        z_start = index;
        index = end;
        i = aic_bitmap_find_next_bit(map, size, index);
        start = i + 1;
        bits = i - z_start;

        /* compare and record the minimum area. */
        if (min_bits > bits) {
            min_bits = bits;
            min_index = z_start;
        }
    }while (start + nbits < size);

    return min_index;
}

unsigned long aic_bitmap_get_total_nz_bit(const unsigned long *map,
                                          unsigned long size,
                                          unsigned long start)
{
    const unsigned long *cur;
    unsigned long serch_bits, pos;
    unsigned long tmp, left_size, total = 0;

    /* start >= size, end serch, return 0 */
    if (start >= size)
        return 0;

    left_size = size;

    /* arrive start word */
    cur = map + BIT_WORD(start);
    pos  = BIT_WORD_OFFSET(start);
    serch_bits = BIT_WORD_ALIGN(start);
    left_size -= serch_bits;

    /* not start with a word align pos, serch in start word. */
    if (pos) {
        tmp = *cur;
        /* clear [0, pos) bits. */
        tmp &= BIT(pos + 1);

        /* the start word is also the last word */
        if (left_size < BITS_PER_LONG)
            goto find_nz_last_word;

        goto find_nz_middle_word;
    }

    /* when left_size >= BITS_PER_LONG, it is not the last word. */
    while (left_size & ~(BITS_PER_LONG - 1)) {
        tmp = *cur;

find_nz_middle_word:
        if (tmp != 0UL)
            total += aic_get_nz_bits(tmp);

        /* move to next word */
        cur++;
        left_size -= BITS_PER_LONG;
    }

    /* left_size is 0, end find. */
    if (!left_size)
        return total;

    /* last word */
    tmp = *cur;
find_nz_last_word:
    /* clear [BITS_PER_LONG - left_size, BITS_PER_LONG) bits. */
    tmp &= (BIT(left_size + 1) - 1);
    if (tmp == 0UL)
        return total;

    total += aic_get_nz_bits(tmp);

    return total;
}


unsigned long aic_bitmap_get_total_z_bit(const unsigned long *map,
                                         unsigned long size,
                                         unsigned long start)
{
    const unsigned long *cur;
    unsigned long serch_bits, pos;
    unsigned long tmp, left_size, total = 0;

    /* start >= size, end serch, return 0 */
    if (start >= size)
        return 0;

    left_size = size;

    /* arrive start word */
    cur = map + BIT_WORD(start);
    pos  = BIT_WORD_OFFSET(start);
    serch_bits = BIT_WORD_ALIGN(start);
    left_size -= serch_bits;

    /* not start with a word align pos, serch in start word. */
    if (pos) {
        tmp = *cur;
        /* set (0, pos] bits. */
        tmp |= (~0UL >>  (BITS_PER_LONG - pos));

        /* the start word is also the last word */
        if (left_size < BITS_PER_LONG)
            goto find_z_last_word;

        goto find_z_middle_word;
    }

    /* when left_size >= BITS_PER_LONG, it is not the last word. */
    while (left_size & ~(BITS_PER_LONG - 1)) {
        tmp = *cur;

find_z_middle_word:
        if (tmp != ~0UL)
            total += aic_get_z_bits(tmp);

        /* move to next word */
        cur++;
        left_size -= BITS_PER_LONG;
    }

    /* left_size is 0, end find. */
    if (!left_size)
        return total;

    /* last word */
    tmp = *cur;
find_z_last_word:
    /* set (left_size, BITS_PER_LONG] bits. */
    tmp |= (~0UL <<  left_size);
    if (tmp == ~0UL)
        return total;

    total += aic_get_z_bits(tmp);

    return total;
}


