/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AIC OSA configurations.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17efb1
#define LOG_MODULE    AMOD_SYS

#include <aic_osa.h>
#include <aos_mem.h>
#include <aos_thread.h>
#include <aos_config.h>
#include <aic_assert.h>
#ifndef WIN32_PLATFORM
#include "ps_pub.h"
#endif

#ifdef WIN32_PLATFORM
/* demo thread1 */
#define AOS_DEMO_THREAD1_ID          16
#define AOS_DEMO_THREAD1_PRI         16
#define AOS_DEMO_STACK1_SIZE         (1*1024)
#define AOS_DEMO_INTQ1_NUM           8
#define AOS_DEMO_EXTQ1_NUM           8
#define AOS_DEMO_THREAD1_NAME        "demo_thread1"
aos_status demo_thread1_init(void);
aos_status demo_thread1_main(aos_fsm_msg *fsm_msg_ptr);
aos_status demo_thread1_reset(void);
aos_status demo_thread1_delete(void);

/* demo thread1 */
#define AOS_DEMO_THREAD2_ID          17
#define AOS_DEMO_THREAD2_PRI         17
#define AOS_DEMO_STACK2_SIZE         (2*1024)
#define AOS_DEMO_INTQ2_NUM           16
#define AOS_DEMO_EXTQ2_NUM           16
#define AOS_DEMO_THREAD2_NAME        "demo_thread2"
aos_status demo_thread2_init(void);
aos_status demo_thread2_main(aos_fsm_msg *fsm_msg_ptr);
aos_status demo_thread2_reset(void);
aos_status demo_thread2_delete(void);

#else
aos_status g_fas_mc_main(aos_fsm_msg *msg_ptr);
aos_status g_fas_mc_init(void);
aos_status g_fas_mc_reset(void);
aos_status g_fas_mc_deinit(void);

aos_status g_ferrc_main(aos_fsm_msg *msg_ptr);
aos_status g_ferrc_init(void);
aos_status g_ferrc_reset(void);
aos_status g_ferrc_delete(void);

aos_status g_fl2_dl_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fl2_dl_init(void);
aos_status g_fl2_dl_reset(void);
aos_status g_fl2_dl_delete(void);

aos_status g_fl2_ul_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fl2_ul_init(void);
aos_status g_fl2_ul_reset(void);
aos_status g_fl2_ul_delete(void);

aos_status g_fl2_ul_data_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fl2_ul_data_init(void);
aos_status g_fl2_ul_data_reset(void);
aos_status g_fl2_ul_data_delete(void);

aos_status g_fel1a_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fel1a_init(void);
aos_status g_fel1a_reset(void);
aos_status g_fel1a_delete(void);

aos_status g_fuai_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fuai_init(void);
aos_status g_fuai_reset(void);
aos_status g_fuai_delete(void);

aos_status g_fspv_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fspv_init(void);
aos_status g_fspv_reset(void);
aos_status g_fspv_delete(void);

aos_status g_fipcc_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fipcc_init(void);
aos_status g_fipcc_reset(void);
aos_status g_fipcc_delete(void);

aos_status g_fatm_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fatm_init(void);
aos_status g_fatm_reset(void);
aos_status g_fatm_delete(void);

aos_status g_fnas_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fnas_init(void);
aos_status g_fnas_reset(void);
aos_status g_fnas_delete(void);

aos_status g_ftest_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_ftest_init(void);
aos_status g_ftest_reset(void);
aos_status g_ftest_delete(void);

aos_status g_fsim_mma_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fsim_mma_init(void);

aos_status g_feNB_mac_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_feNB_mac_init(void);
aos_status g_feNB_mac_reset(void);
aos_status g_feNB_mac_delete(void);

aos_status g_feNB_rlc_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_feNB_rlc_init(void);
aos_status g_feNB_rlc_reset(void);
aos_status g_feNB_rlc_delete(void);

