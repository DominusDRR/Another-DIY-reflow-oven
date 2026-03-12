/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    apppid.c

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

#include "apppid.h"
#include "definitions.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define MAX_SAFE_TEMP 265.0f

#define DT_CLAMP_MAX_S 5.0f // if dt > 5s, we limit it to this (avoids large jumps)
// PID limits
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APPPID_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APPPID_DATA apppidData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/
extern uint32_t abs_diff_uint32(uint32_t a, uint32_t b);
extern uint8_t getPressedBtn(void);
extern float getThermocoupleAverageTemp (void);
extern float getKp(void);
extern float getKi(void);
extern float getKd(void);
extern bool IsMaxTaskIdle (void);
extern void startTemperatureReading(void);
// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/
bool IsPIDTaskIdle(void);
void initializeTaskPID(float setPoint);
float getLastMeasurement(void);
void stopTaskPID(void);
void setReferenceTaskPID(float setPoint);

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

bool IsPIDTaskIdle(void)
{
    if (APPPID_STATE_IDE == apppidData.state)
    {
        return true;
    }
    return false;
}

void initializeTaskPID(float setPoint)
{
    apppidData.setpoint = setPoint;
    apppidData.state = APPPID_STATE_INITIALIZE_PID;
}

float getLastMeasurement(void)
{
    return apppidData.tempMeasurement;
}

void stopTaskPID(void)
{
    APPPID_Initialize();
}
void setReferenceTaskPID(float setPoint)
{
    if (APPPID_STATE_IDE != apppidData.state && apppidData.setpoint != setPoint)
    {
        apppidData.setpoint = setPoint;
    }
}
/*******************************************************************************
  Function:
    void APPPID_Initialize ( void )

  Remarks:
    See prototype in apppid.h.
 */

void APPPID_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    apppidData.state = APPPID_STATE_IDE;
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    SSR_Clear();
    apppidData.tempMeasurement = 10.0f;
}
/******************************************************************************
  Function:
    void APPPID_Tasks ( void )

  Remarks:
    See prototype in apppid.h.
 */

void APPPID_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( apppidData.state )
    {
        /* Application's initial state. */
        case APPPID_STATE_IDE: break; //It is in standby mode until a decision is made to use this task.
        case APPPID_STATE_INITIALIZE_PID:
        {
            apppidData.windowMs = 1000;      // ventana 1 s (usa 1000..3000 según SSR)
            apppidData.integral = 0.0f;
            apppidData.prevError = 0.0f;
            apppidData.state = APPPID_STATE_REQUEST_START_TEMPERATURE_READING;
            break;
        }
        /* TODO: implement your application state machine.*/
        case APPPID_STATE_REQUEST_START_TEMPERATURE_READING:
        {
            if (IsMaxTaskIdle())
            {
                startTemperatureReading();
                apppidData.state = APPPID_STATE_WAIT_TEMPERATURE_READING;
            }
            break;
        }
        case APPPID_STATE_WAIT_TEMPERATURE_READING:
        {
            if (IsMaxTaskIdle())
            {
                apppidData.tempMeasurement = getThermocoupleAverageTemp();
                if (apppidData.tempMeasurement >= MAX_SAFE_TEMP)
                {
                    SSR_Clear();
                    apppidData.state = APPPID_STATE_IDE;// aquí podrías loggear o setear una alarma
                    return;
                }
                //Calculo PI (sin D) ---
                float error = apppidData.setpoint - apppidData.tempMeasurement;
                float P = getKp() * error;
                float dt_s = (float)apppidData.windowMs / 1000.0f;
                if (dt_s <= 0.0f) 
                {
                    dt_s = 1.0f; // protección (no debería pasar)
                }
                // Anti-windup simple:
                // Probaremos integrar tentativamente y permitiremos la integración
                // sólo si no empuja la salida más allá de los límites en la misma dirección del error.
                float tentativeIntegral = apppidData.integral + getKi() * error * dt_s;
                float tentativeU = P + tentativeIntegral;
                if (!((tentativeU > 1.0f && error > 0.0f) || (tentativeU < 0.0f && error < 0.0f) ))
                {
                    apppidData.integral = tentativeIntegral;
                }
                // salida continua  (0..1)
                float u = P + apppidData.integral;
                if (u > 1.0f) 
                {
                    u = 1.0f;
                }
                if (u < 0.0f) 
                {
                    u = 0.0f;
                }
                //Time-proportional dentro de la ventana (bloqueante) ---
                // Calcula tiempo ON dentro de la ventana y ejecútalo directamente.
                apppidData.onTimeMs = (uint32_t)( u * (float)apppidData.windowMs );//onTimeMs se necesita en otro estado, por eso existe onTimeMsCycles para no alterarlo
                if (apppidData.onTimeMs > apppidData.windowMs) 
                {
                    apppidData.onTimeMs = apppidData.windowMs;
                }
                if (apppidData.onTimeMs > 0)
                {
                    SSR_Set();
                    apppidData.onTimeMsCycles = _1ms * apppidData.onTimeMs;// convierte el tiempo a ciclos verdadero 
                    apppidData.adelay = RTC_Timer32CounterGet();
                    apppidData.state = APPPID_STATE_TURN_ON_WINDOW;
                }
                else
                {
                    apppidData.state = APPPID_STATE_TURN_OFF_WINDOW;
                }
            }
            break;
        }
        case APPPID_STATE_TURN_ON_WINDOW:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(),apppidData.adelay) >= apppidData.onTimeMsCycles) 
            {
                apppidData.state = APPPID_STATE_TURN_OFF_WINDOW;
            }
            break;
        }
        case APPPID_STATE_TURN_OFF_WINDOW:
        {
            // Apaga por el resto de la ventana
            apppidData.offTimeMs = apppidData.windowMs - apppidData.onTimeMs;
            if (apppidData.offTimeMs > 0)
            {
                SSR_Clear();
                apppidData.offTimeMs *= _1ms;
                apppidData.adelay = RTC_Timer32CounterGet();
                apppidData.state = APPPID_STATE_WAIT_TURN_OFF_WINDOW;
            }
            else
            {
                apppidData.state = APPPID_STATE_REQUEST_START_TEMPERATURE_READING;
            }
            break;
        }
        case APPPID_STATE_WAIT_TURN_OFF_WINDOW:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(),apppidData.adelay) >= apppidData.offTimeMs) 
            {
                apppidData.state = APPPID_STATE_REQUEST_START_TEMPERATURE_READING;;
            }
            break;
        }
        /* The default state should never be executed. */
        default: break; /* TODO: Handle error in application's state machine. */
    }
}


/*******************************************************************************
 End of File
 */
