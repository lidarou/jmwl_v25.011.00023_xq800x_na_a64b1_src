#ifndef __TRNG_PHY_H__
#define __TRNG_PHY_H__

#include <aic_io.h>
#include <aic_type.h>
#include <aic_reg_base.h>
#include <aic_auto_reg.h>

#include "trng_rf.h"

#define TRNG_DATA_VALID_FLAG        0x1
#define DRBG_DATA_VALID_FLAG        0x2

void trng_int_ctrl(bool enable);
void drbg_int_ctrl(bool enable);
void trng_int_clr(void);
void drbg_int_clr(void);
void drbg_enable_ctrl(bool enable);
void trng_enable_ctrl(bool enable);
void trng_write_prot_magic(void);
void trng_startup_num(u8 startup_num);
void trng_sample_ctrl(u32 acc_num, u32 sample_div);
void trng_esro_ctrl(u32 esro_num);
void trng_reseed(void);
void drbg_swseed(int *data, int size);
void trng_auto_reseed(void);
void drbg_shift_num_set(u32 shift_num);
void drbg_tap_ctrl(u32 tap_val);
void trng_read_data_done(void);
void drbg_read_data_done(void);
void trng_read_data(int *data, int size);
void drbg_read_data(int *data, int size);
ISR_RET_E trng_irq_handler(void *param);

#endif
