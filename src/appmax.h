/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    appmax.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APPMAX_Initialize" and "APPMAX_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APPMAX_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APPMAX_H
#define _APPMAX_H

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
#include "driver/spi/drv_spi.h"
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
#define  SPI0_NO_ERROR       0x00
#define  SPI0_OPEN_ERROR     0x01
#define  SPI0_SET_INSTANCE   0x02    
#define  SPI0_TRANSMISSION   0x03       
#define  SPI0_HANLDER_ERROR  0x04  
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
    APPMAX_STATE_INIT=0,
    APPMAX_STATE_DRIVER_SETUP,
    /* TODO: Define states used by the application state machine. */
    APPMAX_STATE_IDLE,
    APPMAX_STATE_START_TEMPERATURE_READING,        
    APPMAX_START_TRANSMISSION,
    APPMAX_TRANSMISSION,
    APPMAX_WAIT_FOR_RECEPTION_END, 
    APPMAX_GET_AVERAGE_TEMPERATURE,        
    APPMAX_STATE_ERROR,
    APPMAX_STATE_DELAY_TO_REOPEN_SPI
} APPMAX_STATES;


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
    APPMAX_STATES state;
    /* TODO: Define any additional data used by the application. */
    DRV_HANDLE drvSPIHandle;
    DRV_SPI_TRANSFER_HANDLE transferHandle;
    DRV_SPI_TRANSFER_SETUP  setup; 
    uint8_t typeOfError;
    uint8_t bufferRX[32]; //Buffer for transmission
    uint8_t numberBytesRecep; //number of bytes to be rx
    bool isTransferComplete;
    uint32_t adelay;
    uint8_t typeThermocoupleError;
    uint8_t averagePointer;
    float averageTemp;
} APPMAX_DATA;

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
    void APPMAX_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APPMAX_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APPMAX_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APPMAX_Initialize ( void );


/*******************************************************************************
  Function:
    void APPMAX_Tasks ( void )

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
    APPMAX_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APPMAX_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APPMAX_H */

/*******************************************************************************
 End of File
 */

