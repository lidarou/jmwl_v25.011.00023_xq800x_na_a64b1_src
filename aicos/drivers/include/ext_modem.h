/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __EXT_MODEM_H__
#define __EXT_MODEM_H__

#include <aic_type.h>
#include <aic_bitmap.h>

struct ext_modem_abnormal_status {
    unsigned int abnormal_status_n;    /* abnormal status gpio number. */
    unsigned int abnormal_status_iomuxaddr;    /* abnormal status iomux address. */
    unsigned int abnormal_status_iomuxval;    /* abnormal status iomux value. */
    unsigned int abnormal_status_pinaddr;    /* abnormal status pad control address. */
    unsigned int abnormal_status_pinval;    /* abnormal status pad control value. */
};

struct ext_modem_audiopa {
    unsigned int audiopa_en_n;    /* audiopa enable gpio number. */
    unsigned int audiopa_en_iomuxaddr;    /* audiopa enable iomux address. */
    unsigned int audiopa_en_iomuxval;    /* audiopa enable iomux value. */
    unsigned int audiopa_en_pinaddr;    /* audiopa enable pad control address. */
    unsigned int audiopa_en_pinval;    /* audiopa enable pad control value. */
};

struct ext_modem_auto_dl {
    unsigned int autodowload_n;    /* autodowload gpio number. */
    unsigned int autodowload_iomuxaddr;    /* autodowload iomux address. */
    unsigned int autodowload_iomuxval;    /* autodowload iomux value. */
    unsigned int autodowload_pinaddr;    /* autodowload pad control address. */
    unsigned int autodowload_pinval;    /* autodowload pad control value. */
};

struct ext_modem_gpio_wakeup {
    unsigned int wakeup_type;    /* 0: gpio mode;1: eipu mode. */
    unsigned int wakeup_mcu_n;    /* gx wakeup mcu gpio number. */
    unsigned int wakeup_mcu_iomuxaddr;    /* gx wakeup mcu iomux address. */
    unsigned int wakeup_mcu_iomuxval;    /* gx wakeup mcu iomux value. */
    unsigned int wakeup_mcu_pinaddr;    /* gx wake up mcu pad control address. */
    unsigned int wakeup_mcu_pinval;    /* gx wake up mcu pad control value. */
    unsigned int wakeup_gx_n;    /* mcu wakeup gx gpio number. */
    unsigned int wakeup_gx_iomuxaddr;    /* mcu wakeup gx iomux address. */
    unsigned int wakeup_gx_iomuxval;    /* mcu wakeup gx iomux value. */
    unsigned int wakeup_gx_pinaddr;    /* mcu wakeup gx pad control address. */
    unsigned int wakeup_gx_pinval;    /* mcu wakeup gx pad control value. */
};

struct ext_modem_sim_detect {
    unsigned int sim_detect_pin_cfg;    /* 0: nothing 1: SIM detect pin reconfiguration */
    unsigned int sim_detect_iomuxaddr;    /* sim detect iomux address. */
    unsigned int sim_detect_iomuxval;    /* sim detect iomux value. */
    unsigned int sim_detect_pinaddr;    /* sim detect pad control address. */
    unsigned int sim_detect_pinval;    /* sim detect pad control value. */
};

#ifdef CONFIG_DRIVER_EXT_MODEM
struct ext_modem_abnormal_status *ext_modem_get_abnormal_status_cfg(void);
struct ext_modem_audiopa *ext_modem_get_audiopa(void);
struct ext_modem_gpio_wakeup *ext_modem_get_gpio_wakeup_cfg(void);
struct ext_modem_auto_dl *ext_modem_get_autodl_cfg(void);
int ext_modem_flowcontrol_enable(void);
unsigned int ext_modem_get_uart_baudrate(void);
#else
static inline struct ext_modem_abnormal_status *ext_modem_get_abnormal_status_cfg(void) {return NULL;}
static inline struct ext_modem_audiopa *ext_modem_get_audiopa(void) {return NULL;}
static inline struct ext_modem_gpio_wakeup *ext_modem_get_gpio_wakeup_cfg(void) {return NULL;}
static inline struct ext_modem_auto_dl *ext_modem_get_autodl_cfg(void) {return NULL;}
static inline int ext_modem_flowcontrol_enable(void) {return 0;}
unsigned int ext_modem_get_uart_baudrate(void) {return 0;}
#endif

#endif /* __SERIAL_INC_H__ */

