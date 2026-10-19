/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of eipu module.
 *   It includes all the eipu common driver.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x12ab80
#define LOG_MODULE    AMOD_EIPU

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_interrupt.h>
#include <aic_reg_base.h>
#include <irq.h>
#include <aon_glb.h>
#include <eipu.h>
#include "eipu_reg.h"
#include <top_glb.h>
#include <eipu_cfg.h>
#include <aic_io.h>
#include <aic_osa.h>
#include "slp_inc.h"

static BOOL is_eipu_init = FALSE;
static EIPU_CHAN_T s_eipu_int_table[MAX_MODE][ EIPU_CHN_NUM ] = {0};

static void EIPU_REG_AND (u32 reg_addr, u32 value)
{
    AIC_WRITE32(reg_addr, AIC_READ32(reg_addr) & value);
}

static void EIPU_REG_OR (u32 reg_addr, u32 value)
{
    AIC_WRITE32(reg_addr, AIC_READ32(reg_addr) | value);
}

u32 EIPU_GetBase(EIPU_MODE_TYPE_E sec_mode)
{
    u32 base_addr = 0;
    u32 eipu_max = sizeof(eipu_bd_cfg) / sizeof(*eipu_bd_cfg);
    u32 i;

    for (i = 0; i < eipu_max; i++) {
        if (sec_mode == eipu_bd_cfg[i].type) {
            base_addr = eipu_bd_cfg[i].base;
        }
    }

    return base_addr;
}

static u32 EIPU_Getoffset(EIPU_MODE_TYPE_E sec_mode)
{
    u32 offset;

#ifdef CONFIG_SUBSYS_AP
    if (sec_mode == ND_MODE)
        offset = EIPU_ND_TYPE_OFFSET;
    else
        offset = EIPU_NORMAL_TYPE_OFFSET;
#else
    offset = EIPU_NORMAL_TYPE_OFFSET;
#endif

    return offset;
}

static u32 EIPU_Getoffset_channel(EIPU_MODE_TYPE_E sec_mode,u32 chan_id)
{
    u32 offset;
#ifdef CONFIG_SUBSYS_AP
        if (sec_mode == ND_MODE)
            offset = EIPU_ND_DBNC_CHANNEL_CTRL(chan_id);
        else
            offset = EIPU_DBNC_CHANNEL_CTRL(chan_id);
#else
        offset = EIPU_DBNC_CHANNEL_CTRL(chan_id);
#endif

    return offset;
}


void EIPU_DbncEn(u32 base_addr, u32 chan_id, BOOL is_en, EIPU_MODE_TYPE_E sec_mode)
{
    u32 offset,addr;

    offset = EIPU_Getoffset_channel(sec_mode,chan_id);

    addr = base_addr + offset;

    if( is_en )
        EIPU_REG_OR(addr, EIPUCTRL_DBNC_EN);
    else
        EIPU_REG_AND(addr, ~EIPUCTRL_DBNC_EN);
}

void EIPU_SetDbnc(u32 base_addr,u32 chan_id, u32 cnt, EIPU_MODE_TYPE_E sec_mode)
{
    u32 offset,addr;

    offset = EIPU_Getoffset_channel(sec_mode,chan_id);

    addr = base_addr + offset;

    EIPU_REG_AND(addr, ~EIPUCTRL_DBNC_CNT_BITMSK);
    EIPU_REG_OR (addr, cnt);
}

