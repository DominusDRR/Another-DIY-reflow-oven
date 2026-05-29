/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    appled.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "appled.h"
#include "definitions.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APPLED_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APPLED_DATA appledData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/
extern uint32_t abs_diff_uint32(uint32_t a, uint32_t b);

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void changeLEDFlashingSpeed (bool fast);
void startStopLEDFlashing(bool start);

/* TODO:  Add any necessary local functions.
*/
void changeLEDFlashingSpeed (bool fast)
{
    if (fast)
    {
        appledData.time = _100ms;
    }
    else
    {
        appledData.time = _500ms;
    }
}

void startStopLEDFlashing(bool start)
{
    if (start)
    {
        changeLEDFlashingSpeed(false); 
        appledData.state = APPLED_STATE_INIT;
    }
    else
    {
        LED_Set();
        appledData.state = APPLED_LED_IDLE;
    }
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPLED_Initialize ( void )

  Remarks:
    See prototype in appled.h.
 */

void APPLED_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appledData.state = APPLED_STATE_INIT;
    RTC_Timer32Start(); // Start RTC
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APPLED_Tasks ( void )

  Remarks:
    See prototype in appled.h.
 */

void APPLED_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appledData.state )
    {
        /* Application's initial state. */
        case APPLED_STATE_INIT:
        {
            appledData.adelay = RTC_Timer32CounterGet();
            appledData.time = _500ms; //By default, the LED flashes at 500ms
            appledData.state = APPLED_LED_STATUS_BLINKING;
            break;
        }
        case APPLED_LED_STATUS_BLINKING:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(), appledData.adelay) > appledData.time)
            {
                appledData.adelay = RTC_Timer32CounterGet();
                LED_Toggle();
            }
            break;
        }
        /* TODO: implement your application state machine.*/
        case APPLED_LED_IDLE: break;
        /* The default state should never be executed. */
        default: break; /* TODO: Handle error in application's state machine. */
    }
}


/*******************************************************************************
 End of File
 */
