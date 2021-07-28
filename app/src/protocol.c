
/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <string.h>
#include <stdlib.h>

#include "protocol.h"
#include "utils.h"
#include "naveg.h"
#include "ledz.h"
#include "hardware.h"
#include "glcd.h"
#include "utils.h"
#include "screen.h"
#include "cli.h"
#include "calibration.h"
#include "mod-protocol.h"

/*
************************************************************************************************************************
*           LOCAL DEFINES
************************************************************************************************************************
*/

#define NOT_FOUND           (-1)
#define MANY_ARGUMENTS      (-2)
#define FEW_ARGUMENTS       (-3)
#define INVALID_ARGUMENT    (-4)


/*
************************************************************************************************************************
*           LOCAL CONSTANTS
************************************************************************************************************************
*/

const char *g_error_messages[] = {
    RESP_ERR_COMMAND_NOT_FOUND,
    RESP_ERR_MANY_ARGUMENTS,
    RESP_ERR_FEW_ARGUMENTS,
    RESP_ERR_INVALID_ARGUMENT
};


/*
************************************************************************************************************************
*           LOCAL DATA TYPES
************************************************************************************************************************
*/

typedef struct CMD_T {
    char* command;
    char** list;
    uint32_t count;
    void (*callback)(proto_t *proto);
} cmd_t;


/*
************************************************************************************************************************
*           LOCAL MACROS
************************************************************************************************************************
*/

#define MAP(x, Omin, Omax, Nmin, Nmax)      ( x - Omin ) * (Nmax -  Nmin)  / (Omax - Omin) + Nmin;

/*
************************************************************************************************************************
*           LOCAL GLOBAL VARIABLES
************************************************************************************************************************
*/

static unsigned int g_command_count = 0;
static cmd_t g_commands[COMMAND_COUNT_DUOX];


/*
************************************************************************************************************************
*           LOCAL FUNCTION PROTOTYPES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL CONFIGURATION ERRORS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL FUNCTIONS
************************************************************************************************************************
*/

static int is_wildcard(const char *str)
{
    if (str && strchr(str, '%') != NULL) return 1;
    return 0;
}


/*
************************************************************************************************************************
*           GLOBAL FUNCTIONS
************************************************************************************************************************
*/

void protocol_parse(msg_t *msg)
{
    uint32_t i, j;
    int32_t index = NOT_FOUND;
    proto_t proto;

    proto.list = strarr_split(msg->data, ' ');
    proto.list_count = strarr_length(proto.list);
    proto.response = NULL;

    // TODO: check invalid argumets (wildcards)

    if (proto.list_count == 0) return;

    unsigned int match, variable_arguments = 0;

    // loop all registered commands
    for (i = 0; i < g_command_count; i++)
    {
        match = 0;
        index = NOT_FOUND;

        // checks received protocol
        for (j = 0; j < proto.list_count && j < g_commands[i].count; j++)
        {
            if (strcmp(g_commands[i].list[j], proto.list[j]) == 0)
            {
                match++;
            }
            else if (match > 0)
            {
                if (is_wildcard(g_commands[i].list[j])) match++;
                else if (strcmp(g_commands[i].list[j], "...") == 0)
                {
                    match++;
                    variable_arguments = 1;
                }
            }
        }

        if (match > 0)
        {
            // checks if the last argument is ...
            if (j < g_commands[i].count)
            {
                if (strcmp(g_commands[i].list[j], "...") == 0) variable_arguments = 1;
            }

            // few arguments
            if (proto.list_count < (g_commands[i].count - variable_arguments))
            {
                index = FEW_ARGUMENTS;
            }

            // many arguments
            else if (proto.list_count > g_commands[i].count && !variable_arguments)
            {
                index = MANY_ARGUMENTS;
            }

            // arguments match
            else if (match == proto.list_count || variable_arguments)
            {
                index = i;
            }

            break;
        }
    }

    // Protocol OK
    if (index >= 0)
    {
        if (g_commands[index].callback)
        {
            g_commands[index].callback(&proto);

            if (proto.response)
            {
                SEND_TO_SENDER(msg->sender_id, proto.response, proto.response_size);

                // The free is not more necessary because response is not more allocated dynamically
                // uncomment bellow line if this change in the future
                // FREE(proto.response);
            }
        }
    }
    // Protocol error
    else
    {
        SEND_TO_SENDER(msg->sender_id, g_error_messages[-index-1], strlen(g_error_messages[-index-1]));
    }

    FREE(proto.list);
}


