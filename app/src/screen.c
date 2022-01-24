
/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <string.h>
#include <stdlib.h>

#include "screen.h"
#include "utils.h"
#include "glcd.h"
#include "glcd_widget.h"
#include "naveg.h"
#include "hardware.h"
#include "cli.h"
#include "mod-protocol.h"
#include "ledz.h"

/*
************************************************************************************************************************
*           LOCAL DEFINES
************************************************************************************************************************
*/

//display conf macro
//#define INVERT_POTENTIOMENTERS
//#define INVERT_HEADERS
#define FOOT_INV_BORDERS

#define FOOTER_NAME_WIDTH       ((DISPLAY_WIDTH * 50)/100)
#define FOOTER_VALUE_WIDTH      (DISPLAY_WIDTH - FOOTER_NAME_WIDTH)

enum {BANKS_LIST, PEDALBOARD_LIST};
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

#define MAP(x, Omin, Omax, Nmin, Nmax)      ( x - Omin ) * (Nmax -  Nmin)  / (Omax - Omin) + Nmin;

/*
************************************************************************************************************************
*           LOCAL GLOBAL VARIABLES
************************************************************************************************************************
*/

static tuner_t g_tuner = {0, NULL, 0, 1};
static uint8_t g_hide_non_assigned_actuators = 0;


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

/*
************************************************************************************************************************
*           GLOBAL FUNCTIONS
************************************************************************************************************************
*/

void screen_set_hide_non_assigned_actuators(uint8_t hide)
{
    g_hide_non_assigned_actuators = hide;
    return;
}

void screen_clear(uint8_t display_id)
{
    glcd_clear(hardware_glcds(display_id), GLCD_WHITE);
}

