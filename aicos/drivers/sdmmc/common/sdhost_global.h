/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sdhost global Interface.
 */

#ifndef _SDHOST_GLOBAL_H_
#define _SDHOST_GLOBAL_H_

#include <reg_base.h>
#include <aic_type.h>
#include <mcu_glb.h>
#include "timer_inc.h"
#include "aic_io.h"
#include "sdmmc_core.h"

/* print detail cmd/response info */
#define TRACE_SD_CMD_RSP_DETAILS 0

/* INT mode or polling mode */
#define SDMMC_INT_ENABLE 1

/* DMA mode */
#define SDMMC_DMA_ENABLE 1

/* PMIC or GPIO to control SD power  */
#ifndef CONFIG_PMIC_I2C
#define SD_POWER_SWITCH_GPIO
#endif

#define SD_HOST_CLK         100000000  /*  */
#define SD_HOST_BLOCK_SIZE  0x200
#define SD_ADDR_SECTOR_MODE 1

#define SD_INT_NUM          IRQ_SDIOH
#define SD_TRAN_CLK         SD_HOST_CLK/1000/4     /* FPGA transfer clk 1MHz(1000KHz) */

#define SDMMC_DETECT_INSERT 1
#define SDMMC_DETECT_REMOVE 2
#define SDMMC_DETECT_RPM    3

/* SDMMC RPM check interval time: 5s */
#define SDMMC_RPM_TIME      5000
#define SDMMC_RPM_CNT       6

// ADMA2 Mode parameters: 2 or 4
#define SD_DESC_SIZE      2

#define SD_HOST_BASE      REG_BASE_AP_SDIOH

#if (USE_SIM == 1)
#define INTC_DEVICE_ID 0
#define INTC_SRC_SDHost_ID 0
#else
//    #define INTC_DEVICE_ID XPAR_MICROBLAZE_0_INTC_DEVICE_ID
//    #define INTC_SRC_SDHost_ID XPAR_MICROBLAZE_0_INTC_SD_HOST_CHIP_MEM_0_INTERRUPT_INTR
#endif

//-----------------------------------------------------------------------------
/// Host Register Set Organization
//-----------------------------------------------------------------------------
#define SD_REG_HRS_BASE  (SD_HOST_BASE + 0x00)

