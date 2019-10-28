
/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include "calibration.h"
#include "hardware.h"
#include "glcd.h"
#include "glcd_widget.h"
#include "ledz.h"
#include "screen.h"
#include "naveg.h"
#include "device.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include <stdbool.h>
#include <string.h>


/*
************************************************************************************************************************
*           LOCAL DEFINES
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*           LOCAL CONSTANTS
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*           LOCAL DATA TYPES
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*           LOCAL MACROS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           LOCAL GLOBAL VARIABLES
************************************************************************************************************************
*/

//global for mode toggling
bool g_calibration_mode = false;

uint8_t current_pot = 0;
uint8_t orientation = 0;

uint8_t error_flag = 0;

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
void toggle_leds(void)
{
    ledz_off(hardware_leds(EXIT_BUTTON), LEDZ_ALL_COLORS);
    ledz_off(hardware_leds(NEXT_POT_BUTTON), LEDZ_ALL_COLORS);
    ledz_off(hardware_leds(PREV_POT_BUTTON), LEDZ_ALL_COLORS);
    ledz_on(hardware_leds(EXIT_BUTTON), EXIT_COLOR);
    ledz_on(hardware_leds(NEXT_POT_BUTTON), PREV_NEXT_COLOR);
    ledz_on(hardware_leds(PREV_POT_BUTTON), PREV_NEXT_COLOR);

    if (orientation==DISPLAY_LEFT)
    {
        ledz_off(hardware_leds(SAVE_MIN_VAL_LEFT_BUTTON), LEDZ_ALL_COLORS);
        ledz_off(hardware_leds(SAVE_MAX_VAL_LEFT_BUTTON), LEDZ_ALL_COLORS);
        ledz_on(hardware_leds(SAVE_MIN_VAL_LEFT_BUTTON), SAVE_VAL_COLOR);
        ledz_on(hardware_leds(SAVE_MAX_VAL_LEFT_BUTTON), SAVE_VAL_COLOR);
        ledz_off(hardware_leds(SAVE_MIN_VAL_RIGHT_BUTTON), LEDZ_ALL_COLORS);
        ledz_off(hardware_leds(SAVE_MAX_VAL_RIGHT_BUTTON), LEDZ_ALL_COLORS);
    }
    else 
    {
        ledz_off(hardware_leds(SAVE_MIN_VAL_RIGHT_BUTTON), LEDZ_ALL_COLORS);
        ledz_off(hardware_leds(SAVE_MAX_VAL_RIGHT_BUTTON), LEDZ_ALL_COLORS);
        ledz_on(hardware_leds(SAVE_MIN_VAL_RIGHT_BUTTON), SAVE_VAL_COLOR);
        ledz_on(hardware_leds(SAVE_MAX_VAL_RIGHT_BUTTON), SAVE_VAL_COLOR);
        ledz_off(hardware_leds(SAVE_MIN_VAL_LEFT_BUTTON), LEDZ_ALL_COLORS);
        ledz_off(hardware_leds(SAVE_MAX_VAL_LEFT_BUTTON), LEDZ_ALL_COLORS);
    }
}

void display_calibration_animation(uint8_t display)
{
    //clear displays
    screen_clear(display);

    //create a title string 
    char pot_str[sizeof(SCREEN_POT_CAL_DEFAULT_NAME) + 2];
    strcpy(pot_str, SCREEN_POT_CAL_DEFAULT_NAME);
    pot_str[sizeof(SCREEN_POT_CAL_DEFAULT_NAME)-1] = (current_pot + '1');
    pot_str[sizeof(SCREEN_POT_CAL_DEFAULT_NAME)] = 0;  

    //show the current pot
    textbox_t title;
    title.color = GLCD_BLACK;
    title.mode = TEXT_SINGLE_LINE;
    title.font = alterebro24;
    title.top_margin = 1;
    title.bottom_margin = 0;
    title.left_margin = 0;
    title.right_margin = 0;
    title.height = 0;
    title.width = 0;
    title.text = pot_str;
    title.align = ALIGN_CENTER_TOP;
    title.y = 0;
    title.x = 1;
    widget_textbox(hardware_glcds(display), &title);

    //show the centered pot graphic
    knob_t knob;
    knob.x = 64;
    knob.y = 35;
    knob.color = GLCD_BLACK;
    knob.orientation = 0;
    knob.mode = 0;
    knob.lock = 0;
    knob.min = 0;
    knob.max = 4095;
    knob.value = hardware_get_pot_value(current_pot);
    widget_knob(hardware_glcds(display), &knob);

    glcd_update(hardware_glcds(display));
}