void screen_pot(uint8_t pot_id, control_t *control)
{
    //select display
    uint8_t display_id = 0;
    if (pot_id >= 4)
        display_id = 1;
    glcd_t *display = hardware_glcds(display_id);

    //The knobs are all oriented from a single posistion, this posistion is the center pixel of the pot circle
    //the knob posistion is used to know if the knob is oriented to the left or right of the screen.
    //The left knobs clear one more pixel in the X posistion that does not get inverted, this is our white middle line in the display
    knob_t knob;
    switch(pot_id)
    {
        case 0:
        case 4:
            knob.x = 28;
            knob.y = 32;
            knob.orientation = 0;
            glcd_rect_fill(display, 0, knob.y -7, 65, 16, GLCD_WHITE);
            break;
        case 1:
        case 5:
            knob.x = 28;
            knob.y = 47;
            knob.orientation = 0;
            glcd_rect_fill(display, 0, knob.y -6, 65, 14, GLCD_WHITE);
            break;
        case 2:
        case 6:
            knob.x = 100;
            knob.y = 32;
            knob.orientation = 1;
            glcd_rect_fill(display, 65, knob.y -7, 63, 16 , GLCD_WHITE);
            break;
        case 3:
        case 7:
            knob.x = 100;
            knob.y = 47;
            knob.orientation = 1;
            glcd_rect_fill(display, 65, knob.y -6, 63, 14 , GLCD_WHITE);
            break;
        default:
            // not handled, trigger no assignment
            control = NULL;
            break;
    }

    //no assignment
    if (!control)
    {
        textbox_t blank_title;

        blank_title.color = GLCD_BLACK;
        blank_title.mode = TEXT_SINGLE_LINE;
        blank_title.font = Terminal3x5;
        blank_title.height = 0;
        blank_title.width = 0;
        blank_title.top_margin = 0;
        blank_title.bottom_margin = 0;
        blank_title.left_margin = 0;
        blank_title.right_margin = 0;
        blank_title.y = knob.y - 3;
        blank_title.align = ALIGN_NONE_NONE;

        if (g_hide_non_assigned_actuators)
        {
            blank_title.x = knob.orientation ? knob.x - 5 : knob.x + 1 ;
            blank_title.text = "-";
        }
        else 
        {
            char text[sizeof(SCREEN_POT_DEFAULT_NAME) + 2];
            strcpy(text, SCREEN_POT_DEFAULT_NAME);
            text[sizeof(SCREEN_POT_DEFAULT_NAME)-1] = pot_id + '1';
            text[sizeof(SCREEN_POT_DEFAULT_NAME)] = 0;

            const uint8_t text_len = strlen(text);
            blank_title.x = knob.orientation ? (knob.x - 8 - (text_len*1.5)) : (knob.x + 3 - (text_len*1.5));

            blank_title.text = text;
        }

        widget_textbox(display, &blank_title);
    }
    else if (control->properties & (FLAG_CONTROL_TOGGLED | FLAG_CONTROL_BYPASS))
    {
        //convert title
        char title_str_bfr[8] = {0};
        strncpy(title_str_bfr, control->label, 7);
        title_str_bfr[7] = '\0';

        //title:
        textbox_t title;
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = Terminal3x5;
        if (knob.orientation)
            title.x = (knob.x - 5 - (strlen(title_str_bfr))*4);
        else
            title.x = (knob.x + 7);
        title.y = knob.y - 2;
        title.height = 0;
        title.width = 0;
        title.top_margin = 0;
        title.bottom_margin = 0;
        title.left_margin = 0;
        title.right_margin = 0;
        title.text = title_str_bfr;
        title.align = ALIGN_NONE_NONE;
        widget_textbox(display, &title);

        //toggle
        toggle_t toggle;
        if (knob.orientation)
        toggle.x = knob.x - 4;
        else 
           toggle.x = knob.x - 26;
        toggle.y = knob.y - 5;
        toggle.width = 31;
        toggle.height = 11;
        toggle.color = GLCD_BLACK;
        toggle.value = (control->properties & FLAG_CONTROL_TOGGLED)?control->value:!control->value;
        widget_toggle(display, &toggle);
    }
    else
    {
        textbox_t value, unit, title;

        //convert title
        char title_str_bfr[8] = {0};
        strncpy(title_str_bfr, control->label, 7);
        title_str_bfr[7] = '\0';

        //convert value
        //value_str is our temporary value which we use for what kind of value representation we need
        //value_str_bfr is the value that gets printed
        char value_str[10] = {0};
        char value_str_bfr[7] = {0};

        if (!control->value_string) {
            //if the value becomes bigger then 9999 (4 characters), then switch to another view 10999 becomes 10.9K
            if (control->value > 9999) {
                float_to_str((control->value/1000), value_str, sizeof(value_str), 1);
                strcat(value_str, "K");
            }
            //else if we have a value bigger then 100, we dont display decimals anymore
            else if (control->value > 99.9) {
            	if (control->value > 999.9) {
		  		int_to_str(control->value, value_str, sizeof(value_str), 0);
            	}
            	else {
                    //not for ints or percantages
                    if ((control->properties & FLAG_CONTROL_INTEGER) || (!strcmp(control->unit, "%"))) {
                        int_to_str(control->value, value_str, sizeof(value_str), 0);
                    }
                    else {    
                    	float_to_str((control->value), value_str, sizeof(value_str), 1);
                    }
            	}
            }
            //else if we have a value bigger then 10 we display just one decimal
            else if (control->value > 9.9) {
                //not for ints
                if (control->properties & FLAG_CONTROL_INTEGER) {
                    int_to_str(control->value, value_str, sizeof(value_str), 0);
                }
                else {
                    float_to_str((control->value), value_str, sizeof(value_str), 6);
                }
            }
            //if the value becomes less then 0 we change to 1 or 0 decimals
            else if (control->value < 0) {
                if (control->value > -99.9) {
                    //not for ints
                    if (control->properties & FLAG_CONTROL_INTEGER) {
                        int_to_str(control->value, value_str, sizeof(value_str), 0);
                    }
                    else if (control->value < -9.9) {
                        float_to_str((control->value), value_str, sizeof(value_str), 1);
                    }
                    else {
                        float_to_str((control->value), value_str, sizeof(value_str), 2);
                    }
                }
                else {
                    if (control->value < -9999.9) {
                        int_to_str(control->value/1000, value_str, sizeof(value_str), 0);
                        strcat(value_str, "K");
                    }
                	int_to_str(control->value, value_str, sizeof(value_str), 0);
                }
            }
            //for values between 0 and 10 display 2 decimals
            else {
                //not for ints
                if (control->properties & FLAG_CONTROL_INTEGER) {
                    int_to_str(control->value, value_str, sizeof(value_str), 0);
                }
                else {
                    float_to_str((control->value), value_str, sizeof(value_str), 6);
                }
            }

            //copy to value_str_bfr, the first 5 char
            strncpy(value_str_bfr, value_str, 5);
            //terminate the text with line ending, since no unit is added to it
            value_str_bfr[5] = '\0';
        }
        else
        {
            //draw the value string
            uint8_t char_cnt_value = strlen(control->value_string);

            if (char_cnt_value > 5)
                char_cnt_value = 5;

            control->value_string[char_cnt_value] = '\0';
            strcpy(value_str_bfr, control->value_string);
            value_str_bfr[char_cnt_value] = '\0';
        }

        //convert unit
        char *unit_str = 0;
        char tmp_unit[5];
        char tmp_unit2[2];
        char tmp_unit_check[10];

        strcpy(tmp_unit_check, control->unit);
        strcpy(tmp_unit, "");
  		strcpy(tmp_unit2, "%");

        if (strcmp(tmp_unit_check, tmp_unit) == 0)
        {
        	unit_str = NULL;  
        }
        else unit_str = control->unit;   

        //knob
        knob.color = GLCD_BLACK;
        knob.lock = control->scroll_dir;
        knob.value = control->value;
        knob.min = control->minimum;
        knob.max = control->maximum;
        knob.min_cal = g_pot_calibrations[0][pot_id];
        knob.max_cal = g_pot_calibrations[1][pot_id];
        if (control->properties & FLAG_CONTROL_LOGARITHMIC)
        {
            knob.mode = 1;
        }
        else knob.mode = 0;
        widget_knob(display, &knob);

        //title:
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = Terminal3x5;
        if (knob.orientation)
            title.x = (knob.x - 5 - (strlen(title_str_bfr))*4);
        else
            title.x = (knob.x + 7);
        title.y = knob.y - 2;
        title.height = 0;
        title.width = 0;
        title.top_margin = 0;
        title.bottom_margin = 0;
        title.left_margin = 0;
        title.right_margin = 0;
        title.text = title_str_bfr;
        title.align = ALIGN_NONE_NONE;
        widget_textbox(display, &title);

        //sets the value y, for both value as unit
        value.y = knob.y - 5;

        //draws the unit
        if (unit_str != NULL)
        {
            if (strcmp(unit_str, tmp_unit2) != 0)
            {
                unit.color = GLCD_BLACK;
                unit.mode = TEXT_SINGLE_LINE;
                unit.font = Terminal3x5;
                if (knob.orientation)
                    unit.x =(knob.x + 8);
                else
                    unit.x = (knob.x - 6 - (strlen(unit_str)*4));
                unit.y = knob.y + 1;
                unit.height = 0;
                unit.width = 0;
                unit.top_margin = 0;
                unit.bottom_margin = 0;
                unit.left_margin = 0;
                unit.right_margin = 0;
                unit.text = unit_str;
                unit.align = ALIGN_NONE_NONE;
                widget_textbox(display, &unit);

                //writing text clears up to 3 pixels below it (for some odd reason TODO)
                //thats why when there is a unit on one of the bottom pots we need to redraw the devision line
                // horizontal footer line
                if (pot_id == 1 || pot_id == 3 || pot_id == 5 || pot_id == 7)
                    glcd_hline(display, 0, 55, DISPLAY_WIDTH, GLCD_BLACK);
            }
            //if unit = %, add it to the value string (if its one of the pots on the left add 4 pixels (1 char))
            else
            {
                value.y += 3;
                value_str_bfr[4] = '\0';
                strcat(value_str_bfr, unit_str);
                //terminate the text with line ending, the % was added, so the index is also increased by 1
                value_str_bfr[5] = '\0';
            }
        }
        else value.y += 3;

        //value
        value.color = GLCD_BLACK;
        value.mode = TEXT_SINGLE_LINE;
        value.font = Terminal3x5;
        if (knob.orientation)
            value.x = (knob.x + 8);
        else
            value.x = (knob.x - 6 - (strlen(value_str_bfr)*4));
        value.height = 0;
        value.width = 0;
        value.top_margin = 0;
        value.bottom_margin = 0;
        value.left_margin = 0;
        value.right_margin = 0;
        value.text = value_str_bfr;
        value.align = ALIGN_NONE_NONE;
        widget_textbox(display, &value);
    }

#ifdef INVERT_POTENTIOMENTERS
    //invert the pot area
    switch(pot_id)
    {
        case 0:
        case 4:
            glcd_rect_invert(display, 0, knob.y -7, 64, 16);
            break;
        case 1:
        case 5:
            glcd_rect_invert(display, 0, knob.y -6, 64, 14);
            break;
        case 2:
        case 6:
            glcd_rect_invert(display, 65, knob.y -7, 63, 16);
            break;
        case 3:
        case 7:
            glcd_rect_invert(display, 65, knob.y -6, 63, 14);
            break;
    }
#else
    glcd_vline(display, 64, 25, 31, GLCD_BLACK);
#endif
}