void EIPU_SetTrg(u32 base_addr,
                 u32 chan_id,
                 EIPU_FUNC_TYPE_E eipu_type,
                 EIPU_TRG_TYPE_E trg_type_e,
                 EIPU_MODE_TYPE_E sec_mode)
{
    u32 addr;

    switch( trg_type_e )
    {
        case HIGH_LEVEL:
            if (eipu_type == EIPU_EDGE_32K || eipu_type == EIPU_EDGE) {
                addr = base_addr + EIPU_EDGE_32K_INT_MODE;
                EIPU_REG_OR(addr, (1<<chan_id));
            }
            addr = base_addr + EIPU_INT_POL;
            EIPU_REG_OR(addr, (1<<chan_id));
            break;

        case LOW_LEVEL:
            if (eipu_type == EIPU_EDGE_32K || eipu_type == EIPU_EDGE) {
                addr = base_addr + EIPU_EDGE_32K_INT_MODE;
                EIPU_REG_OR(addr, (u32)(1<<chan_id));
            }
            addr = base_addr + EIPU_INT_POL;
            EIPU_REG_AND(addr, ~(1<<chan_id));
            break;
#ifdef CONFIG_SUBSYS_AP
        case RISING_EDGE:
            if (eipu_type == EIPU_EDGE_32K || eipu_type == EIPU_EDGE) {
                addr = base_addr + EIPU_EDGE_32K_INT_BOTH;
                EIPU_REG_AND(addr, ~(u32)(1<<chan_id));

                addr = base_addr + EIPU_EDGE_32K_INT_MODE;
                EIPU_REG_AND(addr, ~(u32)(1<<chan_id));
                addr = base_addr + EIPU_INT_POL;
                EIPU_REG_OR(addr, (1<<chan_id));
            }
            else
                aicx_info(0x0c49a8, "d", "EIPU type %d not support RISING_EDGE\n",s_eipu_int_table[sec_mode][chan_id].type);
            break;

        case FALLING_EDGE:
            if (eipu_type == EIPU_EDGE_32K || eipu_type == EIPU_EDGE) {
                addr = base_addr + EIPU_EDGE_32K_INT_BOTH;
                EIPU_REG_AND(addr, ~(u32)(1<<chan_id));

                addr = base_addr + EIPU_EDGE_32K_INT_MODE;
                EIPU_REG_AND(addr, ~(u32)(1<<chan_id));
                addr = base_addr + EIPU_INT_POL;
                EIPU_REG_AND(addr, ~(u32)(1<<chan_id));
                }
            else
                aicx_info(0x0b01d4, "d", "EIPU type %d not support FALLING_EDGE\n",s_eipu_int_table[sec_mode][chan_id].type);
            break;

        case BOTH_EDGE:
            if (eipu_type == EIPU_EDGE_32K || eipu_type == EIPU_EDGE) {
                addr = base_addr + EIPU_EDGE_32K_INT_BOTH;
                EIPU_REG_OR(addr, (u32)(1<<chan_id));
                // select edge trig
                addr = base_addr + EIPU_EDGE_32K_INT_MODE;
                EIPU_REG_AND(addr, ~(u32)(1<<chan_id));
                }
            else
                aicx_info(0x01fdc7, "d", "EIPU type %d not support BOTH_EDGE\n",s_eipu_int_table[sec_mode][chan_id].type);
            break;
#endif
        default: break;
    }
}

void EIPU_IntEn(u32 base_addr,
                u32 chan_id,
                BOOL is_en)
{
    u32 addr = base_addr + EIPU_INT_EN;

    if(is_en)
        EIPU_REG_OR(addr, (u32)(1<<chan_id));
    else
        EIPU_REG_AND(addr, ~(u32)(1<<chan_id));
}

void EIPU_IntDisAll(void)
{
    u32 i,j,offset;
    u32 eipu_max = sizeof(eipu_bd_cfg) / sizeof(*eipu_bd_cfg);

    for (j = 0; j < eipu_max; j++) {
        for (i = 0; i<=EIPU_DBNC; i++) {
            offset = EIPU_Getoffset(eipu_bd_cfg[j].type);
            AIC_WRITE32(eipu_bd_cfg[j].base + offset*i + EIPU_INT_EN, 0);
            AIC_WRITE32(eipu_bd_cfg[j].base + offset*i + EIPU_INT_CLR, BIT(eipu_bd_cfg[j].chn) - 1);
        }
    }
}

void EIPU_ClrIntSts(u32 base_addr, u32 chan_id)
{
   u32 addr = base_addr + EIPU_INT_CLR;

   EIPU_REG_OR(addr, (1<<chan_id));
}

void EIPU_StartTrg(u32 base_addr, u32 chan_id)
{
    u32 addr = base_addr + EIPU_DBNC_TRG_CTRL;

    EIPU_REG_OR(addr, (1<<chan_id));
}
int EIPU_FindBit(u32 eiu_index);