/// general information register
#define SD_REG_HRS0       (SD_REG_HRS_BASE + 0x00)
/// initialization setting register
#define SD_REG_HRS1       (SD_REG_HRS_BASE + 0x04)
/// DMA setting register
#define SD_REG_HRS2       (SD_REG_HRS_BASE + 0x08)
/// AXI Errors
#define SD_REG_HRS3       (SD_REG_HRS_BASE + 0x0c)
/// PHY Register Address
#define SD_REG_HRS4       (SD_REG_HRS_BASE + 0x10)
/// PHY Register Pointer
#define SD_REG_HRS5       (SD_REG_HRS_BASE + 0x14)
/// eMMC mode
#define SD_REG_HRS6       (SD_REG_HRS_BASE + 0x18)
/// PHY programmable delay
#define SD_REG_HRS7       (SD_REG_HRS_BASE + 0x1c)
/// IO Delay Register
#define SD_REG_HRS8       (SD_REG_HRS_BASE + 0x20)
/// PHY Controll and Status Register
#define SD_REG_HRS9       (SD_REG_HRS_BASE + 0x24)
/// Host Controller SDCLK start point adjustment
#define SD_REG_HRS10      (SD_REG_HRS_BASE + 0x28)
/// Host Interrupt Status
#define SD_REG_HRS12      (SD_REG_HRS_BASE + 0x30)
/// Host Status Enable
#define SD_REG_HRS13      (SD_REG_HRS_BASE + 0x34)
/// Host Signal Enable
#define SD_REG_HRS14      (SD_REG_HRS_BASE + 0x38)
/// Unused
#define SD_REG_HRS15      (SD_REG_HRS_BASE + 0x3c)
/// CMD/DAT output delay
#define SD_REG_HRS16      (SD_REG_HRS_BASE + 0x40)
/// Unused
#define SD_REG_HRS17      (SD_REG_HRS_BASE + 0x44)
/// Unused
#define SD_REG_HRS18      (SD_REG_HRS_BASE + 0x48)
/// HWInit SD Magic Number
#define SD_REG_HRS29      (SD_REG_HRS_BASE + 0x74)
/// Host Capability Register
#define SD_REG_HRS30      (SD_REG_HRS_BASE + 0x78)
/// Host Controller Version
#define SD_REG_HRS31      (SD_REG_HRS_BASE + 0x7C)
/// FSM Monitor Register
#define SD_REG_HRS32      (SD_REG_HRS_BASE + 0x80)
/// Tune Status 0 Register
#define SD_REG_HRS33      (SD_REG_HRS_BASE + 0x84)
/// Tune Status 1 Register
#define SD_REG_HRS34      (SD_REG_HRS_BASE + 0x88)
/// Unused
#define SD_REG_HRS35      (SD_REG_HRS_BASE + 0x8C)
/// Boot Status Register
#define SD_REG_HRS36      (SD_REG_HRS_BASE + 0x90)
/// Unused
#define SD_REG_HRS37      (SD_REG_HRS_BASE + 0x94)
/// Unused
#define SD_REG_HRS38      (SD_REG_HRS_BASE + 0x98)
/// Unused
#define SD_REG_HRS39      (SD_REG_HRS_BASE + 0x9C)
/// Base Address 0 for Auto-configuration descriptor mechanism
#define SD_REG_HRS40      (SD_REG_HRS_BASE + 0xA0)
/// Base Address 1 for Auto-configuration descriptor mechanism
#define SD_REG_HRS41      (SD_REG_HRS_BASE + 0xA4)
/// Auto-configuration descriptor mechanism control
#define SD_REG_HRS42      (SD_REG_HRS_BASE + 0xA8)
/// Error status for auto-configuration descriptor mechanism
#define SD_REG_HRS43      (SD_REG_HRS_BASE + 0xAC)
/// Unused
#define SD_REG_HRS44      (SD_REG_HRS_BASE + 0xB0)
/// Unused
#define SD_REG_HRS45      (SD_REG_HRS_BASE + 0xB4)
/// Unused
#define SD_REG_HRS46      (SD_REG_HRS_BASE + 0xB8)
/// Unused
#define SD_REG_HRS47      (SD_REG_HRS_BASE + 0xBC)
/// Unused
#define SD_REG_HRS48      (SD_REG_HRS_BASE + 0xC0)
/// Unused
#define SD_REG_HRS59      (SD_REG_HRS_BASE + 0xEC)
//------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Slot Register Set Organization
//-----------------------------------------------------------------------------
#define SD_REG_SRS_BASE  (SD_HOST_BASE + 0x200)

