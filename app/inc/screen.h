
/*
************************************************************************************************************************
*
************************************************************************************************************************
*/

#ifndef SCREEN_H
#define SCREEN_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdint.h>
#include "config.h"
#include "data.h"


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

void screen_set_hide_non_assigned_actuators(uint8_t hide);
void screen_clear(uint8_t display_id);
void screen_encoder(uint8_t display_id, control_t *control);
void screen_footer(uint8_t id, const char *name, const char *value, int16_t property);
void screen_pot(uint8_t pot_id, control_t *control);
void screen_tool(uint8_t tool, uint8_t display_id);
void screen_bp_list(const char *title, bp_list_t *list);
void screen_system_menu(menu_item_t *item);
void screen_tuner(float frequency, char *note, int8_t cents);
void screen_tuner_input(uint8_t input);
void screen_image(uint8_t display, const uint8_t *image);
void screen_top_info(const void *data, uint8_t update);
void screen_master_vol(float volume_val);
void screen_text_box(uint8_t display, uint8_t x, uint8_t y, const char *text);

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
