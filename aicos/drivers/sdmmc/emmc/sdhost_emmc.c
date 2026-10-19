/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the sdhost emmc function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x013860
#define LOG_MODULE    AMOD_SDMMC

#include <stdint.h>
#include "sdhost_global.h"
#include "sdhost_emmc.h"

#define check(func) { \
 int status = func;  \
 if (status){\
   sd_printf("ERROR nr 0x%X, %s %d\n", status, __FILE__, __LINE__); \
   while(1) \
   return (status);\
}}


// Extended CSD Register
uint32_t ext_csd[512/4];
cq_task_t cq_task[32];

// ----------------------------------------------------------------------------
int eMMC_Card_init (char low_voltage_en) {
  unsigned long _status;

  sd_printf("eMMC Card init...\n");

  // CMD0 - reset all cards to IDLE state
  sd_delay(10000); // at least 74 clks before first command
  sd_printf("Issue Card reset (CMD0)\n");
  check( SDHost_send_cmd( (0 << CI) | (0<<CRCCE) | (0<<RTS), 0x00000000, 0) );

  // CMD1 - send operational condition and go to READY state
  sd_printf("Issue (CMD1)\n");
  do {
    _status = 0x00ff8000; // 2.7-3.6V
    if (low_voltage_en == 1) _status |= (1 << 7);  // 1.8V
    _status |= (SD_ADDR_SECTOR_MODE << 30); // byte or sector address mode
    check( SDHost_send_cmd( (1 << CI) | (2<<RTS), _status, 0) );  // voltage and address mode

    _status = IORD_32_SRS(SD_REG_SRS4);
  } while ((_status & 0x80000000) == 0);

  // CMD2 - get CardID - go to IDENT state
  check( SDHost_send_cmd((2 << CI) | (0<<CRCCE) | (1<<RTS), 0x00000000, 0) );
  _status = IORD_32_SRS(SD_REG_SRS4);

  // CMD3 - send RCA to eMMC card and go to STBY
  sd_card_rca = 0x1234;
  check( SDHost_send_cmd((3 << CI) | (3<<CRCCE) | (2<<RTS), sd_card_rca << 16, 0) );

  sd_printf("eMMC Card initialized with RCA number 0x%x\n", sd_card_rca);
  // -- initialization finished --
  return 0;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
int eMMC_select_card (unsigned int sd_card_rca) {

  // CMD7 - select card, go to TRANS state
  if (sd_card_rca == 0x0000) {
    sd_printf("Deselect card\n");

#if 1
    check( SDHost_send_cmd((7 << CI) | (0<<CRCCE) | (0<<RTS), 0x0000, 0) ); //no response on deselect card
#else
    uint32_t _status;
    check( SDHost_send_cmd((7 << CI) | (3<<CRCCE) | (3<<RTS), 0x0000, 0) ); //R1 expected after deselect

    // response R1b - busy signal is transmitted on the DAT0 line
    do {
      _status = IORD_32_SRS(SD_REG_SRS12);
    } while ((_status & (1<<TC) ) == 0); // exit loop when TC (transfer complete) flag is set to 1
#endif

  }
  else {
    sd_printf("Select card: 0x%x\n", sd_card_rca);
    check( SDHost_send_cmd((7 << CI) | (3<<CRCCE) | (2<<RTS), sd_card_rca << 16, 0) );
  }
  return 0;

}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
int eMMC_set_mode (char bit_width, char mode) {
/*
*  bit_width[3:0]: bus_width 1,4,8
*  bit_width[4]: ddr_mode bit
*  bit_width[5]: enhanced_strobe bit
*  mode:
*      0 - Standard mode   (0-26MHz)
*      1 - High Speed mode (0-52MHz)
*      2 - HS200           (0-200MHz)
*      3 - HS400           (0-200MHz, DDR)
*  For allowed confugrations see:
*    eMMC5.0 JADEC standard, ch. 5.3.2 Bus Speed Modes (Table 4)
*/

    unsigned long _status;
//    uint8_t _ext_csd_184 = 0;
    unsigned long _val;
    char _bit_width;
    uint8_t _ddr_mode;
    uint8_t _enhanced_strobe;

    sd_printf("Set bit width to %d\n", bit_width & 0x0f);

    _bit_width = bit_width & 0x0f;
    _ddr_mode = bit_width & 0x10 ? 1 : 0;
    _enhanced_strobe = bit_width & 0x20 ? 1 : 0;

    // select bit width in SD Controller registers
    // ---------------------------------
    _status = IORD_32_SRS(SD_REG_SRS10);
    _status &= ~((1<<DTW) | (1<<EDTW));

    switch (_bit_width) {
    case 0x01: _bit_width = 0x00; break;  // bus width 1b
    case 0x04: _bit_width = 0x01; _status |= (1<<DTW);  break;  // bus width 4b
    case 0x08: _bit_width = 0x02; _status |= (1<<EDTW) | (1<<DTW); break;  // bus width 8b
    default: sd_printf("ERROR: wrong bit width selected\n"); return 1;
    }
    IOWR_32_SRS (SD_REG_SRS10, _status);

#if 1
    // CMD6 - SWITCH select bus width in eMMC card
    _val = ((_bit_width << 8) | (183<<16) | (3 << 24)); //183 - BUS_WIDTH byte, 3 - ACCESS write byte
    check( SDHost_send_cmd((6 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
    // response R1b - busy signal is transmitted on the DAT0 line
    do {
        _status = IORD_32_SRS(SD_REG_SRS12);
    } while ((_status & (1<<TC) ) == 0); // exit loop when TC (command complete) flag is set to 1
    // ---------------------------------
#endif

#if 0 /* You can disable checking of supported modes to increase simulation performance*/
    // Read card type
    // CMD8 - send ext csd
    IOWR_32_SRS (SD_REG_SRS1,  (7 << HDMABB) | (1 << BCCT) | 512);    // data size: 512B of EXT_CSD
    check( SDHost_send_cmd((8 << CI) | (1 << DPS) | (3 << CRCCE) | (2<<RTS) | (1<<DTDS), 0, 0) );

    while( ((IORD_32_SRS(SD_REG_SRS9) >> BRE ) & 0x01) == 0 ); // wait for BRE
    int i;
    sd_printf("\n\n");
    for (i=0; i<(512/4); i++) {  // read 512B, 4B at once
        _val = IORD_32_SRS(SD_REG_SRS8);

        sd_printf("EXT_CSD[%03d] = 0x%02x ",  (4*i+ 0), (_val >>  0) & 0xff);
        sd_printf("EXT_CSD[%03d] = 0x%02x ",  (4*i+ 1), (_val >>  8) & 0xff);
        sd_printf("EXT_CSD[%03d] = 0x%02x ",  (4*i+ 2), (_val >> 16) & 0xff);
        sd_printf("EXT_CSD[%03d] = 0x%02x\n", (4*i+ 3), (_val >> 24) & 0xff);

        if (i == 196/4) _status = _val & 0xff; // remember DEVICE_TYPE [196] register
        if (i == 184/4) _ext_csd_184 = _val & 0xff;
    }
    sd_printf("DEVICE_TYPE = 0x%x\n", _status);

    if (( (mode == eMMC_MODE_HS      ) && ((_status & (3 << 0)) == 0) ) ||
      ( (mode == eMMC_MODE_HS      ) && _ddr_mode && ((_status & (3 << 2)) == 0) ) ||
      ( (mode == eMMC_MODE_HS200   ) && ((_status & (3 << 4)) == 0) ) ||
      ( (mode == eMMC_MODE_HS400   ) && ((_status & (3 << 6)) == 0) ))
    {
        sd_printf("Selected work mode is not supported by inserted card. Exiting...\n");
        return 1;
    }

    do {
        _status = IORD_32_SRS(SD_REG_SRS12);
    } while ((_status & (1<<TC) ) == 0); // exit loop when TC (command complete) flag is set to 1

    if (_enhanced_strobe)
    {
      /* Check if device support HS400 Enhanced Strobe mode */
      if (_ext_csd_184 != 0x01)
      {
          sd_printf("Device does not support Enhanced Strobe\n");
          return 1;
      }
    }
#endif

    if ((mode == eMMC_MODE_HS400) && !_enhanced_strobe) {
        // before switching to the HS400 host must switch to the HS200 mode and perform calibration
        eMMC_set_mode (0x08, eMMC_MODE_HS200);
        IOWR_32_SRS(SD_REG_HRS10, 0x00030000);
        IOWR_32_SRS(SD_REG_HRS16, 0x00000000);
        eMMC_card_tuning(8);
        // now you can switch to HS400 mode...
        // CMD6 - SWITCH work mode
        _val = ((mode << 8) | (185<<16) | (3 << 24)); //185 - HS_TIMING byte, 3 - ACCESS write byte
        check( SDHost_send_cmd((6 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
        // response R1b - busy signal is transmitted on the DAT0 line
        do {
            _status = IORD_32_SRS(SD_REG_SRS12);
        } while ((_status & (1<<TC) ) == 0); // exit loop when TC (transfer complete) flag is set to 1
    }

    if (mode == eMMC_MODE_HS400) {
        // CMD6 - additional DDR setting needed for HS400
        _val = ((6 << 8) | (183<<16) | (3 << 24)); //183 - BUS_WIDTH byte, 3 - ACCESS write byte
        if (_enhanced_strobe)
            _val |= 0x80 << 8;
        check( SDHost_send_cmd((6 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
        do {
            _status = IORD_32_SRS(SD_REG_SRS12);
        } while ((_status & (1<<TC) ) == 0); // exit loop when TC (transfer complete) flag is set to 1
    }

    if ((mode != eMMC_MODE_HS400) || _enhanced_strobe) {
        // ---------------------------------
        // CMD6 - SWITCH work mode
        _val = ((mode << 8) | (185<<16) | (3 << 24)); //185 - HS_TIMING byte, 3 - ACCESS write byte
        check( SDHost_send_cmd((6 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
        // response R1b - busy signal is transmitted on the DAT0 line
        do {
            _status = IORD_32_SRS(SD_REG_SRS12);
        } while ((_status & (1<<TC) ) == 0); // exit loop when TC (transfer complete) flag is set to 1
    }

    switch (mode) {
    case eMMC_MODE_SDR:     _val = 0x01; break;
    case eMMC_MODE_HS:      _val = _ddr_mode ? 0x03 : 0x02; break; // HS DDR or SDR mode
    case eMMC_MODE_HS200:   _val = 0x04; break;
    case eMMC_MODE_HS400:   _val = _enhanced_strobe ? 0x06 : 0x05; break;
    default:                _val = 0x00; break; // disabled
    }

    IOWR_32_SRS(SD_REG_SRS11, 0);   // disable SDCE - it will be re-enabled when SDHost_set_clk is called
    sd_delay(10);
    IOWR_32_SRS (SD_REG_HRS6, _val);

    // change clock frequency to max allowed by selected workmode
    if ( (mode == eMMC_MODE_HS400) || (mode == eMMC_MODE_HS200))
        SDHost_set_clk(SD_TRAN_CLK);//SDHost_set_clk(200000);
    else if (mode == eMMC_MODE_HS)
        SDHost_set_clk(SD_TRAN_CLK);//SDHost_set_clk(50000);
    else
        SDHost_set_clk(400);

    // DDR enabled (only for 4-bit/8-bit)
    if (_ddr_mode) {
        _bit_width |= (1<<2);
        _val = ((_bit_width << 8) | (183<<16) | (3 << 24)); //183 - BUS_WIDTH byte, 3 - ACCESS write byte
        check( SDHost_send_cmd((6 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
        // response R1b - busy signal is transmitted on the DAT0 line
        do {
            _status = IORD_32_SRS(SD_REG_SRS12);
        } while ((_status & (1<<TC) ) == 0); // exit loop when TC (command complete) flag is set to 1
    }

return 0;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Update BOOT_CONFIG[179] register in EXT_CSD register by enabling the Boot
// Acknowledge and selecting Partition 1 for boot operation
int eMMC_prep_card_for_boot (unsigned int arg) {
  return eMMC_ext_csd_write(179, arg); // BOOT_CONFIG <- arg
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
int eMMC_ext_csd_write (unsigned int addr, unsigned int arg) {
  unsigned long _status;
  unsigned long _val;

  _val = ((arg << 8) | (addr<<16) | (3 << 24)); // 3 - ACCESS write byte
  check( SDHost_send_cmd((6 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
  // response R1b - busy signal is transmitted on the DAT0 line
  do {
    _status = IORD_32_SRS(SD_REG_SRS12);
  } while ((_status & (1<<TC) ) == 0); // exit loop when TC (transfer complete) flag is set to 1

  IOWR_32_SRS(SD_REG_SRS12, 3);
  return 0;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
//unsigned int eMMC_ext_csd_read (unsigned int addr) {
//  unsigned long _status;
//  unsigned long _val;
//
//  // CMD8 - send ext csd
//  IOWR_32_SRS (SD_REG_SRS1,  (7 << HDMABB) | (1 << BCCT) | 512);    // data size: 512B of EXT_CSD
//  check( SDHost_send_cmd((8 << CI) | (1 << DPS) | (3 << CRCCE) | (2<<RTS) | (1<<DTDS), 0, 0) );
//
//  while( ((IORD_32_SRS(SD_REG_SRS9) >> BRE ) & 0x01) == 0 ); // wait for BRE
//  int i;
//  for (i=0; i<(512/4); i++) {  // read 512B, 4B at once
//    _val = IORD_32_SRS(SD_REG_SRS8);
//    if (i == addr/4)
//      _status = (_val >> (8 * (addr & 3))) & 0xff;
//  }
//  IOWR_32_SRS(SD_REG_SRS12, 3);
//  return _status;
//}
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
int eMMC_ext_csd_read () {
  sd_printf("eMMC_ext_csd_read");
  // CMD8 - send ext csd
  IOWR_32_SRS (SD_REG_SRS1,  (7 << HDMABB) | (1 << BCCT) | 512);    // data size: 512B of EXT_CSD
  check( SDHost_send_cmd((8 << CI) | (1 << DPS) | (3 << CRCCE) | (2<<RTS) | (1<<DTDS), 0, 0) );

  while( ((IORD_32_SRS(SD_REG_SRS9) >> BRE ) & 0x01) == 0 ); // wait for BRE
  int i;
  for (i=0; i<(512/4); i++) {  // read 512B, 4B at once
    volatile uint32_t data;
    //sd_printf("eMMC_ext_csd_read %d", i);
    data = IORD_32_SRS(SD_REG_SRS8);
    ext_csd[i] = data;
  }
  IOWR_32_SRS(SD_REG_SRS12, 3);
  sd_printf("eMMC_ext_csd_read end");
  return 0;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
unsigned int eMMC_ext_csd_read_byte (unsigned int addr) {
    return (ext_csd[addr/4] >> (8 * (addr & 3))) & 0xff;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Reset eMMC card to PreIdle state.
int eMMC_hw_reset () {
  unsigned long _status;
  unsigned long _val;

  _val = 0xf0f0f0f0;
  check( SDHost_send_cmd((0 << CI) | (0<<CRCCE) | (0<<RTS), _val, 0) );
  // no response
  do {
    _status = IORD_32_SRS(SD_REG_SRS12);
  } while ((_status & (1<<CC) ) == 0); // exit loop when CC (command complete) flag is set to 1

  _status = IORD_32_SRS(SD_REG_SRS12);
  IOWR_32_SRS(SD_REG_SRS12, 1);

  IOWR_32_SRS(SD_REG_SRS11, 0);   // disable SDCE
  sd_delay(10);

  return 0;

}
// ----------------------------------------------------------------------------

// ============================================================================
// EMMC TUNING PROCEDURES
// ============================================================================
// eMMC_tuning_pattern
const uint32_t eMMC_PATTERN_4b[64/4] = {
  0x00ff0fff,
  0xccc3ccff,
  0xffcc3cc3,
  0xeffefffe,
  0xddffdfff,
  0xfbfffbff,
  0xff7fffbf,
  0xefbdf777,
  0xf0fff0ff,
  0x3cccfc0f,
  0xcfcc33cc,
  0xeeffefff,
  0xfdfffdff,
  0xffbfffdf,
  0xfff7ffbb,
  0xde7b7ff7};

const uint32_t eMMC_PATTERN_8b[128/4] = {
  0xff00ffff,
  0x0000ffff,
  0xccccffff,
  0xcccc33cc,
  0xcc3333cc,
  0xffffcccc,
  0xffffeeff,
  0xffeeeeff,
  0xffddffff,
  0xddddffff,
  0xbbffffff,
  0xbbffffff,
  0xffffffbb,
  0xffffff77,
  0x77ff7777,
  0xffeeddbb,
  0x00ffffff,
  0x00ffffff,
  0xccffff00,
  0xcc33cccc,
  0x3333cccc,
  0xffcccccc,
  0xffeeffff,
  0xeeeeffff,
  0xddffffff,
  0xddffffff,
  0xffffffdd,
  0xffffffbb,
  0xffffbbbb,
  0xffff77ff,
  0xff7777ff,
  0xeeddbb77};


// ----------------------------------------------------------------------------
// eMMC_card_tune - eMMC tuning procedure for HS200 and SH400 mode
// ----------------------------------------------------------------------------
unsigned long eMMC_card_tuning (unsigned char _data_width) {
  uint32_t _pattern_buff[128/4];
  uint32_t _status, _prev;
  char     _err;
  unsigned char i, j, _block_size, _pattern_ok[40];
  unsigned char _pos, _length, _curr_length; // used for looking for longest valid delay chain value


  if (_data_width == 4) _block_size = 64; else _block_size = 128;

  // decrease data timeout counter value
  _status = IORD_32_SRS(SD_REG_SRS11);
  _prev = _status;
  _status &= ~(0x0f<<DTCV);
  // _status |=  (0x02<<DTCV);
  IOWR_32_SRS(SD_REG_SRS11, _status);

  for (j=0; j<40; j++) {
    SDHost_set_tune_val(j);
    IOWR_32_SRS (SD_REG_SRS1, _block_size);  // block size

    SDHost_send_cmd( ((21 << CI) | (1 << DPS) | (3 << CRCCE) | (2 << RTS) | (1 << DTDS)), 0x00000000, 0);

    do {
      _status = IORD_32_SRS(SD_REG_SRS12);
      if ((( _status & ((1 << EDT) | (1 << EDCRC) | (1 << EDEB))) > 0) ||
          ((( _status & (1 << BRR)) > 0) && ((_status & ((1 << ETC) | (1 << ECCRC) | (1 << ECEB) | (1 << ECI))) > 0 )))
      {

        // Error - DAT and CMD reset
        _status = IORD_32(SD_REG_SRS11);
        _status |= (1 << SRDAT);
        _status |= (1 << SRCMD);
        IOWR_32_SRS(SD_REG_SRS11, _status);
        while ((IORD_32_SRS(SD_REG_SRS11) >> SRDAT) == 0x01);
        while ((IORD_32_SRS(SD_REG_SRS11) >> SRCMD) == 0x01);

        _err = 1;  // indicate error
        break;  // check next tuning value
      }
      else
        _err = 0;  // no error
    }
    while( ((_status >> BRR ) & 0x01) == 0 ); // wait for BRE

    if (_err == 0) {
      // get data from buffer
      for (i=0; i<(_block_size/4); i++) {
        _pattern_buff[i] = IORD_32_SRS (SD_REG_SRS8);  // read 4B of data
        //         sd_printf("_pattern_buff[%d] = 0x%x\n", i, _pattern_buff[i]);
      }
      while ((IORD_32_SRS(SD_REG_SRS12) & (1<<TC) ) == 0); // exit loop when TC (transfer complete) flag is set to 1

      // compare data with pattern
      _pattern_ok[j] = 1;
      for (i=0; i<_block_size/4; i++) {
        if ( (_data_width == 4 && (_pattern_buff[i] != eMMC_PATTERN_4b[i])) || \
            (_data_width == 8 && (_pattern_buff[i] != eMMC_PATTERN_8b[i])) ) {
          _pattern_ok[j] = 0;
          sd_printf ("Compare: _pattern_buff = 0x%x, eMMC_PATTERN_4b = 0x%x\n", _pattern_buff[i], eMMC_PATTERN_4b[i]);
          break; // read pattern is not correct - exit loop
        }
      }
    }
    else
      _pattern_ok[j] = 0;
  }

  // select the best tuning value
  _pos = _length = _curr_length = 0;
  for (i=0; i<40; i++) {
    //     sd_printf("Tuning val [%d]: %d\n", i, _pattern_ok[i]);
    if (_pattern_ok[i] == 1) {
      _curr_length++;
      if (_curr_length > _length) {
        _pos = i - _length;
        _length++;
        //         sd_printf("DEBUG: _pos = %d, _length = %d\n", _pos, _length);
      }
    }
    else {
      _curr_length = 0;
    }
  }
  _pos = _pos + _length/2;
  sd_printf("Delay value set to: %d\n", _pos);
  SDHost_set_tune_val(_pos);

  // set the previous data timeout counter value:
  IOWR_32_SRS(SD_REG_SRS11, _prev);


  return 0;
}
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
int eMMC_card_features ()
{
  eMMC_ext_csd_read();
  /* int i; */
  /* for (i=0; i<512; i++) */
  /* { */
  /*     sd_printf("%3d %2x\n", i, eMMC_ext_csd_read_byte(i)); */
  /* } */
  sd_printf("[RO] CSD_STRUCTURE[%d]: %x\n", 194, eMMC_ext_csd_read_byte(194));
  sd_printf("[RO] EXT_CSD_REV[%d]: %x\n", 192, eMMC_ext_csd_read_byte(192));
  switch (eMMC_ext_csd_read_byte(192))
  {
  case 0:
      sd_printf("    Revision 1.0 (for MMC v4.0)\n");
      break;
  case 1:
      sd_printf("    Revision 1.1 (for MMC v4.1)\n");
      break;
  case 2:
      sd_printf("    Revision 1.2 (for MMC v4.2)\n");
      break;
  case 3:
      sd_printf("    Revision 1.3 (for MMC v4.3)\n");
      break;
  case 4:
      sd_printf("    Revision 1.4 (Obsolete)\n");
      break;
  case 5:
      sd_printf("    Revision 1.5 (for MMC v4.41)\n");
      break;
  case 6:
      sd_printf("    Revision 1.6 (for MMC v4.5, v4.51)\n");
      break;
  case 7:
      sd_printf("    Revision 1.7 (for MMC v5.0, v5.01)\n");
      break;
  case 8:
      sd_printf("    Revision 1.8 (for MMC v5.1)\n");
      break;
  default:
      sd_printf("    Error: Unknown revision\n");
      break;
  }

  sd_printf("[RO] BOOT_INFO[%d]: %x\n", 228, eMMC_ext_csd_read_byte(228));
  if (eMMC_ext_csd_read_byte(228) & 0x7)
  {
      sd_printf("    HS_BOOT_MODE[2]: %d\n", (eMMC_ext_csd_read_byte(228) >> 2) & 1);
      sd_printf("    DDR_BOOT_MODE[1]: %d\n", (eMMC_ext_csd_read_byte(228) >> 1) & 1);
      sd_printf("    ALT_BOOT_MODE[0]: %d\n", (eMMC_ext_csd_read_byte(228) >> 0) & 1);
  }
  sd_printf("[RO] BOOT_SIZE_MULT[%d]: %x\n", 226, eMMC_ext_csd_read_byte(226));

  sd_printf("[RO] CMDQ_SUPPORT[%d]: %d (1 - CQ supported)\n", 308, eMMC_ext_csd_read_byte(308) & 0x1);
  sd_printf("[RO] CMDQ_DEPTH[%d]: %d (number of IDs - 1)\n", 307, eMMC_ext_csd_read_byte(307) /*& 0x1f*/);
  return 0;
}
// ----------------------------------------------------------------------------

//----------------------------------------------------------------------
// eMMC Command Queue Enable
//----------------------------------------------------------------------
int eMMC_card_cq_enable (unsigned int value)
{
  unsigned int _value;
  //sd_printf("eMMC_card_cq_enable()\n");
  _value = eMMC_ext_csd_read_byte(15) & 1;

  if (_value != value)
  {
    eMMC_ext_csd_write (15, value);
    eMMC_ext_csd_read();
    sd_printf("[RW] Command Queue Mode Enable[%d]: %d -> %d\n", 15, _value, eMMC_ext_csd_read_byte(15));
  }
  else
  {
    sd_printf("[RW] Command Queue Mode Enable[%d]: %d\n", 15, _value);
    sd_printf("Warning: Command Queue Mode Enable - control bit wasn't changed\n");
    sd_printf("Sending CMD6 though to workaround card bug that CQ enabled bit stays 1 after CMD0, but the card is in improper state\n");
    eMMC_ext_csd_write (15, value);
    eMMC_ext_csd_read();
  }
  return 0;
}
//----------------------------------------------------------------------
// eMMC Command Queue Status
//----------------------------------------------------------------------
uint32_t eMMC_card_cq_status ()
{
  uint32_t _status;
  uint32_t cq_status;
  uint32_t _val;

  IOWR_32_SRS(SD_REG_SRS12, 3);

  _val =
  // [31:16] RCA
      ((sd_card_rca & 0xffff) << 16) |
  // [15] SQS
      ((1 & 0x1) << 15) |
  // [14:1] stuff bits
  // [0] HPI - must be 0
      ((0 & 0x1) << 0);

  check( SDHost_send_cmd((13 << CI) | (3<<CRCCE) | (2<<RTS), _val, 0));
  do {
      _status = IORD_32_SRS(SD_REG_SRS12);
  } while ((_status & (1<<CC) ) == 0);

  IOWR_32_SRS(SD_REG_SRS12, 1);
  cq_status = IORD_32_SRS(SD_REG_SRS4);
  return cq_status;
}

//----------------------------------------------------------------------
// eMMC Command Queue - Put
//----------------------------------------------------------------------
// Putting new task on the queue
//
uint32_t eMMC_card_cq_put (uint32_t cq_task_num)
{
  uint32_t _status;
  uint32_t _val;

  IOWR_32_SRS(SD_REG_SRS12, 3);

  _val =
  // [31] Reliable Write Request
      ((cq_task[cq_task_num].rel_wr_req & 0x1) << 31) |
  // [30] DD: "1" read / "0" write
      ((cq_task[cq_task_num].read & 0x1) << 30) |
  // [29] Tag request
      ((cq_task[cq_task_num].tag_req & 0x1) << 29) |
  // [28:25] Context ID
      ((cq_task[cq_task_num].cont_id & 0xf) << 25) |
  // [24]: Forced Programming
      ((cq_task[cq_task_num].force_prog & 0x1) << 24) |
  // [23] Priority: "0" simple / "1" high
      ((cq_task[cq_task_num].priority & 0x1) << 23) |
  // [22:21] reserved
  // [20:16] Task ID
      ((cq_task[cq_task_num].id & 0x1f) << 16) |
  // [15:0] Number of Blocks
      ((cq_task[cq_task_num].block_cnt & 0xffff) << 0);

  sd_printf("cq_task = 0x%08x\n", cq_task[cq_task_num]);
  sd_printf("_val = 0x%08x\n", _val);

  check( SDHost_send_cmd((44 << CI) | (3<<CRCCE) | (2<<RTS), _val, 0));
  do {
    _status = IORD_32_SRS(SD_REG_SRS12);
  } while ((_status & (1<<CC) ) == 0);

  IOWR_32_SRS(SD_REG_SRS12, 1);
  cq_task[cq_task_num].cmd44_resp = IORD_32_SRS(SD_REG_SRS4);

  _val =
      // [31:0] Start block address
      cq_task[cq_task_num].address;

  check( SDHost_send_cmd((45 << CI) | (3<<CRCCE) | (2<<RTS), _val, 0));
  do {
    _status = IORD_32_SRS(SD_REG_SRS12);
  } while ((_status & (1<<CC) ) == 0);

  IOWR_32_SRS(SD_REG_SRS12, 1);
  cq_task[cq_task_num].cmd45_resp = IORD_32_SRS(SD_REG_SRS4);

  return 0;
}

//----------------------------------------------------------------------
// eMMC Command Queue Execution
//----------------------------------------------------------------------
uint32_t eMMC_card_cq_exec (uint32_t cq_task_num, uint32_t use_adma2)
{
  uint32_t _status;
  uint32_t _val;

  IOWR_32_SRS(SD_REG_SRS12, 3);

  _val =
  // [31:21] reserved
  // [20:16] Task ID
      ((cq_task[cq_task_num].id & 0x1f) << 16);
  // [15:0] reserved

    // DMASEL = 0 - SDMA, DMASEL = 2 - ADMA2
  _status = IORD_32_SRS(SD_REG_SRS10);
  _status &= ~(3 << DMASEL);
  if (use_adma2 == 1) _status |= (2 << DMASEL); // ADMA2
  IOWR_32_SRS (SD_REG_SRS10, _status);        // DMASEL

  IOWR_32_SRS (SD_REG_SRS0, cq_task[cq_task_num].sys_address);  // system memory address
  IOWR_32_SRS (SD_REG_SRS1,
               (7 << HDMABB) |
               (cq_task[cq_task_num].block_cnt << BCCT) |
               SD_HOST_BLOCK_SIZE); // data size (block_cnt * SD_HOST_BLOCK_SIZE)

  if (cq_task[cq_task_num].read)
      _status = (46 << CI);
  else
      _status = (47 << CI);
  _status |= ((1 << DPS) | (3 << CRCCE) | (2 << RTS) | (1 << MSBS) | (0 << ACE) | (1 << DMAE));
  _status |= (1 << BCE);  // specified blocks count

  check( SDHost_send_cmd(_status, _val, 0)); // write
  do {
      _status = IORD_32_SRS(SD_REG_SRS12);
  } while ((_status & (1<<TC) ) == 0);

  IOWR_32_SRS(SD_REG_SRS12, 3);
  cq_task[cq_task_num].cmd_cqexe_resp = IORD_32_SRS(SD_REG_SRS4);
  return 0;
}

//----------------------------------------------------------------------
// eMMC Command Queue Initialization
//----------------------------------------------------------------------
uint32_t eMMC_cq_init ()
{
  int cq_cnt;
  for (cq_cnt=0; cq_cnt<32; cq_cnt++)
  {
      cq_task[cq_cnt].rel_wr_req     = 0; // DESC
      cq_task[cq_cnt].read           = 0; // DESC
      cq_task[cq_cnt].tag_req        = 0; // DESC
      cq_task[cq_cnt].cont_id        = 0; // DESC
      cq_task[cq_cnt].force_prog     = 0; // DESC
      cq_task[cq_cnt].priority       = 0; // DESC
      cq_task[cq_cnt].block_cnt      = 4; // DESC
      cq_task[cq_cnt].address        = 0x00000000; // DESC
      cq_task[cq_cnt].id             = cq_cnt;
      cq_task[cq_cnt].sys_address    = 0x00000000;
      cq_task[cq_cnt].cmd44_resp     = 0;
      cq_task[cq_cnt].cmd45_resp     = 0;
      cq_task[cq_cnt].cmd_cqexe_resp = 0;
      //sd_printf("cq_task[%d] = 0x%08x\n", cq_cnt, cq_task[cq_cnt]);
  }

  return 0;
}

/*----------------------------------------------------------------------
  Update CQ Descriptor
  --------------------------------------------------------------------*/
uint32_t eMMC_cq_desc (uint32_t pos, unsigned char* desc_buff,
                       uint32_t conf, uint32_t len, uint32_t blk_addr,
                       uint64_t sys_addr)
{
    desc_buff[16*pos   ] =  conf            & 0xff;
    desc_buff[16*pos+1 ] = (conf >>  8)     & 0xff;
    desc_buff[16*pos+2 ] = (conf >> 16)     & 0xff;
    desc_buff[16*pos+3 ] = (conf >> 24)     & 0xff;

    desc_buff[16*pos+4 ] =  blk_addr        & 0xff;
    desc_buff[16*pos+5 ] = (blk_addr >>  8) & 0xff;
    desc_buff[16*pos+6 ] = (blk_addr >> 16) & 0xff;
    desc_buff[16*pos+7 ] = (blk_addr >> 24) & 0xff;

    desc_buff[16*pos+8 ] =  len             & 0xff;
    desc_buff[16*pos+9 ] = (len >>  8)      & 0xff;
    desc_buff[16*pos+10] = (len >> 16)      & 0xff;
    desc_buff[16*pos+11] = (len >> 24)      & 0xff;

    desc_buff[16*pos+12] =  sys_addr        & 0xff;
    desc_buff[16*pos+13] = (sys_addr >>  8) & 0xff;
    desc_buff[16*pos+14] = (sys_addr >> 16) & 0xff;
    desc_buff[16*pos+15] = (sys_addr >> 24) & 0xff;

    return 0;
}
uint32_t eMMC_cq_desc_64 (uint32_t pos, unsigned char* desc_buff,
                          uint32_t conf, uint32_t len, uint32_t blk_addr,
                          uint64_t sys_addr)
{
    desc_buff[32*pos   ] =  conf            & 0xff;
    desc_buff[32*pos+1 ] = (conf >>  8)     & 0xff;
    desc_buff[32*pos+2 ] = (conf >> 16)     & 0xff;
    desc_buff[32*pos+3 ] = (conf >> 24)     & 0xff;

    desc_buff[32*pos+4 ] =  blk_addr        & 0xff;
    desc_buff[32*pos+5 ] = (blk_addr >>  8) & 0xff;
    desc_buff[32*pos+6 ] = (blk_addr >> 16) & 0xff;
    desc_buff[32*pos+7 ] = (blk_addr >> 24) & 0xff;

    desc_buff[32*pos+8 ] = 0;
    desc_buff[32*pos+9 ] = 0;
    desc_buff[32*pos+10] = 0;
    desc_buff[32*pos+11] = 0;

    desc_buff[32*pos+12] = 0;
    desc_buff[32*pos+13] = 0;
    desc_buff[32*pos+14] = 0;
    desc_buff[32*pos+15] = 0;

    desc_buff[32*pos+16] =  len             & 0xff;
    desc_buff[32*pos+17] = (len >>  8)      & 0xff;
    desc_buff[32*pos+18] = (len >> 16)      & 0xff;
    desc_buff[32*pos+19] = (len >> 24)      & 0xff;

    desc_buff[32*pos+20] =  sys_addr        & 0xff;
    desc_buff[32*pos+21] = (sys_addr >>  8) & 0xff;
    desc_buff[32*pos+22] = (sys_addr >> 16) & 0xff;
    desc_buff[32*pos+23] = (sys_addr >> 24) & 0xff;

    desc_buff[32*pos+24] = (sys_addr >> 32) & 0xff;
    desc_buff[32*pos+25] = (sys_addr >> 40) & 0xff;
    desc_buff[32*pos+26] = (sys_addr >> 48) & 0xff;
    desc_buff[32*pos+27] = (sys_addr >> 56) & 0xff;

    desc_buff[32*pos+28] = 0;
    desc_buff[32*pos+29] = 0;
    desc_buff[32*pos+30] = 0;
    desc_buff[32*pos+31] = 0;

    return 0;
}
/*----------------------------------------------------------------------
  Update CQ Descriptor (DCMD)
  --------------------------------------------------------------------*/
uint32_t eMMC_cq_desc_dcmd (uint32_t pos, unsigned char* desc_buff,
                            uint32_t conf, uint32_t arg)
{
    int i;
    desc_buff[16*pos   ] = (uint8_t)( conf        & 0xff);
    desc_buff[16*pos+1 ] = (uint8_t)((conf >>  8) & 0xff);
    desc_buff[16*pos+2 ] = (uint8_t)((conf >> 16) & 0xff);
    desc_buff[16*pos+3 ] = (uint8_t)((conf >> 24) & 0xff);

    desc_buff[16*pos+4 ] = (uint8_t)( arg         & 0xff);
    desc_buff[16*pos+5 ] = (uint8_t)((arg  >>  8) & 0xff);
    desc_buff[16*pos+6 ] = (uint8_t)((arg  >> 16) & 0xff);
    desc_buff[16*pos+7 ] = (uint8_t)((arg  >> 24) & 0xff);

    for (i=8; i<16; i++)
        desc_buff[16*pos+i] = 0xff;

    return 0;
}

uint32_t eMMC_cq_desc_64_dcmd (uint32_t pos, unsigned char* desc_buff,
                               uint32_t conf, uint32_t arg)
{
    int i;
    desc_buff[32*pos   ] = (uint8_t)( conf        & 0xff);
    desc_buff[32*pos+1 ] = (uint8_t)((conf >>  8) & 0xff);
    desc_buff[32*pos+2 ] = (uint8_t)((conf >> 16) & 0xff);
    desc_buff[32*pos+3 ] = (uint8_t)((conf >> 24) & 0xff);

    desc_buff[32*pos+4 ] = (uint8_t)( arg         & 0xff);
    desc_buff[32*pos+5 ] = (uint8_t)((arg  >>  8) & 0xff);
    desc_buff[32*pos+6 ] = (uint8_t)((arg  >> 16) & 0xff);
    desc_buff[32*pos+7 ] = (uint8_t)((arg  >> 24) & 0xff);

    for (i=8; i<32; i++)
        desc_buff[32*pos+i] = 0xff;

    return 0;
}

/*----------------------------------------------------------------------
  Display CQ Descriptor
  --------------------------------------------------------------------*/
uint32_t eMMC_cq_desc_disp (unsigned char* desc_buff)
{
    uint32_t i, j, k;
    for (i=0; i<32; i++)
    {
        sd_printf("0x%016x DESC[%02d]", (uint32_t)desc_buff + 4*(i*4), i);
        for (j=0; j<4; j++)
        {
            sd_printf(" ");
            for (k=0; k<4; k++)
            {
                sd_printf("%02x", desc_buff[4*(i*4+j)+k]);
            }
        }
        sd_printf("\n");
    }
    return 0;
}

uint32_t eMMC_cq_desc_64_disp (unsigned char* desc_buff)
{
    uint32_t i, j, k;
    for (i=0; i<64; i++)
    {
        sd_printf("0x%016x DESC[%02d]", (uint32_t)desc_buff + 4*(i*4), i);
        for (j=0; j<4; j++)
        {
            sd_printf(" ");
            for (k=0; k<4; k++)
            {
                sd_printf("%02x", desc_buff[4*(i*4+j)+k]);
            }
        }
        sd_printf("\n");
    }
    return 0;
}

/*----------------------------------------------------------------------
  Halt request
  --------------------------------------------------------------------*/
void eMMC_CQ_halt()
{
    uint32_t data;

    /* Halt request */
    sd_printf("Halt requested\n");
    IOWR_32_SRS(SD_REG_CQRS03, 0x1);

    while (1)
    {
        /* Wait for Halt acknowledge */
        data = IORD_32_SRS(SD_REG_CQRS03);
        if (data & 1)
            break;
        //sd_printf(".");
    }
    sd_printf("Halt acknowledged\n");
    return;
}

/*----------------------------------------------------------------------
  Release after halt
  --------------------------------------------------------------------*/
void eMMC_CQ_release()
{
    /* Release request */
    sd_printf("Release requested\n");
    IOWR_32_SRS(SD_REG_CQRS03, 0x0);
    return;
}

/*----------------------------------------------------------------------
  Software reset for CMD/DAT
  --------------------------------------------------------------------*/
void eMMC_CQ_reset()
{
    uint32_t _status;
    uint32_t _srs11;
    _status = IORD_32_SRS(SD_REG_SRS12);
    if (_status & (0xf << 16))
    {
        sd_printf("Software reset for CMD\n");
        _srs11 = IORD_32_SRS(SD_REG_SRS11);
        _srs11 |= (1 << SRCMD);
        IOWR_32_SRS(SD_REG_SRS11, _srs11);
    }
    if (_status & (0x7 << 20))
    {
        sd_printf("Software reset for DAT\n");
        _srs11 = IORD_32_SRS(SD_REG_SRS11);
        _srs11 |= (1 << SRDAT);
        IOWR_32_SRS(SD_REG_SRS11, _srs11);
    }
    if (_status & (0x1 << 24))
    {
        sd_printf("Software reset for CMD (Auto CMD)\n");
        _srs11 = IORD_32_SRS(SD_REG_SRS11);
        _srs11 |= (1 << SRCMD);
        IOWR_32_SRS(SD_REG_SRS11, _srs11);
    }

    /* Check if Software reset is completed */
    do {
        _srs11 = IORD_32_SRS(SD_REG_SRS11);
    } while ((_srs11 & ((1<<SRCMD) | (1<<SRDAT))) != 0);
    sd_printf("Software reset finished\n");
    return;
}

/*----------------------------------------------------------------------
  Discard entire queue
  --------------------------------------------------------------------*/
int eMMC_CQ_discard_all()
{
    uint32_t _val;
    uint32_t _status;

    sd_printf("Issue CMD48 (Discard entire queue)\n");
    _val = 1; // OP-Code=1
    check( SDHost_send_cmd((48 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
    // response R1b - busy signal is transmitted on the DAT0 line
    sd_printf("Waiting for CC and TC\n");
    while (1)
    {
        _status = IORD_32_SRS(SD_REG_SRS12);
        if ((_status & 0x3) == ((1<<CC) | (1<<TC)) )
            break;
    }

    return 0;
}

/*----------------------------------------------------------------------
  Discard selected task
  --------------------------------------------------------------------*/
int eMMC_CQ_discard(int task_id)
{
    uint32_t _val;
    uint32_t _status;

    sd_printf("Issue CMD48 (Discard task %2d)\n", task_id);
    _val = (task_id << 16) | 2; // Task ID & OP-Code=2
    check( SDHost_send_cmd((48 << CI) | (3<<CRCCE) | (3<<RTS), _val, 0) );
    // response R1b - busy signal is transmitted on the DAT0 line
    sd_printf("Waiting for CC and TC\n");
    while (1)
    {
        _status = IORD_32_SRS(SD_REG_SRS12);
        if ((_status & 0x3) == ((1<<CC) | (1<<TC)) )
            break;
    }

    return 0;
}

int eMMC_CQ_is_error()
{
    uint32_t _data;
    int _status = 0;
    _data = IORD_32_SRS(SD_REG_SRS12);

    /* CMD Error */
    if ((_data >> ETC) & 0xf)
        _status |= (1 << 0);

    /* DAT Error */
    if ((_data >> EDT) & 0x7)
        _status |= (1 << 1);

    /* Auto CMD Error */
    if ((_data >> EA12) & 1)
        _status |= (1 << 2);

    /* DMA error */
    if ((_data >> EADMA) & 1)
        _status |= (1 << 3);

    return _status;
}

int eMMC_CQ_error_recovery(uint16_t rca)
{
    int _task_id;
    uint32_t _status;
    uint32_t _task_complete_notification;
    uint32_t _task_error_information;
    uint32_t _device_pending_task;
    int _task_id_transfer_valid;
    int _task_id_transfer_error;
    int _task_id_response_valid;
    int _task_id_response_error;
    int _task_id_response_cmdidx;
    int _err_id;
    uint32_t _clear_status;

    eMMC_CQ_halt();
    eMMC_CQ_reset();

    _status = IORD_32_SRS(SD_REG_SRS12);
    _device_pending_task = IORD_32_SRS(SD_REG_CQRS13);
    _task_complete_notification = IORD_32_SRS(SD_REG_CQRS11);
    _task_error_information = IORD_32_SRS(SD_REG_CQRS21);
    _task_id_transfer_valid = (_task_error_information >> 31) & 1;
    _task_id_response_valid = (_task_error_information >> 15) & 1;
    _task_id_transfer_error = (_task_error_information >> 24) & 0x1f;
    _task_id_response_error = (_task_error_information >>  8) & 0x1f;
    _task_id_response_cmdidx = (_task_error_information >> 0) & 0x3f;

    sd_printf("SRS12 status %8x\n", _status);

    /* DAT Error */
    if ((_status >> 20) & 0x7)
        eMMC_CQ_abort(rca);

    /* DMA error */
    else if ((_status >> 25) & 0x1)
        eMMC_CQ_abort(rca);


    /* Data present task */

    if (_task_complete_notification != 0)
    {
        /* Some of the tasks are completed */
        for (_task_id=0; _task_id<32; _task_id++)
        {
            if ((_task_complete_notification >> _task_id) & 1)
            {
                _err_id = 0;

                if (_task_id_transfer_valid &&
                    _task_id_transfer_error == _task_id)
                {
                    sd_printf("Task %2d transfer err detected\n", _task_id);

                    if ((_status >> 16) & 0xf)
                        _err_id = 1 << 0;

                    if ((_status >> 20) & 0x7)
                        _err_id = 1 << 1;

                    if ((_status >> 25) & 0x1)
                        _err_id = 1 << 2;
                }

                if (_task_id_response_valid &&
                    _task_id_response_error == _task_id)
                {
                    sd_printf("Task %2d response err detected\n", _task_id);

                    if ((_status >> 16) & 0xf)
                        _err_id = 1 << 0;

                    if ((_status >> 20) & 0x7)
                        _err_id = 1 << 1;

                    if ((_status >> 25) & 0x1)
                        _err_id = 1 << 2;
                }

                if (_err_id == 0)
                {
                    sd_printf("Task %2d completed\n", _task_id);
                }
                else if (_err_id)
                {
                    sd_printf("Task %2d error (stat: %0x)\n", _task_id,
                              _device_pending_task);
                    eMMC_CQ_discard(_task_id);

                    IOWR_32_SRS(SD_REG_CQRS14, (1 << _task_id));
                    do {
                        _clear_status = IORD_32_SRS(SD_REG_CQRS14);
                    } while (_clear_status);
                }
            }
        }
    }

    /* eMMC_CQ_discard_all(); */

    /* Clear Error Interrupt Status */
    IOWR_32_SRS(SD_REG_SRS12, 0xffff0000);
    eMMC_CQ_release();

    return 0;
}


int eMMC_CQ_abort(uint16_t rca)
{
    uint32_t _status12;
    uint32_t _status13;
    uint32_t _status;
    char _tmp_buff[65];
    uint32_t * _tmp_word = (uint32_t *)_tmp_buff;
    _tmp_word[0] = 0x00000000 |  (1<<31);
    _tmp_word[1] = 0x00000000;
    _tmp_word[2] = 0x00000000;
    _tmp_word[3] = 0x00000000;
    _tmp_word[4] = 0x00000000;
    _tmp_word[5] = 0x00000000;
    _tmp_word[6] = 0x00000000;
    _tmp_word[7] = 0x00000000;
    _tmp_word[8] = 0x00000000;
    _tmp_word[9] = 0x00000000;
    _tmp_word[10] = 0x00000000;
    _tmp_word[11] = 0x00000000;
    _tmp_word[12] = 0x00000000;
    _tmp_word[13] = 0x00000000;
    _tmp_word[14] = 0x00000000;
    _tmp_word[15] = 0x00000000;
    
    /* Disable VIP messages (possible Illegal CMD12) */
    //gpio_set_tri( 0);
    //gpio_write( 2);
    sd_printf("Try to use serdes write.");
    serdes_write(_tmp_buff,512);
    _status12 = SDHost_send_cmd( (12 << CI) | /* Index: 12 */
                                 (3<<CT)    | /* Command Type */
                                 (3<<CRCCE) | /* Index & CRC check enabled */
                                 (3<<RTS),    /* Response Type: R1b */
                                 0x00000000,  /* Argument: 0x0 */
                                 0);          /* Do not used int */

    /* Wait for Transfer Complete */
    do {
        _status = IORD_32_SRS(SD_REG_SRS12);
    } while((_status & (1<<TC)) == 0);

    /* Enable VIP messages */
     _tmp_word[0] = 0x00000000;
   //gpio_write(0);
    serdes_write(_tmp_buff,512);
    if (_status12 >> 15) /* Error detected */
    {
        if ((_status12 >> 16) & 0xf) /* Command error */
        {
            eMMC_CQ_reset();

            _status13 = SDHost_send_cmd( (13 << CI) | /* Index: 13 */
                                         (3<<CRCCE) | /* Index & CRC check enabled */
                                         (2<<RTS),    /* Response Type: R1 */
                                         rca<<16,     /* Argument: RCA<<16 */
                                         0);          /* Do not used int */

            if ((_status13 >> 16) & 0xf) /* Command error */
            {
                sd_printf("ERROR unrecoverable error - invalid or no response after CMD13\r");
                return 1;
            }
            return 0;
        }

        if ((_status12 >> 20) & 0x7) /* Timeout error */
        {
            sd_printf("ERROR unrecoverable error - timeout error after CMD12\r");
            return 1;
        }
    }
    return 0;
}


uint32_t eMMC_CQ_task_submit(uint32_t _var)
{
    IOWR_32_SRS(SD_REG_CQRS10, _var);
    sd_printf("CQRS10 (TaskDoorBell) = 0x%08x\n", IORD_32_SRS(SD_REG_CQRS10));
    return _var;
}


int count_onces(uint32_t vector)
{
    int i;
    int result = 0;

    for (i=0; i<32; i++)
        if (vector & (1<<i))
            result++;
    return result;
}

int task_id(uint32_t vector)
{
    int i;
    for (i=0; i<32; i++)
        if (vector & (1<<i))
            return i;
    return 0;
}


uint32_t eMMC_CQ_taskdoorbell()
{
    return IORD_32_SRS(SD_REG_CQRS10);
}

uint32_t eMMC_CQ_task_complete_notificaiton()
{
    return IORD_32_SRS(SD_REG_CQRS11);
}