/// System address register it contains physical memory address for DMA operations
#define SD_REG_SRS0   (SD_REG_SRS_BASE + 0x00)
/// Block count and size register it contains informations about transmission size
#define SD_REG_SRS1   (SD_REG_SRS_BASE + 0x04)
/// Argument register it contains command argument of SD device
#define SD_REG_SRS2   (SD_REG_SRS_BASE + 0x08)
/// Transfer mode and command information register
#define SD_REG_SRS3   (SD_REG_SRS_BASE + 0x0C)
/// Response #0 register
#define SD_REG_SRS4   (SD_REG_SRS_BASE + 0x10)
/// Response #1 register
#define SD_REG_SRS5   (SD_REG_SRS_BASE + 0x14)
/// Response #2 register
#define SD_REG_SRS6   (SD_REG_SRS_BASE + 0x18)
/// Response #3 register
#define SD_REG_SRS7   (SD_REG_SRS_BASE + 0x1C)
/// Buffer data port register
#define SD_REG_SRS8   (SD_REG_SRS_BASE + 0x20)
/// Preset state register it contains the status of the SLOT
#define SD_REG_SRS9   (SD_REG_SRS_BASE + 0x24)
/// Host control settings #0 register
#define SD_REG_SRS10  (SD_REG_SRS_BASE + 0x28)
/// Host control settings #1 register
#define SD_REG_SRS11  (SD_REG_SRS_BASE + 0x2C)
/// Interrupt status register
#define SD_REG_SRS12  (SD_REG_SRS_BASE + 0x30)
/// Interrupt status enable register
#define SD_REG_SRS13  (SD_REG_SRS_BASE + 0x34)
/// Interrupt signal enable register
#define SD_REG_SRS14  (SD_REG_SRS_BASE + 0x38)
/// Auto CMD12 error status register
#define SD_REG_SRS15  (SD_REG_SRS_BASE + 0x3C)
/// Capabilities register
#define SD_REG_SRS16  (SD_REG_SRS_BASE + 0x40)
/// Capabilities #1
#define SD_REG_SRS17  (SD_REG_SRS_BASE + 0x44)
/// Maximum current capabilities
#define SD_REG_SRS18  (SD_REG_SRS_BASE + 0x48)
/// Event Trigger Register
#define SD_REG_SRS20  (SD_REG_SRS_BASE + 0x50)
/// ADMA Error Status Register
#define SD_REG_SRS21  (SD_REG_SRS_BASE + 0x54)
/// ADMA System Address Register
#define SD_REG_SRS22  (SD_REG_SRS_BASE + 0x58)
#define SD_REG_SRS23  (SD_REG_SRS_BASE + 0x5c)
/** Preset Values #0  */
#define SD_REG_SRS24  (SD_REG_SRS_BASE + 0x60)
/** Preset Values #1  */
#define SD_REG_SRS25  (SD_REG_SRS_BASE + 0x64)
/** Preset Values #2  */
#define SD_REG_SRS26  (SD_REG_SRS_BASE + 0x68)
/** Preset Values #3  */
#define SD_REG_SRS27  (SD_REG_SRS_BASE + 0x6C)
/// ADMA3 ID Address Register
#define SD_REG_SRS30  (SD_REG_SRS_BASE + 0x78)
#define SD_REG_SRS31  (SD_REG_SRS_BASE + 0x7C)
// UHS-II Block Size
#define SD_REG_SRS32  (SD_REG_SRS_BASE + 0x80)
// UHS-II Block Count
#define SD_REG_SRS33  (SD_REG_SRS_BASE + 0x84)
// UHS-II Command Packet #1
#define SD_REG_SRS34  (SD_REG_SRS_BASE + 0x88)
// UHS-II Command Packet #2
#define SD_REG_SRS35  (SD_REG_SRS_BASE + 0x8c)
// UHS-II Command Packet #3
#define SD_REG_SRS36  (SD_REG_SRS_BASE + 0x90)
// UHS-II Command Packet #4
#define SD_REG_SRS37  (SD_REG_SRS_BASE + 0x94)
// UHS-II Command Packet #5
#define SD_REG_SRS38  (SD_REG_SRS_BASE + 0x98)
// UHS-II Command and Transfer Mode
#define SD_REG_SRS39  (SD_REG_SRS_BASE + 0x9c)
// UHS-II Response #1
#define SD_REG_SRS40  (SD_REG_SRS_BASE + 0xA0)
// UHS-II Response #2
#define SD_REG_SRS41  (SD_REG_SRS_BASE + 0xA4)
// UHS-II Response #3
#define SD_REG_SRS42  (SD_REG_SRS_BASE + 0xA8)
// UHS-II Response #4
#define SD_REG_SRS43  (SD_REG_SRS_BASE + 0xAc)
// UHS-II Response #5
#define SD_REG_SRS44  (SD_REG_SRS_BASE + 0xB0)
// UHS-II MSG Select
#define SD_REG_SRS45  (SD_REG_SRS_BASE + 0xB4)
// UHS-II MSG Register
#define SD_REG_SRS46  (SD_REG_SRS_BASE + 0xB8)
// UHS-II Timing Control and Software Reset
#define SD_REG_SRS48  (SD_REG_SRS_BASE + 0xC0)
// UHS-II Error Interrupt Status
#define SD_REG_SRS49  (SD_REG_SRS_BASE + 0xC4)
// UHS-II Error Interrupt Status Enable
#define SD_REG_SRS50  (SD_REG_SRS_BASE + 0xC8)
// UHS-II Error Interrupt Signal Enable
#define SD_REG_SRS51  (SD_REG_SRS_BASE + 0xCc)
// Pointer for UHS-II Host Capabilities and Settings
#define SD_REG_SRS56  (SD_REG_SRS_BASE + 0xE0)
// Pointer for UHS-II Control registers
#define SD_REG_SRS57  (SD_REG_SRS_BASE + 0xE4)
// Pointer for UHS-II Vendor Specific Area
#define SD_REG_SRS58  (SD_REG_SRS_BASE + 0xE8)