static void null_screen_encoded(glcd_t *display, uint8_t display_id)
{
    textbox_t blank_title;
    blank_title.color = GLCD_BLACK;
    blank_title.mode = TEXT_SINGLE_LINE;
    blank_title.font = Terminal3x5;
    blank_title.height = 0;
    blank_title.width = 0;
    blank_title.top_margin = 0;
    blank_title.bottom_margin = 0;
    blank_title.left_margin = 0;
    blank_title.right_margin = 0;
    blank_title.y = 14;
    blank_title.align = ALIGN_NONE_NONE;

    if (g_hide_non_assigned_actuators)
    {
        blank_title.text = "-";
        blank_title.x = (DISPLAY_WIDTH/2) - 1;
    }
    else 
    {
        char text[sizeof(SCREEN_ROTARY_DEFAULT_NAME) + 2];
        strcpy(text, SCREEN_ROTARY_DEFAULT_NAME);
        text[sizeof(SCREEN_ROTARY_DEFAULT_NAME)-1] = display_id + '1';
        text[sizeof(SCREEN_ROTARY_DEFAULT_NAME)] = 0;
        blank_title.x = ((DISPLAY_WIDTH/2) - ((strlen(text)*4)/2));
        blank_title.text = text;

    }

    widget_textbox(display, &blank_title);
    glcd_hline(display, 0, 24, DISPLAY_WIDTH, GLCD_BLACK);
}