#ifdef CONFIG_SUBSYS_AP
int EIPU_UnRegCallback(u32 eiu_index, EIPU_FUNC_TYPE_E eipu_type, EIPU_MODE_TYPE_E sec_mode)
{
    u32 base_addr,offset,chan_id;
    offset = EIPU_Getoffset(sec_mode);
    base_addr = EIPU_GetBase(sec_mode);
    chan_id = EIPU_FindBit(eiu_index);

    if (base_addr)
        base_addr += offset * eipu_type;
    EIPU_IntEn( base_addr, chan_id, FALSE );

    return AIC_SUCCESS;
}
#else
int EIPU_UnRegCallback(u32 chan_id, EIPU_FUNC_TYPE_E eipu_type, EIPU_MODE_TYPE_E sec_mode)
{
    u32 base_addr,offset;
    offset = EIPU_Getoffset(sec_mode);
    base_addr = EIPU_GetBase(sec_mode);
    if (base_addr)
        base_addr += offset * eipu_type;
    EIPU_IntEn( base_addr, chan_id, FALSE );

    return AIC_SUCCESS;
}
#endif

EIPU_PIN_T s_eipu[] = {
    {0,0},//nd bit0
    {1,0},//aon bit0
    {2,1},//nd bit1
    {3,2},//nd bit2
    {4,1},//aon bit1
    {5,2},//aon bit2
    {6,3},//aon bit3
    {7,4},//aon bit4
    {8,5},//aon bit5
};

int EIPU_FindBit(u32 eiu_index)
{
    u32 i,chan_id = 0xff;
    u32 eipu_max = sizeof(s_eipu) / sizeof(*s_eipu);

    for (i=0; i<eipu_max; i++) {
        if (eiu_index == s_eipu[i].eiu_pinnum) {
            chan_id = s_eipu[i].eiu_ctrlnum;
            break;
        }
    }

    return chan_id;
}
#ifdef CONFIG_SUBSYS_AP
int EIPU_FindMode(u32 eiu_index)
{
    EIPU_MODE_TYPE_E sec_mode = NOSEC_MODE;

    switch (eiu_index)
    {
        case 0:
        case 2:
        case 3:
            sec_mode = ND_MODE;
            break;
        case 1:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            sec_mode = NOSEC_MODE;
            break;
        default:
            AIC_ASSERT(0);
            break;
    }

    return sec_mode;
}


int EIPU_FindIndex(u32 eiu_index, EIPU_MODE_TYPE_E sec_mode)
{
    u32 pin_index = 0;
    if (sec_mode == NOSEC_MODE) {
        switch (eiu_index)
        {
            case 0:
                pin_index = 1;
                break;
            case 1:
                pin_index = 4;
                break;
            case 2:
                pin_index = 5;
                break;
            case 3:
                pin_index = 6;
                break;
            case 4:
                pin_index = 7;
                break;
            case 5:
                pin_index = 8;
                break;
            default:
                AIC_ASSERT(0);
                break;
        }
    }

    else if (sec_mode == ND_MODE) {
        switch (eiu_index)
        {
            case 0:
                pin_index = 0;
                break;
            case 1:
                pin_index = 2;
                break;
            case 2:
                pin_index = 3;
                break;
            default:
                AIC_ASSERT(0);
                break;
        }
    }

    return pin_index;
}
#endif

