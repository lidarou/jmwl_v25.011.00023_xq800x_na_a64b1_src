/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef UX_DCD_AIC_H
#define UX_DCD_AIC_H

#include <stdint.h>
#include <usb_device.h>

/* Define AIC generic equivalences.  */

#define UX_DCD_AIC_SLAVE_CONTROLLER                           0x80
#ifndef UX_DCD_AIC_MAX_ED
#define UX_DCD_AIC_MAX_ED                                     4
#endif /* UX_DCD_AIC_MAX_ED */
#define UX_DCD_AIC_IN_FIFO                                    3


#define UX_DCD_AIC_FLUSH_RX_FIFO                              0x00000010
#define UX_DCD_AIC_FLUSH_TX_FIFO                              0x00000020
#define UX_DCD_AIC_FLUSH_FIFO_ALL                             0x00000010
#define UX_DCD_AIC_ENDPOINT_SPACE_SIZE                        0x00000020
#define UX_DCD_AIC_ENDPOINT_CHANNEL_SIZE                      0x00000020


/* Define USB AIC physical endpoint status definition.  */

#define UX_DCD_AIC_ED_STATUS_UNUSED                            0
#define UX_DCD_AIC_ED_STATUS_USED                              1
#define UX_DCD_AIC_ED_STATUS_TRANSFER                          2
#define UX_DCD_AIC_ED_STATUS_STALLED                           4

/* Define USB AIC physical endpoint state machine definition.  */

#define UX_DCD_AIC_ED_STATE_IDLE                               0
#define UX_DCD_AIC_ED_STATE_DATA_TX                            1
#define UX_DCD_AIC_ED_STATE_DATA_RX                            2
#define UX_DCD_AIC_ED_STATE_STATUS_TX                          3
#define UX_DCD_AIC_ED_STATE_STATUS_RX                          4

/* Define USB AIC endpoint transfer status definition.  */

#define UX_DCD_AIC_ED_TRANSFER_STATUS_IDLE                     0
#define UX_DCD_AIC_ED_TRANSFER_STATUS_SETUP                    1
#define UX_DCD_AIC_ED_TRANSFER_STATUS_IN_COMPLETION            2
#define UX_DCD_AIC_ED_TRANSFER_STATUS_OUT_COMPLETION           3


typedef struct UX_DCD_AIC_ED_STRUCT
{
    UCHAR ux_dcd_aic_ed_status;
    UCHAR ux_dcd_aic_ed_state;
    UCHAR ux_dcd_aic_ed_index;
    UCHAR ux_dcd_aic_ed_direction;
    struct UX_SLAVE_ENDPOINT_STRUCT
                    *ux_dcd_aic_ed_endpoint;
} UX_DCD_AIC_ED;

typedef struct UX_DCD_AIC_STRUCT
{
    struct UX_SLAVE_DCD_STRUCT
                        *ux_dcd_aic_dcd_owner;
    struct UX_DCD_AIC_ED_STRUCT
                        ux_dcd_aic_ed[UX_MAX_ED];
    UDC_Device *usb_dev;
} UX_DCD_AIC;

/* Define USB AIC DCD prototypes.  */
UINT _ux_dcd_aic_function(UX_SLAVE_DCD *dcd, UINT function,
            VOID *parameter);
UINT _ux_dcd_aic_initialize(UDC_Device *ud);
UINT _ux_dcd_aic_initialize_complete(VOID);
UINT _ux_dcd_aic_endpoint_stall(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint);
void _ux_dcd_aic_DataInStageCallback(uint8_t epnum);
void _ux_dcd_aic_DataOutStageCallback(uint8_t epnum);
UINT _ux_dcd_aic_transfer_request(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_TRANSFER *transfer_request);
UINT _ux_dcd_aic_endpoint_create(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint);
UINT _ux_dcd_aic_endpoint_reset(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint);
UINT _ux_dcd_aic_endpoint_status(UX_DCD_AIC *dcd_aic,
                ULONG endpoint_index);
UINT _ux_dcd_aic_frame_number_get(UX_DCD_AIC *dcd_aic,
                ULONG *frame_number);
UINT  _ux_dcd_aic_endpoint_destroy(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint);
UINT  _ux_dcd_aic_transfer_abort(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_TRANSFER *transfer_request);

#define ux_dcd_aic_initialize _ux_dcd_aic_initialize
#endif /* UX_DCD_AIC_H */