void screen_encoder(uint8_t display_id, control_t *control)
{
    glcd_t *display = hardware_glcds(display_id);
    glcd_rect_fill(display, 0, 8, DISPLAY_WIDTH, 16, GLCD_WHITE);

    if (!control)
        return null_screen_encoded(display, display_id);

    // list type control
    if (control->properties & (FLAG_CONTROL_ENUMERATION | FLAG_CONTROL_SCALE_POINTS))
    {
        uint8_t scalepoint_count_local = control->scale_points_count > 64 ? 64 : control->scale_points_count;

        char **labels_list = MALLOC(sizeof(char*) * scalepoint_count_local);

        if (labels_list == NULL)
        {
            return null_screen_encoded(display, display_id);
        }

        if (control->scroll_dir)
            control->scroll_dir = 1;
        else
            control->scroll_dir = 0;

        uint8_t i;
        for (i = 0; i < scalepoint_count_local; i++)
        {
            labels_list[i] = control->scale_points[i]->label;
        }

        listbox_t list;
        list.x = 0;
        list.y = 10;
        list.height = 13;
        list.color = GLCD_BLACK;
        list.font = Terminal3x5;
        list.selected = control->step;
        list.count = scalepoint_count_local;
        list.list = labels_list;
        list.direction = control->scroll_dir;
        list.line_space = 1;
        list.line_top_margin = 1;
        list.line_bottom_margin = 0;
        list.text_left_margin = 1;
        list.name = control->label;
        widget_listbox3(display, &list);

        FREE(labels_list);
    }
    else if (control->properties & (FLAG_CONTROL_TOGGLED | FLAG_CONTROL_BYPASS))
    {
        toggle_t toggle;
        toggle.x = 0;
        toggle.y = 10;
        toggle.width = DISPLAY_WIDTH;
        toggle.height = 13;
        toggle.color = GLCD_BLACK;
        toggle.value = (control->properties & FLAG_CONTROL_TOGGLED)?control->value:!control->value;
        toggle.label = control->label;
        widget_toggle_encoder(display, &toggle);
    }

    // integer type control
    else if (control->properties & FLAG_CONTROL_INTEGER)
    {
        char *title_str_bfr = (char *) MALLOC(15 * sizeof(char));
        strncpy(title_str_bfr, control->label, 14);
        title_str_bfr[14] = '\0';

        textbox_t title;
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = Terminal3x5;
        title.height = 0;
        title.width = 0;
        title.top_margin = 0;
        title.bottom_margin = 0;
        title.left_margin = 0;
        title.right_margin = 0;
        title.text = title_str_bfr;
        title.align = ALIGN_NONE_NONE;
        title.x = ((DISPLAY_WIDTH / 4) - (strlen(title_str_bfr) * 1.5));
        title.y = 14;
        widget_textbox(display, &title);
        FREE(title_str_bfr);

        textbox_t int_box;
        char *value_str = (char *) MALLOC(16 * sizeof(char));
        char value_str_bfr[5] = {0};
        int_to_str(control->value, value_str, sizeof(value_str), 0);
        int_box.color = GLCD_BLACK;
        int_box.mode = TEXT_SINGLE_LINE;
        int_box.font = alterebro15nums;
        int_box.height = 0;
        int_box.width = 0;
        int_box.top_margin = 0;
        int_box.bottom_margin = 0;
        int_box.left_margin = 0;
        int_box.right_margin = 0;
        strncpy(value_str_bfr, value_str, 4);
        value_str_bfr[4] = '\0';
        FREE(value_str);
        int_box.text = value_str_bfr;
        int_box.align = ALIGN_NONE_NONE;
        int_box.x = (((DISPLAY_WIDTH / 4) + (DISPLAY_WIDTH / 2) )- (strlen(value_str_bfr) * 1.5));
        int_box.y = 13;
        widget_textbox(display, &int_box);
    }
    //liniar/logaritmic control type, Bar graphic
    else
    {
        textbox_t value, title;

        //convert title
        char *title_str_bfr = (char *) MALLOC(25 * sizeof(char));
        strncpy(title_str_bfr, control->label, 24);
        title_str_bfr[24] = '\0';

        char value_str_bfr[11] = {0};
        char *unit_str_bfr = NULL;

        if (!control->value_string) {
            //convert value
            //value_str is our temporary value which we use for what kind of value representation we need
            //value_str_bfr is the value that gets printed
            char value_str[10] = {0};

            //if the value becomes bigger then 9999 (4 characters), then switch to another view 10999 becomes 10.9K
            if (control->value > 9999) {
                int_to_str(control->value/1000, value_str, sizeof(value_str) - 1, 0);
                strcat(value_str, "K");
            }
            //else if we have a value bigger then 100, we dont display decimals anymore
            else if (control->value > 99.9) {
                int_to_str(control->value, value_str, sizeof(value_str), 0);
            }
            //else if we have a value bigger then 10 we display just one decimal
            else if (control->value > 9.9) {
                float_to_str((control->value), value_str, sizeof(value_str), 1);
            }
            //if the value becomes less then 0 we change to 1 or 0 decimals
            else if (control->value < 0) {
                if (control->value > -99.9) {
                    float_to_str(control->value, value_str, sizeof(value_str), 1);
                }
                else if (control->value < -9999.9) {
                    int_to_str((control->value/1000), value_str, sizeof(value_str) - 1, 0);
                    strcat(value_str, "K");
                }
                else int_to_str(control->value, value_str, sizeof(value_str), 0);
            }
            //for values between 0 and 10 display 2 decimals
            else {
                float_to_str(control->value, value_str, sizeof(value_str), 2);
            }

            //copy to value_str_bfr, the first 5 char
            strncpy(value_str_bfr, value_str, 5);
            //terminate the text with line ending
            value_str_bfr[5] ='\0';

            //convert unit
            const char *unit_str;
            unit_str = (strcmp(control->unit, "") == 0 ? NULL : control->unit);

            //adds the unit to the value
            if (unit_str != NULL) {
                unit_str_bfr = MALLOC(strlen(value_str_bfr)+strlen(unit_str)+3);
                strcpy(unit_str_bfr, value_str_bfr);
                strcat(unit_str_bfr, " ");
                strcat(unit_str_bfr, unit_str);
                value.text = unit_str_bfr;
            }
            else {
                value.text = value_str_bfr;
            }
        }
        else {
            //draw the value string
            uint8_t char_cnt_value = strlen(control->value_string);

            if (char_cnt_value > 10)
                char_cnt_value = 10;

            control->value_string[char_cnt_value] = '\0';
            strcpy(value_str_bfr, control->value_string);
            value_str_bfr[char_cnt_value] = '\0';
            value.text = value_str_bfr;
        }

        //title:
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = Terminal3x5;
        title.x = 3;
        title.y = 10;
        title.height = 0;
        title.width = 0;
        title.top_margin = 0;
        title.bottom_margin = 0;
        title.left_margin = 2;
        title.right_margin = 0;
        title.text = title_str_bfr;
        title.align = ALIGN_LEFT_NONE;
        widget_textbox(display, &title);

        FREE(title_str_bfr);

        //value
        value.color = GLCD_BLACK;
        value.mode = TEXT_SINGLE_LINE;
        value.font = Terminal3x5;
        value.x = 100;
        value.y = 10;
        value.height = 0;
        value.width = 0;
        value.top_margin = 0;
        value.bottom_margin = 0;
        value.left_margin = 0;
        value.right_margin = 2;
        value.align = ALIGN_RIGHT_NONE;
        widget_textbox(display, &value);

        FREE (unit_str_bfr);

        bar_t volume_bar;
        volume_bar.x = 2;
        volume_bar.y = 17;
        volume_bar.width = 124;
        volume_bar.height = 5;
        if (control->screen_indicator_widget_val == -1) {
            volume_bar.step = control->step;
            volume_bar.steps = control->steps - 1;
        }
        else {
            volume_bar.step = control->screen_indicator_widget_val * 100;
            volume_bar.steps = 100;
        }
        widget_bar_indicator(display, &volume_bar);
    }

    glcd_hline(display, 0, 24, DISPLAY_WIDTH, GLCD_BLACK);
}

