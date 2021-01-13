
/*
************************************************************************************************************************
*           INCLUDE FILES
************************************************************************************************************************
*/

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "config.h"
#include "hardware.h"
#include "serial.h"
#include "protocol.h"
#include "glcd.h"
#include "ledz.h"
#include "actuator.h"
#include "data.h"
#include "naveg.h"
#include "screen.h"
#include "cli.h"
#include "comm.h"
#include "images.h"
#include "calibration.h"
#include "uc1701.h"

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

#define UNUSED_PARAM(var)   do { (void)(var); } while (0)
#define TASK_NAME(name)     ((const char * const) (name))
#define ACTUATOR_TYPE(act)  (((button_t *)(act))->type)

#define ACTUATORS_QUEUE_SIZE    30
#define RESERVED_QUEUE_SPACES	5

/*
************************************************************************************************************************
*           LOCAL GLOBAL VARIABLES
************************************************************************************************************************
*/

static volatile xQueueHandle g_actuators_queue;
static uint8_t g_msg_buffer[WEBGUI_COMM_RX_BUFF_SIZE];

/*
************************************************************************************************************************
*           LOCAL FUNCTION PROTOTYPES
************************************************************************************************************************
*/

// local functions
static void actuators_cb(void *actuator);

// tasks
static void procotol_task(void *pvParameters);
static void displays_task(void *pvParameters);
static void actuators_task(void *pvParameters);
static void cli_task(void *pvParameters);
static void setup_task(void *pvParameters);

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
*           MAIN FUNCTION
************************************************************************************************************************
*/

int main(void)
{
    // initialize hardware
    hardware_setup();

    // this task is used to setup the system and create the other tasks
    xTaskCreate(setup_task, NULL, 256, NULL, 1, NULL);

    // start the scheduler
    vTaskStartScheduler();

    // should never reach here!
    for(;;);
}


/*
************************************************************************************************************************
*           LOCAL FUNCTIONS
************************************************************************************************************************
*/

// this callback is called from UART ISR in case of error
void serial_error(uint8_t uart_id, uint32_t error)
{
    UNUSED_PARAM(uart_id);
    UNUSED_PARAM(error);
}