void protocol_add_command(const char *command, void (*callback)(proto_t *proto))
{
    if (g_command_count >= COMMAND_COUNT_DUOX) while (1);

    char *cmd = str_duplicate(command);
    g_commands[g_command_count].command = cmd;
    g_commands[g_command_count].list = strarr_split(cmd, ' ');
    g_commands[g_command_count].count = strarr_length(g_commands[g_command_count].list);
    g_commands[g_command_count].callback = callback;
    g_command_count++;
}


void protocol_response(const char *response, proto_t *proto)
{
    static char response_buffer[32];

    proto->response = response_buffer;

    proto->response_size = strlen(response);
    if (proto->response_size >= sizeof(response_buffer))
        proto->response_size = sizeof(response_buffer) - 1;

    strncpy(response_buffer, response, sizeof(response_buffer)-1);
    response_buffer[proto->response_size] = 0;
}

void protocol_send_response(const char *response, const uint8_t value ,proto_t *proto)
{
    char buffer[20];
    uint8_t i = 0;
    memset(buffer, 0, sizeof buffer);

    i = copy_command(buffer, response); 

    // insert the value on buffer
    i += int_to_str(value, &buffer[i], sizeof(buffer) - i, 0);

    protocol_response(&buffer[0], proto);
}

void protocol_remove_commands(void)
{
    unsigned int i;

    for (i = 0; i < g_command_count; i++)
    {
        FREE(g_commands[i].command);
        FREE(g_commands[i].list);
    }
}

//initialize all protocol commands
void protocol_init(void)
{
    // protocol definitions
    protocol_add_command(CMD_PING, cb_ping);
    protocol_add_command(CMD_SAY, cb_say);
    protocol_add_command(CMD_LED, cb_led);
    protocol_add_command(CMD_GLCD_TEXT, cb_glcd_text);
    protocol_add_command(CMD_GLCD_DIALOG, cb_glcd_dialog);
    protocol_add_command(CMD_GLCD_DRAW, cb_glcd_draw);
    protocol_add_command(CMD_DISP_BRIGHTNESS, cb_disp_brightness);
    protocol_add_command(CMD_GUI_CONNECTED, cb_gui_connection);
    protocol_add_command(CMD_GUI_DISCONNECTED, cb_gui_connection);
    protocol_add_command(CMD_CONTROL_ADD, cb_control_add);
    protocol_add_command(CMD_CONTROL_REMOVE, cb_control_rm);
    protocol_add_command(CMD_CONTROL_SET, cb_control_set);
    protocol_add_command(CMD_CONTROL_GET, cb_control_get);
    protocol_add_command(CMD_INITIAL_STATE, cb_initial_state);
    protocol_add_command(CMD_TUNER, cb_tuner);
    protocol_add_command(CMD_RESPONSE, cb_resp);
    protocol_add_command(CMD_RESTORE, cb_restore);
    protocol_add_command(CMD_DUO_BOOT, cb_boot);
    protocol_add_command(CMD_MENU_ITEM_CHANGE, cb_menu_item_changed);
    protocol_add_command(CMD_PEDALBOARD_CLEAR, cb_pedalboard_clear);
    protocol_add_command(CMD_PEDALBOARD_NAME_SET, cb_pedalboard_name);
    protocol_add_command(CMD_DUOX_PAGES_AVAILABLE, cb_pages_available);
    protocol_add_command(CMD_SELFTEST_SAVE_POT_CALIBRATION, cb_save_pot_cal_val);
    protocol_add_command(CMD_SELFTEST_SKIP_CONTROL_ENABLE, cb_set_selftest_control_skip);
    protocol_add_command(CMD_SELFTEST_CHECK_CALIBRATION, cb_check_cal);
    protocol_add_command(CMD_RESET_EEPROM, cb_clear_eeprom);
    protocol_add_command(CMD_DUOX_SET_CONTRAST, cb_set_disp_contrast);
    protocol_add_command(CMD_DUOX_EXP_OVERCURRENT, cb_exp_overcurrent);
}

/*
************************************************************************************************************************
*           PROTOCOL CALLBACKS
************************************************************************************************************************
*/

void cb_ping(proto_t *proto)
{
    g_ui_communication_started = 1;
    g_protocol_busy = false;
    protocol_send_response(CMD_RESPONSE, 0, proto);
}

void cb_say(proto_t *proto)
{
    protocol_response(proto->list[1], proto);
}