void screen_footer(uint8_t id, const char *name, const char *value, int16_t property)
{
    glcd_t *display = hardware_glcds((id < 2)?DISPLAY_LEFT:DISPLAY_RIGHT);

    //we dont display foots when in fool mode
    if (naveg_is_tool_mode((id < 2)?DISPLAY_LEFT:DISPLAY_RIGHT)) return;

    uint8_t align = 0;
    switch (id)
    {
        case 0:
            // clear the footer area
            glcd_rect_fill(display, 0, 56, 64, 8, GLCD_WHITE);
        break;
        case 1:
            align = 1;
            // clear the footer area
            glcd_rect_fill(display, 65, 56, 63, 8, GLCD_WHITE);
        break;
        case 2:
            // clear the footer area
            glcd_rect_fill(display, 0, 56, 64, 8, GLCD_WHITE);
        break;
        case 3:
            align = 1;
            // clear the footer area
            glcd_rect_fill(display, 65, 56, 63, 8, GLCD_WHITE);
        break;
        default:
            return;
        break;
    }

    // horizontal footer line
    glcd_hline(display, 0, 55, DISPLAY_WIDTH, GLCD_BLACK);

    //vertival footer line
    glcd_vline(display, 64, 56, 8, GLCD_BLACK);

    if (name == NULL || value == NULL)
    {
        char text[sizeof(SCREEN_FOOT_DEFAULT_NAME) + 2];
        strcpy(text, SCREEN_FOOT_DEFAULT_NAME);
        text[sizeof(SCREEN_FOOT_DEFAULT_NAME)-1] = (id + '1');
        text[sizeof(SCREEN_FOOT_DEFAULT_NAME)] = 0;    

        textbox_t title;
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = Terminal3x5;
        title.top_margin = 0;
        title.bottom_margin = 1;
        title.left_margin = 0;
        title.right_margin = 0;
        title.height = 0;
        title.text = g_hide_non_assigned_actuators? "-" : text;  
        title.width = 0;
        title.align = align ? ALIGN_RCENTER_BOTTOM : ALIGN_LCENTER_BOTTOM;
        title.y = 0;

        widget_textbox(display, &title);
        return;
    }

    ///checks if its toggle/trigger or a value
    else if (property & (FLAG_CONTROL_TOGGLED | FLAG_CONTROL_BYPASS | FLAG_CONTROL_TRIGGER | FLAG_CONTROL_MOMENTARY))
    {
    	// draws the name field
    	char *title_str_bfr = (char *) MALLOC(16 * sizeof(char));
    	textbox_t footer;
    	footer.color = GLCD_BLACK;
    	footer.mode = TEXT_SINGLE_LINE;
    	footer.font = Terminal3x5;
    	footer.height = 0;
    	footer.top_margin = 0;
    	footer.bottom_margin = 1;
    	footer.left_margin = 0;
    	footer.width = 0;
    	footer.right_margin = 0;
    	strncpy(title_str_bfr, name, 15);
    	title_str_bfr[15] = '\0';
    	footer.text = title_str_bfr;
    	footer.y = 0;
    	footer.align = align ? ALIGN_RCENTER_BOTTOM : ALIGN_LCENTER_BOTTOM;
    	widget_textbox(display, &footer);
    	FREE(title_str_bfr);

#ifndef FOOT_INV_BORDERS
    uint8_t foor_inv_box_L_x = 0;
    uint8_t foor_inv_box_R_x = 65;
    uint8_t foor_inv_box_y = 56;
    uint8_t foor_inv_box_L_width = 63;
    uint8_t foor_inv_box_R_width = 64;
    uint8_t foor_inv_box_height = 8;
#else
    uint8_t foor_inv_box_L_x = 0;
    uint8_t foor_inv_box_R_x = 66;
    uint8_t foor_inv_box_y = 57;
    uint8_t foor_inv_box_L_width = 62;
    uint8_t foor_inv_box_R_width = 63;
    uint8_t foor_inv_box_height = 7;
#endif

        if (property & FLAG_CONTROL_MOMENTARY)
        {
            //reverse
            if (property & FLAG_CONTROL_REVERSE)
            {
                if ((value[1] != 'N') && !(property & FLAG_CONTROL_BYPASS))
                {
                    if (align)
                        glcd_rect_invert(display, foor_inv_box_R_x, foor_inv_box_y, foor_inv_box_L_width, foor_inv_box_height);
                    else
                        glcd_rect_invert(display, foor_inv_box_L_x, foor_inv_box_y, foor_inv_box_R_width, foor_inv_box_height);
                }
                else if (property & FLAG_CONTROL_BYPASS)
                {
                    if (value[1] == 'N')
                    {
                        if (align)
                            glcd_rect_invert(display, foor_inv_box_R_x, foor_inv_box_y, foor_inv_box_L_width, foor_inv_box_height);
                        else
                            glcd_rect_invert(display, foor_inv_box_L_x, foor_inv_box_y, foor_inv_box_R_width, foor_inv_box_height);
                    }
                }
            }
            else 
            {
                if ((value[1] == 'N') && !(property & FLAG_CONTROL_BYPASS))
                {
                    if (align)
                        glcd_rect_invert(display, foor_inv_box_R_x, foor_inv_box_y, foor_inv_box_L_width, foor_inv_box_height);
                    else
                        glcd_rect_invert(display, foor_inv_box_L_x, foor_inv_box_y, foor_inv_box_R_width, foor_inv_box_height);
                }
                else if (property & FLAG_CONTROL_BYPASS)
                {
                    if (value[1] != 'N')
                    {
                        if (align)
                            glcd_rect_invert(display, foor_inv_box_R_x, foor_inv_box_y, foor_inv_box_L_width, foor_inv_box_height);
                        else
                            glcd_rect_invert(display, foor_inv_box_L_x, foor_inv_box_y, foor_inv_box_R_width, foor_inv_box_height);
                    }
                } 
            }
        }
        else if (value[1] == 'N')
        {
            if (align)
                glcd_rect_invert(display, foor_inv_box_R_x, foor_inv_box_y, foor_inv_box_L_width, foor_inv_box_height);
            else
                glcd_rect_invert(display, foor_inv_box_L_x, foor_inv_box_y, foor_inv_box_R_width, foor_inv_box_height);
        }

        return;
    }
    //scalepoints
    else
    {
        uint8_t char_cnt_name = strlen(name);
        uint8_t char_cnt_value = strlen(value);

        if ((char_cnt_value + char_cnt_name) > 14)
        {
            //both bigger then the limmit
            if ((char_cnt_value > 7)&&(char_cnt_name > 7))
            {
                char_cnt_name = 7;
                char_cnt_value = 7;
            }
            //value bigger then 7, name not
            else if (char_cnt_value > 7)
            {
                char_cnt_value = 14 - char_cnt_name;
            }
            //name bigger then 7, value not
            else if (char_cnt_name > 7)
            {
                char_cnt_name = 14 - char_cnt_value;
            }
        }

        char *title_str_bfr = (char *) MALLOC((char_cnt_name + 1) * sizeof(char));
        char *value_str_bfr = (char *) MALLOC((char_cnt_value + 1) * sizeof(char));

        // draws the name field
        textbox_t footer;
        footer.color = GLCD_BLACK;
        footer.mode = TEXT_SINGLE_LINE;
        footer.font = Terminal3x5;
        footer.height = 0;
        footer.top_margin = 0;
        footer.bottom_margin = 1;
        footer.left_margin = 2;
        footer.width = 0;
        footer.right_margin = 0;
        strncpy(title_str_bfr, name, char_cnt_name);
        title_str_bfr[char_cnt_name] = '\0';
        footer.text = title_str_bfr;
        footer.y = 0;
        footer.align = align ? ALIGN_RLEFT_BOTTOM : ALIGN_LEFT_BOTTOM;
        widget_textbox(display, &footer);
        FREE(title_str_bfr);

        // draws the value field
        footer.width = 0;
        footer.right_margin = 2;
        strncpy(value_str_bfr, value, char_cnt_value);
        value_str_bfr[char_cnt_value] =  '\0';
        footer.text = value_str_bfr;
        footer.align = align ? ALIGN_RIGHT_BOTTOM : ALIGN_LRIGHT_BOTTOM;
        widget_textbox(display, &footer);
        FREE(value_str_bfr);
    }
}