//-----------------------------------------------------------------------------
#define SD_REG_ERS_BASE  (SD_HOST_BASE + 0x300)

// UHS-II General Settings
#define SD_REG_ERS00  (SD_REG_ERS_BASE + 0x00)
// UHS-II PHY Settings
#define SD_REG_ERS01  (SD_REG_ERS_BASE + 0x04)
// UHS-II Link/Tran Settings #1
#define SD_REG_ERS02  (SD_REG_ERS_BASE + 0x08)
// UHS-II Link/Tran Settings #2
#define SD_REG_ERS03  (SD_REG_ERS_BASE + 0x0c)
// UHS-II General Capabilities
#define SD_REG_ERS04  (SD_REG_ERS_BASE + 0x10)
// UHS-II PHY Capabilities
#define SD_REG_ERS05  (SD_REG_ERS_BASE + 0x14)
// UHS-II Link/Tran Capabilities
#define SD_REG_ERS06  (SD_REG_ERS_BASE + 0x18)
// UHS-II Link/Tran Capabilities
#define SD_REG_ERS07  (SD_REG_ERS_BASE + 0x1C)

//-----------------------------------------------------------------------------
/// Command Queue Register Set Organization
//-----------------------------------------------------------------------------
#define SD_REG_CQRS_BASE  (SD_HOST_BASE + 0x400)

#define SD_REG_CQRS00 (SD_REG_CQRS_BASE + 0x00)
#define SD_REG_CQRS01 (SD_REG_CQRS_BASE + 0x04)
#define SD_REG_CQRS02 (SD_REG_CQRS_BASE + 0x08)
#define SD_REG_CQRS03 (SD_REG_CQRS_BASE + 0x0c)
#define SD_REG_CQRS04 (SD_REG_CQRS_BASE + 0x10)
#define SD_REG_CQRS05 (SD_REG_CQRS_BASE + 0x14)
#define SD_REG_CQRS06 (SD_REG_CQRS_BASE + 0x18)
#define SD_REG_CQRS07 (SD_REG_CQRS_BASE + 0x1c)
#define SD_REG_CQRS08 (SD_REG_CQRS_BASE + 0x20)
#define SD_REG_CQRS09 (SD_REG_CQRS_BASE + 0x24)
#define SD_REG_CQRS10 (SD_REG_CQRS_BASE + 0x28)
#define SD_REG_CQRS11 (SD_REG_CQRS_BASE + 0x2c)
#define SD_REG_CQRS12 (SD_REG_CQRS_BASE + 0x30)
#define SD_REG_CQRS13 (SD_REG_CQRS_BASE + 0x34)
#define SD_REG_CQRS14 (SD_REG_CQRS_BASE + 0x38)
#define SD_REG_CQRS16 (SD_REG_CQRS_BASE + 0x40)
#define SD_REG_CQRS17 (SD_REG_CQRS_BASE + 0x44)
#define SD_REG_CQRS18 (SD_REG_CQRS_BASE + 0x48)
#define SD_REG_CQRS20 (SD_REG_CQRS_BASE + 0x50)
#define SD_REG_CQRS21 (SD_REG_CQRS_BASE + 0x54)
#define SD_REG_CQRS22 (SD_REG_CQRS_BASE + 0x58)
#define SD_REG_CQRS23 (SD_REG_CQRS_BASE + 0x5c)