aos_status g_feNB_pdcp_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_feNB_pdcp_init(void);
aos_status g_feNB_pdcp_reset(void);
aos_status g_feNB_pdcp_delete(void);

aos_status g_feNB_rrc_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_feNB_rrc_init(void);
aos_status g_feNB_rrc_reset(void);
aos_status g_feNB_rrc_delete(void);

aos_status g_f_DH_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_f_DH_init(void);
aos_status g_f_DH_reset(void);
aos_status g_f_DH_delete(void);

aos_status g_fnet_mc_main(aos_fsm_msg * fsm_msg_ptr);
aos_status g_fnet_mc_init(void);
aos_status g_fnet_mc_reset(void);
aos_status g_fnet_mc_delete(void);

aos_status net_main(void);
aos_status net_init(void);
aos_status net_reset(void);
aos_status net_delete(void);

aos_status ue_main(aos_fsm_msg * fsm_msg_ptr);
aos_status ue_init(void);
aos_status ue_reset(void);
aos_status ue_delete(void);

aos_status l1c_lte_hmp_main(aos_fsm_msg * fsm_msg_ptr);
u32 l1c_lte_hmp_init(void);

void l1c_lte_sfe_main(aos_fsm_msg *stp_msg);
u32 l1c_lte_sfe_init(aos_fsm_msg *stp_msg);

void l1c_lte_dp_main(aos_fsm_msg *stp_msg);
u32 l1c_lte_dp_init(aos_fsm_msg *stp_msg);

void l1c_lte_l1pp_main(aos_fsm_msg *stp_msg);
u32 l1c_lte_l1pp_init(aos_fsm_msg *stp_msg);

void l1c_lte_up_main(aos_fsm_msg *stp_msg);
u32 l1c_lte_up_init(aos_fsm_msg *stp_msg);

void l1c_lte_l1rsp_main(aos_fsm_msg *stp_msg);
u32 l1c_lte_l1rsp_init(aos_fsm_msg *stp_msg);

aos_status g_fpfstub_main(aos_fsm_msg *fsm_msg_ptr);
aos_status g_fpfstub_init();

aos_status sim_it_main(aos_fsm_msg *fsm_msg_ptr);

aos_status ims_core_main(aos_fsm_msg * fsm_msg_ptr);
aos_status ims_core_init(void);
aos_status ims_core_reset(void);
aos_status ims_core_delete(void);

aos_status aRtp_main(aos_fsm_msg * fsm_msg_ptr);
aos_status aRtp_init(void);
aos_status aRtp_reset(void);
aos_status aRtp_delete(void);

aos_status dummy_init(void)
{
    return 0;
}
#endif

