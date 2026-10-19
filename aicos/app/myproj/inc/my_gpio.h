#ifndef __MY_GPIO_H__
#define __MY_GPIO_H__

#define GPIO_INPUT      0
#define GPIO_OUTPUT     1

#define GPIO_DEFAULT    0
#define GPIO_PULLUP     1
#define GPIO_PULLDN     2


// NOTE: 做输出的GPIO口调用读会导致系统复位
#define JM_GpioGet aic_gpio_level_get
#define JM_GpioSet aic_gpio_output

typedef struct
{
    uint8     dir;
    uint8     pull;
    uint8     level;
} GPIO_CFG_S;

typedef struct
{
    int             gpioId;
    uint8           func;   // not used
    GPIO_CFG_S      gpioCfg;
} MY_GPIO_CONFIG_STRUCT;


int my_gpio_init(void);

#endif

