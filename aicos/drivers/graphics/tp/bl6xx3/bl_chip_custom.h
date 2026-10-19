#ifndef BL_CHIP_CUSTOM_H
#define BL_CHIP_CUSTOM_H

#define     TS_CHIP          BL6XX3
#define     BTL_CHECK_CHIPID
#define     CTP_USE_HW_I2C
//#define     CTP_USE_SW_I2C
#ifdef CTP_USE_SW_I2C
#define     CTP_SLAVE_ADDR		(0x2D<<1)
#endif
#ifdef CTP_USE_HW_I2C
#define     CTP_SLAVE_ADDR      (0x2C)//(0x2D)
#endif
#define     GPIO_EINT
#define     RESET_PIN_WAKEUP
//#define     INT_PIN_WAKEUP
#define     BL_UPDATE_FIRMWARE_ENABLE
#define     TPD_RES_X        128
#define     TPD_RES_Y        128
//#define     BL_DEBUG
#endif