void screen_top_info(const void *data, uint8_t update)
{
    static char* pedalboard_name = NULL;

    glcd_t *display = hardware_glcds(0);

    if (pedalboard_name == NULL)
    {
        pedalboard_name = (char *) MALLOC(32 * sizeof(char));
        strcpy(pedalboard_name, "default");
    }

    if (update)
    {
        const char **name_list = (const char**)data;

        // get first list name, copy it to our string buffer
        const char *name_string = *name_list;
        strncpy(pedalboard_name, name_string, 31);
        pedalboard_name[31] = 0; // strncpy might not have final null byte

        // go to next name in list
        name_string = *(++name_list);

        while (name_string && ((strlen(pedalboard_name) + strlen(name_string) + 1) < 31))
        {
            strcat(pedalboard_name, " ");
            strcat(pedalboard_name, name_string);
            name_string = *(++name_list);
        }
        pedalboard_name[31] = 0;
    }

    //we dont display inside a menu
    if (naveg_is_tool_mode(DISPLAY_LEFT)) return;

    // clear the name area
    glcd_rect_fill(display, 0, 0, DISPLAY_WIDTH, 7, GLCD_WHITE);

    textbox_t title;
    title.color = GLCD_BLACK;
    title.mode = TEXT_SINGLE_LINE;
    title.font = Terminal3x5;
    title.top_margin = 1;
    title.bottom_margin = 0;
    title.left_margin = 0;
    title.right_margin = 0;
    title.height = 0;
    title.width = 0;
    title.text = pedalboard_name;
    title.align = ALIGN_CENTER_TOP;
    title.y = 0;
    title.x = 1;
    widget_textbox(display, &title);

    // horizontal footer line
    glcd_hline(display, 0, 7, DISPLAY_WIDTH, GLCD_BLACK);

#ifdef INVERT_HEADERS
    glcd_rect_invert(display, 0, 0, DISPLAY_WIDTH, 7);
#endif
}

