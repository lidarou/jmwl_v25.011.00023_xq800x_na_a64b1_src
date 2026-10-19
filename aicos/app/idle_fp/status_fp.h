

#ifndef __STATUS_FP_H__
#define __STATUS_FP_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void(*status_radio_func_callback) (uint32_t radio_type,void * user_data);


    void status_radio_set_func_callback(status_radio_func_callback fun,void * user_data);

    uint8_t get_radio_signal_type();
    uint8_t get_radio_signal_lev();
    uint8_t get_radio_ims_status();
    uint8_t * get_radio_operator_name();



    typedef void(*status_bat_func_callback) (void * user_data);

    void status_bat_set_func_callback(status_bat_func_callback fun,void * user_data);

    uint8_t get_bat_capacity();

    uint8_t get_bat_chg_status();

    bool get_is_sim_inited_status();

    void status_fp_init(void);
    bool get_is_sim_exist_status();

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /* __STATUS_FP_H__ */

