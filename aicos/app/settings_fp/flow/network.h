

/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *  This file is obtain sim card information.
 */
#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /**********************
     * GLOBAL PROTOTYPES
     **********************/

    /**
    *network_Flight_mode_resp
    *@param: [IN] callback response data.
    *@size: [IN] callback response data size.
    * Return: 0 is set flight mode succeed, outher fail.
    **/
    int32_t network_Flight_mode_resp(void * param,uint32_t size);

    /**
    *network_select_sim_resp
    *@param: [IN] callback response data.
    *@size: [IN] callback response data size.
    *@select_sim_open_radio_callback: [IN] select sim radio callback.
    * Return: 0 is select sim succeed, outher fail.
    **/
    int32_t network_select_sim_resp(void * p_param,uint32_t size,void * select_sim_open_radio_callback);

    /**
    network_get_sim_status
    *@cur_card_id: [IN] id of the card currently connected to the network.
    * Return: if no sim card return 0, else return other value.
    **/
    uint32_t network_get_sim_status(uint8_t * cur_card_id);

    /**
    network_Flight_radio_on
    *@cur_card_id: [IN] id of the card currently connected to the network.
    *@set_radio_status: [IN] true-open radio, false-close radio.
    *@flight_mode_callback: [IN] set radio callback.
    * Return: if no sim card return 0, else return other value.
    **/
    int32_t network_Flight_radio_on(uint8_t cur_card_id,bool set_radio_status,void * flight_mode_callback);

    /**
    network_volte_enable
    *@cur_card_id: [IN] id of the card currently connected to the network.
    *@volte_switch: [IN] true-open volte, false-close volte
    * Return: if no sim card return 0, else return other value.
    **/
    int32_t network_volte_enable(uint8_t cur_card_id,bool volte_switch);

    /**
    *network_mode_init
    *@flight_mode: [OUT] get radio status .
    *@volte_mode: [OUT] get volte status.
    * Return: if no sim card return 0, else return other value.
    **/
    void network_mode_init(bool * flight_mode,bool * volte_mode);

    /**
    *network_select_sim_card
    *@cur_card_id: [IN] Switch to this card id.
    *@select_sim_callback: [IN] set select sim callback.
    * Return: if no sim card return 0, else return other value.
    **/
    int32_t network_select_sim_card(uint8_t cur_card_id,void * select_sim_callback);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __NETWORK_H__ */