void display_calibration_text(uint8_t display)
{
    //clear displays
    screen_clear(display);

    glcd_t *txt_display = hardware_glcds(display);

    //show the explanation of the mode
    textbox_t title;
    title.color = GLCD_BLACK;
    title.mode = TEXT_SINGLE_LINE;
    title.font = alterebro24;
    title.top_margin = 1;
    title.bottom_margin = 0;
    title.left_margin = 0;
    title.right_margin = 0;
    title.height = DISPLAY_HEIGHT;
    title.width = DISPLAY_WIDTH;
    title.text = CALIBRATION_TITLE_TEXT;
    title.align = ALIGN_CENTER_TOP;
    title.y = 0;
    title.x = 1;
    widget_textbox(txt_display, &title);

    glcd_hline(txt_display, 0, 15, DISPLAY_WIDTH, GLCD_BLACK);
    glcd_hline(txt_display, 0, 16, DISPLAY_WIDTH, GLCD_BLACK);

    textbox_t explanation_text;
    explanation_text.color = GLCD_BLACK;
    explanation_text.mode = TEXT_MULTI_LINES;
    explanation_text.font = SMfont;
    explanation_text.top_margin = 1;
    explanation_text.bottom_margin = 0;
    explanation_text.left_margin = 0;
    explanation_text.right_margin = 0;
    explanation_text.height = DISPLAY_HEIGHT;
    explanation_text.width = DISPLAY_WIDTH;
    explanation_text.text = CALIBRATION_EXPLANATION_TEXT;
    explanation_text.align = ALIGN_CENTER_NONE;
    explanation_text.y = 18;
    explanation_text.x = 1;
    widget_textbox(txt_display, &explanation_text);  

    glcd_update(txt_display);
}

void display_error_message(uint8_t display, uint8_t error_message)
{
    error_flag = 1;

    //clear displays
    screen_clear(display);
    screen_clear(!display);

    char error_txt[32] = {0};

    switch (error_message)
    {
        case ERROR_POT_NOT_FOUND:
            strcpy(error_txt, POT_NOT_FOUND_MESSAGE);
            error_txt[sizeof(POT_NOT_FOUND_MESSAGE)] = 0;
        break;

        case ERROR_RANGE_TO_SMALL:
            strcpy(error_txt, RANGE_TO_SMALL_MESSAGE);
            error_txt[sizeof(RANGE_TO_SMALL_MESSAGE)] = 0;
        break;

        default:
            strcpy(error_txt, UNKNOWN_ERROR_MESSAGE);
            error_txt[sizeof(UNKNOWN_ERROR_MESSAGE)] = 0;
        break;
    }

    //make all LED's red
    uint8_t i = 0;
    for (i =0; i < LEDS_COUNT; i++)
    {
        if (i != EXIT_BUTTON)
        {
            ledz_off(hardware_leds(i), LEDZ_ALL_COLORS);
            ledz_on(hardware_leds(i), RED);
        }
        else 
        {
            ledz_off(hardware_leds(i), LEDZ_ALL_COLORS);
            ledz_on(hardware_leds(i), WHITE);   
        }
    }

    //show the explanation of the mode
    textbox_t title;
    title.color = GLCD_BLACK;
    title.mode = TEXT_SINGLE_LINE;
    title.font = alterebro49;
    title.top_margin = 1;
    title.bottom_margin = 0;
    title.left_margin = 0;
    title.right_margin = 0;
    title.height = DISPLAY_HEIGHT;
    title.width = DISPLAY_WIDTH;
    title.text = "ERROR";
    title.align = ALIGN_CENTER_NONE;
    title.y = 20;
    title.x = 1;
    widget_textbox(hardware_glcds(display), &title);

    textbox_t explanation_text;
    explanation_text.color = GLCD_BLACK;
    explanation_text.mode = TEXT_SINGLE_LINE;
    explanation_text.font = SMfont;
    explanation_text.top_margin = 1;
    explanation_text.bottom_margin = 0;
    explanation_text.left_margin = 0;
    explanation_text.right_margin = 0;
    explanation_text.height = DISPLAY_HEIGHT;
    explanation_text.width = DISPLAY_WIDTH;
    explanation_text.text = error_txt;
    explanation_text.align = ALIGN_CENTER_NONE;
    explanation_text.y = 30;
    explanation_text.x = 1;
    widget_textbox(hardware_glcds(!display), &explanation_text);

    glcd_update(hardware_glcds(display));
    glcd_update(hardware_glcds(!display));
}

