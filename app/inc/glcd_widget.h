
/*
************************************************************************************************************************
*
************************************************************************************************************************
*/

#ifndef  GLCD_WIDGET_H
#define  GLCD_WIDGET_H


/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdint.h>
#include "glcd.h"
#include "fonts.h"


/*
************************************************************************************************************************
*           DO NOT CHANGE THESE DEFINES
************************************************************************************************************************
*/

typedef enum {ALIGN_LEFT_TOP, ALIGN_LEFT_MIDDLE, ALIGN_LEFT_BOTTOM,
              ALIGN_CENTER_TOP,ALIGN_CENTER_MIDDLE, ALIGN_CENTER_BOTTOM,
              ALIGN_RIGHT_TOP, ALIGN_RIGHT_MIDDLE, ALIGN_RIGHT_BOTTOM,
              ALIGN_NONE_NONE, ALIGN_LEFT_NONE, ALIGN_RIGHT_NONE, ALIGN_CENTER_NONE,
              ALIGN_LCENTER_BOTTOM, ALIGN_RCENTER_BOTTOM, ALIGN_LRIGHT_BOTTOM, ALIGN_RLEFT_BOTTOM} align_t;

typedef enum {TEXT_SINGLE_LINE, TEXT_MULTI_LINES} text_mode_t;

typedef enum {GRAPH_TYPE_LINEAR, GRAPH_TYPE_LOG, GRAPH_TYPE_V, GRAPH_TYPE_A} graph_type_t;

typedef enum {OK_ONLY, OK_CANCEL, CANCEL_ONLY, YES_NO, EMPTY_POPUP} popup_type_t;


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


typedef struct TEXTBOX_T {
    uint8_t x, y, width, height, color;
    align_t align;
    const char *text;
    const uint8_t *font;
    uint8_t top_margin, bottom_margin, right_margin, left_margin;
    text_mode_t mode;
} textbox_t;

typedef struct LISTBOX_T {
    uint8_t x, y, width, height, color;
    uint8_t hover, selected, count;
    char** list;
    const uint8_t *font;
    uint8_t line_space, line_top_margin, line_bottom_margin;
    uint8_t text_left_margin;
    uint8_t direction;
    const char *name;
} listbox_t;

typedef struct TOGGLE_T {
    uint8_t x, y;
    uint8_t color;
    uint8_t width, height;
    int32_t value;
    const char *label;
} toggle_t;

typedef struct KNOB_T {
    uint8_t x, y;
    uint8_t color;
    uint8_t orientation;
    uint8_t mode;
    uint8_t lock;
    float min, max, value;
    uint16_t min_cal, max_cal;
} knob_t;

typedef struct BAR_T {
    uint8_t x, y;
    uint8_t color;
    uint8_t width, height;
    int32_t step, steps;
} bar_t;

typedef struct PEAKMETER_T {
    float value, peak;
} peakmeter_t;

typedef struct TUNER_T {
    float frequency;
    char *note;
    int8_t cents;
    uint8_t input;
} tuner_t;

typedef struct POPUP_T {
    uint8_t x, y, width, height;
    const uint8_t *font;
    const char *title, *content;
    popup_type_t type;
    uint8_t button_selected;
} popup_t;


/*
************************************************************************************************************************
*           GLOBAL VARIABLES
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           MACROS
************************************************************************************************************************
*/


/*
************************************************************************************************************************
*           FUNCTION PROTOTYPES
************************************************************************************************************************
*/

void widget_textbox(glcd_t *display, textbox_t *textbox);
void widget_listbox(glcd_t *display, listbox_t *listbox);
void widget_listbox3(glcd_t *display, listbox_t *listbox);
void widget_peakmeter(glcd_t *display, uint8_t pkm_id, peakmeter_t *pkm);
void widget_toggle_encoder(glcd_t *display, toggle_t *toggle);
void widget_toggle(glcd_t *display, toggle_t *toggle); 
void widget_tuner(glcd_t *display, tuner_t *tuner);
void widget_popup(glcd_t *display, popup_t *popup);
void widget_knob(glcd_t *display, knob_t *knob);
void widget_bar_indicator(glcd_t *display, bar_t *bar); 


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
