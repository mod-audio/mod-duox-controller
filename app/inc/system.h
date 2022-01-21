
/*
************************************************************************************************************************
* This library defines the system related functions, inclusive the system menu callbacks
************************************************************************************************************************
*/

#ifndef SYSTEM_H
#define SYSTEM_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include "data.h"

#include <stdint.h>

/*
************************************************************************************************************************
*           DO NOT CHANGE THESE DEFINES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           CONFIGURATION DEFINES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           GLOBAL VARIABLES
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*           MACRO'S
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

// system menu callbacks
//used when opening profiles menu
uint8_t system_get_current_profile(void);
//used when changing the master volume from control mode
float system_master_volume_cb(float value, int event);
//used when a gain item is closed
void system_save_gains_cb(void *arg, int event);
//called from various places in the system to change a menu item value
void system_update_menu_value(uint8_t item_ID, uint16_t value);

//used to save or reset the current loaded PB
void system_pedalboard_cb(void *arg, int event);
//used for all bluetooth actions
void system_bluetooth_cb(void *arg, int event);

//control for input 1 + 2
void system_inp_0_volume_cb(void *arg, int event);
//control for input 1
void system_inp_1_volume_cb(void *arg, int event);
//control for input 2
void system_inp_2_volume_cb(void *arg, int event);
//control for output 1 + 2
void system_outp_0_volume_cb(void *arg, int event);
//control for output 1
void system_outp_1_volume_cb(void *arg, int event);
//control for output 2
void system_outp_2_volume_cb(void *arg, int event);
//control headphone volume
void system_hp_volume_cb(void *arg, int event);

//IO processing parts
//sets noisegate channel
void system_ng_channel(void *arg, int event);
//sets noisegate threshold
void system_ng_threshold(void *arg, int event);
//sets noisegate decay
void system_ng_decay(void *arg, int event);
//sets compressor mode
void system_compressor_mode(void *arg, int event);
//sets compressor release
void system_compressor_release(void *arg, int event);
//sets pedalboard gain
void system_pb_gain(void *arg, int event);

void system_services_cb(void *arg, int event);
void system_versions_cb(void *arg, int event);
void system_release_cb(void *arg, int event);
void system_device_cb(void *arg, int event);
void system_tag_cb(void *arg, int event);
void system_upgrade_cb(void *arg, int event);
void system_volume_cb(void *arg, int event);

void system_banks_cb(void *arg, int event);
void system_display_cb(void *arg, int event);
void system_display_contrast_lcb(void *arg, int event);
void system_display_contrast_rcb(void *arg, int event);
void system_sl_in_cb (void *arg, int event);
void system_sl_out_cb (void *arg, int event);
void system_cv_exp_cb (void *arg, int event);
void system_cv_hp_cb (void *arg, int event);
void system_tuner_cb (void *arg, int event);
void system_play_cb (void *arg, int event);
void system_quick_bypass_cb (void *arg, int event);
void system_midi_src_cb (void *arg, int event);
void system_midi_send_cb (void *arg, int event);
void system_ss_prog_change_cb (void *arg, int event);
void system_pb_prog_change_cb (void *arg, int event);
void system_tempo_cb (void *arg, int event);
void system_bpb_cb (void *arg, int event);
void system_bypass_cb (void *arg, int event);
void system_load_pro_cb(void *arg, int event);
void system_save_pro_cb(void *arg, int event);
void system_master_vol_link_cb(void *arg, int event);
void system_exp_mode_cb(void *arg, int event);

void system_master_volume_link(uint8_t link_value);
void system_qbp_channel_cb (void *arg, int event);
void system_hide_actuator_cb(void *arg, int event);
void system_lock_pots_cb(void *arg, int event);
void system_pot_callibration(void *arg, int event);
void system_page_mode_cb(void *arg, int event);
void system_led_brightness_cb(void *arg, int event);

//set the USB B mode of the device, NEEDS A REBOOT
void system_usb_b_cb(void *arg, int event);

/*
************************************************************************************************************************
*           CONFIGURATION ERRORS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           END HEADER
************************************************************************************************************************
*/

#endif