//-----------------------------------------------------------------------------
/// ASF Register Set Organization
//-----------------------------------------------------------------------------
#define SD_REG_ASF_BASE  (SD_HOST_BASE + 0x100)

#define SD_REG_ASF_INT_STATUS           (SD_REG_ASF_BASE + 0x00)
#define SD_REG_ASF_INT_RAW_STATUS       (SD_REG_ASF_BASE + 0x04)
#define SD_REG_ASF_INT_MASK             (SD_REG_ASF_BASE + 0x08)
#define SD_REG_ASF_INT_TEST             (SD_REG_ASF_BASE + 0x0c)
#define SD_REG_FATAL_NONFATAL_SELECT    (SD_REG_ASF_BASE + 0x10)
#define SD_REG_SRAM_CORR_FAULT_STATUS   (SD_REG_ASF_BASE + 0x20)
#define SD_REG_SRAM_UNCORR_FAULT_STATUS (SD_REG_ASF_BASE + 0x24)
#define SD_REG_SRAM_FAULT_STATUS        (SD_REG_ASF_BASE + 0x28)
#define SD_REG_TO_CTRL                  (SD_REG_ASF_BASE + 0x30)
#define SD_REG_TO_FAULT_MASK            (SD_REG_ASF_BASE + 0x34)
#define SD_REG_TO_FAULT_STATUS          (SD_REG_ASF_BASE + 0x38)
#define SD_REG_PROTOCOL_FAULT_MASK      (SD_REG_ASF_BASE + 0x40)
#define SD_REG_PROTOCOL_FAULT_STATUS    (SD_REG_ASF_BASE + 0x44)

