/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software phy of pmic 2nd interrupt module.
 *   It includes all the pmic 2nd int phy common function api.
*/
#ifndef __PMU_INT_H__
#define __PMU_INT_H__

struct pmic_2nd_irq_ops{
    void (*pmic_2nd_irq_enable)(int irq_num);
    void (*pmic_2nd_irq_disable)(int irq_num);
    int (*pmic_1st_irq_num_get)(int irq_num);
    void (*pmic_2nd_irq_disable_all)(void);
    void (*pmic_2nd_irq_clear_all)(void);
    u32 (*pmic_2nd_irq_en_sts_get)(int irq_num);
    int (*pmic_2nd_irq_msk_append)(int irq_msk);
};

void pmic_2nd_irq_ops_register(struct pmic_2nd_irq_ops *ops);
#endif /* __PMU_INT_H__ */