void screen_tool(uint8_t tool, uint8_t display_id)
{
    bp_list_t *bp_list;
    glcd_t *display = hardware_glcds(display_id);

    switch (tool)
    {
        case DISPLAY_TOOL_SYSTEM:
            naveg_reset_menu();
            naveg_enter(DISPLAY_TOOL_SYSTEM);
            break;

        case DISPLAY_TOOL_TUNER:
            g_tuner.frequency = 0.0;
            g_tuner.note = "?";
            g_tuner.cents = 0;
            widget_tuner(display, &g_tuner);
            break;

        case DISPLAY_TOOL_NAVIG:
            bp_list = naveg_get_banks();

            if (naveg_banks_mode_pb() == BANKS_LIST)
            {
                screen_bp_list("BANKS", bp_list);
            }
            else 
            {
                screen_bp_list(naveg_get_current_pb_name(), bp_list);
            }
            break;
    }
}

void screen_bp_list(const char *title, bp_list_t *list)
{
    if (!naveg_is_tool_mode(DISPLAY_RIGHT))
        return; 

    listbox_t list_box;
    textbox_t title_box;

    glcd_t *display = hardware_glcds(1);

    // clears the title
    glcd_rect_fill(display, 0, 0, DISPLAY_WIDTH, 9, GLCD_WHITE);

    // draws the title
    title_box.color = GLCD_BLACK;
    title_box.mode = TEXT_SINGLE_LINE;
    title_box.font = Terminal3x5;
    title_box.top_margin = 0;
    title_box.bottom_margin = 0;
    title_box.left_margin = 0;
    title_box.right_margin = 0;
    title_box.height = 0;
    title_box.width = 0;
    title_box.text = title;
    title_box.align = ALIGN_CENTER_TOP;
    widget_textbox(display, &title_box);

    // title line separator
    glcd_hline(display, 0, 9, DISPLAY_WIDTH, GLCD_BLACK_WHITE);

    // draws the list
    if (list)
    {
        uint8_t count = strarr_length(list->names);
        list_box.x = 0;
        list_box.y = 11;
        list_box.width = 128;
        list_box.height = 53;
        list_box.color = GLCD_BLACK;
        list_box.hover = list->hover - list->page_min;
        list_box.selected = list->selected - list->page_min;
        list_box.count = count;
        list_box.list = list->names;
        list_box.font = Terminal3x5;
        list_box.line_space = 2;
        list_box.line_top_margin = 1;
        list_box.line_bottom_margin = 1;
        list_box.text_left_margin = 2;
        widget_listbox(display, &list_box);
    }
    else
    {
        if (naveg_ui_status())
        {
            textbox_t message;
            message.color = GLCD_BLACK;
            message.mode = TEXT_SINGLE_LINE;
            message.font = alterebro24;
            message.top_margin = 0;
            message.bottom_margin = 0;
            message.left_margin = 0;
            message.right_margin = 0;
            message.height = 0;
            message.width = 0;
            message.text = "To access banks here please disconnect from the graphical interface";
            message.align = ALIGN_CENTER_MIDDLE;
            widget_textbox(display, &message);
        }
    }
}

void screen_system_menu(menu_item_t *item)
{
    // return if system is disabled
    if (!naveg_is_tool_mode(DISPLAY_TOOL_SYSTEM))
        return;

    static menu_item_t *last_item = NULL;

    glcd_t *display;
    if (item->desc->id == ROOT_ID)
    {
        display = hardware_glcds(DISPLAY_TOOL_SYSTEM);
    }
    else if (item->desc->id == TUNER_ID)
    {
        return;
    }
    else
    {
        display = hardware_glcds(1);
    }

    // clear screen
    glcd_clear(display, GLCD_WHITE);

    // draws the title
    textbox_t title_box;
    title_box.color = GLCD_BLACK;
    title_box.mode = TEXT_SINGLE_LINE;
    title_box.font = Terminal3x5;
    title_box.top_margin = 0;
    title_box.bottom_margin = 0;
    title_box.left_margin = 0;
    title_box.right_margin = 0;
    title_box.height = 0;
    title_box.width = 0;
    title_box.align = ALIGN_CENTER_TOP;
    title_box.text = item->name;

    if ((item->desc->type == MENU_NONE) || (item->desc->type == MENU_TOGGLE))
    {
        if (last_item)
        {
            title_box.text = last_item->name;
            if (title_box.text[strlen(item->name) - 1] == ']')
                title_box.text = last_item->desc->name;
        }
    }
    else if (title_box.text[strlen(item->name) - 1] == ']')
    {
        title_box.text = item->desc->name;
    }
    widget_textbox(display, &title_box);

    // title line separator
    glcd_hline(display, 0, 9, DISPLAY_WIDTH, GLCD_BLACK_WHITE);
    glcd_hline(display, 0, 10, DISPLAY_WIDTH, GLCD_WHITE);

    // menu list
    listbox_t list;
    list.x = 0;
    list.y = 11;
    list.width = 128;
    list.height = 53;
    list.color = GLCD_BLACK;
    list.font = Terminal3x5;
    list.line_space = 2;
    list.line_top_margin = 1;
    list.line_bottom_margin = 1;
    list.text_left_margin = 2;

    // popup
    popup_t popup;
    popup.x = 0;
    popup.y = 0;
    popup.width = DISPLAY_WIDTH;
    popup.height = DISPLAY_HEIGHT;
    popup.font = Terminal3x5;
    switch (item->desc->type)
    {
        case MENU_LIST:
        case MENU_SELECT:
            list.hover = item->data.hover;
            list.selected = item->data.selected;
            list.count = item->data.list_count;
            list.list = item->data.list;
            widget_listbox(display, &list);
            if ((last_item && last_item->desc->id != TEMPO_ID) || item->desc->id == SYSTEM_ID || item->desc->id == BYPASS_ID)
                last_item = item;
            break;

        case MENU_CONFIRM:
        case MENU_CONFIRM2:
        case MENU_CANCEL:
        case MENU_OK:
        case MENU_MESSAGE:
            if (item->desc->type == MENU_CANCEL)
                popup.type = CANCEL_ONLY;
            else if ((item->desc->type == MENU_OK) || !strcmp("WARNING", item->data.popup_header))
                popup.type = OK_ONLY;
            else if (item->desc->type == MENU_MESSAGE)
                popup.type = EMPTY_POPUP;
            else
                popup.type = YES_NO;
            popup.title = item->data.popup_header;
            popup.content = item->data.popup_content;
            popup.button_selected = item->data.hover;
            widget_popup(display, &popup);
            break;

        case MENU_TOGGLE:
            if (item->desc->parent_id == PROFILES_ID || item->desc->id == EXP_CV_INP || item->desc->id == EXP_MODE || item->desc->id == HP_CV_OUTP)
            {
                popup.type = YES_NO;
                popup.title = item->data.popup_header;
                popup.content = item->data.popup_content;
                popup.button_selected = item->data.hover;
                widget_popup(display, &popup);
            }
            else if (last_item)
            {
                list.hover = last_item->data.hover;
                list.selected = last_item->data.selected;
                list.count = last_item->data.list_count;
                list.list = last_item->data.list;
                widget_listbox(display, &list);
            }
            break;

        case MENU_VOL:
        case MENU_SET:
        case MENU_NONE:
        case MENU_RETURN:
            if (last_item)
            {
                list.hover = last_item->data.hover;
                list.selected = last_item->data.selected;
                list.count = last_item->data.list_count;
                list.list = last_item->data.list;
                widget_listbox(display, &list);
            }
            break;
    }
}