#define CQCAT             8  /* Clear All Tasks */
#define CQDCE             12 /* Direct Command (DCMD) Enable */
#define CQDCLR            0  /* Direct Command Last Response */
#define CQDPT             0  /* Device Pending Tasks */
#define CQDQS             0  /* Device Queue Status */
#define CQDTECI           16 /* Data Transfer Error Command Index */
#define CQDTEFV           31 /* Data Transfer Error Fields Valid */
#define CQDTETID          24 /* Data Transfer Error Task ID */
#define CQE               0  /* Command Queuing Enable */
#define CQHAC             0  /* Halt Complete Interrupt (HAC) */
#define CQHACSI           0  /* Halt Complete Signal Enable (HAC) */
#define CQHACST           0  /* Halt Complete Status Enable (HAC) */
#define CQHLT             0  /* Halt */
#define CQICCTH           8  /* Interrupt Coalescing Counter Threshold (ICCTH) */
#define CQICCTHWEN        15 /* Interrupt Coalescing Counter Threshold Write Enable (ICCTHWEN) */
#define CQICCTR           16 /* Counter and Timer Reset(ICCTR) */
#define CQICED            31 /* Interrupt Coalescing Enable/Disable */
#define CQICSB            20 /* Interrupt Coalescing Status Bit (ICSB) */
#define CQICTOVAL         0  /* Interrupt Coalescing Timeout Value (ICTOVAL) */
#define CQICTOVALEN       7  /* Interrupt Coalescing Timeout Value Write Enable (ICTOVALWEN) */
#define CQLCRA            0  /* Last Command Response Argument */
#define CQLCRI            0  /* Last Command Response Index */
#define CQREDI            2  /* Response Error Detected Interrupt (RED) */
#define CQREDSI           2  /* Response Error Detected Signal Enable (TCC) */
#define CQREDST           2  /* Response Error Detected Status Enable (RED) */
#define CQRMECI           0  /* Response Mode Error Command Index */
#define CQRMEFV           15 /* Response Mode Error Fields Valid */
#define CQRMEM            0  /* Response Mode Error Mask */
#define CQRMETID          8  /* Response Mode Error Task ID */
#define CQSQSR            0  /* Send Queue Status RCA */
#define CQSSCBC           16 /* Send Status Command Block Counter (CBC) */
#define CQSSCIT           0  /* Send Status Command Idle Timer (CIT) */
#define CQTBLBA           0  /* Task Descriptor List Base Address (lower) */
#define CQTC              0  /* Command Queuing Task Clear */
#define CQTCC             1  /* Task Complete Interrupt (TCC) */
#define CQTCCSI           1  /* Task Complete Signal Enable (TCC) */
#define CQTCCST           1  /* Task Complete Status Enable (TCC) */
#define CQTCL             3  /* Task Cleared (TCL) */
#define CQTCLSI           3  /* Task Cleared Signal Enable (TCL) */
#define CQTCLST           3  /* Task Cleared Status Enable (TCL) */
#define CQTCN             0  /* Task Completion Notification */
#define CQTD              0  /* Command Queuing Task Doorbell */
#define CQTDLBAU          0  /* Task Descriptor List Base Address (upper) */
#define CQTDS             8  /* Task Descriptor Size */
#define CQVN1             8  /* eMMC Major Version Number */
#define CQVN2             4  /* eMMC Minor Version Number */
#define CQVN3             0  /* eMMC Version Suffix */
#define ITCFMUL           12 /* Internal Timer Clock Frequency Multiplier (ITCFMUL) */
#define ITCFVAL           0  /* Internal Timer Clock Frequency Value (ITCFVAL) */
#define CQCAT_W           1  /* Clear All Tasks */
#define CQDCE_W           1  /* Direct Command (DCMD) Enable */
#define CQDCLR_W          32 /* Direct Command Last Response */
#define CQDPT_W           32 /* Device Pending Tasks */
#define CQDQS_W           32 /* Device Queue Status */
#define CQDTECI_W         6  /* Data Transfer Error Command Index */
#define CQDTEFV_W         1  /* Data Transfer Error Fields Valid */
#define CQDTETID_W        5  /* Data Transfer Error Task ID */
#define CQE_W             1  /* Command Queuing Enable */
#define CQHAC_W           1  /* Halt Complete Interrupt (HAC) */
#define CQHACSI_W         1  /* Halt Complete Signal Enable (HAC) */
#define CQHACST_W         1  /* Halt Complete Status Enable (HAC) */
#define CQHLT_W           1  /* Halt */
#define CQICCTH_W         5  /* Interrupt Coalescing Counter Threshold (ICCTH) */
#define CQICCTHWEN_W      1  /* Interrupt Coalescing Counter Threshold Write Enable (ICCTHWEN) */
#define CQICCTR_W         1  /* Counter and Timer Reset(ICCTR) */
#define CQICED_W          1  /* Interrupt Coalescing Enable/Disable */
#define CQICSB_W          1  /* Interrupt Coalescing Status Bit (ICSB) */
#define CQICTOVAL_W       7  /* Interrupt Coalescing Timeout Value (ICTOVAL) */
#define CQICTOVALEN_W     1  /* Interrupt Coalescing Timeout Value Write Enable (ICTOVALWEN) */
#define CQLCRA_W          32 /* Last Command Response Argument */
#define CQLCRI_W          6  /* Last Command Response Index */
#define CQREDI_W          1  /* Response Error Detected Interrupt (RED) */
#define CQREDSI_W         1  /* Response Error Detected Signal Enable (TCC) */
#define CQREDST_W         1  /* Response Error Detected Status Enable (RED) */
#define CQRMECI_W         6  /* Response Mode Error Command Index */
#define CQRMEFV_W         1  /* Response Mode Error Fields Valid */
#define CQRMEM_W          32 /* Response Mode Error Mask */
#define CQRMETID_W        5  /* Response Mode Error Task ID */
#define CQSQSR_W          16 /* Send Queue Status RCA */
#define CQSSCBC_W         4  /* Send Status Command Block Counter (CBC) */
#define CQSSCIT_W         16 /* Send Status Command Idle Timer (CIT) */
#define CQTBLBA_W         32 /* Task Descriptor List Base Address (lower) */
#define CQTC_W            32 /* Command Queuing Task Clear */
#define CQTCC_W           1  /* Task Complete Interrupt (TCC) */
#define CQTCCSI_W         1  /* Task Complete Signal Enable (TCC) */
#define CQTCCST_W         1  /* Task Complete Status Enable (TCC) */
#define CQTCL_W           1  /* Task Cleared (TCL) */
#define CQTCLSI_W         1  /* Task Cleared Signal Enable (TCL) */
#define CQTCLST_W         1  /* Task Cleared Status Enable (TCL) */
#define CQTCN_W           32 /* Task Completion Notification */
#define CQTD_W            32 /* Command Queuing Task Doorbell */
#define CQTDLBAU_W        32 /* Task Descriptor List Base Address (upper) */
#define CQTDS_W           1  /* Task Descriptor Size */
#define CQVN1_W           4  /* eMMC Major Version Number */
#define CQVN2_W           4  /* eMMC Minor Version Number */
#define CQVN3_W           4  /* eMMC Version Suffix */
#define ITCFMUL_W         4  /* Internal Timer Clock Frequency Multiplier (ITCFMUL) */
#define ITCFVAL_W         10 /* Internal Timer Clock Frequency Value (ITCFVAL) */

