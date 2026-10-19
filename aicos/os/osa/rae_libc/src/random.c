/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a3cf9
#define LOG_MODULE    AMOD_AOS

#include <stdlib.h>
#include <aos_utl.h>

#ifdef RAND_MAX
#undef RAND_MAX
#define RAND_MAX          ((int)0x7FFFFFFF)
#else
#define RAND_MAX          ((int)0x7FFFFFFF)
#endif


int rand(void)
{
    return (int)aos_random_value((int)0, RAND_MAX);
}

void srand(unsigned int seed)
{
    aos_set_random_seed(seed);
}
