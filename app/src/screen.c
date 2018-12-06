
/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include "screen.h"
#include "utils.h"
#include "glcd.h"
#include "glcd_widget.h"
#include "naveg.h"
#include "hardware.h"
#include <string.h>
#include <stdlib.h>

/*
************************************************************************************************************************
*           LOCAL DEFINES
************************************************************************************************************************
*/

#define FOOTER_NAME_WIDTH       ((DISPLAY_WIDTH * 50)/100)
#define FOOTER_VALUE_WIDTH      (DISPLAY_WIDTH - FOOTER_NAME_WIDTH)


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

static tuner_t g_tuner = {0, NULL, 0, 1};


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

void screen_clear(uint8_t display_id)
{
    glcd_clear(hardware_glcds(display_id), GLCD_WHITE);
}

void screen_pot(uint8_t pot_id, control_t *control)
{   
    //select display
    uint8_t display_id = 0;
    if (pot_id >= 4) display_id = 1;
    glcd_t *display = hardware_glcds(display_id);

    knob_t knob;
    //FIXME: pot position hardcoded
    switch(pot_id)
    {
        case 0:
        case 4:
            knob.x = 27;              
            knob.y = 32; 
            knob.orientation = 0;
            glcd_rect_fill(display, 0, knob.y -8, 65, 16 , GLCD_WHITE);
            break;
        case 1:
        case 5:
            knob.x = 27;             
            knob.y = 47;
            knob.orientation = 0;
            glcd_rect_fill(display, 0, knob.y -7, 65, 16 , GLCD_WHITE);
            break;
        case 2:
        case 6:
            knob.x = 100;            
            knob.y = 32;
            knob.orientation = 1;
            glcd_rect_fill(display, 65, knob.y -8, 63, 16 , GLCD_WHITE);
            break;
        case 3:
        case 7:
            knob.x = 100;             
            knob.y = 47; 
            knob.orientation = 1;
            glcd_rect_fill(display, 65, knob.y -7, 63, 16 , GLCD_WHITE);
            break;
    }

    //no assignment
    if (!control)
    {
        char text[sizeof(SCREEN_POT_DEFAULT_NAME) + 2];
        strcpy(text, SCREEN_POT_DEFAULT_NAME);
        text[sizeof(SCREEN_POT_DEFAULT_NAME)-1] = pot_id + '1';
        text[sizeof(SCREEN_POT_DEFAULT_NAME)] = 0;

        textbox_t blank_title;
        blank_title.color = GLCD_BLACK;
        blank_title.mode = TEXT_SINGLE_LINE;
        blank_title.font = SMfont;
        blank_title.height = 0;
        blank_title.width = 0;
        blank_title.top_margin = 0;
        blank_title.bottom_margin = 0;
        blank_title.left_margin = 0;
        blank_title.right_margin = 0;
        blank_title.text = text;
        blank_title.x = knob.orientation ? (knob.x - 8 - (strlen(text)*1.5)) : (knob.x + 3 - (strlen(text)*1.5));
        blank_title.y = knob.y - 3;
        blank_title.align = ALIGN_NONE_NONE;
        widget_textbox(display, &blank_title);
    }
    else
    {
        textbox_t value, unit, title;

        //convert title
        char title_str_bfr[8];
        strncpy(title_str_bfr, control->label, 7);
        //strcat(title_str_bfr, '\0');

        //convert value
        char value_str[6];
        char value_str_bfr[6]; 
        if (control->value > 9999)
        {
            float tmp_value = control->value / 1000;
            int_to_str(tmp_value, value_str, sizeof(value_str), 0);
            strcat(value_str, "K");
        }  
        else if (control->value > 99.9) int_to_str(control->value, value_str, sizeof(value_str), 0);
        else if (control->value > 9.9) float_to_str((control->value), value_str, sizeof(value_str), 1);
        else if (control->value < -9.9)
        {
            if (control->value > -99.9) 
            {
                float_to_str(control->value, value_str, sizeof(value_str), 1);
            }
            else if (control->value < -9999.9) 
            {
                int_to_str((control->value/1000), value_str, sizeof(value_str), 0);
                strcat(value_str, "K");
            }
            else int_to_str(control->value, value_str, sizeof(value_str), 0);
        }
        else float_to_str(control->value, value_str, sizeof(value_str), 2);
        strncpy(value_str_bfr, value_str, 5);
        FREE(value_str);

        //convert unit
        const char *unit_str;
        unit_str = (strcmp(control->unit, "") == 0 ? NULL : control->unit); 

        //draw  line vertical
        glcd_vline(display, 64, 24, 31, GLCD_WHITE);

        //draw horizontal
        glcd_hline(display, 0, 24, DISPLAY_WIDTH, GLCD_BLACK);

        //knob
        knob.color = GLCD_BLACK;
        knob.steps = control->steps;
        knob.step = control->step;
        widget_knob(display, &knob);

        //title:
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = SMfont;
        if (knob.orientation) title.x = (knob.x - 6 - (strlen(title_str_bfr))*4);
        else title.x = (knob.x + 8);
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

        //FREE(title_str_bfr);
    
        //sets the value y, for both value as unit
        value.y = knob.y - 5;
        
        //draws the unit
        if ((unit_str != NULL) && (strchr(unit_str, '%')==NULL))
        {
            unit.color = GLCD_BLACK;
            unit.mode = TEXT_SINGLE_LINE;
            unit.font = SMfont;
            if (knob.orientation) unit.x =(knob.x + 8);
            else unit.x = (knob.x - 6 - (strlen(unit_str)*4));
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
        }
        //no unit or %, change value posistion
        else 
        {
            value.y += 3; 

            //if unit = %, add it to the value string (if its one of the pots on the left add 4 pixels (1 char)) 
            if (strchr(unit_str, '%')) strcat(value_str_bfr, unit_str);
        }

        //value
        value.color = GLCD_BLACK;
        value.mode = TEXT_SINGLE_LINE;
        value.font = SMfont;
        if (knob.orientation)  value.x = (knob.x + 8);
        else  value.x = (knob.x - 6 - (strlen(value_str_bfr)*4));
        value.height = 0;
        value.width = 0;
        value.top_margin = 0;
        value.bottom_margin = 0;
        value.left_margin = 0;
        value.right_margin = 0;
        value.text = value_str_bfr;
        value.align = ALIGN_NONE_NONE;
        widget_textbox(display, &value);
        //FREE (value_str_bfr);
    }
    
    //invert the pot area
    switch(pot_id)
    {
        case 0:
        case 4:
            glcd_rect_invert(display, 0, knob.y -7, 64, 15);
            break;
        case 1:
        case 5:
            glcd_rect_invert(display, 0, knob.y -7, 64, 16);
            break;
        case 2:
        case 6:
            glcd_rect_invert(display, 65, knob.y -7, 63, 15);
            break;
        case 3:
        case 7:
            glcd_rect_invert(display, 65, knob.y -7, 63, 16);
            break;
    }
}

