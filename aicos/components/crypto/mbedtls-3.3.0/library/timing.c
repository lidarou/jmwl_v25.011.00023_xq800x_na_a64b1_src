/*
 *  Portable interface to the CPU cycle counter
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x10472f
#define LOG_MODULE    AMOD_SYS

#ifdef CONFIG_NX_BSD
#include <nxd_bsd.h>
#endif
#include "common.h"
#include <timer_inc.h>
#if defined(MBEDTLS_TIMING_C)

#include "mbedtls/timing.h"
#include "mbedtls/platform.h"

#if !defined(MBEDTLS_TIMING_ALT)

/* time.h should be included independently of MBEDTLS_HAVE_TIME. If the
 * platform matches the ifdefs above, it will be used. */
#include <time.h>
//#include <sys/time.h>
struct _hr_time
{
    struct timeval start;
};

/**
 * \brief          Return the elapsed time in milliseconds
 *
 * \warning        May change without notice
 *
 * \param val      points to a timer structure
 * \param reset    If 0, query the elapsed time. Otherwise (re)start the timer.
 *
 * \return         Elapsed time since the previous reset in ms. When
 *                 restarting, this is always 0.
 *
 * \note           To initialize a timer, call this function with reset=1.
 *
 *                 Determining the elapsed time and resetting the timer is not
 *                 atomic on all platforms, so after the sequence
 *                 `{ get_timer(1); ...; time1 = get_timer(1); ...; time2 =
 *                 get_timer(0) }` the value time1+time2 is only approximately
 *                 the delay since the first reset.
 */
unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val,
                                       int reset)
{
    /* disable the default solution which gets delay by gettimeofday API */
#if 0
    struct _hr_time *t = (struct _hr_time *)val;

    if (reset) {
        gettimeofday(&t->start, NULL);
        return 0;
    } else {
        unsigned long delta;
        struct timeval now;
        gettimeofday(&now, NULL);
        delta = (now.tv_sec  - t->start.tv_sec) * 1000ul
              + (now.tv_usec - t->start.tv_usec) / 1000;
        return delta;
    }
#else
    u64 *start_time_in_ms = (u64 *)val;

    u64 now_ms = aic_get_tick();
    if (reset) {
        *start_time_in_ms = now_ms;
        return 0;
    } else {
        unsigned long delta;

        delta = (unsigned long)(now_ms - *start_time_in_ms);
        return delta;
    }
#endif
}

/*
 * Set delays to watch
 */
void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *)data;

    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;

    if (fin_ms != 0)
        (void)mbedtls_timing_get_timer( &ctx->timer, 1);
}

/*
 * Get number of delays expired
 */
int mbedtls_timing_get_delay(void *data)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *)data;
    unsigned long elapsed_ms;

    if (ctx->fin_ms == 0)
        return -1;

    elapsed_ms = mbedtls_timing_get_timer(&ctx->timer, 0);

    if (elapsed_ms >= ctx->fin_ms)
        return 2;

    if (elapsed_ms >= ctx->int_ms)
        return 1;

    return 0;
}

/*
 * Get the final delay.
 */
uint32_t mbedtls_timing_get_final_delay(const mbedtls_timing_delay_context *data)
{
    return data->fin_ms;
}

#endif /* !MBEDTLS_TIMING_ALT */
#endif /* MBEDTLS_TIMING_C */