int EIPU_RegCallback(u32 eiu_index,
                     EIPU_FUNC_TYPE_E eipu_type,
                     EIPU_CALLBACK callback,
                     EIPU_TRG_TYPE_E trg_type_e,
                     u32 dbnc_cnt_ms,
                     EIPU_MODE_TYPE_E sec_mode)
{
    u32 base_addr,offset,chan_id;
    u32 result = AIC_ENOENT;
    u32 chn = 0;
    u32 eipu_max = sizeof(eipu_bd_cfg) / sizeof(*eipu_bd_cfg);

#ifdef CONFIG_SUBSYS_AP
    sec_mode = EIPU_FindMode(eiu_index);

    if (eipu_max == 1) {
        if (sec_mode == ND_MODE)
        chn = eipu_bd_cfg[0].chn;
    }
    else if (eipu_max == 2) {
        if (sec_mode == ND_MODE)
            chn = eipu_bd_cfg[0].chn;
        else if (sec_mode == NOSEC_MODE)
            chn = eipu_bd_cfg[1].chn;
    }
#else
    if (eipu_max == 1) {
        if ((sec_mode == CP_MODE))
            chn = eipu_bd_cfg[0].chn;
    }
#endif

    offset = EIPU_Getoffset(sec_mode);

    chan_id = EIPU_FindBit(eiu_index);

    if(chan_id >= chn) {
        aicx_info(0x174418, "dd", "EIPU chan_id = %d exceed %d\n", chan_id, chn);
        return result;
    }
    else {
        base_addr = EIPU_GetBase(sec_mode);
        if (base_addr)
            base_addr += offset * eipu_type;
        else {
            aicx_info(0x192c85, "", "EIPU base_addr is 0 !!!\n", chan_id, base_addr);
            return result;
        }
        if(s_eipu_int_table[sec_mode][chan_id].callback == NULL) {
            s_eipu_int_table[sec_mode][chan_id].callback = callback;
            s_eipu_int_table[sec_mode][chan_id].type = eipu_type;
        }
    }

    switch( eipu_type )
    {
        case EIPU_DBNC:
            EIPU_SetTrg(base_addr, chan_id, eipu_type, trg_type_e, sec_mode);
            EIPU_SetDbnc(base_addr, chan_id, dbnc_cnt_ms, sec_mode);
            if (dbnc_cnt_ms == 0)
                EIPU_DbncEn(base_addr, chan_id, FALSE, sec_mode);
            EIPU_ClrIntSts(base_addr, chan_id);
            aic_udelay(100);
            EIPU_IntEn(base_addr, chan_id, TRUE);

            if (dbnc_cnt_ms != 0)
                EIPU_StartTrg(base_addr, chan_id);
            break;
#ifdef CONFIG_SUBSYS_AP
        case EIPU_LATCH:
        case EIPU_EDGE_32K:
        case EIPU_EDGE:
            EIPU_SetTrg(base_addr,chan_id,eipu_type,trg_type_e,sec_mode);
            EIPU_ClrIntSts(base_addr, chan_id);
            aic_udelay(100);
            EIPU_IntEn(base_addr,chan_id, TRUE);
            break;
#endif
        default:
            break;
    }

    return AIC_SUCCESS;
}


int EIPU_RegCallback_Ex(u32 eiu_index,
                     EIPU_FUNC_TYPE_E eipu_type,
                     EIPU_CALLBACK callback,
                     EIPU_TRG_TYPE_E trg_type_e,
                     u32 dbnc_cnt_ms,
                     EIPU_MODE_TYPE_E sec_mode,
                     EIPU_WORK_MODE_E work_mode)
{
    u32 result = AIC_ENOENT;
    u32 chan_id;

#ifdef CONFIG_SUBSYS_AP
    sec_mode = EIPU_FindMode(eiu_index);
#endif
    chan_id = EIPU_FindBit(eiu_index);
    s_eipu_int_table[sec_mode][chan_id].wk_mode = work_mode;

    result = EIPU_RegCallback(eiu_index,eipu_type,callback,trg_type_e,dbnc_cnt_ms,sec_mode);

    return result;
}

u32 EIPU_GetMskIntSts(u32 base,u32 chan_id,EIPU_FUNC_TYPE_E * ptr_eipu_type, EIPU_MODE_TYPE_E sec_mode)
{
    u32 addr,offset,value = 0;

    if (s_eipu_int_table[sec_mode][chan_id].callback == NULL) {
        return value;
    }
    offset = EIPU_Getoffset(sec_mode);

    addr = offset*s_eipu_int_table[sec_mode][chan_id].type;
    addr += EIPU_INT_MSKSTS;
    addr += base;

    *ptr_eipu_type = s_eipu_int_table[sec_mode][chan_id].type;

    value = (AIC_READ32(addr) & (1<<chan_id));

    return value;
}

u32 EIPU_GetData(u32 base_addr, u32 chan_id)
{
    return AIC_READ32(base_addr + EIPU_INT_INDATA) & (1<<chan_id);
}

u32 EIPU_GetIntMode(u32 base_addr, u32 chan_id)
{
    return AIC_READ32(base_addr + EIPU_EDGE_32K_INT_MODE) & (1<<chan_id);
}

u32 EIPU_GetBoth(u32 base_addr, u32 chan_id)
{
    return AIC_READ32(base_addr + EIPU_EDGE_32K_INT_BOTH) & (1<<chan_id);
}

u32 EIPU_GetPol(u32 base_addr, u32 chan_id)
{
    return AIC_READ32(base_addr + EIPU_INT_POL) & (1<<chan_id);
}

u32 EIPU_GetDbncEn(u32 base_addr, u32 chan_id, EIPU_MODE_TYPE_E sec_mode)
{
    u32 offset,addr;

    offset = EIPU_Getoffset_channel(sec_mode,chan_id);

    addr = base_addr + offset;

    return AIC_READ32(addr) & EIPUCTRL_DBNC_EN;
}

