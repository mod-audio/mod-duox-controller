
#ifndef  ACTUATOR_H
#define  ACTUATOR_H


/*
*********************************************************************************************************
*   INCLUDE FILES
*********************************************************************************************************
*/

#include <stdint.h>
#include "config.h"


/*
*********************************************************************************************************
*   DO NOT CHANGE THESE DEFINES
*********************************************************************************************************
*/

// Actuators types
typedef enum {
    BUTTON, ROTARY_ENCODER, POT
} actuator_type_t;

// Actuators properties
typedef enum {
    BUTTON_HOLD_TIME, ENCODER_STEPS
} actuator_prop_t;

// Events definition
#define EV_NONE                 0x00
#define EV_BUTTON_CLICKED       0x01
#define EV_BUTTON_PRESSED       0x02
#define EV_BUTTON_RELEASED      0x04
#define EV_BUTTON_HELD          0x08
#define EV_ALL_BUTTON_EVENTS    0x0F
#define EV_ENCODER_TURNED       0x10
#define EV_ENCODER_TURNED_CW    0x20
#define EV_ENCODER_TURNED_ACW   0x40
#define EV_ALL_ENCODER_EVENTS   0xEF
#define EV_POT_TURNED           0x80


/*
*********************************************************************************************************
*   CONFIGURATION DEFINES
*********************************************************************************************************
*/

#define MAX_ACTUATORS               20

// The actuator is activated with ZERO or ONE?
#define BUTTON_ACTIVATED            0
#define ENCODER_ACTIVATED           0

// Clock peririod definition (in miliseconds)
#define CLOCK_PERIOD                1

// Debounce configuration (in miliseconds)
#define BUTTON_PRESS_DEBOUNCE       15
#define BUTTON_RELEASE_DEBOUNCE     50
#define ENCODER_PRESS_DEBOUNCE      35
#define ENCODER_RELEASE_DEBOUNCE    100

// Encoders configuration
#define ENCODER_RESOLUTION          25


/*
*********************************************************************************************************
*   DATA TYPES
*********************************************************************************************************
*/

#define actuators_common_fields         \
    uint8_t id;                         \
    actuator_type_t type;               \
    uint8_t status, control;            \
    uint8_t events_flags;               \
    void (*event) (void *actuator);     \
    uint8_t debounce;

typedef struct BUTTON_T {
    actuators_common_fields

    uint8_t port, pin;
    uint16_t hold_time, hold_time_counter;
} button_t;

typedef struct ENCODER_T {
    actuators_common_fields

    uint8_t port, pin, port_chA, pin_chA, port_chB, pin_chB;
    uint16_t hold_time, hold_time_counter;
    uint8_t steps, state;
    int8_t counter;
} encoder_t;

typedef struct POT_T {
    actuators_common_fields

    uint8_t port, pin, function, channel;
    uint16_t value;
} pot_t;


/*
*********************************************************************************************************
*   GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*   MACRO'S
*********************************************************************************************************
*/

// Macros to read actuators state
#define BUTTON_CLICKED(status)      ((status) & EV_BUTTON_CLICKED)
#define BUTTON_PRESSED(status)      ((status) & EV_BUTTON_PRESSED)
#define BUTTON_RELEASED(status)     ((status) & EV_BUTTON_RELEASED)
#define BUTTON_HOLD(status)         ((status) & EV_BUTTON_HELD)
#define ENCODER_TURNED(status)      ((status) & EV_ENCODER_TURNED)
#define ENCODER_TURNED_CW(status)   ((status) & EV_ENCODER_TURNED_CW)
#define ENCODER_TURNED_ACW(status)  ((status) & EV_ENCODER_TURNED_ACW)
#define POT_TURNED(status)          ((status) & EV_POT_TURNED)



/*
*********************************************************************************************************
*   FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void actuator_create(actuator_type_t type, uint8_t id, void *actuator);
void actuator_set_pins(void *actuator, const uint8_t *pins);
void actuator_set_prop(void *actuator, actuator_prop_t prop, uint16_t value);
void actuator_enable_event(void *actuator, uint8_t events_flags);
void actuator_set_event(void *actuator, void (*event)(void *actuator));
uint8_t actuator_get_status(void *actuator);
void actuators_clock(void);
uint8_t actuator_get_acceleration(void);
uint16_t actuator_pot_value(uint8_t pot_id);

/*
*********************************************************************************************************
*   CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*   END HEADER
*********************************************************************************************************
*/

#endif