/* standard thread config table */
aos_thread_config g_thread_config_tbl[] =
{
#ifdef WIN32_PLATFORM
#if 0
    {   AOS_TRUE,
        AOS_DEMO_THREAD1_ID,
        AOS_DEMO_THREAD1_PRI,
        AOS_DEMO_STACK1_SIZE,
        AOS_DEMO_INTQ1_NUM,
        AOS_DEMO_EXTQ1_NUM,
        AOS_DEMO_THREAD1_NAME,
        {demo_thread1_init,demo_thread1_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,demo_thread1_reset,demo_thread1_delete
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_DEMO_THREAD2_ID,
        AOS_DEMO_THREAD2_PRI,
        AOS_DEMO_STACK2_SIZE,
        AOS_DEMO_INTQ2_NUM,
        AOS_DEMO_EXTQ2_NUM,
        AOS_DEMO_THREAD2_NAME,
        {demo_thread2_init,demo_thread2_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,demo_thread2_reset,demo_thread2_delete
        #endif
        }
    },
#endif
#else
    {   AOS_TRUE,
        PF_Stub_TASK_ID,
        PF_Stub_TASK_PRI,
        PF_Stub_TASK_STACK_SIZE,
        PF_Stub_TASK_INT_Q_NUM,
        PF_Stub_TASK_EXT_Q_NUM,
        "PF_STUB",
        {g_fpfstub_init, g_fpfstub_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,dummy_init, dummy_init
        #endif
        }
    },

    {   AOS_TRUE,
        NET_TASK_ID,
        NET_TASK_PRI,
        NET_TASK_STACK_SIZE,
        NET_TASK_INT_Q_NUM,
        NET_TASK_EXT_Q_NUM,
        "NET",
        {net_init, dummy_init
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,dummy_init, dummy_init
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_TEST_THREAD_ID,
        AOS_TEST_THREAD_PRI,
        AOS_TEST_THREAD_STACK_SIZE,
        AOS_TEST_THREAD_INT_Q_NUM,
        AOS_TEST_THREAD_EXT_Q_NUM,
        "TEST",
        {g_ftest_init, g_ftest_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_ftest_reset, g_ftest_delete
        #endif
        }
    },

    {   AOS_TRUE,
        UE_TASK_ID,
        UE_TASK_PRI,
        UE_TASK_STACK_SIZE,
        UE_TASK_INT_Q_NUM,
        UE_TASK_EXT_Q_NUM,
        "UE",
        {ue_init, ue_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,ue_reset, ue_delete
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_ASMC_THREAD_ID,
        AOS_ASMC_THREAD_PRI,
        AOS_ASMC_THREAD_STACK_SIZE,
        AOS_ASMC_THREAD_INT_Q_NUM,
        AOS_ASMC_THREAD_EXT_Q_NUM,
        "ASMC",
        {g_fas_mc_init, g_fas_mc_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fas_mc_reset, g_fas_mc_deinit
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_ATM_THREAD_ID,
        AOS_ATM_THREAD_PRI,
        AOS_ATM_THREAD_STACK_SIZE,
        AOS_ATM_THREAD_INT_Q_NUM,
        AOS_ATM_THREAD_EXT_Q_NUM,
        "ATM",
        {g_fatm_init, g_fatm_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fatm_reset, g_fatm_delete
        #endif
        }
    },

    {   AOS_FALSE,
        AOS_L2UL_THREAD_ID,
        AOS_L2UL_THREAD_PRI,
        AOS_L2UL_THREAD_STACK_SIZE,
        AOS_L2UL_THREAD_INT_Q_NUM,
        AOS_L2UL_THREAD_EXT_Q_NUM,
        "L2_UL",
        {g_fl2_ul_init, g_fl2_ul_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
        ,g_fl2_ul_reset, g_fl2_ul_delete
        #endif
        }
    },

    {   AOS_FALSE,
        AOS_EL2UL_DATA_THREAD_ID,
        AOS_L2UL_DATA_THREAD_PRI,
        AOS_L2UL_DATA_THREAD_STACK_SIZE,
        AOS_L2UL_DATA_THREAD_INT_Q_NUM,
        AOS_L2UL_DATA_THREAD_EXT_Q_NUM,
        "L2_UL_DATA",
        {g_fl2_ul_data_init, g_fl2_ul_data_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fl2_ul_data_reset, g_fl2_ul_data_delete
        #endif
        }
    },

    {   AOS_FALSE,
        AOS_L2DL_THREAD_ID,
        AOS_L2DL_THREAD_PRI,
        AOS_L2DL_THREAD_STACK_SIZE,
        AOS_L2DL_THREAD_INT_Q_NUM,
        AOS_L2DL_THREAD_EXT_Q_NUM,
        "L2_DL",
        {g_fl2_dl_init, g_fl2_dl_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fl2_dl_reset, g_fl2_dl_delete
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_UAI_THREAD_ID,
        AOS_UAI_THREAD_PRI,
        AOS_UAI_THREAD_STACK_SIZE,
        AOS_UAI_THREAD_INT_Q_NUM,
        AOS_UAI_THREAD_EXT_Q_NUM,
        "UAI",
        {g_fuai_init, g_fuai_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fuai_reset, g_fuai_delete
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_MMA_THREAD_ID,
        AOS_MMA_THREAD_PRI,
        AOS_MMA_THREAD_STACK_SIZE,
        AOS_MMA_THREAD_INT_Q_NUM,
        AOS_MMA_THREAD_EXT_Q_NUM,
        "MMA",
        {g_fspv_init, g_fspv_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fspv_reset, g_fspv_delete
        #endif
        }
    },

    {   AOS_FALSE,
        AOS_IPCC_THREAD_ID,
        AOS_IPCC_THREAD_PRI,
        AOS_IPCC_THREAD_STACK_SIZE,
        AOS_IPCC_THREAD_INT_Q_NUM,
        AOS_IPCC_THREAD_EXT_Q_NUM,
        "IPCC",
        {g_fipcc_init, g_fipcc_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fipcc_reset, g_fipcc_delete
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_NAS_THREAD_ID,
        AOS_NAS_THREAD_PRI,
        AOS_NAS_THREAD_STACK_SIZE,
        AOS_NAS_THREAD_INT_Q_NUM,
        AOS_NAS_THREAD_EXT_Q_NUM,
        "NAS",
        {g_fnas_init, g_fnas_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
        ,g_fnas_reset, g_fnas_delete
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_ERRC_THREAD_ID,
        AOS_ERRC_THREAD_PRI,
        AOS_ERRC_THREAD_STACK_SIZE,
        AOS_ERRC_THREAD_INT_Q_NUM,
        AOS_ERRC_THREAD_EXT_Q_NUM,
        "ERRC",
        {g_ferrc_init, g_ferrc_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_ferrc_reset, g_ferrc_delete
        #endif
        }
    },

    {   AOS_TRUE,
        eNB_MAC_TASK_ID,
        eNB_MAC_TASK_PRI,
        eNB_MAC_TASK_STACK_SIZE,
        eNB_MAC_TASK_INT_Q_NUM,
        eNB_MAC_TASK_EXT_Q_NUM,
        "eNB_MAC",
        {g_feNB_mac_init, g_feNB_mac_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_feNB_mac_reset, g_feNB_mac_delete
        #endif
        }
    },

    {   AOS_TRUE,
        eNB_RLC_TASK_ID,
        eNB_RLC_TASK_PRI,
        eNB_RLC_TASK_STACK_SIZE,
        eNB_RLC_TASK_INT_Q_NUM,
        eNB_RLC_TASK_EXT_Q_NUM,
        "eNB_RLC",
        {g_feNB_rlc_init, g_feNB_rlc_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_feNB_rlc_reset, g_feNB_rlc_delete
        #endif
        }
    },

    {   AOS_TRUE,
        eNB_PDCP_TASK_ID,
        eNB_PDCP_TASK_PRI,
        eNB_PDCP_TASK_STACK_SIZE,
        eNB_PDCP_TASK_INT_Q_NUM,
        eNB_PDCP_TASK_EXT_Q_NUM,
        "eNB_PDCP",
        {g_feNB_pdcp_init, g_feNB_pdcp_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_feNB_pdcp_reset, g_feNB_pdcp_delete
        #endif
        }
    },

    {   AOS_TRUE,
        NET_LTE_L3_TASK_ID,
        NET_LTE_L3_TASK_PRI,
        NET_LTE_L3_TASK_STACK_SIZE,
        NET_LTE_L3_TASK_INT_Q_NUM,
        NET_LTE_L3_TASK_EXT_Q_NUM,
        "NET_LTE_L3",
        {g_feNB_rrc_init, g_feNB_rrc_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_feNB_rrc_reset, g_feNB_rrc_delete
        #endif
        }
    },

    {   AOS_TRUE,
        DATA_HANDLE_TASK_ID,
        DATA_HANDLE_TASK_PRI,
        DATA_HANDLE_TASK_STACK_SIZE,
        DATA_HANDLE_TASK_INT_Q_NUM,
        DATA_HANDLE_TASK_EXT_Q_NUM,
        "DATA_HANDLE",
        {g_f_DH_init, g_f_DH_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_f_DH_reset, g_f_DH_delete
        #endif
        }
    },

    {   AOS_TRUE,
        NET_MC_TASK_ID,
        NET_MC_TASK_PRI,
        NET_MC_TASK_STACK_SIZE,
        NET_MC_TASK_INT_Q_NUM,
        NET_MC_TASK_EXT_Q_NUM,
        "NET_MC",
        {g_fnet_mc_init, g_fnet_mc_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,g_fnet_mc_reset, g_fnet_mc_delete
        #endif
        }
    },

#ifdef CONFIG_COMM_IMS
    {   AOS_TRUE,
        AOS_IMS_CORE_THREAD_ID,
        AOS_IMS_CORE_THREAD_PRI,
        AOS_IMSCORE_THREAD_STACK_SIZE,
        AOS_IMSCORE_THREAD_INT_Q_NUM,
        AOS_IMSCORE_THREAD_EXT_Q_NUM,
        "IMS_CORE",
        {ims_core_init, ims_core_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,ims_core_reset, ims_core_delete
        #endif
        }
    },

    {   AOS_TRUE,
        AOS_IMS_ARTP_THREAD_ID,
        AOS_IMS_ARTP_THREAD_PRI,
        AOS_IMSARTP_THREAD_STACK_SIZE,
        AOS_IMSARTP_THREAD_INT_Q_NUM,
        AOS_IMSARTP_THREAD_EXT_Q_NUM,
        "IMS_aRTP",
        {aRtp_init, aRtp_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,aRtp_reset, aRtp_delete
        #endif
        }
    },
#endif

#ifdef PHY_IT_TEST
    {   AOS_FALSE,
        L1C_LTE_HMP_TASK_ID,
        L1C_HMP_TASK_PRI, L1C_HMP_TASK_STACK,
        L1C_TASK_INT_Q_NUM,
        L1C_HMP_TASK_EXT_Q_NUM,
        "L1C_HMP",
        {l1c_lte_hmp_init, l1c_lte_hmp_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
        ,dummy_init, dummy_init
        #endif
        }
    },

    {   AOS_FALSE,
        L1C_LTE_SFE_TASK_ID,
        L1C_SFE_TASK_PRI,
        L1C_SFE_TASK_STACK,
        L1C_TASK_INT_Q_NUM,
        L1C_SFE_TASK_EXT_Q_NUM,
        "L1C_SFE",
        {l1c_lte_sfe_init, l1c_lte_sfe_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,dummy_init, dummy_init
        #endif
        }
    },

    {   AOS_FALSE,
        L1C_LTE_DP_TASK_ID,
        L1C_DP_TASK_PRI,
        L1C_DP_TASK_STACK,
        L1C_TASK_INT_Q_NUM,
        L1C_DP_TASK_EXT_Q_NUM,
        "L1C_DP",
        {l1c_lte_dp_init, l1c_lte_dp_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,dummy_init, dummy_init
        #endif
        }
    },

    {   AOS_FALSE,
        L1C_LTE_L1PP_TASK_ID,
        L1C_L1PP_TASK_PRI,
        L1C_L1PP_TASK_STACK,
        L1C_TASK_INT_Q_NUM,
        L1C_L1PP_TASK_EXT_Q_NUM,
        "L1C_L1PP",
        {l1c_lte_l1pp_init, l1c_lte_l1pp_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,dummy_init, dummy_init
        #endif
        }
    },

    {   AOS_FALSE,
        L1C_LTE_UP_TASK_ID,
        L1C_UP_TASK_PRI,
        L1C_UP_TASK_STACK,
        L1C_TASK_INT_Q_NUM,
        L1C_UP_TASK_EXT_Q_NUM,
        "L1C_UP",
        {l1c_lte_up_init, l1c_lte_up_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,dummy_init, dummy_init
        #endif
        }
    },

    {   AOS_FALSE,
        L1C_LTE_L1RSP_TASK_ID,
        L1C_L1RSP_TASK_PRI,
        L1C_L1RSP_TASK_STACK,
        L1C_TASK_INT_Q_NUM,
        L1C_L1RSP_TASK_EXT_Q_NUM,
        "L1C_L1RSP",
        {l1c_lte_l1rsp_init, l1c_lte_l1rsp_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,dummy_init, dummy_init
        #endif
        }
    },
#endif

    {   AOS_FALSE,
        SIM_IT_STUB_TASK_ID,
        SIM_IT_STUB_TASK_PRI,
        SIM_IT_STUB_TASK_STACK_SIZE,
        SIM_IT_STUB_TASK_INT_Q_NUM,
        SIM_IT_STUB_TASK_EXT_Q_NUM,
        "SIM_IT_STUB",
        {dummy_init, sim_it_main
        #ifdef CONFIG_TASK_RESET_AND_DEL
         , dummy_init, dummy_init
        #endif
        }
    },
#endif

    {   AOS_FALSE,
        AOS_STANDARD_THREADS,
        0,
        0,
        0,
        0,
        "",
        {AOS_NULL, AOS_NULL
        #ifdef CONFIG_TASK_RESET_AND_DEL
         ,AOS_NULL, AOS_NULL
        #endif
        }
    }
};

/* pool description table, max cluster 255 */
/* pool 0 for common */
aos_mem_desc_tbl g_aos_common_mem[] =
{
    {8,300},
    {16,190},
    {32,250},
    {64,340},
    {128,350},
    {256,256},
    {512,172},
    {1024,128},
    {1024*2,64},
    {1024*4, 64},
    {1024*8, 32},
    {1024*16, 32},
    {1024*32, 16},
    {1024*64, 16},
    {1024*128, 8},
    {1024*256, 8},
    {1024*512, 4},
    {0,0}
};

/* pool 1 for FSM message header */
aos_mem_desc_tbl g_aos_fsm_header_mem[] =
{
    {sizeof(aos_fsm_msg), AOS_FSMMSG_CNT},
    {0,0}
};

/* aos pool table */
aos_pool aos_pool_tbl[AOS_MAX_POOLS] =
{
    {&g_aos_common_mem[0],      {0, 0, AOS_NULL, AOS_NULL},
    AOS_COMMON_POOL_ID,         4,         0},

    {&g_aos_fsm_header_mem[0],  {0, 0, AOS_NULL, AOS_NULL},
    AOS_FSM_MSG_HEADER_POOL_ID, 4,         0}
};

/**
* aos_init_standard_thread() - create aos standard threads, which are described
* in g_thread_config_tbl in aos_config.c .
* Return: AOS_SUCCESS means success, AOS_ERROR means error.
*/
u32 aos_init_standard_thread(void)
{
    u32 ret = AOS_ERROR;
    aos_thread_config *thread_config_ptr = AOS_NULL;

    thread_config_ptr = g_thread_config_tbl;
    while (thread_config_ptr->thread_id < AOS_STANDARD_THREADS) {
        ret = aos_create_standard_thread(thread_config_ptr->thread_id,
                                         thread_config_ptr->thread_name,
                                         aos_standard_thread_entry,
                                         (void *)&thread_config_ptr->thread_id,
                                         thread_config_ptr->stack_size,
                                         thread_config_ptr->intq_num,
                                         NULL,
                                         thread_config_ptr->extq_num,
                                         NULL,
                                         thread_config_ptr->thread_pri,
                                         AOS_AUTO_START);
        if (ret) {
            AIC_ASSERT(ret);
            return AOS_ERROR;
        }

        thread_config_ptr++;
    }

    return AOS_SUCCESS;
}

u32 get_thread_config_tbl_idx(aos_thread_id thread_id)
{
    u32 index;

    for (index = 0;
         index < sizeof(g_thread_config_tbl)/sizeof(aos_thread_config);
         index++) {
        if (g_thread_config_tbl[index].thread_id == thread_id)
            return index;
    }

    return AOS_ERROR;
}