u32 EIPU_GetRaw(u32 base_addr, u32 chan_id)
{
    return AIC_READ32(base_addr + EIPU_INT_RAWSTS) & (1<<chan_id);
}

EIPU_MODE_TYPE_E EIPU_GetMode(IRQn_Type irq_num)
{
    EIPU_MODE_TYPE_E sec_mode = 0;
    u32 eipu_max = sizeof(eipu_bd_cfg) / sizeof(*eipu_bd_cfg);
    u32 i;

    for (i = 0; i < eipu_max; i++) {
        if (irq_num == eipu_bd_cfg[i].irq_no) {
            sec_mode = eipu_bd_cfg[i].type;
        }
    }

    return sec_mode;
}

#ifdef CONFIG_SUBSYS_AP
u32 EIPU_GetIndata_Ex(u32 pin_idx)
{
    return EIPU_GetIndata(0, 0, pin_idx);
}
#endif

u32 EIPU_GetIndata(EIPU_MODE_TYPE_E sec_mode,EIPU_FUNC_TYPE_E eipu_type,u32 pin_idx)
{
    u32         offset;
#ifdef CONFIG_SUBSYS_AP
    u32         chn_id;

    sec_mode = EIPU_FindMode(pin_idx);
    chn_id = EIPU_FindBit(pin_idx);
    offset = EIPU_Getoffset(sec_mode);

    return EIPU_GetData(EIPU_GetBase(sec_mode) + offset * eipu_type,chn_id);
#else
    offset = EIPU_Getoffset(sec_mode);

    return EIPU_GetData(EIPU_GetBase(sec_mode) + offset * eipu_type,pin_idx);
#endif
}

void EIPU_SetPol(EIPU_MODE_TYPE_E sec_mode,EIPU_FUNC_TYPE_E eipu_type,u32 pin_idx,u32 pol_val)
{
    u32         offset;
#ifdef CONFIG_SUBSYS_AP
    u32         chn_id;

    sec_mode = EIPU_FindMode(pin_idx);
    chn_id = EIPU_FindBit(pin_idx);
    offset = EIPU_Getoffset(sec_mode);

    if (pol_val)
        EIPU_REG_OR(EIPU_GetBase(sec_mode) +  offset * eipu_type + EIPU_INT_POL, (1<<chn_id));
    else
        EIPU_REG_AND(EIPU_GetBase(sec_mode) +  offset * eipu_type + EIPU_INT_POL, ~(u32)(1<<chn_id));
#else
    offset = EIPU_Getoffset(sec_mode);

    if (pol_val)
        EIPU_REG_OR(EIPU_GetBase(sec_mode) +  offset * eipu_type + EIPU_INT_POL, (1<<pin_idx));
    else
        EIPU_REG_AND(EIPU_GetBase(sec_mode) +  offset * eipu_type + EIPU_INT_POL, ~(u32)(1<<pin_idx));
#endif
}

ISR_RET_E EIPU_Isr(void *int_ptr)
{
    u32         i,offset;
    u32         state = 0;
    u32         base_addr;
    EIPU_FUNC_TYPE_E eipu_type;
    u32         eipu_intmode;
    u32         eipu_both;
    u32         eipu_pol;
    u32         base_secmode;
    EIPU_MODE_TYPE_E eipu_secmode;
    IRQn_Type   int_num  =*(IRQn_Type *)int_ptr;
#ifdef CONFIG_SUBSYS_AP
    u32         pin_idx;
#endif
    eipu_secmode = EIPU_GetMode(int_num);

    if (eipu_secmode >= MAX_MODE)
        AIC_ASSERT(0);

    base_secmode = EIPU_GetBase(eipu_secmode);

    offset = EIPU_Getoffset(eipu_secmode);

    for( i=0; i<EIPU_CHN_NUM; i++ )
    {
        // get channel's mask state
        if( EIPU_GetMskIntSts(base_secmode, i, &eipu_type,eipu_secmode) )
        {
            base_addr = base_secmode+offset*eipu_type;

            // disable int to avoid interrupt continus
            EIPU_IntEn( base_addr, i, FALSE );

            if( s_eipu_int_table[eipu_secmode][i].callback != NULL )
            {
                state =  EIPU_GetData(base_addr, i);
                eipu_intmode = EIPU_GetIntMode(base_addr, i);
                eipu_both = EIPU_GetBoth(base_addr, i);
                eipu_pol = EIPU_GetPol(base_addr, i);
#ifdef CONFIG_SUBSYS_AP
                pin_idx = EIPU_FindIndex(i, eipu_secmode);
                (*s_eipu_int_table[eipu_secmode][i].callback)(pin_idx, state, eipu_pol);
#else
                (*s_eipu_int_table[eipu_secmode][i].callback)(i, state, eipu_pol);
#endif
            }
            else
                aicx_info(0x011355, "dd", "s_eipu_int_table[%d][%d] is null!!\n",eipu_secmode,i);

            // clear int flag to wait the next interrupt
            EIPU_ClrIntSts(base_addr, i);

            // dbnc bypass mode:wait eipuinput sync 2 32k tick
            if((eipu_type == EIPU_DBNC) && !EIPU_GetDbncEn(base_addr,i,eipu_secmode))
                aic_udelay(100);

            // enable int for next trig
            if (s_eipu_int_table[eipu_secmode][i].wk_mode == BOTH_WK_MODE)
                EIPU_IntEn(base_addr, i, TRUE);

            // start trig signal in dbnc mode
            if((eipu_type == EIPU_DBNC) && EIPU_GetDbncEn(base_addr,i,eipu_secmode))
                EIPU_StartTrg(base_addr, i);

            break;
        }
    }

    return NONE_WORKER;
}

