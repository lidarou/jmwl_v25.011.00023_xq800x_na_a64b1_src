
/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Capacitive_TP_Cypress_CY8CTMA340.h
 *
 *
 * Description:
 * ------------
 *   Capacitive Touch Panel I2C read/write Driver header file
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 *****************************************************************************/
#ifndef __TP_TS_H__
#define __TP_TS_H__

#if 1 //USE_BTL6133

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bl_chip_custom.h"
#include "aic_type.h"
#include <gpio_inc.h>

#define   MDELAY(n)    ctp_delay_ms(n)
#define   UDELAY(n)    ctp_delay_us(n)

#ifdef RESET_PIN_WAKEUP
#define CTP_RESET_PIN               GPIO_TOP_22
#define CTP_SET_RESET_PIN_REQUEST   aic_gpio_request(CTP_RESET_PIN);
#define CTP_SET_RESET_PIN_FREE      aic_gpio_free(CTP_RESET_PIN);
#define CTP_SET_RESET_PIN_HIGH      aic_gpio_output(CTP_RESET_PIN, GPIO_OUTPUT_LVL_HIGH)
#define CTP_SET_RESET_PIN_LOW       aic_gpio_output(CTP_RESET_PIN, GPIO_OUTPUT_LVL_LOW)
#endif

#ifdef   GPIO_EINT
#define CTP_EINT_PIN                GPIO_TOP_21
#define CTP_EINT_PIN_REQUEST        aic_gpio_request(CTP_EINT_PIN)
#define CTP_EINT_PIN_FREE           aic_gpio_free(CTP_EINT_PIN);
#define CTP_SET_I2C_EINT_OUTPUT		aic_gpio_output(CTP_EINT_PIN,GPIO_OUTPUT_LVL_HIGH)
#define CTP_SET_I2C_EINT_INPUT		aic_gpio_input(CTP_EINT_PIN, GPIO_TRIGGER_EDGE_ANY, NULL, NULL);
#define CTP_SET_I2C_EINT_HIGH		aic_gpio_output(CTP_EINT_PIN,GPIO_OUTPUT_LVL_HIGH)
#define CTP_SET_I2C_EINT_LOW		aic_gpio_output(CTP_EINT_PIN,GPIO_OUTPUT_LVL_LOW)
#endif

#ifdef   CTP_USE_SW_I2C
#define  CTP_I2C_DELAY			50
#define CTP_I2C_DATA_PIN		GPIO11
#define CTP_I2C_CLK_PIN			GPIO10

#define CTP_WRITE		CTP_SLAVE_ADDR
#define CTP_READ		(CTP_SLAVE_ADDR+1)
#define CTP_ACK_COUNTER	10

#define CTP_SET_I2C_CLK_OUTPUT		gpio_set_direction(CTP_I2C_CLK_PIN,GPIO_OUT)
#define CTP_SET_I2C_CLK_HIGH		gpio_output_set(CTP_I2C_CLK_PIN,1)
#define CTP_SET_I2C_CLK_LOW			gpio_output_set(CTP_I2C_CLK_PIN,0)

#define CTP_SET_I2C_DATA_OUTPUT		gpio_set_direction(CTP_I2C_DATA_PIN,GPIO_OUT)
#define CTP_SET_I2C_DATA_INPUT		gpio_set_direction(CTP_I2C_DATA_PIN,GPIO_IN)
#define CTP_SET_I2C_DATA_HIGH		gpio_output_set(CTP_I2C_DATA_PIN,1)
#define CTP_SET_I2C_DATA_LOW		gpio_output_set(CTP_I2C_DATA_PIN,0)
#define CTP_GET_I2C_DATA_BIT		gpio_input_get(CTP_I2C_DATA_PIN)

#define CTP_I2C_START_BIT \
	{ \
		volatile unsigned int j; \
		CTP_SET_I2C_CLK_OUTPUT; \
		CTP_SET_I2C_DATA_OUTPUT; \
		CTP_SET_I2C_CLK_HIGH; \
		CTP_SET_I2C_DATA_HIGH; \
		for(j=0;j<CTP_I2C_DELAY;j++);\
		CTP_SET_I2C_DATA_LOW; \
		for(j=0;j<CTP_I2C_DELAY;j++);\
		CTP_SET_I2C_CLK_LOW; \
	}

	#define CTP_I2C_STOP_BIT \
	{ \
		volatile unsigned int j; \
		CTP_SET_I2C_CLK_OUTPUT; \
		CTP_SET_I2C_DATA_OUTPUT; \
		CTP_SET_I2C_CLK_LOW; \
		CTP_SET_I2C_DATA_LOW; \
		for(j=0;j<CTP_I2C_DELAY;j++);\
		CTP_SET_I2C_CLK_HIGH; \
		for(j=0;j<CTP_I2C_DELAY;j++);\
		CTP_SET_I2C_DATA_HIGH; \
		for(j=0;j<CTP_I2C_DELAY;j++);\
	}
#endif

#ifdef CTP_USE_HW_I2C // sq 20210308
extern int CTP_FLASH_I2C_WRITE(unsigned char i2c_addr, unsigned char *value, unsigned short len);
extern int CTP_FLASH_I2C_READ(unsigned char i2c_addr, unsigned char *value, unsigned short len);
#else
extern char CTP_FLASH_I2C_WRITE(unsigned char i2c_addr, unsigned char *value, unsigned short len);
extern char CTP_FLASH_I2C_READ(unsigned char i2c_addr, unsigned char *value, unsigned short len);
#endif
extern int ctp_bl_ts_init(void);
extern char ctp_bl_read_chip_id(void);

#define   SET_WAKEUP_HIGH    bl_ts_set_intup(1)	 // 需要设置GPIO口
#define   SET_WAKEUP_LOW	 bl_ts_set_intup(0)	 // 需要设置GPIO口
#define   bl_ts_set_irq()    bl_ts_set_intmode(1)

bool get_down_point(u16 *x, u16 *y);
bool get_move_point(u16 *x, u16 *y);
bool get_up_point(u16 *x, u16 *y);

void ctp_delay_ms(unsigned short time);
void ctp_delay_us(unsigned int time);

void bl_ts_set_intup(char level);
unsigned char bl_ts_get_chip_id(void);

void ctp_enter_sleep(void);
void ctp_exit_sleep(void);

#endif
#endif
