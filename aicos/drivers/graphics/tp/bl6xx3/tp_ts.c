
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
*   Capacitive_TP_Cypress_CY8CTMA340.c
*
*
* Description:
* ------------
*   Capacitive Touch Panel I2C read/write Driver
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
*------------------------------------------------------------------------------
* Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
*============================================================================
*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e5469
#define LOG_MODULE    AMOD_GPAPH
#include "tp_ts.h"

#if 1 //BL6XX3

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <aic_errno.h>
#include <aic_init.h>
#include "bl_chip_common.h"
#include "eipu.h"
#include "i2c.h"
#include "aic_io.h"
#include "pin_ctrl.h"
#include "fw_update.h"
#include "slp_inc.h"

#define TP_TASK_PRIORITY               (5)
#define TP_STACK_SIZE                  (1024)
#define TP_RELEASE_CHECK_INTERNAL      (30)

#define TP_DATA_BYTE         (6)

typedef struct
{
	u16 x;
	u16 y;
}bl61xx3_point_struct;

static bl61xx3_point_struct bl_down_point = {0};
static bl61xx3_point_struct bl_move_point = {0};
static bl61xx3_point_struct bl_up_point = {0};

static aos_thread_id s_tp_thread = AOS_INVALID_THREAD_ID;
static bool s_tp_irq = false;

static bool is_sleep = true;
#ifdef BTL_CHECK_CHIPID
static unsigned char s_chip_id = 0x00;
#endif

void ctp_delay_us(unsigned int time)
{
    aos_usleep(time);//aic_udelay
}

void ctp_delay_ms(unsigned short time)
{
    aic_mdelay(time);//aos_sleep
}

void bl_ts_set_intmode(char mode)
{
	if(1) //if(0 == mode)
	{//GPIO mode
		CTP_SET_I2C_EINT_OUTPUT;
	}
	else if(1 == mode)
	{//INT mode
		CTP_SET_I2C_EINT_INPUT;
	}
}

void bl_ts_set_intup(char level)
{
	if(level==1)
		CTP_SET_I2C_EINT_HIGH;
	else if(level==0)
		CTP_SET_I2C_EINT_LOW;
}

#ifdef INT_PIN_WAKEUP
void bl_ts_int_wakeup(void)
{
	bl_ts_set_intmode(0);
	bl_ts_set_intup(1);
	ctp_delay_ms(20);
	bl_ts_set_intup(0);
	ctp_delay_ms(1);
	bl_ts_set_intup(1);
	ctp_delay_ms(20);
	bl_ts_set_intmode(1);
}
#endif

#ifdef  RESET_PIN_WAKEUP
void bl_ts_reset_wakeup(void)
{
	CTP_SET_RESET_PIN_HIGH;
	ctp_delay_ms(20);
	CTP_SET_RESET_PIN_LOW;
	ctp_delay_ms(20);
	CTP_SET_RESET_PIN_HIGH;
	ctp_delay_ms(20);
}
#endif

#ifdef	CTP_USE_SW_I2C
void ctp_i2c_init(void)
{
	CTP_SET_I2C_CLK_OUTPUT;
	CTP_SET_I2C_CLK_HIGH;
	CTP_SET_I2C_DATA_OUTPUT;
	CTP_SET_I2C_DATA_HIGH;

}

char CTP_I2C_send_byte(char send_byte)
{
	volatile signed char i;
	volatile unsigned int j;
	volatile unsigned int k=0;
	char ack;

	for (i=7;i>=0;i--)
	{	/* data bit 7~0 */
		if (send_byte & (1<<i))
		{
			CTP_SET_I2C_DATA_HIGH;
		}
		else
		{
			CTP_SET_I2C_DATA_LOW;
		}
		for(j=0;j<CTP_I2C_DELAY;j++);
		CTP_SET_I2C_CLK_HIGH;
		for(j=0;j<CTP_I2C_DELAY;j++);
		CTP_SET_I2C_CLK_LOW;
		for(j=0;j<CTP_I2C_DELAY;j++);
	}

	CTP_SET_I2C_DATA_INPUT;
	CTP_SET_I2C_CLK_HIGH;
	for(j=0;j<CTP_I2C_DELAY;j++);

	for(k=0;k<CTP_ACK_COUNTER;k++)
	{
		if(CTP_GET_I2C_DATA_BIT == 0)
		{
			ack=1;
			break;
		}
		else
		{
			ack=0;
		}
	}

	CTP_SET_I2C_CLK_LOW;
	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_DATA_OUTPUT;
	ctp_delay_us(2);
	return ack;
}