// this callback is called from a ISR
static void actuators_cb(void *actuator)
{
    if (g_protocol_busy)
    {
        if (!naveg_dialog_status()) return;
    }  

    static uint8_t i, info[ACTUATORS_QUEUE_SIZE][3];

    // does a copy of actuator id and status
    uint8_t *actuator_info;
    actuator_info = info[i];
    if (++i == ACTUATORS_QUEUE_SIZE) i = 0;

    // fills the actuator info vector
    actuator_info[0] = ((button_t *)(actuator))->type;
    actuator_info[1] = ((button_t *)(actuator))->id;
    actuator_info[2] = actuator_get_status(actuator);

    //we always reserve 5 empty spaces in the queue, these are for non analog actuators
    //when these are triggered they are moved to the front of the queue for imidiate excecution. 
    //the analog actuators can not overflow the whole queue, there is always 5 places left. 
   	//when an overflow happens, samples of the analog actuator are not send, basicly lowering the resolution for us already
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

   	if ((ACTUATOR_TYPE(actuator) == BUTTON) || (ACTUATOR_TYPE(actuator) == ROTARY_ENCODER))
   	{
   		xQueueSendToFrontFromISR(g_actuators_queue, &actuator_info, &xHigherPriorityTaskWoken);
   	}
   	else 
   	{
   		if (uxQueueSpacesAvailable(g_actuators_queue) > RESERVED_QUEUE_SPACES)
   		{
    		// queue actuator info
			xQueueSendToBackFromISR(g_actuators_queue, &actuator_info, &xHigherPriorityTaskWoken);
   		}
   		else 
   			return;

   	}
   	
   	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


/*
************************************************************************************************************************
*           TASKS
************************************************************************************************************************
*/

static void procotol_task(void *pvParameters)
{
    UNUSED_PARAM(pvParameters);

    hardware_eneble_serial_interupt(WEBGUI_SERIAL);

    protocol_init();

    while (1)
    {
        uint32_t msg_size;
        g_protocol_busy = false;
        system_lock_comm_serial(g_protocol_busy);
        // blocks until receive a new message
        ringbuff_t *rb = comm_webgui_read();
        msg_size = ringbuff_read_until(rb, g_msg_buffer, WEBGUI_COMM_RX_BUFF_SIZE, 0);
        // parses the message
        if (msg_size > 0)
        {
            //if parsing messages block the actuator messages. 
            g_protocol_busy = true;
            system_lock_comm_serial(g_protocol_busy);
            msg_t msg;
            msg.sender_id = 0;
            msg.data = (char *) g_msg_buffer;
            msg.data_size = msg_size;
            protocol_parse(&msg);
        }
    }
}

static void displays_task(void *pvParameters)
{
    UNUSED_PARAM(pvParameters);

    uint8_t i = 0;
    uint32_t count = 0;

    while (1)
    {
        // update GLCD
        glcd_update(hardware_glcds(i));
        if (++i == GLCD_COUNT) i = 0;

        // I know, this is embarrassing
        if (naveg_need_update())
        {
            if (++count == 500000)
            {
                naveg_update();
                count = 0;
            }
        }
        else
        {
            count = 0;
        }

        taskYIELD();
    }
}

static void actuators_task(void *pvParameters)
{
    UNUSED_PARAM(pvParameters);

    uint8_t type, id, status;
    uint8_t *actuator_info;

    while (1)
    {
        portBASE_TYPE xStatus;

        // take the actuator from queue 
        xStatus = xQueueReceive(g_actuators_queue, &actuator_info, portMAX_DELAY);

        // check if must enter in the restore mode
        if (cli_restore(RESTORE_STATUS) == NOT_LOGGED)
            cli_restore(RESTORE_CHECK_BOOT);

        // checks if actuator has successfully taken
        if (xStatus == pdPASS && cli_restore(RESTORE_STATUS) == LOGGED_ON_SYSTEM)
        {
            type = actuator_info[0];
            id = actuator_info[1];
            status = actuator_info[2];

            // encoders
            if (type == ROTARY_ENCODER)
            {
                //we dont use the encoders in calibration mode
                if (!g_calibration_mode) 
                {
                    if (BUTTON_CLICKED(status))
                    {
                        naveg_enter(id);
                    }
                    if (BUTTON_HOLD(status))
                    {
                        id ? naveg_master_volume(id) : 
                        naveg_toggle_tool(id, id);          
                    }
                    if (BUTTON_RELEASED(status))
                    {
                        if (id && naveg_is_master_vol()) naveg_master_volume(id);
                    }
                    if (ENCODER_TURNED_CW(status))
                    {
                        naveg_inc_control(id);
                        naveg_down(id);
                    }
                    if (ENCODER_TURNED_ACW(status))
                    {
                        naveg_dec_control(id);
                        naveg_up(id);
                    }
                }
                glcd_update(hardware_glcds(id));
            }

            else if (type == POT)
            {
                if (POT_TURNED(status))
                {   
                    //check if we are in calibration mode
                    if (g_calibration_mode)
                    {
                        calibration_pot_change(id);
                    }
                    else 
                    {
                        naveg_pot_change(id);
                    }
                }
                
                glcd_update(hardware_glcds((id > 3) ? 1 : 0));
            }

            // footswitches
            else if (type == BUTTON)
            {
                if ((BUTTON_PRESSED(status)) && !(BUTTON_HOLD(status)))
                {
                    //check if we are in calibration mode
                    if (g_calibration_mode)
                    {
                        calibration_button_pressed(id);
                    }
                    else 
                    {
                        naveg_foot_change(id, 1);
                    }
                }

                if (BUTTON_RELEASED(status))
                {
                    //we dont use these button events in callibration mode
                    if (!g_calibration_mode) 
                    {
                        //trigger LED 
                        naveg_foot_change(id, 0);
                    }
                }

                if (BUTTON_HOLD(status))
                {
                   	//we dont use these button events in callibration mode
                    if (!g_calibration_mode)
                    {
                        if (id > 3) naveg_save_snapshot(id);
                    }
                }

                glcd_update(hardware_glcds((id > 1) ? 1 : 0));
            }
        }
    }
}

static void cli_task(void *pvParameters)
{
    UNUSED_PARAM(pvParameters);

    //hardware_coreboard_power(COREBOARD_INIT);

	hardware_eneble_serial_interupt(CLI_SERIAL);

    while (1)
    {
        cli_process();

        if ((uxTaskPriorityGet(NULL) > 2) && (cli_restore(RESTORE_STATUS) == LOGGED_ON_SYSTEM))
        {
            //change own priority
            vTaskPrioritySet(NULL, 2);
        }
    }
}

static void setup_task(void *pvParameters)
{
    UNUSED_PARAM(pvParameters);

    // draw start up images
    screen_image(0, mod_logo); 
    glcd_update(hardware_glcds(0));
    screen_image(1, mod_duo);
    glcd_update(hardware_glcds(1));

    // CLI initialization
    cli_init();

    // initialize the communication resources
    comm_init();

    // create the queues
    g_actuators_queue = xQueueCreate(ACTUATORS_QUEUE_SIZE, sizeof(uint8_t *));
    
    // create the tasks
    xTaskCreate(procotol_task, TASK_NAME("pro"), 512, NULL, 4, NULL);
    xTaskCreate(actuators_task, TASK_NAME("act"), 256, NULL, 3, NULL);
    xTaskCreate(cli_task, TASK_NAME("cli"), 128, NULL, 4, NULL);
    xTaskCreate(displays_task, TASK_NAME("disp"), 128, NULL, 1, NULL);

    // actuators callbacks
    uint8_t i;
    for (i = 0; i < SLOTS_COUNT; i++)
    {
        actuator_set_event(hardware_actuators(ENCODER0 + i), actuators_cb);
        actuator_enable_event(hardware_actuators(ENCODER0 + i), EV_ALL_ENCODER_EVENTS);
    }
    for (i = 0; i < FOOTSWITCHES_COUNT; i++)
    {
        actuator_set_event(hardware_actuators(FOOTSWITCH0 + i), actuators_cb);
        actuator_enable_event(hardware_actuators(FOOTSWITCH0 + i), EV_ALL_BUTTON_EVENTS);
    }
    for (i = 0; i < POTS_COUNT; i++)
    {
        actuator_set_event(hardware_actuators(POT0 + i), actuators_cb);
        actuator_enable_event(hardware_actuators(POT0 + i), EV_POT_TURNED);
    }

    // init the navigation
    naveg_init();

    // deletes itself
    vTaskDelete(NULL);
}


/*
************************************************************************************************************************
*           PROTOCOL CALLBACKS
************************************************************************************************************************
*/

void reset_queue(void)
{
    xQueueReset(g_actuators_queue);
}

/*
************************************************************************************************************************
*           ERRORS CALLBACKS
************************************************************************************************************************
*/

// TODO: better error feedback for below functions

void HardFault_Handler(void)
{
    ledz_on(hardware_leds(0), MAGENTA);
    while (1);
}

void MemManage_Handler(void)
{
    ledz_on(hardware_leds(1), MAGENTA);
    while (1);
}

void BusFault_Handler(void)
{
    ledz_on(hardware_leds(2), MAGENTA);
    while (1);
}

void UsageFault_Handler(void)
{
    ledz_on(hardware_leds(3), MAGENTA);
    while (1);
}

void vApplicationMallocFailedHook(void)
{
    ledz_on(hardware_leds(4), MAGENTA);
    while (1);
}

void vApplicationIdleHook(void)
{
    //should not reach here, however this should also not include the while(1)
    ledz_on(hardware_leds(6), MAGENTA);
    while (1);
}

void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
    UNUSED_PARAM(pxTask);
    glcd_t *glcd0 =  hardware_glcds(0);
    glcd_clear(glcd0, GLCD_WHITE);
    glcd_text(glcd0, 0, 0, "stack overflow", NULL, GLCD_BLACK);
    glcd_text(glcd0, 0, 10, (const char *) pcTaskName, NULL, GLCD_BLACK);
    glcd_update(glcd0);
    ledz_on(hardware_leds(5), CYAN);
    while (1);
}