// ---------------- BIT FIELD DEFINES ------------------
// HRS02
#define OTN      16   /* Number of Outstanding Transfers */
#define PBL      0    /* Programmable Burst Length */

// HRS 20, 21, 22, 23
#define CLK_IDLES    0

// HRS24
#define PDEN     0
#define CGEN     1

//  HRS32
#define FSM_EN      31   /* FSM monitor start/ready */
#define FSM_ADDR    16   /* FSM monitor address */
#define FSM_STATE    0   /* FSM monitor state id */

// SRS01
#define TBS      0   /* Block size */
#define HDMABB  12
#define BCCT    16   /* Block count */
// SRS03
#define DMAE     0   /* DMA enable */
#define BCE      1   /* Block count enable */
#define ACE      2
#define DTDS     4
#define MSBS     5
#define RECT     6  /*response type R1/R5*/
#define RECE     7  /*response check en*/
#define RID      8  /*response interupt disable*/
#define RTS     16  /* Response type */
#define SCF     18
#define CRCCE   19  /* CRC check enable */
#define CICE    20
#define DPS     21
#define CT      22
#define CI      24  /* Command index */
// SRS09
#define CICMD    0
#define CIDAT    1
#define DLA      2
#define RTR      3
#define DATSL2   4
#define WTA      8
#define RTA      9
#define BWE     10  /* Buffer write enable */
#define BRE     11  /* Buffer read enable */
#define CIns    16  /* Card Inserted */
#define CSS     17
#define CDSL    18
#define WPSL    19
#define DATSL1  20
#define CMDSL1  24
#define CNIBE   27
#define SCMDS   28
#define U2DOR   29
#define U2SYN   30
#define U2DET   31
// SRS10
#define LEDC     0
#define DTW      1  /* Data width */
#define HSE      2  /* High speed enable */
#define DMASEL   3  /* DMA select */
#define EDTW     5
#define SDTL     6
#define SDSS     7
#define BP       8
#define BVS      9
#define BP2     12
// SRS 11
#define ICE      0
#define ICS      1
#define SDCE     2
#define CGS      5
#define USDCLKFS 6
#define SDCLKFS  8
#define DTCV    16
#define SFRA    24
#define SRCMD   25
#define SRDAT   26
// SRS 12, 13, 14
#define CC       0
#define TC       1
#define BGE      2
#define DINT     3
#define BWR      4
#define BRR      5
#define CIN      6
#define CR       7
#define CINT     8
#define INT_A    9
#define INT_B   10
#define INT_C   11
#define RTNGE   12
#define EINT    15
#define ETC     16
#define ECCRC   17
#define ECEB    18
#define ECI     19
#define EDT     20
#define EDCRC   21
#define EDEB    22
#define ECL     23
#define EA12    24
#define EADMA   25
#define ETNG    26
// SRS15
#define A64B    29
#define HV4E    28
#define EXTNG   22
#define SCS     23
#define U2IE    24
#define UMS     16