char CTP_I2C_get_byte(void)
{
	volatile signed char i;
	volatile unsigned int j;
	unsigned char get_byte=0;

	CTP_SET_I2C_DATA_INPUT;
	for (i=7;i>=0;i--)
	{       /* data bit 7~0 */
		CTP_SET_I2C_CLK_LOW;
		for(j=0;j<CTP_I2C_DELAY;j++);
		CTP_SET_I2C_CLK_HIGH;
		for(j=0;j<CTP_I2C_DELAY;j++);
		if (CTP_GET_I2C_DATA_BIT)
			get_byte |= (1<<i);
	}
	/* don't care bit, 9th bit */

	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_CLK_LOW;
	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_DATA_OUTPUT;
	ctp_delay_us(2);
	return get_byte;
}

unsigned char CTP_I2C_get_byte_with_ack(unsigned char uc_ack_lvl)
{
	volatile signed char i;
	volatile unsigned int j;
	unsigned char get_byte=0;

	CTP_SET_I2C_DATA_INPUT;
	for(j=0;j<CTP_I2C_DELAY;j++);
	for (i=7;i>=0;i--)
	{       /* data bit 7~0 */
		CTP_SET_I2C_CLK_LOW;
		for(j=0;j<CTP_I2C_DELAY;j++);
		CTP_SET_I2C_CLK_HIGH;
		for(j=0;j<CTP_I2C_DELAY;j++);
		if (CTP_GET_I2C_DATA_BIT)
			get_byte |= (1<<i);
		for(j=0;j<CTP_I2C_DELAY;j++);
	}
	/* don't care bit, 9th bit */

	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_CLK_LOW;
	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_DATA_OUTPUT;

	if (uc_ack_lvl == 1)
	{
		CTP_SET_I2C_DATA_HIGH;
	}
	else
	{
		CTP_SET_I2C_DATA_LOW;
	}

	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_CLK_HIGH;

	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_CLK_LOW;

	for(j=0;j<CTP_I2C_DELAY;j++);
	CTP_SET_I2C_DATA_LOW;
	ctp_delay_us(2);
	return get_byte;
}

char CTP_FLASH_I2C_WRITE(unsigned char i2c_addr, unsigned char *value, unsigned short len)
{

	volatile unsigned int j;
	unsigned char get_byte = 0;
	unsigned short i;

	CTP_I2C_START_BIT;
	for(j=0;j<CTP_I2C_DELAY;j++);

	//if(!CTP_I2C_send_byte(reg<<1))	//FLASH_ADDR + WRITE
	if(!CTP_I2C_send_byte(i2c_addr))	//FLASH_ADDR + WRITE
		return(1);
	for(j=0;j<CTP_I2C_DELAY;j++);

	//for(i = 0; i< len - 1; i++)
	for(i = 0; i< len ; i++)
	{
		if(!CTP_I2C_send_byte(*value++)) //FLASH COMMAND
			return(1);
		for(j=0;j<CTP_I2C_DELAY;j++);
	}

	CTP_I2C_STOP_BIT;
	for(j=0;j<CTP_I2C_DELAY;j++);

	return 0;
}

char CTP_FLASH_I2C_READ(unsigned char i2c_addr, unsigned char *value, unsigned short len)
{

	volatile unsigned int j;
	unsigned char get_byte = 0;
	unsigned short i;

	CTP_I2C_START_BIT;
	//if(!CTP_I2C_send_byte( (reg<<1) | 1))	// FLASH READ
	if(!CTP_I2C_send_byte( i2c_addr | 0x01) )	// FLASH READ
		return(1);

	if (len <= 1)
	{
		//*value =CTP_I2C_get_byte();
		*value =CTP_I2C_get_byte_with_ack(1);
		for(j=0;j<CTP_I2C_DELAY;j++);
	}
	else
	{
		for (i = 0; i< len - 1; i++)
		{
			*value++ =CTP_I2C_get_byte_with_ack(0);
		}
		*value =CTP_I2C_get_byte_with_ack(1);
	}
	CTP_I2C_STOP_BIT;
	return 0;
}
#endif