void screen_tuner(float frequency, char *note, int8_t cents)
{
    g_tuner.frequency = frequency;
    g_tuner.note = note;
    g_tuner.cents = cents;

    // checks if tuner is enable and update it
    if (naveg_is_tool_mode(DISPLAY_TOOL_TUNER))
        widget_tuner(hardware_glcds(1), &g_tuner);
}

void screen_tuner_input(uint8_t input)
{
    g_tuner.input = input;

    // checks if tuner is enable and update it
    if (naveg_is_tool_mode(DISPLAY_TOOL_TUNER))
        widget_tuner(hardware_glcds(1), &g_tuner);
}

void screen_image(uint8_t display, const uint8_t *image)
{
    glcd_t *display_img = hardware_glcds(display);
    glcd_draw_image(display_img, 0, 0, image, GLCD_BLACK);
}

void screen_master_vol(float volume_val)
{
    char name[40] = "MASTER VOLUME:";

    static char str_buf[10] = {};
    if (volume_val < -29) {
        strncpy(str_buf, "-INF", 5);
        str_buf[5] = '\0';
        strcat(name, str_buf);
    }
    else {
        int_to_str(volume_val, str_buf, sizeof(str_buf), 0);
        if (volume_val > 0)
            strcat(name, "+");
        strcat(name, str_buf);
        strcat(name, " dB");
    }

    glcd_t *display = hardware_glcds(1);

    // clear the name area
    glcd_rect_fill(display, 0, 0, DISPLAY_WIDTH, 7, GLCD_WHITE);

    textbox_t title;
    title.color = GLCD_BLACK;
    title.mode = TEXT_SINGLE_LINE;
    title.font = Terminal3x5;
    title.top_margin = 1;
    title.bottom_margin = 0;
    title.left_margin = 1;
    title.right_margin = 0;
    title.height = 0;
    title.width = 0;
    title.text = name;
    title.align = ALIGN_LEFT_TOP;
    title.y = 0;
    title.x = 0;
    widget_textbox(display, &title);

    bar_t volume_bar;
    volume_bar.x = (127-45);
    volume_bar.y = 1;
    volume_bar.width = 45;
    volume_bar.height = 5;
    volume_bar.step = MAP(volume_val, -30, 20, 0, 50);
    volume_bar.steps = 50;
    widget_bar_indicator(display, &volume_bar);

    // horizontal line
    glcd_hline(display, 0, 7, DISPLAY_WIDTH, GLCD_BLACK);

    if (naveg_is_master_vol()) glcd_rect_invert (display, 0, 0, DISPLAY_WIDTH, 7);

#ifdef INVERT_HEADERS
    glcd_rect_invert(display, 0, 0, DISPLAY_WIDTH, 7);
#endif

}

void screen_text_box(uint8_t display, uint8_t x, uint8_t y, const char *text)
{
    glcd_t *hardware_display = hardware_glcds(display);

    textbox_t text_box;
    text_box.color = GLCD_BLACK;
    text_box.mode = TEXT_MULTI_LINES;
    text_box.font = Terminal3x5;
    text_box.top_margin = 1;
    text_box.bottom_margin = 0;
    text_box.left_margin = 1;
    text_box.right_margin = 0;
    text_box.height = 63;
    text_box.width = 127;
    text_box.text = text;
    text_box.align = ALIGN_NONE_NONE;
    text_box.y = y;
    text_box.x = x;
    widget_textbox(hardware_display, &text_box);
}