// SRS32
#define U2TBS   0
// SRS33
#define U2BCCT  0
// SRS39
#define U2CPL   24
#define U2CTP   22
#define U2DPS   21
#define U2HFD   15
#define U2EBSY  14
#define U2RID    8
#define U2RECE   7
#define U2RT     6
#define U2BBM    5
#define U2DTDS   4
#define U2BCE    1
#define U2DMAE   0
// SRS48
#define TCFDL    20
#define TCFCR    16
#define TRANRST  1
#define FULLRST  0

// ERS00
#define PWRM     0
// ERS01
#define HNLD    20
#define HNLS    16
#define HIBE    15
#define SPDR     6
// ERS02
#define HNF      8
// ERS03
#define HNDG     0

// SD Host variables
extern unsigned int sd_card_rca;
extern const unsigned char idelay_table [10];

// delay regs address
#define REG_DELAY_HS          0x00
#define REG_DELAY_DEFAULT     0x01
#define REG_DELAY_UHSI_SDR12  0x02
#define REG_DELAY_UHSI_SDR25  0x03
#define REG_DELAY_UHSI_SDR50  0x04
#define REG_DELAY_UHSI_DDR50  0x05
#define REG_DELAY_MMC_LEGACY  0x06
#define REG_DELAY_MMC_SDR     0x07
#define REG_DELAY_MMC_DDR     0x08
#define REG_DELAY_MMC_HS400   0x09

#define REG_DLL_LOCK_VALUE         9
#define REG_DLL_MULT              10
#define REG_DLL_SDCLK_DELAY       11
#define REG_DLL_SDCLK_DELAY_HSMMC 12
#define REG_DLL_STROBE_DELAY      13
#define REG_IFM_MODE              14
#define REG_DLL_RST               15
#define REG_PHASE_DTCT_SEL        16
#define REG_LOCK_NUM              17
#define REG_DLL_START_POINT       18

#define TRACE_UHSII_CONGIF_DETAILS 0

// SDIO card types
#define SDIO_CTYPE_SD   0
#define SDIO_CTYPE_IO   1

// ADMA3
#define SD_CMD_COUNT     4
#define CMDDESC_SIZE     8   // 2*4 B
#define ADMA2DESC_SIZE   4*SD_DESC_SIZE      // 16 4*4 B  or 8
#define INTDESC_SIZE     4*SD_DESC_SIZE   // 4*4 B


typedef struct  {
    uint32_t srs00;
    uint32_t srs01;
    uint32_t srs02;
    uint32_t srs03;
    unsigned char* data_buff;
    unsigned long size;      // data buffer size
    unsigned int desc_len;   // use that length in descriptor length field
    char desc_len_26bit;
} SD_DESC_PAIR;

// SD Host functions
void sdhost_clk_src_sel(void);
void sdhost_init ();
void sdhost_config(sdmmc_host *host);
void sdhost_disconfig(sdmmc_host *host);

void sdhost_set_clk(sdmmc_host *host, uint32_t clock);
void sdhost_disable_clk();
void sdhost_enable (boolean enable);
uint32_t sdhost_read_phy_reg(uint32_t addr);
void sdhost_set_phy_reg(uint32_t addr, uint32_t data);
void sdhost_set_power(sdmmc_host *host, uint32_t voltage, boolean b_on);
void sdhost_set_width(uint32_t width);
u32 sdhost_detect_card(u32 wait_option);
void sdhost_send_rpm();

#endif /* _SDHOST_GLOBAL_H_ */