void cb_led(proto_t *proto)
{
    ledz_t *led = hardware_leds(atoi(proto->list[1]));

    uint8_t value[3] = {atoi(proto->list[2]), atoi(proto->list[3]), atoi(proto->list[4])}; 
    ledz_set_color(CMD_COLOR_ID,value);

    if (proto->list_count == 7)
    {
        ledz_set_state(led, atoi(proto->list[1]), CMD_COLOR_ID, 1, 0, 0, 0);
    }
    else
    {
        ledz_set_state(led, atoi(proto->list[1]), CMD_COLOR_ID, 2, atoi(proto->list[5]), atoi(proto->list[6]), LED_BLINK_INFINIT);
    }

    protocol_send_response(CMD_RESPONSE, 0, proto);
}

void cb_glcd_text(proto_t *proto)
{
    uint8_t glcd_id, x, y;
    glcd_id = atoi(proto->list[1]);
    x = atoi(proto->list[2]);
    y = atoi(proto->list[3]);

    if (glcd_id >= GLCD_COUNT) return;

    screen_text_box(glcd_id, x, y, proto->list[4]);
    protocol_send_response(CMD_RESPONSE, 0, proto);
}

void cb_glcd_dialog(proto_t *proto)
{
    uint8_t val = naveg_dialog(proto->list[1], NULL);
    protocol_send_response(CMD_RESPONSE, val, proto);
}

void cb_glcd_draw(proto_t *proto)
{
    uint8_t glcd_id, x, y;
    glcd_id = atoi(proto->list[1]);
    x = atoi(proto->list[2]);
    y = atoi(proto->list[3]);

    if (glcd_id >= GLCD_COUNT) return;

    protocol_send_response(CMD_RESPONSE, 0, proto);

    uint8_t msg_buffer[WEBGUI_COMM_RX_BUFF_SIZE];

    str_to_hex(proto->list[4], msg_buffer, sizeof(msg_buffer));
    glcd_draw_image(hardware_glcds(glcd_id), x, y, msg_buffer, GLCD_BLACK);
}

void cb_gui_connection(proto_t *proto)
{
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);
    
    //clear the buffer so we dont send any messages
    comm_webgui_clear_rx_buffer();
    comm_webgui_clear_tx_buffer();

    if (strcmp(proto->list[0], CMD_GUI_CONNECTED) == 0)
        naveg_ui_connection(UI_CONNECTED);
    else
        naveg_ui_connection(UI_DISCONNECTED);

    protocol_send_response(CMD_RESPONSE, 0, proto);

    //we are done supposedly closing the menu, we can unlock the actuators
    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_disp_brightness(proto_t *proto)
{
    hardware_glcd_brightness(atoi(proto->list[1]));

    protocol_send_response(CMD_RESPONSE, 0, proto);
}

