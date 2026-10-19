/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines all the PMIC I2C APIs exported for other module.
 */

#ifndef __INCLUDE_PMIC_I2C_H__
#define __INCLUDE_PMIC_I2C_H__

#include "pmic_i2c_core.h"

int pmic_i2c_transfer(struct pmic_i2c_client *client, struct i2c_msg *msgs, int num);
int pmic_i2c_send(struct pmic_i2c_client *client, u8 *buf, int num);
int pmic_i2c_send_then_read(struct pmic_i2c_client *client, u8 *reg,
                              int reg_len, u8 *buf, int num);
void pmic_i2c_send_async(struct pmic_i2c_client *client, u8 *buf, int num);
int pmic_i2c_transfer_async_status(struct pmic_i2c_client *client);

#endif /* __INCLUDE_PMIC_I2C_H__ */
