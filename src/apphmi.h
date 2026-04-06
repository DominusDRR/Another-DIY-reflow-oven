/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    apphmi.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APPHMI_Initialize" and "APPHMI_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APPHMI_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APPHMI_H
#define _APPHMI_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
#define NO_ERROR_HMI            0x00
#define ERROR_SIMPLE_PREHEAT    0x01
#define ERROR_COMPLEX_PREHEAT   0x02    
#define ERROR_FLUX              0x03    
#define ERROR_REFLOW            0x04    
// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APPHMI_STATE_INIT=0,
    APPHMI_STATE_WAIT_DELAY_FOR_LOGO,
    /* TODO: Define states used by the application state machine. */
    APPHMI_STATE_CLEAR_LCD,        
    APPHMI_STATE_DRAW_HOME_MENU,   
    APPHMI_STATE_UPDATE_HOME_MENU,
    APPHMI_STATE_WAIT_USER_SELECTION_HOME_MENU,
    APPHMI_STATE_REQUEST_START_TEMPERATURE_READING,
    APPHMI_STATE_WAIT_TEMPERATURE_READING,
    APPHMI_STATE_START_WRITE_CURRENT_TEMPERATURE,        
    APPHMI_STATE_DISPLAY_THERMOCOUPLE_ERRORS,
    APPHMI_STATE_GET_THERMOCUPLE_TEMPERATUE,   
    APPHMI_STATE_GET_INTERNAL_TEMPERATUE,        
    APPHMI_STATE_WRITE_CURRENT_TEMPERATURE,
    APPHMI_STATE_WRITE_INTERNAL_TEMPERATURE,
    APPHMI_STATE_UPDATE_TEMPERATURES,
    APPHMI_STATE_REQUEST_START_SET_PARAMETERS,
    APPHMI_STATE_WAI_USER_SELECTION_PARAMETERS_MENU,
    APPHMI_STATE_START_EDIT_PARAMETER,
    APPHMI_STATE_SET_NAME_PARAMETER,
    APPHMI_STATE_SET_VALUE_PARAMETER,        
    APPHMI_STATE_UPDATE_LCD_VALUE_PARAMETER,
    APPHMI_STATE_WAIT_USER_CHANGE_PARAMETERS,
    APPHMI_STATE_WAIT_ESCALATED_MESSAGE_CLEAN,
    APPHMI_STATE_DISPLAY_PARAMETER_CHANGED_MESSAGE,
    APPHMI_STATE_WAIT_DELAY_MESSAGE_PARAMETER_CHANGED,
    APPHMI_STATE_VERIFY_PARAMETERS_HAVE_LOGICAL_VALUES,
    APPHMI_STATE_GRAPHICALLY_DISPLAY_CHANGED_PARAMETERS,
    APPHMI_STATE_WAIT_USER_PRESS_ANY_BUTTON,
    APPHMI_STATE_DRAW_MENU_APPROVE_EDIT_CANCEL_MODIFIED_PARAMETERS,
    APPHMI_STATE_WAIT_USER_SELECTION_MENU_APPROVE_EDIT_CANCEL_MODIFIED_PARAMETERS,
    APPHMI_STATE_WAIT_EERAM_TASK_IDLE,
    APPHMI_STATE_WAIT_EERAM_UPDATE_COMPLETED,
    APPHMI_STATE_WAIT_UPDATED_PARAMETERS_MESSAGE,
    APPHMI_STATE_GRAPH_X_AND_Y_AXES,    
    APPHMI_STATE_START_SIMPLE_PREHEATING_PROCESS,
    APPHMI_STATE_START_PREHEATING_PROCESS,        
    APPHMI_STATE_FLUX_ACTIVATION,
    APPHMI_STATE_REFLOW,   
    APPHMI_STATE_COOLING,        
    APPHMI_STATE_ERROR,
    APPHMI_STATE_SHOW_ERROR        
} APPHMI_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APPHMI_STATES state;
    /* TODO: Define any additional data used by the application. */
    APPHMI_STATES stateToReturn;
    uint8_t messagePointer;
    uint8_t selectedOption;
    uint8_t typeErrorThermocuple;
    uint32_t adelay;
    float thermocoupleTemp;
    float internalTemp;
    char bufferForStrings[40];
    bool doNotClearLCD;
    uint8_t firstVisibleIndex;
    uint16_t backupParameterTemperatureTime;
    float backupPIDConstant;
    bool parametersBeenChanged;
    uint32_t x;
    uint32_t y;
    uint32_t stepsOfTheTotalProcessingTime;
    uint32_t estimatedTimeProcessWouldTake;
    uint32_t timeProcessTakes;
    uint8_t typeError;
    float initialTemperature;
    float ramp;
    float elapsed_s;
    uint32_t adelayGraph; //If two delays are needed, one for processing and one for graphing, the latter is used for the graphs.
    bool doNotRecalculateSetPoint;
} APPHMI_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPHMI_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APPHMI_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APPHMI_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APPHMI_Initialize ( void );


/*******************************************************************************
  Function:
    void APPHMI_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APPHMI_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APPHMI_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APPHMI_H */

/*******************************************************************************
 End of File
 */

