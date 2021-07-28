
/*
************************************************************************************************************************
*
************************************************************************************************************************
*/

#ifndef PROTOCOL_H
#define PROTOCOL_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdint.h>
#include "comm.h"


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

// defines the function to send responses to sender
#define SEND_TO_SENDER(id,msg,len)      comm_webgui_send(msg,len)


/*
************************************************************************************************************************
*           DATA TYPES
************************************************************************************************************************
*/

// This struct is used on callbacks argument
typedef struct PROTO_T {
    char **list;
    uint32_t list_count;
    char *response;
    uint32_t response_size;
} proto_t;

// This struct must be used to pass a message to protocol parser
typedef struct MSG_T {
    int sender_id;
    char *data;
    uint32_t data_size;
} msg_t;


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

void protocol_init(void);
void protocol_parse(msg_t *msg);
void protocol_add_command(const char *command, void (*callback)(proto_t *proto));
void protocol_response(const char *response, proto_t *proto);
void protocol_remove_commands(void);


//callbacks
void cb_ping(proto_t *proto);
void cb_say(proto_t *proto);
void cb_led(proto_t *proto);
void cb_glcd_text(proto_t *proto);
void cb_glcd_dialog(proto_t *proto);
void cb_glcd_draw(proto_t *proto);
void cb_gui_connection(proto_t *proto);
void cb_disp_brightness(proto_t *proto);
void cb_control_add(proto_t *proto);
void cb_control_rm(proto_t *proto);
void cb_control_set(proto_t *proto);
void cb_control_get(proto_t *proto);
void cb_control_set_index(proto_t *proto);
void cb_initial_state(proto_t *proto);
void cb_bank_config(proto_t *proto);
void cb_tuner(proto_t *proto);
void cb_resp(proto_t *proto);
void cb_restore(proto_t *proto);
void cb_boot(proto_t *proto);
void cb_menu_item_changed(proto_t *proto);
void cb_pedalboard_clear(proto_t *proto);
void cb_pedalboard_name(proto_t *proto);
void cb_pages_available(proto_t *proto);
void cb_save_pot_cal_val(proto_t *proto);
void cb_check_cal(proto_t *proto);
void cb_set_selftest_control_skip(proto_t *proto);
void cb_clear_eeprom(proto_t *proto);
void cb_set_disp_contrast(proto_t *proto);
void cb_exp_overcurrent(proto_t *proto);

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
