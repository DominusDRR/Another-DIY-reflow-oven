/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    apppid.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APPPID_Initialize" and "APPPID_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APPPID_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APPPID_H
#define _APPPID_H

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
    APPPID_STATE_IDE=0,
    APPPID_STATE_REQUEST_START_TEMPERATURE_READING,        
    /* TODO: Define states used by the application state machine. */
    APPPID_STATE_INITIALIZE_PID,        
    APPPID_STATE_WAIT_TEMPERATURE_READING,
    APPPID_STATE_TURN_ON_WINDOW,
    APPPID_STATE_TURN_OFF_WINDOW,
    APPPID_STATE_WAIT_TURN_OFF_WINDOW,
} APPPID_STATES;


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
    APPPID_STATES state;
    /* TODO: Define any additional data used by the application. */
    uint32_t adelay;
    uint32_t windowMs;      // ventana 1 s (usa 1000..3000 según SSR)
    float integral;
    float prevError;
    float   controlOutput;       // 0.0..1.0
    float   tempMeasurement;     // ya la tienes
    float   setpoint;            // ya la tienes
    uint32_t lastWindowStartMs; // timestamp inicio ventana (opcional para info)
    uint32_t lastMeasurementMs; // opcional para debug/timestamps
    uint32_t onTimeMs;
    uint32_t onTimeMsCycles;
    uint32_t offTimeMs;
} APPPID_DATA;

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
    void APPPID_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APPPID_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APPPID_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APPPID_Initialize ( void );


/*******************************************************************************
  Function:
    void APPPID_Tasks ( void )

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
    APPPID_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APPPID_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APPPID_H */

/*******************************************************************************
 End of File
 */