#ifdef CTP_USE_HW_I2C
struct i2c_client bl6xx3_client = {
    .addr = CTP_SLAVE_ADDR,
    .index = 4,
};

void ctp_i2c_init(void)
{
    int ret = 0;

    ret = i2c_open(&bl6xx3_client);

	bl_log_trace("ctp_i2c_init ret:%d", ret);
}

int CTP_FLASH_I2C_READ(unsigned char i2c_addr, unsigned char *value, unsigned short len)
{
	bl6xx3_client.addr = i2c_addr;

	return i2c_master_recv(&bl6xx3_client, value, len);
}

int CTP_FLASH_I2C_WRITE(unsigned char i2c_addr, unsigned char *value, unsigned short len)
{
	bl6xx3_client.addr = i2c_addr;

	return i2c_master_send(&bl6xx3_client, value, len);
}
#endif

static int add_point(char *buf)
{
    u16 x = 0;
	u16 y = 0;

    x = (((unsigned short)(buf[2] & 0x0f))<< 8) | buf[3];
    y = (((unsigned short)(buf[4] & 0x0f))<< 8) | buf[5];

	bl_log_trace("add_point_state %x %d %d", (buf[2] & 0xc0), x, y);

	switch(buf[2] & 0xc0)
	{
		case 0x00: //down
			bl_down_point.x = x;
			bl_down_point.y = y;
			break;

		case 0x80: //move
			bl_move_point.x = x;
			bl_move_point.y = y;
			break;

		case 0x40: //up
			bl_up_point.x = x;
			bl_up_point.y = y;
			break;

		default:
			bl_up_point.x = x;
			bl_up_point.y = y;
			break;
	}

    return 0;
}

bool get_down_point(u16 *x, u16 *y)
{
	if(bl_down_point.x != 0 || bl_down_point.y != 0)
	{
		*x = bl_down_point.x;
		*y = bl_down_point.y;

		bl_down_point.x = 0;
		bl_down_point.y = 0;

		return true;
	}

	return false;
}

bool get_move_point(u16 *x, u16 *y)
{
	if(bl_move_point.x != 0 || bl_move_point.y != 0)
	{
		*x = bl_move_point.x;
		*y = bl_move_point.y;

		bl_move_point.x = 0;
		bl_move_point.y = 0;

		return true;
	}

	return false;
}

bool get_up_point(u16 *x, u16 *y)
{
	if(bl_up_point.x != 0 || bl_up_point.y != 0)
	{
		*x = bl_up_point.x;
		*y = bl_up_point.y;

		bl_up_point.x = 0;
		bl_up_point.y = 0;

		return true;
	}

    return false;
}

static void bl6133_hisr(void)
{
    int ret = 0;
    char buf[TP_DATA_BYTE];
    unsigned char readPointCmd = TD_STAT_ADDR;

    ret = CTP_FLASH_I2C_WRITE(CTP_SLAVE_ADDR,&readPointCmd, 1);
    bl_log_trace("bl6133_hisr 1 ret:%d", ret);

    ret = CTP_FLASH_I2C_READ(CTP_SLAVE_ADDR, buf, sizeof(buf));
    bl_log_trace("bl6133_hisr 2 ret:%d", ret);

    add_point(buf);
}

#if 1
static void bl6133_eipu_callback(u32 index, u32 value, u32 eipu_pol)
{
    //bl_log_trace("bl6133_eipu_callback %d %d %d", index, value, eipu_pol);
    s_tp_irq = true;
    return;
}
#else
static void bl6133_gpio_callback(void * param)
{
    bl_log_trace("bl6133_gpio_callback");
    bl6133_hisr();
    return;
}
#endif