uint8_t check_range_sufficient(uint8_t pot)
{
    uint16_t read_buffer_min = 0;
    uint16_t read_buffer_max = 0;

    switch(pot)
    {
        case 0: 
            EEPROM_Read(0, POT_1_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_1_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 1: 
            EEPROM_Read(0, POT_2_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_2_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 2: 
            EEPROM_Read(0, POT_3_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_3_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 3: 
            EEPROM_Read(0, POT_4_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_4_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 4: 
            EEPROM_Read(0, POT_5_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_5_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 5: 
            EEPROM_Read(0, POT_6_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_6_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 6: 
            EEPROM_Read(0, POT_7_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_7_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 7: 
            EEPROM_Read(0, POT_8_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
            EEPROM_Read(0, POT_8_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        default:
            display_error_message(orientation, ERROR_POT_NOT_FOUND);
        break;

    }

    //if range is not good, return 0. if the range is bug eneugh return 1 
    if ((read_buffer_max - read_buffer_min) < 1000)
        return 0;
    else return 1; 
}

/*
************************************************************************************************************************
*           GLOBAL FUNCTIONS
************************************************************************************************************************
*/

//displays the calibration screens, takes current pot to callibrate as argument
void calibration_write_display(uint8_t pot)
{
    current_pot = pot;

    orientation = (pot < 4)?DISPLAY_LEFT:DISPLAY_RIGHT;

    //print to the screens
    display_calibration_animation(orientation);
    display_calibration_text(!orientation);

    //show the correct LED Colors
    toggle_leds();
}

void calibration_pot_change(uint8_t pot)
{
    //check if this is the pot we are displaying, if so get value and display
    if (pot != current_pot) return;

    display_calibration_animation(orientation);
}

void calibration_button_pressed(uint8_t button)
{
    switch (button)
    {
        //actuator buttons

        case EXIT_BUTTON:

            //cant exit when an error acoured, just clear the error message and start again
            if (error_flag)
            {   
                //flag becomes 0 again
                error_flag = 0;

                //restart the calibration
                calibration_write_display(current_pot);
                return;
            }

            //if the range was to small to be accepted, we cant exit
            if (!check_range_sufficient(current_pot))
            {
                display_error_message(orientation, ERROR_RANGE_TO_SMALL);
                return;
            } 

            //all is safe, we can exit
            g_calibration_mode = false;

            //exit the menu show normal operation
            naveg_toggle_tool(DISPLAY_TOOL_SYSTEM, DISPLAY_LEFT);

            //update the value's and restore LED states
            naveg_update_calibration();

        break;

        case NEXT_POT_BUTTON:
            
            //if the range was to small to be accepted
            if (!check_range_sufficient(current_pot))
            {
                display_error_message(orientation, ERROR_RANGE_TO_SMALL);
                return;
            }

            if (current_pot < POTS_COUNT -1)
                current_pot++;

            //if we changed orientation, redraw both screens
            if (((current_pot < 4)?DISPLAY_LEFT:DISPLAY_RIGHT) != orientation)
            {
                orientation = (current_pot < 4)?DISPLAY_LEFT:DISPLAY_RIGHT;

                display_calibration_animation(orientation);
                display_calibration_text(!orientation);

                ledz_on(hardware_leds(SAVE_MIN_VAL_RIGHT_BUTTON), SAVE_VAL_COLOR);
                ledz_on(hardware_leds(SAVE_MAX_VAL_RIGHT_BUTTON), SAVE_VAL_COLOR);
                ledz_off(hardware_leds(SAVE_MIN_VAL_LEFT_BUTTON), LEDZ_ALL_COLORS);
                ledz_off(hardware_leds(SAVE_MAX_VAL_LEFT_BUTTON), LEDZ_ALL_COLORS);
            }
            //if not just the animation screen
            else 
            {
                display_calibration_animation(orientation);
            }

        break;

        case PREV_POT_BUTTON:
            
            //if the range was to small to be accepted
            if (!check_range_sufficient(current_pot))
            {
                display_error_message(orientation, ERROR_RANGE_TO_SMALL);
                return;
            }

            if (current_pot > 0)
                current_pot--;

            //if we changed orientation, redraw both screens
            if (((current_pot < 4)?DISPLAY_LEFT:DISPLAY_RIGHT) != orientation)
            {
                orientation = (current_pot < 4)?DISPLAY_LEFT:DISPLAY_RIGHT;
                
                display_calibration_animation(orientation);
                display_calibration_text(!orientation);

                ledz_on(hardware_leds(SAVE_MIN_VAL_LEFT_BUTTON), SAVE_VAL_COLOR);
                ledz_on(hardware_leds(SAVE_MAX_VAL_LEFT_BUTTON), SAVE_VAL_COLOR);
                ledz_off(hardware_leds(SAVE_MIN_VAL_RIGHT_BUTTON), LEDZ_ALL_COLORS);
                ledz_off(hardware_leds(SAVE_MAX_VAL_RIGHT_BUTTON), LEDZ_ALL_COLORS);
            }
            //if not just the animation screen
            else 
            {
                display_calibration_animation(orientation);
            }

        break;

        case SAVE_MIN_VAL_LEFT_BUTTON:
            
            //this button is turned off now
            if (orientation) return;

            calibration_write_min(current_pot);
        break;
        
        case SAVE_MAX_VAL_LEFT_BUTTON:
            
            //this button is turned off now
            if (orientation) return;

            calibration_write_max(current_pot);
        break;

        case SAVE_MIN_VAL_RIGHT_BUTTON:
            
            //this button is turned off now
            if (!orientation) return;

        break;

        case SAVE_MAX_VAL_RIGHT_BUTTON:
            
            //this button is turned off now
            if (!orientation) return;

            calibration_write_max(current_pot);
        break;
    }
}

uint16_t calibration_get_min(uint8_t pot)
{
    uint16_t read_buffer_min = 0;

    switch(pot)
    {
        case 0: 
            EEPROM_Read(0, POT_1_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        case 1: 
            EEPROM_Read(0, POT_2_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        case 2: 
            EEPROM_Read(0, POT_3_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        case 3: 
            EEPROM_Read(0, POT_4_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        case 4: 
            EEPROM_Read(0, POT_5_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        case 5: 
            EEPROM_Read(0, POT_6_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        case 6: 
            EEPROM_Read(0, POT_7_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        case 7: 
            EEPROM_Read(0, POT_8_MIN_CALIBRATION_ADRESS, &read_buffer_min, MODE_16_BIT, 1);
        break;

        default:
            read_buffer_min = POT_LOWER_THRESHOLD;
        break;
    }

    return read_buffer_min;
}

uint16_t calibration_get_max(uint8_t pot)
{
    uint16_t read_buffer_max = 0;

    switch(pot)
    {
        case 0: 
            EEPROM_Read(0, POT_1_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 1: 
            EEPROM_Read(0, POT_2_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 2: 
            EEPROM_Read(0, POT_3_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 3: 
            EEPROM_Read(0, POT_4_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 4: 
            EEPROM_Read(0, POT_5_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 5: 
            EEPROM_Read(0, POT_6_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 6: 
            EEPROM_Read(0, POT_7_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        case 7: 
            EEPROM_Read(0, POT_8_MAX_CALIBRATION_ADRESS, &read_buffer_max, MODE_16_BIT, 1);
        break;

        default:
            read_buffer_max = POT_UPPER_THRESHOLD;
        break;
    }

    return read_buffer_max;
}

void calibration_write_default(void)
{
    //set all min value's
    uint16_t write_buffer = POT_LOWER_THRESHOLD;
    EEPROM_Write(0, POT_1_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_2_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_3_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_4_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_5_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_6_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_7_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_8_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    
    write_buffer = POT_UPPER_THRESHOLD;
    EEPROM_Write(0, POT_1_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_2_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_3_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_4_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_5_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_6_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_7_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
    EEPROM_Write(0, POT_8_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
}

//function to check if all value's are valid
uint8_t calibration_check_valid(void)
{
    uint8_t valid = 1;

    //check all pots 1 by 1
    uint8_t i = 0;
    for (i =0; i < POTS_COUNT; i++)
    {
        valid = check_range_sufficient(i);
        
        //if one is not valid exit loop
        if (!valid) 
        {
            return 0; 
        }
    }
    return 1; 
}

void calibration_write_max(uint8_t pot)
{
    //substract 10 because ADC value's fluctuate. 
    uint16_t write_buffer = (hardware_get_pot_value(pot) - 10);

    switch(pot)
    {
        case 0: 
            EEPROM_Write(0, POT_1_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 1: 
            EEPROM_Write(0, POT_2_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 2: 
            EEPROM_Write(0, POT_3_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 3: 
            EEPROM_Write(0, POT_4_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 4: 
            EEPROM_Write(0, POT_5_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 5: 
            EEPROM_Write(0, POT_6_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 6: 
            EEPROM_Write(0, POT_7_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 7: 
            EEPROM_Write(0, POT_8_MAX_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        //ERROR
        default:
            if (g_calibration_mode)
            {
                display_error_message(orientation, ERROR_POT_NOT_FOUND);
            }
            else 
            {
                return;
            }
        break;
    }
}

void calibration_write_min(uint8_t pot)
{
    //add 10 because ADC value's fluctuate. 
    uint16_t write_buffer = (hardware_get_pot_value(pot) + 10);

    switch(pot)
    {
        case 0: 
            EEPROM_Write(0, POT_1_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 1: 
            EEPROM_Write(0, POT_2_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 2: 
            EEPROM_Write(0, POT_3_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 3: 
            EEPROM_Write(0, POT_4_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;
                
        case 4: 
            EEPROM_Write(0, POT_5_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 5: 
            EEPROM_Write(0, POT_6_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 6: 
            EEPROM_Write(0, POT_7_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        case 7: 
            EEPROM_Write(0, POT_8_MIN_CALIBRATION_ADRESS, &write_buffer, MODE_16_BIT, 1);
        break;

        default:
            if (g_calibration_mode)
            {
                display_error_message(orientation, ERROR_POT_NOT_FOUND);
            }
            else 
            {
                return;
            }
        break;
    }
}