void screen_encoder(uint8_t display_id, control_t *control)
{  
    glcd_t *display = hardware_glcds(display_id);
    glcd_rect_fill(display, 0, 8, DISPLAY_WIDTH, 16, GLCD_WHITE);

    if (!control)
    {
        char text[sizeof(SCREEN_ROTARY_DEFAULT_NAME) + 2];
        strcpy(text, SCREEN_ROTARY_DEFAULT_NAME);
        text[sizeof(SCREEN_ROTARY_DEFAULT_NAME)-1] = display_id + '1';
        text[sizeof(SCREEN_ROTARY_DEFAULT_NAME)] = 0;

        textbox_t blank_title;
        blank_title.color = GLCD_BLACK;
        blank_title.mode = TEXT_SINGLE_LINE;
        blank_title.font = SMfont;
        blank_title.height = 0;
        blank_title.width = 0;
        blank_title.top_margin = 0;
        blank_title.bottom_margin = 0;
        blank_title.left_margin = 0;
        blank_title.right_margin = 0;
        blank_title.text = text;
        blank_title.x = ((DISPLAY_WIDTH/2) - ((strlen(text)*4)/2));
        blank_title.y = 14;
        blank_title.align = ALIGN_NONE_NONE;
        widget_textbox(display, &blank_title);
        return;
    }

    // integer type control
    if (control->properties == CONTROL_PROP_INTEGER)
    {

        textbox_t title;
        char *title_str_bfr = (char *) MALLOC(8 * sizeof(char));
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = SMfont;
        title.height = 0;
        title.width = 0;
        title.top_margin = 0;
        title.bottom_margin = 0;
        title.left_margin = 0;
        title.right_margin = 0;
        strncpy(title_str_bfr, control->label, 7);
        //strcat(title_str_bfr, '\0');
        title.text = title_str_bfr;
        title.align = ALIGN_NONE_NONE;
        title.x = ((DISPLAY_WIDTH / 4) - (strlen(title_str_bfr) * 1.5));
        title.y = 14;
        widget_textbox(display, &title);
        FREE(title_str_bfr);

        textbox_t int_box;
        char *value_str = (char *) MALLOC(16 * sizeof(char));
        char *value_str_bfr = (char *) MALLOC(5 * sizeof(char));
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
        FREE(value_str);
        //strcat(value_str_bfr, '\0');
        int_box.text = value_str_bfr;
        int_box.align = ALIGN_NONE_NONE;
        int_box.x = (((DISPLAY_WIDTH / 4) + (DISPLAY_WIDTH / 2) )- (strlen(value_str_bfr) * 1.5));
        int_box.y = 13;
        widget_textbox(display, &int_box);
        FREE(value_str_bfr);
    }

    // list type control
    else if (control->properties == CONTROL_PROP_ENUMERATION ||
             control->properties == CONTROL_PROP_SCALE_POINTS)
    {
        static char *labels_list[128];

        if (control->scroll_dir) control->scroll_dir = 1;
        else control->scroll_dir = 0;

        uint8_t i;
        for (i = 0; i < control->scale_points_count; i++)
        {
            labels_list[i] = control->scale_points[i]->label;
        }

        listbox_t list;
        list.x = 0;
        list.y = 10;
        list.height = 13;
        list.color = GLCD_BLACK;
        list.font = SMfont;
        list.selected = control->step;
        list.count = control->scale_points_count;
        list.list = labels_list;
        list.direction = control->scroll_dir;
        list.line_space = 1;
        list.line_top_margin = 1;
        list.line_bottom_margin = 0;
        list.text_left_margin = 1;
        list.name = control->label;
        widget_listbox3(display, &list);
    }

    glcd_hline(display, 0, 7, DISPLAY_WIDTH, GLCD_BLACK);
    glcd_hline(display, 0, 24, DISPLAY_WIDTH, GLCD_BLACK);
}

