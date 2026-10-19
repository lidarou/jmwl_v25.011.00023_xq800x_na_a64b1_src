/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is aw87390 driver header file.
 *
 *****************************************************************************/

#define AW_I2C_SLAVE_ADDR   0x58
#define AW_I2C_ID           3

int aw_write(unsigned int reg, unsigned int value);
unsigned int aw_read(unsigned int reg);