void EIPU_SetSel(u32 chn_id,u32 chn_sel)
{
    AIC_WRITE32(REG_AON_GLB_CP_EIPU_IN0_SEL, chn_sel);
}

void EIPU_Open(void)
{
    u32 eipu_max = sizeof(eipu_bd_cfg) / sizeof(*eipu_bd_cfg);
    u32 i;

    for (i = 0; i < eipu_max; i++) {
        register_irq(eipu_bd_cfg[i].irq_no, EIPU_Isr, (void *)NULL, NULL, NULL);
        AIC_WRITE32(eipu_bd_cfg[i].aon_eb, 1);
    }
}

//#define DEBUG_EIPU
#ifdef DEBUG_EIPU
static void eipu_callback(u32 index, u32 value, u32 eipu_pol)
{
    u32 indata;
    indata = EIPU_GetIndata(NOSEC_MODE, EIPU_DBNC, index);

    EIPU_SetPol(NOSEC_MODE, EIPU_DBNC, index, !indata);

    aicx_info(0x14de0a, "xx", "EIPU indata 0x%x,index 0x%x\n", indata, index);

    return;
}
static void eipu_callback_nd(u32 index, u32 value, u32 eipu_pol)
{
    u32 indata;
    indata = EIPU_GetIndata(ND_MODE, EIPU_DBNC, index);

    EIPU_SetPol(ND_MODE, EIPU_DBNC, index, !indata);

    aicx_info(0x1bb853, "xx", "EIPU nd indata 0x%x,index 0x%x\n", indata, index);

    return;
}
#endif

int EIPU_Init(void)
{
    if(!is_eipu_init)
    {
        is_eipu_init = TRUE;
        EIPU_Open();
    }

#ifdef DEBUG_EIPU
    //EIPU_RegCallback(EXTINT_CP, EIPU_EDGE_32K, eipu_callback, FALLING_EDGE, 0, NOSEC_MODE);
    EIPU_RegCallback(4, EIPU_DBNC, eipu_callback, LOW_LEVEL, 0x20, NOSEC_MODE); //single key
    EIPU_RegCallback(0, EIPU_DBNC, eipu_callback_nd, LOW_LEVEL, 0x20, ND_MODE); //uboot key
#endif
    return 0;
}

int EIPU_Reset(void)
{
    u32 eipu_max = sizeof(eipu_bd_cfg) / sizeof(*eipu_bd_cfg);
    u32 i;

    for (i = 0; i < eipu_max; i++) {
        AIC_WRITE32(eipu_bd_cfg[i].rst, 1);
        AIC_WRITE32(eipu_bd_cfg[i].rst, 0);
        aic_udelay(100);
    }

    is_eipu_init = FALSE;
    memset(s_eipu_int_table, 0, MAX_MODE*EIPU_CHN_NUM*sizeof(EIPU_CHAN_T));
    EIPU_IntDisAll();
    EIPU_Init();

    return 0;
}

AIC_DRV_P1_INIT_EXPORT(EIPU_Reset);