void cb_control_add(proto_t *proto)
{
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    control_t *control = data_parse_control(proto->list);
    naveg_add_control(control, 1);

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_control_rm(proto_t *proto)
{
    g_ui_communication_started = 1;
    
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    naveg_remove_control(atoi(proto->list[1]));

    uint8_t i;
    for (i = 2; i < TOTAL_ACTUATORS + 1; i++)
    {
        if ((proto->list[i]) != NULL)
        {
            naveg_remove_control(atoi(proto->list[i]));
        }
        else break;
    }

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_control_set(proto_t *proto)
{
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    naveg_set_control(atoi(proto->list[1]), atof(proto->list[2]));
    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_control_get(proto_t *proto)
{
     //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    float value;
    value = naveg_get_control(atoi(proto->list[1]));

    protocol_send_response(CMD_RESPONSE, value, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_initial_state(proto_t *proto)
{;
    
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    g_ui_communication_started = 1;
    naveg_initial_state(atoi(proto->list[1]), atoi(proto->list[2]), atoi(proto->list[3]), proto->list[4], proto->list[5], &(proto->list[6]));

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_tuner(proto_t *proto)
{
        //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    screen_tuner(atof(proto->list[1]), proto->list[2], atoi(proto->list[3]));
    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_resp(proto_t *proto)
{
    comm_webgui_response_cb(proto->list);
}

void cb_restore(proto_t *proto)
{
    //clear all screens 
    screen_clear(DISPLAY_LEFT);
    screen_clear(DISPLAY_RIGHT);

    cli_restore(RESTORE_INIT);
    protocol_send_response(CMD_RESPONSE, 0, proto);
}

void cb_boot(proto_t *proto)
{
    g_self_test_mode = false;
    g_self_test_cancel_button = false;

    //set the quick bypass link
    system_update_menu_value(MENU_ID_QUICK_BYPASS, atoi(proto->list[2]));

    //set the tuner mute state 
    system_update_menu_value(MENU_ID_TUNER_MUTE, atoi(proto->list[3]));

    //set the current user profile 
    system_update_menu_value(PROFILES_ID, atoi(proto->list[4]));

    //set the master volume link
    system_update_menu_value(MENU_ID_MASTER_VOL_PORT, atoi(proto->list[5]));
    
    //set the master volume value
    float master_vol_value = atof(proto->list[6]);
    //-60 is our 0, we dont use lower values right now (doesnt make sense because of log scale)
    if (master_vol_value < -60) master_vol_value = -60;
    //convert value for screen
    uint8_t screen_val =  ( ( master_vol_value - -60 ) / (0 - -60) ) * (100 - 0) + 0;
    screen_master_vol(screen_val);

    naveg_turn_on_pagination_leds();

    protocol_send_response(CMD_RESPONSE, 0, proto);
}

void cb_menu_item_changed(proto_t *proto)
{
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    naveg_menu_item_changed_cb(atoi(proto->list[1]), atoi(proto->list[2]));
    
    uint8_t i;
    for (i = 3; i < ((MENU_ID_TOP+1) * 2); i+=2)
    {
        if (atoi(proto->list[i]) != 0)
        {
            naveg_menu_item_changed_cb(atoi(proto->list[i]), atoi(proto->list[i+1]));
        }
        else break;
    }

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_pedalboard_clear(proto_t *proto)
{
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

    //clear controls
    uint8_t i;
    for (i = 0; i < TOTAL_ACTUATORS; i++)
    {
        naveg_remove_control(i);
    }

    //clear snapshots
    //we dont care yet about which snapshot, thats why hardcoded
    naveg_clear_snapshot(6);
    naveg_clear_snapshot(5);
    naveg_clear_snapshot(4);

    //reset the page to page 1
    naveg_reset_page();

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_pedalboard_name(proto_t *proto)
{
    //lock actuators
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);

	screen_top_info(&proto->list[1] , 1);

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_pages_available(proto_t *proto)
{
	naveg_pages_available(atoi(proto->list[1]), atoi(proto->list[2]), atoi(proto->list[3]), atoi(proto->list[4]), atoi(proto->list[5]), atoi(proto->list[6]));

    naveg_turn_on_pagination_leds();

    protocol_send_response(CMD_RESPONSE, 0, proto);
}

void cb_save_pot_cal_val(proto_t *proto)
{
    //lock actuators and clear tx buffer
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);
    comm_webgui_clear_tx_buffer();

    //if the first argument == 1, we save the max value, if ==0 we save the min value
    if(atoi(proto->list[1]) == 1)
    {
        calibration_write_max(atoi(proto->list[2]));
    } 
    else 
    {
        calibration_write_min(atoi(proto->list[2]));
    }

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_check_cal(proto_t *proto)
{
    //lock actuators and clear tx buffer
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);
    comm_webgui_clear_tx_buffer();

    if (calibration_check_valid_pot(atoi(proto->list[1])))
    {
        protocol_send_response(CMD_RESPONSE, 1, proto);
    }
    //range not good
    else
    {
        protocol_send_response(CMD_RESPONSE, 0, proto);
    }

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_set_selftest_control_skip(proto_t *proto)
{
    //lock actuators and clear tx buffer
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);
    comm_webgui_clear_tx_buffer();

    g_self_test_cancel_button = true; 

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_clear_eeprom(proto_t *proto)
{
    //lock actuators and clear tx buffer
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);
    comm_webgui_clear_tx_buffer();

    hardware_reset_eeprom();    

    //!! THIS NEEDS AN HMI RESET TO TAKE PROPER EFFECT

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_set_disp_contrast(proto_t *proto)
{
    //lock actuators and clear tx buffer
    g_protocol_busy = true;
    system_lock_comm_serial(g_protocol_busy);
    comm_webgui_clear_tx_buffer();

    int value_bfr = MAP(atoi(proto->list[2]), 0, 100, UC1701_PM_MIN, UC1701_PM_MAX);
    uc1701_set_custom_value(hardware_glcds(atoi(proto->list[1])), value_bfr, UC1701_RR_DEFAULT);

    protocol_send_response(CMD_RESPONSE, 0, proto);

    g_protocol_busy = false;
    system_lock_comm_serial(g_protocol_busy);
}

void cb_exp_overcurrent(proto_t *proto)
{
    (void) proto;

    naveg_dialog("Exp port shorted\nTo avoid harm to your device\nthe unit switched back to CV\nmode\n\nPlease check your exp pedal\nand cables", "WARNING");

    naveg_reload_display();
}