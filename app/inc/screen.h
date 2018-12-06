
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

void screen_clear(uint8_t display_id);
void screen_encoder(uint8_t display_id, control_t *control);
void screen_footer(uint8_t id, const char *name, const char *value);
void screen_pot(uint8_t pot_id, control_t *control);
void screen_tool(uint8_t tool, uint8_t display_id);
void screen_bp_list(const char *title, bp_list_t *list);
void screen_system_menu(menu_item_t *item);
void screen_tuner(float frequency, char *note, int8_t cents);
void screen_tuner_input(uint8_t input);
void screen_image(uint8_t display, const uint8_t *image);
void screen_top_info(void *data, menu_item_t *item);
void screen_master_vol(uint8_t volume_val);

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