void screen_footer(uint8_t id, const char *name, const char *value)
{
    glcd_t *display = hardware_glcds((id < 2)?0:1);
    
    uint8_t align = 0;
    switch (id)
    {
        case 0:
            //display = hardware_glcds(0);
            // clear the footer area
            glcd_rect_fill(display, 0, 56, 64, 8, GLCD_WHITE);
        break;
        case 1:
            //display = hardware_glcds(0);
            align = 1;
            // clear the footer area
            glcd_rect_fill(display, 65, 56, 63, 8, GLCD_WHITE);
        break;
        case 2:
            //display = hardware_glcds(1);

            // clear the footer area
            glcd_rect_fill(display, 0, 56, 64, 8, GLCD_WHITE);
        break;
        case 3:
            //display = hardware_glcds(1);
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

    if (name == NULL && value == NULL)
    {
        char text[sizeof(SCREEN_FOOT_DEFAULT_NAME) + 2];
        strcpy(text, SCREEN_FOOT_DEFAULT_NAME);
        text[sizeof(SCREEN_FOOT_DEFAULT_NAME)-1] = (id + '1');
        text[sizeof(SCREEN_FOOT_DEFAULT_NAME)] = 0;
     
        textbox_t title;
        title.color = GLCD_BLACK;
        title.mode = TEXT_SINGLE_LINE;
        title.font = SMfont;
        title.top_margin = 0;
        title.bottom_margin = 1;
        title.left_margin = 0;
        title.right_margin = 0;
        title.height = 0;
        title.width = 0;
        title.text = text;
        title.align = align ? ALIGN_RCENTER_BOTTOM : ALIGN_LCENTER_BOTTOM;
        title.y = 0;
        widget_textbox(display, &title);
        return;
    }

    ///checks if its toggle or pwm
    else if((value[1] == 'F')||(value[1] == 'N'))
    {
        // draws the name field
        char *title_str_bfr = (char *) MALLOC(16 * sizeof(char));
        textbox_t footer;
        footer.color = GLCD_BLACK;
        footer.mode = TEXT_SINGLE_LINE;
        footer.font = SMfont;
        footer.height = 0;
        footer.top_margin = 0;
        footer.bottom_margin = 1;
        footer.left_margin = 0;
        footer.width = 0;
        footer.right_margin = 0;
        strncpy(title_str_bfr, name, 15);
        //strcat(title_str_bfr, '\0');
        footer.text = title_str_bfr;
        footer.y = 0;
        footer.align = align ? ALIGN_RCENTER_BOTTOM : ALIGN_LCENTER_BOTTOM;
        widget_textbox(display, &footer);  
        FREE(title_str_bfr);

        if (value[1] == 'N')
        {
            if (align) glcd_rect_invert(display, 66, 57, 62, 7);
            else glcd_rect_invert(display, 0, 57, 63, 7);
            return;
        }
    }
    //other footers 
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
            else if (char_cnt_value > 7) char_cnt_value = 14 - char_cnt_name; 
            //name bigger then 7, value not
            else if (char_cnt_name > 7) char_cnt_name = 14 - char_cnt_value;
        }

        char *title_str_bfr = (char *) MALLOC(char_cnt_name * sizeof(char));
        char *value_str_bfr = (char *) MALLOC(char_cnt_value * sizeof(char));

        // draws the name field
        textbox_t footer;
        footer.color = GLCD_BLACK;
        footer.mode = TEXT_SINGLE_LINE;
        footer.font = SMfont;
        footer.height = 0;
        footer.top_margin = 0;
        footer.bottom_margin = 1;
        footer.left_margin = 2;
        footer.width = 0;
        footer.right_margin = 0;
        strncpy(title_str_bfr, name, char_cnt_name);
        //strcat(title_str_bfr, '\0');
        footer.text = title_str_bfr;
        footer.y = 0;
        footer.align = align ? ALIGN_RLEFT_BOTTOM : ALIGN_LEFT_BOTTOM;
        widget_textbox(display, &footer);  
        FREE(title_str_bfr);

        // draws the value field
        footer.width = 0;
        footer.right_margin = 2;
        strncpy(value_str_bfr, value, char_cnt_value);
        //strcat(value_str_bfr, '\0');
        footer.text = value_str_bfr;
        footer.align = align ? ALIGN_RIGHT_BOTTOM : ALIGN_LRIGHT_BOTTOM;
        widget_textbox(display, &footer); 
        FREE(value_str_bfr);
    } 
}