void ctp_enter_sleep(void)
{
    if(is_sleep == false)
    {
        unsigned char sleepCmd[2] = {0xa5,0x03};
        unsigned char aa=0, bb=0;

        CTP_SET_RESET_PIN_REQUEST;

        bb =  CTP_FLASH_I2C_WRITE(CTP_SLAVE_ADDR, sleepCmd, sizeof(sleepCmd));
        bl_log_trace( "ctp_enter_sleep aa=%d,bb=%d",aa,bb);
        ctp_delay_ms(100);

        //bl_ts_set_intmode(0);
        //bl_ts_set_intup(1);

        bl_log_trace( "ctp_enter_sleep");

        is_sleep = true;

        CTP_SET_RESET_PIN_FREE;

        // TODO: need ?  pin_eiu_int0_f6tof3();

        if ( AOS_INVALID_THREAD_ID != s_tp_thread ) {
            aos_suspend_thread(s_tp_thread);
        }

        EIPU_UnRegCallback(4, EIPU_DBNC, NOSEC_MODE);
    }
}

void ctp_exit_sleep(void)
{
    if(is_sleep)
    {
        is_sleep = false;

        CTP_SET_RESET_PIN_REQUEST;

#ifdef RESET_PIN_WAKEUP
        bl_ts_reset_wakeup();
#endif
#ifdef INT_PIN_WAKEUP
        bl_ts_int_wakeup();
#endif

        CTP_SET_RESET_PIN_FREE;
        //bl_ts_set_intmode(1);

        bl_log_trace( "ctp_exit_sleep");

        // TODO:need ? pin_eiu_int0_f3tof6();

        if ( AOS_INVALID_THREAD_ID != s_tp_thread ) {
            aos_resume_thread(s_tp_thread);
        }

        EIPU_RegCallback(4, EIPU_DBNC, bl6133_eipu_callback,
                            LOW_LEVEL, 0x20, NOSEC_MODE);
    }
}

static void tp_thread_entry(void *thread_input)
{
    while(1)
    {
        if(s_tp_irq)
        {
            s_tp_irq = false;
            bl6133_hisr();
        }
        aos_sleep(5);
    }
}

unsigned char bl_ts_get_chip_id(void)
{
    return s_chip_id;
}

int ctp_bl_ts_init(void)
{
    char  ret = 0;

    CTP_SET_RESET_PIN_REQUEST;
    CTP_EINT_PIN_REQUEST;

#ifdef  RESET_PIN_WAKEUP
    bl_ts_reset_wakeup();
#endif
#ifdef INT_PIN_WAKEUP
    bl_ts_int_wakeup();
#endif

    ctp_i2c_init();
    MDELAY(20);

#ifdef  BTL_CHECK_CHIPID
    bl_ts_set_intmode(0);
    bl_get_chip_id(&s_chip_id);
    bl_ts_set_intmode(1);
    ctp_delay_ms(10);
    if((ret < 0)|| (s_chip_id != BTL_FLASH_ID))
    {
        bl_log_trace("ctp_bl_ts_init:Read chipID Fail:chipID = %x\n",s_chip_id);
        return -1;
    }
    else
    {
         bl_log_trace("ctp_bl_ts_init:Read chipID succes:chipID = %x\n",s_chip_id);
    }
#endif

#ifdef  BL_AUTO_UPDATE_FARMWARE
    bl_ts_set_intmode(0);
    bl_auto_update_fw();
    bl_ts_set_intmode(1);
    ctp_delay_ms(10);
#endif

    bl_ts_set_intmode(0);
    bl_ts_set_intup(1);

    ctp_delay_ms(1);

    CTP_SET_RESET_PIN_FREE;
    CTP_EINT_PIN_FREE;

    pin_eiu_int0_f3tof6();

#if 1
    // move to ctp_exit_sleep
    //EIPU_RegCallback(4, EIPU_DBNC, bl6133_eipu_callback,
    //                     LOW_LEVEL, 0x20, NOSEC_MODE);
#else
    aic_gpio_input(CTP_EINT_PIN,
                    GPIO_TRIGGER_LVL_LOW,
                    bl6133_gpio_callback,
                    NULL);
#endif

    ctp_exit_sleep();

    s_tp_thread = aos_create_thread("tp_thread",
                                    tp_thread_entry,
                                    NULL,
                                    TP_STACK_SIZE,
                                    0,
                                    NULL,
                                    TP_TASK_PRIORITY,
                                    AOS_AUTO_START);

    return !ret;
}

int Tp_Ts_Init(void)
{
    ctp_bl_ts_init();

    return AIC_SUCCESS;
}

AIC_APP_L1_INIT_EXPORT(Tp_Ts_Init);

#endif //USE_BTL6133

