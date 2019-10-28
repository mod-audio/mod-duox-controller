
/*
************************************************************************************************************************
* Calibration - Calibrate the potentiometers and write to EEPROM
************************************************************************************************************************
*/

#ifndef CALIBRATION_H
#define CALIBRATION_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdint.h>
#include <stdbool.h>

/*
************************************************************************************************************************
*           CONFIGURATION DEFINES
************************************************************************************************************************
*/
#define CALIBRATION_TITLE_TEXT 			"CALIBRATION MODE"
#define CALIBRATION_EXPLANATION_TEXT 	"TO CALIBRATE THE MDX KNOBS\nPRESS THE BLUE BUTTONS TO\nSELECT A KNOB THE LEFT YELLOW\nBUTTON TO SAVE THE MINIMAL\nVALUE THE RIGHT YELLOW BUTTON\nTO SAVE THE MAX VALUE, OR THE\nGREEN BUTTON TO EXIT"
//defines for button and led behaviour
#define EXIT_BUTTON                 5
#define NEXT_POT_BUTTON             6
#define PREV_POT_BUTTON             4
#define SAVE_MIN_VAL_LEFT_BUTTON    0
#define SAVE_MAX_VAL_LEFT_BUTTON    1
#define SAVE_MIN_VAL_RIGHT_BUTTON   2
#define SAVE_MAX_VAL_RIGHT_BUTTON   3

#define EXIT_COLOR                  GREEN
#define PREV_NEXT_COLOR             BLUE
#define SAVE_VAL_COLOR              YELLOW
#define ERROR_COLOR                 RED

//error codes
#define ERROR_POT_NOT_FOUND         0
#define ERROR_RANGE_TO_SMALL        1

//error messages
#define RANGE_TO_SMALL_MESSAGE		"RANGE OF THE KNOB IS TO SMALL"
#define POT_NOT_FOUND_MESSAGE		"POTENTIOMETER IS NOT FOUND"
#define UNKNOWN_ERROR_MESSAGE		"AN UNKNOWN ERROR OCCURRED"
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
bool g_calibration_mode;

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

void calibration_write_display(uint8_t pot);
void calibration_pot_change(uint8_t pot);
void calibration_button_pressed(uint8_t button);
uint16_t calibration_get_min(uint8_t pot);
uint16_t calibration_get_max(uint8_t pot);
void calibration_write_default(void);
uint8_t calibration_check_valid(void);
void calibration_write_max(uint8_t pot);
void calibration_write_min(uint8_t pot);
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