void screen_top_info(void *data, menu_item_t *item)
{
    (void) item;
    char **list = data;

    glcd_t *display = hardware_glcds(0);
    
    // clear the name area
    glcd_rect_fill(display, 0, 0, DISPLAY_WIDTH, 7, GLCD_WHITE);

    textbox_t title;
    title.color = GLCD_BLACK;
    title.mode = TEXT_SINGLE_LINE;
    title.font = SMfont;
    title.top_margin = 1;
    title.bottom_margin = 0;
    title.left_margin = 0;
    title.right_margin = 0;
    title.height = 0;
    title.width = 0;
    title.text = list[1];
    title.align = ALIGN_CENTER_TOP;
    title.y = 0;
    title.x = 1;
    widget_textbox(display, &title);

    // horizontal footer line
    glcd_hline(display, 0, 7, DISPLAY_WIDTH, GLCD_BLACK);
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
            if (bp_list && bp_list->selected == 0)
                bp_list->selected = 1;
            //if we already have a bank selected we enter that bank automaticly 
            else {
                bp_list->hover = bp_list->selected; 
                naveg_enter(1);
                return;
                 }
            screen_bp_list("BANKS", bp_list);
            break;
    }
}

void screen_bp_list(const char *title, bp_list_t *list)
{
    listbox_t list_box;
    textbox_t title_box;

    glcd_t *display = hardware_glcds(1);

    // clears the title
    glcd_rect_fill(display, 0, 0, DISPLAY_WIDTH, 9, GLCD_WHITE);

    // draws the title
    title_box.color = GLCD_BLACK;
    title_box.mode = TEXT_SINGLE_LINE;
    title_box.font = SMfont;
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
        list_box.hover = list->hover;
        list_box.selected = list->selected;
        list_box.count = count;
        list_box.list = list->names;
        list_box.font = SMfont;
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

    static menu_item_t *last_item;

    glcd_t *display;
    if (item->desc->id == ROOT_ID)
    {    
        display = hardware_glcds(DISPLAY_TOOL_SYSTEM);
    }
    else if (item->desc->id == TUNER_ID) return;
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
    title_box.font = SMfont;
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
        title_box.text = last_item->name;
        if (title_box.text[strlen(item->name) - 1] == ']') title_box.text = last_item->desc->name; 
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
    list.font = SMfont;
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
    popup.font = SMfont;

    switch (item->desc->type)
    {
        case MENU_LIST:
        case MENU_SELECT:
       
            list.hover = item->data.hover;
            list.selected = item->data.selected;
            list.count = item->data.list_count;
            list.list = item->data.list;
            widget_listbox(display, &list);
            last_item = item;
            break;

        case MENU_CONFIRM:
        case MENU_CONFIRM2:
        case MENU_CANCEL:
        case MENU_OK:
            if (item->desc->type == MENU_CANCEL) popup.type = CANCEL_ONLY;
            else if (item->desc->type == MENU_OK) popup.type = OK_ONLY;
            else popup.type = YES_NO;
            popup.title = item->data.popup_header;
            popup.content = item->data.popup_content;
            popup.button_selected = item->data.hover;
            widget_popup(display, &popup);
            break;

        case MENU_TOGGLE:
            if (item->desc->parent_id == PROFILES_ID ||  item->desc->id == EXP_CV_INP || item->desc->id == HP_CV_OUTP)
            {    
                popup.type = YES_NO;
                popup.title = item->data.popup_header;
                popup.content = item->data.popup_content;
                popup.button_selected = item->data.hover;
                widget_popup(display, &popup);
            }
            else 
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
            list.hover = last_item->data.hover;
            list.selected = last_item->data.selected;
            list.count = last_item->data.list_count;
            list.list = last_item->data.list;
            widget_listbox(display, &list);
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

void screen_master_vol(uint8_t volume_val)
{
    char str_buf[8];
    char name[40] = "MASTER VOLUME: ";
    int_to_str(((volume_val)), str_buf, sizeof(str_buf), 1);
    strcat(name, str_buf);

    glcd_t *display = hardware_glcds(1);
    
    // clear the name area
    glcd_rect_fill(display, 0, 0, DISPLAY_WIDTH, 7, GLCD_WHITE);

    textbox_t title;
    title.color = GLCD_BLACK;
    title.mode = TEXT_SINGLE_LINE;
    title.font = SMfont;
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
    volume_bar.x = (127-52);
    volume_bar.y = 1;
    volume_bar.width = 52;
    volume_bar.height = 5;
    volume_bar.step = volume_val;
    volume_bar.steps = 100;
    widget_bar_indicator(display, &volume_bar); 

    // horizontal line
    glcd_hline(display, 0, 7, DISPLAY_WIDTH, GLCD_BLACK);

    if (naveg_is_master_vol()) glcd_rect_invert (display, 0, 0, DISPLAY_WIDTH, 7);

}