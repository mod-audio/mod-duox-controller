
/*
************************************************************************************************************************
*
************************************************************************************************************************
*/

#ifndef NAVEG_H
#define NAVEG_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdbool.h>
#include <stdint.h>
#include "data.h"
#include "glcd_widget.h"


/*
************************************************************************************************************************
*           DO NOT CHANGE THESE DEFINES
************************************************************************************************************************
*/

#define ALL_EFFECTS     -1
#define ALL_CONTROLS    ":all"
enum {UI_DISCONNECTED, UI_CONNECTED};


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
bool g_protocol_busy;
bool g_self_test_mode;
bool g_self_test_cancel_button;
bool g_ui_communication_started;
bool g_device_booted;
bool g_dialog_active;
float g_pot_calibrations[2][POTS_COUNT];

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

// initialize the navigation nodes and structs
void naveg_init(void);
// sets the initial state of banks/pedalboards navigation
void naveg_initial_state(uint16_t max_menu, uint16_t page_min, uint16_t page_max, char *bank_uid, char *pedalboard_uid, char **pedalboards_list);
// tells to navigation core the UI status
void naveg_ui_connection(uint8_t status);
// adds the control to end of the controls list
void naveg_add_control(control_t *control, uint8_t protocol);
// removes the control from controls list
void naveg_remove_control(uint8_t hw_id);
// increment the control value
void naveg_inc_control(uint8_t display);
// decrement the control value
void naveg_dec_control(uint8_t display);
// sets the control value
void naveg_set_control(uint8_t hw_id, float value);
// gets the control value
float naveg_get_control_value(uint8_t hw_id);
control_t* naveg_get_control(uint8_t hw_id);
// change the foot value
void naveg_foot_change(uint8_t foot, uint8_t pressed);
void naveg_pot_change(uint8_t pot);
// toggle between control and tool
void naveg_toggle_tool(uint8_t tool, uint8_t display);
//save snapshots
void naveg_save_snapshot(uint8_t foot);
//clear snapshots
void naveg_clear_snapshot(uint8_t foot);
// returns if display is in tool mode
uint8_t naveg_is_tool_mode(uint8_t display);
//toggle / set master volume 
void naveg_master_volume(uint8_t set);
//increments the mater volume
void naveg_set_master_volume(uint8_t set);
//returns if current state is master volume
uint8_t naveg_is_master_vol(void);
// stores the banks list
void naveg_set_banks(bp_list_t *bp_list);
// returns the banks list
bp_list_t *naveg_get_banks(void);
// configurates the bank
void naveg_bank_config(bank_config_t *bank_conf);
// stores the pedalboards list of current bank
void naveg_set_pedalboards(bp_list_t *bp_list);
// returns the pedalboards list of current bank
bp_list_t *naveg_get_pedalboards(void);
// runs the enter action on tool mode
void naveg_enter(uint8_t display);
// runs the up action on tool mode
void naveg_up(uint8_t display);
// runs the down action on tool mode
void naveg_down(uint8_t display);
// resets to root menu
void naveg_reset_menu(void);
// update the navigation screen if necessary
void naveg_update(void);
int naveg_need_update(void);

void naveg_set_active_pedalboard(uint8_t pedalboard_index);
void naveg_set_pb_list_update(void);
bool naveg_get_pb_list_update(void);
void naveg_update_pb_list(void);

uint8_t naveg_dialog(const char *msg, char *title);
uint8_t naveg_ui_status(void);
void naveg_pages_available(uint8_t page_1, uint8_t page_2, uint8_t page_3, uint8_t page_4, uint8_t page_5, uint8_t page_6);
//refreshes screen with current menu item
void naveg_settings_refresh(uint8_t display_id);
//returns if tap tempo enabled 
uint8_t naveg_tap_tempo_status(uint8_t id);
//updates all items in a menu
void naveg_menu_refresh(uint8_t display_id);
//updates a specific item in a menu
void naveg_update_gain(uint8_t display_id, uint8_t update_id, float value, float min, float max);

void naveg_bypass_refresh(uint8_t bypass_1, uint8_t bypass_2, uint8_t quick_bypass);

void naveg_set_acceleration(uint8_t display_id);

void naveg_print_pb_name(uint8_t display);

void naveg_lock_pots(uint8_t lock);

void naveg_set_page_mode(uint8_t mode);

void naveg_menu_item_changed_cb(uint8_t item_ID, uint16_t value);

uint8_t naveg_banks_mode_pb(void);

char* naveg_get_current_pb_name(void);

void naveg_reset_page(void);

void naveg_draw_foot(control_t *control);

uint8_t naveg_dialog_status(void);

void naveg_update_calibration(void);

void naveg_turn_on_pagination_leds(void);

void naveg_reload_display(void);

uint8_t naveg_get_actuator_type(uint8_t hw_id);

menu_item_t *naveg_get_menu_item_by_ID(uint16_t menu_id);

void naveg_set_reboot_value(uint8_t boot_value);

void naveg_turn_off_leds(void);

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
