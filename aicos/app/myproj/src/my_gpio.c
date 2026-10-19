/********************************************************************
**版权所有         深圳市几米物联有限公司
**文件名称:        my_gpio.c
**文件描述:        设备平台GPIO初始化模块
**当前版本:        V1.0
**作    者:        张庆灿(Nestor Zhang) zhangqingcan@jimiiot.com
**完成日期:        2020.02.29
*********************************************************************/
#include "my_common.h"


MY_GPIO_CONFIG_STRUCT my_gpio_table[] =
{
    // GPIO26
    {MY_GPS_POWER,   0, {GPIO_OUTPUT,  GPIO_DEFAULT, 0} },  // GPS LDO EN

    {GPIO_PIN_MAX, 0, {GPIO_OUTPUT, GPIO_DEFAULT, 0} },
};


void my_gpio_init_id(int gpioId, uint8 dir, uint8 pull, uint8 level)
{
    if (dir == GPIO_OUTPUT)
    {
        if (level)
        {
            aic_gpio_output(gpioId, GPIO_OUTPUT_LVL_HIGH);
        } else {
            aic_gpio_output(gpioId, GPIO_OUTPUT_LVL_LOW);
        }
    }
    else if (dir == GPIO_INPUT)
    {
        // TODO 中断触发单独再配置
        aic_gpio_input(gpioId, GPIO_INPUT_NOINT, NULL, NULL);
    }
}


/*
 * Init all GPIOs what we need.
 */
int my_gpio_init(void)
{
    MY_GPIO_CONFIG_STRUCT *pCfg = my_gpio_table;

    while (pCfg->gpioId != GPIO_PIN_MAX)
    {
        my_gpio_init_id(pCfg->gpioId, pCfg->gpioCfg.dir,
                        pCfg->gpioCfg.pull, pCfg->gpioCfg.level);

        pCfg++;
    }

    return 0;
}

