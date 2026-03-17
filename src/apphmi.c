/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    apphmi.c

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

#include "apphmi.h"
#include "definitions.h"
#include <stdio.h>
#include <string.h>
//#include <stdint.h>
//#include <stdbool.h>
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define TOTAL_ITEMS          (sizeof(msgsParametersMenu)/sizeof(*msgsParametersMenu)) // 8 messages
#define VISIBLE_ROWS         6
#define TOLERANCIA_SEC 10.0f    //10 seconds
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APPHMI_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APPHMI_DATA apphmiData;

const unsigned char *msgsHomeMenu[] = 
{
    (unsigned char *)" CHOOSE OPTION", // max 14 chars
    (unsigned char *)" ", 
    (unsigned char *)"Start process",
    (unsigned char *)"Set Parameters",
    (unsigned char *)"Current Temp"
};

const unsigned char *msgsParametersMenu[] = 
{
    (unsigned char *)"1 Temp preheat", // max 14 chars
    (unsigned char *)"2 Time preheat", 
    (unsigned char *)"3 Temp flux act",
    (unsigned char *)"4 Time flux act",
    (unsigned char *)"5 Temp reflow",
    (unsigned char *)"6 Time reflow",
    (unsigned char *)"7 Temp cooling",
    (unsigned char *)"8 Time cooling",
    (unsigned char *)"9 Kp constant ",
    (unsigned char *)"10 Ki constant",
    (unsigned char *)"11 Kd constant"
};

const unsigned char *msgsMenuParametersChanged[] = 
{
    (unsigned char *)" CHOOSE OPTION", // max 14 chars
    (unsigned char *)" ", 
    (unsigned char *)"Edit parameter", 
    (unsigned char *)"View graph",
    (unsigned char *)"Cancel changes",
    (unsigned char *)"Accept changes"
};

//Times and temperatures reached by my oven
const float Tdata[] = { 27.0f, 50.0f, 100.0f, 150.0f }; // time en seconds
const float tdata[] = {  0.0f, 50.0f, 100.0f, 180.0f }; // Temp
int N = 4; //Four temperature samples
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
/* TODO:  Add any necessary callback functions.
*/
extern bool IsGLCDTaskIdle (void);
extern void LCDClear(void);
extern void LCDUpdate(void);
extern void LCDLine (int32_t x1, int32_t y1, int32_t x2, int32_t y2);
extern void LCDStr(uint8_t row, const uint8_t *dataPtr, bool inv, bool updateLCD);
extern void LCDChrXY_Scaled(uint8_t x, uint8_t y, const uint8_t *dataPtr, uint8_t scale, bool updateLCD);
extern void LCDStr_Scaled_Clear(uint8_t x, uint8_t y, uint8_t len, uint8_t scale, bool updateLCD);
extern void LCDTinyStr(uint8_t x, uint8_t y, const char *dataPtr, bool updateLCD);
extern void drawInitialLogo(void);

extern uint32_t abs_diff_uint32(uint32_t a, uint32_t b);
extern uint8_t getPressedBtn(void);

extern uint8_t getTypeThermocoupleError(void);
extern float getThermocoupleAverageTemp (void);
extern float getInternalTemp(void);
extern bool IsMaxTaskIdle (void);
extern void startTemperatureReading(void);

extern uint16_t returnParameterTempTime(uint8_t index);
extern float returnConstantsPID(uint8_t index);
extern uint16_t increaseParameterTempTime(uint8_t index);
extern uint16_t decreaseParameterTempTime(uint8_t index); 
extern float increaseConstantsPID(uint8_t index);
extern float decreaseConstantsPID(uint8_t index);
extern void setParameter(uint8_t index, uint16_t value);
extern void setConstantsPID(uint8_t index, float constantPID);
extern void upDatetEERAMvalues(void);
extern bool IsEERAMMTaskIdle (void);
extern void initializeWriteEERAM(void);
extern bool IsPIDTaskIdle(void);
extern void initializeTaskPID(float setPoint);
extern float getLastMeasurement(void);
extern float getPreHeatTime(void);
extern float getPreHeatTemp(void);
extern uint32_t getStepsTotalProcessingTime(void);
extern void stopTaskPID(void);
extern void setReferenceTaskPID(float setPoint);
extern float getFluxTime(void);
extern float getFluxTemp(void);
//bool IsONFFTaskIdle(void);
//void initializeTaskONOFF(void);
//float getLastMeasurementONOFF(void);
// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
void returnHomeMenu(void);
void goParametersMenu(void);
char isTemperatureOrTime(uint8_t index);
float predictTimeForT(float Treq);
void updateCoordinatesY(float temp);
void plotTemperatureLine(void);
void displayTemperatureOnScreen(void);
/* TODO:  Add any necessary local functions.
*/
void returnHomeMenu(void)
{
    apphmiData.typeErrorThermocuple = 0xFF; 
    apphmiData.selectedOption = 0x02;
    apphmiData.doNotClearLCD = false; //This flag can be set to true, so it is cleared for the proxy.
    apphmiData.state = APPHMI_STATE_CLEAR_LCD;
}
void goParametersMenu(void)
{
    LCDClear();
    apphmiData.messagePointer = 0x00;
    apphmiData.selectedOption = 0x00;
    apphmiData.firstVisibleIndex = 0x00;
    apphmiData.state =  APPHMI_STATE_REQUEST_START_SET_PARAMETERS;
}
char isTemperatureOrTime(uint8_t index)
{
    return ((index & 1) == 0) ? 'C' : 's';
}
// Returns estimated time (seconds) until T_req is reached
// If T_req <= 27, it returns 0.
// If T_req > 150, it extrapolates using the last segment.
// returns estimated times to reach T (using piecewise interpolation)
float predictTimeForT(float Treq)
{
    if (Treq <= Tdata[0]) 
    {
        return tdata[0];
    }
    for (int i = 0; i < N-1; ++i) //4 -1
    {
        if (Treq <= Tdata[i+1])
        {
            float Ta = Tdata[i], Tb = Tdata[i+1];
            float ta = tdata[i], tb = tdata[i+1];
            float frac = (Treq - Ta) / (Tb - Ta);
            return ta + frac * (tb - ta);
        }
    }
    // T_req > max medido: extrapolacion lineal del ultimo tramo
    {
        int i = N-2;
        float Ta = Tdata[i], Tb = Tdata[i+1];
        float ta = tdata[i], tb = tdata[i+1];
        float slope = (tb - ta) / (Tb - Ta); // s por grado
        return ta + slope * (Treq - Ta);
    }
}

void updateCoordinatesY(float temp)
{
    apphmiData.y = (uint32_t)(1410.0f/29.0f - temp*47.0f/290.0f);  
}

void plotTemperatureLine(void)
{
    uint32_t xPrevious = apphmiData.x;
    uint32_t yPrevious = apphmiData.y;
    apphmiData.x++;
    if (apphmiData.x < 83)
    {
        updateCoordinatesY(getLastMeasurement());//apphmiData.y = (uint32_t)(1410.0f/29.0f - getLastMeasurement()*47.0f/290.0f); 
        if (apphmiData.y > 47) //Avoid writing beyond 47
        {
            apphmiData.y = 47;
        }
        LCDLine (xPrevious,yPrevious,apphmiData.x,apphmiData.y);
        apphmiData.adelayGraph = RTC_Timer32CounterGet(); 
    }
}

void displayTemperatureOnScreen(void)
{
    snprintf(apphmiData.bufferForStrings, sizeof(apphmiData.bufferForStrings),"Temp %.1f C",getLastMeasurement());
    LCDTinyStr(35,8,apphmiData.bufferForStrings,true);
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPHMI_Initialize ( void )

  Remarks:
    See prototype in apphmi.h.
 */

void APPHMI_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    apphmiData.state = APPHMI_STATE_INIT;
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
    apphmiData.typeErrorThermocuple = 0xFF; //This way I force the thermocouple status to update for the first time.
    apphmiData.doNotClearLCD = false;
}


/******************************************************************************
  Function:
    void APPHMI_Tasks ( void )

  Remarks:
    See prototype in apphmi.h.
 */

void APPHMI_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( apphmiData.state )
    {
        /* Application's initial state. */
        case APPHMI_STATE_INIT:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                drawInitialLogo();
                apphmiData.adelay = RTC_Timer32CounterGet();
                apphmiData.state = APPHMI_STATE_WAIT_DELAY_FOR_LOGO;
            }
            break;
        }
        case APPHMI_STATE_WAIT_DELAY_FOR_LOGO:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay) > _2000ms)
            {
                apphmiData.selectedOption = 0x02;
                apphmiData.state = APPHMI_STATE_CLEAR_LCD;
            }
            break;
        }
        case APPHMI_STATE_CLEAR_LCD:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                LCDClear();
                apphmiData.messagePointer = 0x00;
                apphmiData.state = APPHMI_STATE_DRAW_HOME_MENU;
            }
            break;
        }
        case APPHMI_STATE_DRAW_HOME_MENU:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                if (apphmiData.messagePointer < 0x05)
                {
                    LCDStr(apphmiData.messagePointer, msgsHomeMenu[apphmiData.messagePointer], (apphmiData.selectedOption == apphmiData.messagePointer), false);
                    apphmiData.messagePointer++;
                }
                else
                {
                    apphmiData.stateToReturn = APPHMI_STATE_WAIT_USER_SELECTION_HOME_MENU;
                    apphmiData.state = APPHMI_STATE_UPDATE_HOME_MENU;
                }
            }
            break;
        }
        /* TODO: implement your application state machine.*/
        case APPHMI_STATE_UPDATE_HOME_MENU:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                LCDUpdate();
                apphmiData.state = apphmiData.stateToReturn;
            }
            break;
        }
        case APPHMI_STATE_WAIT_USER_SELECTION_HOME_MENU:
        {
            uint8_t button = getPressedBtn();
            if (NO_BUTTON_PRESSED != button && IsGLCDTaskIdle())
            {
                switch (button)
                {
                    case OK_BUTTON_PRESSED:
                    {
                        if (0x04 == apphmiData.selectedOption)
                        {
                            apphmiData.adelay = RTC_Timer32CounterGet();
                            apphmiData.state = APPHMI_STATE_REQUEST_START_TEMPERATURE_READING;
                        }
                        else if (0x03 == apphmiData.selectedOption) // To parameters Menu
                        {
                            apphmiData.parametersBeenChanged = false;
                            goParametersMenu();
                        }
                        else if (0x02 == apphmiData.selectedOption)
                        {
                            apphmiData.messagePointer = 0x00; 
                            apphmiData.state = APPHMI_STATE_GRAPH_X_AND_Y_AXES;
                        }
                        break;
                    }
                    case UP_BUTTON_PRESSED:
                    case DOWN_BUTTON_PRESSED:
                    {
                        if (button == UP_BUTTON_PRESSED)
                        {
                            apphmiData.selectedOption--;
                            if (apphmiData.selectedOption < 2)
                            {
                                apphmiData.selectedOption = 4;
                            }
                        }
                        else // DOWN
                        {
                            apphmiData.selectedOption++;
                            if (apphmiData.selectedOption > 4)
                            {
                                apphmiData.selectedOption = 2;
                            }
                        }
                        apphmiData.state = APPHMI_STATE_CLEAR_LCD;
                        break;
                    }
                    default: break; // case ESC
                }
            }
            break;
        }
        /*** Current temperature display ***/
        case APPHMI_STATE_REQUEST_START_TEMPERATURE_READING:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay) > _500ms)
            {
                if (IsMaxTaskIdle())
                {
                    startTemperatureReading();
                    apphmiData.state = APPHMI_STATE_WAIT_TEMPERATURE_READING;
                    return; //Avoid the if below if it happens
                }
                else
                {
                    apphmiData.adelay = RTC_Timer32CounterGet();
                }
            }
            if (ESC_BUTTON_PRESSED == getPressedBtn()) // Return to the HOME menu
            {
                returnHomeMenu();
            }
            break;
        }
        case APPHMI_STATE_WAIT_TEMPERATURE_READING:
        {
            if (IsMaxTaskIdle())
            {
                apphmiData.state = APPHMI_STATE_START_WRITE_CURRENT_TEMPERATURE;
            }
            break;
        }
        case APPHMI_STATE_START_WRITE_CURRENT_TEMPERATURE:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                if (apphmiData.typeErrorThermocuple != getTypeThermocoupleError())
                {
                    apphmiData.typeErrorThermocuple = getTypeThermocoupleError();
                    if (apphmiData.doNotClearLCD)
                    {
                        apphmiData.doNotClearLCD = false;
                    }
                    else
                    {
                        LCDClear();
                    }
                    apphmiData.state = APPHMI_STATE_DISPLAY_THERMOCOUPLE_ERRORS;
                    return; //So that the if below is not executed
                }
                apphmiData.adelay = RTC_Timer32CounterGet();
                apphmiData.state = APPHMI_STATE_REQUEST_START_TEMPERATURE_READING;
            }
            break;
        }
        case APPHMI_STATE_DISPLAY_THERMOCOUPLE_ERRORS:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                switch (apphmiData.typeErrorThermocuple)
                {
                    case OPEN_THERMOCOUPLE:     LCDStr(0x02,(unsigned char *)"Thermocouple  open",false, true);     break;
                    case SHORT_CIRCUIT_TO_GND:  LCDStr(0x02,(unsigned char *)"Thermocouple  to GNC",false, true);   break;
                    case SHORT_CIRCUIT_TO_VCC:  LCDStr(0x02,(unsigned char *)"Thermocouple  to Vcc",false, true);   break;
                    default:                    apphmiData.state = APPHMI_STATE_GET_THERMOCUPLE_TEMPERATUE;         return;
                }
                apphmiData.adelay = RTC_Timer32CounterGet();
                apphmiData.state = APPHMI_STATE_REQUEST_START_TEMPERATURE_READING;
            }
            break;
        }
        case APPHMI_STATE_GET_THERMOCUPLE_TEMPERATUE://Split into several states, so that the task does not completely take over the CPU
        {
            if (IsMaxTaskIdle())
            {
                apphmiData.thermocoupleTemp = getThermocoupleAverageTemp();//getThermocoupleTemp();
                apphmiData.state = APPHMI_STATE_GET_INTERNAL_TEMPERATUE;
            }
            break;
        }
        case APPHMI_STATE_GET_INTERNAL_TEMPERATUE:
        {
            apphmiData.internalTemp = getInternalTemp();
            apphmiData.state = APPHMI_STATE_WRITE_CURRENT_TEMPERATURE;
            break;
        }
        case APPHMI_STATE_WRITE_CURRENT_TEMPERATURE:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                snprintf(apphmiData.bufferForStrings, sizeof(apphmiData.bufferForStrings),"Thermocuple   Temp %.1f C", apphmiData.thermocoupleTemp);
                LCDStr(0, (uint8_t*)apphmiData.bufferForStrings, false, false);
                apphmiData.state = APPHMI_STATE_WRITE_INTERNAL_TEMPERATURE;
            }
            break;
        }
        case APPHMI_STATE_WRITE_INTERNAL_TEMPERATURE:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                //static char buf2[30];
                snprintf(apphmiData.bufferForStrings, sizeof(apphmiData.bufferForStrings),"Internal      Temp %.1f C", apphmiData.internalTemp);
                LCDStr(3, (uint8_t*)apphmiData.bufferForStrings, false, false);
                apphmiData.state = APPHMI_STATE_UPDATE_TEMPERATURES;
            }
            break;
        }
        case APPHMI_STATE_UPDATE_TEMPERATURES:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                LCDUpdate();
                apphmiData.adelay = RTC_Timer32CounterGet();
                apphmiData.typeErrorThermocuple = 0xFF; //I do this so that another temperature
                apphmiData.doNotClearLCD = true; //I prevent the screen from being cleaned, it produces a flickering and does not look natural.
                apphmiData.state = APPHMI_STATE_REQUEST_START_TEMPERATURE_READING; 
            }
            break;
        }
        /*****************************************************************************************************************/
        /** MENU SET PARAMETERS **/
        case APPHMI_STATE_REQUEST_START_SET_PARAMETERS:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                if (apphmiData.messagePointer < VISIBLE_ROWS)
                {
                    uint8_t idx = apphmiData.firstVisibleIndex + apphmiData.messagePointer;
                    if (idx < TOTAL_ITEMS) //VISIBLE_ROWS = 6
                    {
                        LCDStr(apphmiData.messagePointer, msgsParametersMenu[idx],(idx == apphmiData.selectedOption), false);
                    }
                    apphmiData.messagePointer++;
                }
                else
                {
                    apphmiData.messagePointer = 0x00;
                    apphmiData.stateToReturn = APPHMI_STATE_WAI_USER_SELECTION_PARAMETERS_MENU;
                    apphmiData.state = APPHMI_STATE_UPDATE_HOME_MENU;
                }
            }
            break;
        }
        case APPHMI_STATE_WAI_USER_SELECTION_PARAMETERS_MENU:
        {
            uint8_t button = getPressedBtn();
            if (NO_BUTTON_PRESSED != button)
            {
                switch (button)
                {
                    case OK_BUTTON_PRESSED:
                    {
                        apphmiData.state = APPHMI_STATE_START_EDIT_PARAMETER;
                        break;
                    }
                    case UP_BUTTON_PRESSED:
                    {   
                        if (apphmiData.selectedOption > 0)
                        {
                            apphmiData.selectedOption--;
                            if (apphmiData.selectedOption < apphmiData.firstVisibleIndex)
                            {
                                apphmiData.firstVisibleIndex = apphmiData.selectedOption;
                            }
                            apphmiData.state = APPHMI_STATE_REQUEST_START_SET_PARAMETERS;
                        }       
                        break;
                    }
                    case DOWN_BUTTON_PRESSED:
                    {
                        if (apphmiData.selectedOption < (TOTAL_ITEMS - 1))
                        {
                            apphmiData.selectedOption++;
                            if (apphmiData.selectedOption >= apphmiData.firstVisibleIndex + VISIBLE_ROWS)// 0 +6
                            {
                                apphmiData.firstVisibleIndex = apphmiData.selectedOption - (VISIBLE_ROWS - 1); //= 6 - 6 + 1 = 1
                            }
                            apphmiData.state = APPHMI_STATE_REQUEST_START_SET_PARAMETERS;
                        }
                        break;
                    }
                    default: 
                    {
                        if (apphmiData.parametersBeenChanged)
                        {
                            apphmiData.messagePointer = 0x00; //I'm going to use messagePointer to do multiple things in a single state machine.
                            apphmiData.state = APPHMI_STATE_VERIFY_PARAMETERS_HAVE_LOGICAL_VALUES;//APPHMI_STATE_GRAPHICALLY_DISPLAY_CHANGED_PARAMETERS;
                            return;
                        }
                        returnHomeMenu();
                    }
                }
            }
            break;
        }
        case APPHMI_STATE_START_EDIT_PARAMETER:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                LCDClear();
                apphmiData.state = APPHMI_STATE_SET_NAME_PARAMETER;
            }
            break;
        }
        case APPHMI_STATE_SET_NAME_PARAMETER:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                //LCDTinyStr(0, 27,"Max: 32.8C", false);
                strncpy(apphmiData.bufferForStrings, (const char *)msgsParametersMenu[apphmiData.selectedOption], 14);
                apphmiData.bufferForStrings[14] = '\0';  // aseguramos que esté terminado en null
                LCDStr(0,(uint8_t *)apphmiData.bufferForStrings,false, false);
                apphmiData.state = APPHMI_STATE_SET_VALUE_PARAMETER;
            }
            break;
        }
        case APPHMI_STATE_SET_VALUE_PARAMETER:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                if (apphmiData.selectedOption < 0x08)
                {
                    apphmiData.backupParameterTemperatureTime = returnParameterTempTime(apphmiData.selectedOption);
                    sprintf(apphmiData.bufferForStrings,"%u %c",apphmiData.backupParameterTemperatureTime,isTemperatureOrTime(apphmiData.selectedOption));
                }
                else
                {
                    apphmiData.backupPIDConstant = returnConstantsPID(apphmiData.selectedOption);
                    sprintf(apphmiData.bufferForStrings, "%.2f", apphmiData.backupPIDConstant);
                }
                LCDChrXY_Scaled(5,15,(uint8_t *)apphmiData.bufferForStrings,2,false);
                apphmiData.state = APPHMI_STATE_UPDATE_LCD_VALUE_PARAMETER;
            }
            break;
        }
        case APPHMI_STATE_UPDATE_LCD_VALUE_PARAMETER:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                LCDUpdate();
                apphmiData.state = APPHMI_STATE_WAIT_USER_CHANGE_PARAMETERS;
            }
            break;
        }
        case APPHMI_STATE_WAIT_USER_CHANGE_PARAMETERS:
        {
            uint8_t button = getPressedBtn();
            if (NO_BUTTON_PRESSED != button && IsGLCDTaskIdle())
            {
                switch (button)
                {
                    case ESC_BUTTON_PRESSED: 
                    {
                        if (apphmiData.selectedOption < 0x08)
                        {
                            setParameter(apphmiData.selectedOption,apphmiData.backupParameterTemperatureTime);//Pressing ESC returns to the original value.
                        }
                        else
                        {
                            setConstantsPID(apphmiData.selectedOption,apphmiData.backupPIDConstant);
                        }
                        apphmiData.parametersBeenChanged = false;
                        goParametersMenu(); 
                        break;
                    }
                    case UP_BUTTON_PRESSED:
                    case DOWN_BUTTON_PRESSED:    
                    {
                        apphmiData.parametersBeenChanged = true;
                        LCDStr_Scaled_Clear(5,15,5,2,true);
                        apphmiData.doNotClearLCD = false;//With that boolean, I will discern whether it is an increase or decrease.
                        if (UP_BUTTON_PRESSED == button)
                        {
                            apphmiData.doNotClearLCD = true;
                        }
                        apphmiData.state = APPHMI_STATE_WAIT_ESCALATED_MESSAGE_CLEAN;
                        break;
                    }
                    default:  
                    {
                        apphmiData.state =  APPHMI_STATE_DISPLAY_PARAMETER_CHANGED_MESSAGE;
                    }
                }
                
            }
            break;
        }
        case APPHMI_STATE_WAIT_ESCALATED_MESSAGE_CLEAN://The LCDChrXY_Scaled function does not clear the above, since it acts directly on each pixel on the screen, not at the byte level in the LCDBuffer.
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                if (apphmiData.selectedOption < 0x08)
                {
                    //sprintf(apphmiData.bufferForStrings, "%u C", increaseParameterTempTime(apphmiData.selectedOption));  // o snprintf(buffer, sizeof(buffer), "%u", valor);
                    if (apphmiData.doNotClearLCD)
                    {
                        sprintf(apphmiData.bufferForStrings,"%u %c",increaseParameterTempTime(apphmiData.selectedOption),isTemperatureOrTime(apphmiData.selectedOption));
                    }
                    else
                    {
                        sprintf(apphmiData.bufferForStrings,"%u %c",decreaseParameterTempTime(apphmiData.selectedOption),isTemperatureOrTime(apphmiData.selectedOption));
                    }
                }
                else
                {
                    if (apphmiData.doNotClearLCD)
                    {
                        sprintf(apphmiData.bufferForStrings, "%.2f", increaseConstantsPID(apphmiData.selectedOption));
                    }
                    else
                    {
                        sprintf(apphmiData.bufferForStrings, "%.2f", decreaseConstantsPID(apphmiData.selectedOption));
                    }
                }
                apphmiData.doNotClearLCD = false; //It is important that it remains false after using it.
                LCDChrXY_Scaled(5,15,(uint8_t *)apphmiData.bufferForStrings,2,true);//set to true to update the LCD immediately
                apphmiData.state = APPHMI_STATE_WAIT_USER_CHANGE_PARAMETERS;
            }
            break;
        }
        case APPHMI_STATE_DISPLAY_PARAMETER_CHANGED_MESSAGE:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                LCDStr(0x04,(uint8_t *)"Parameter changed!",false, true);
                apphmiData.adelay = RTC_Timer32CounterGet();
                apphmiData.state = APPHMI_STATE_WAIT_DELAY_MESSAGE_PARAMETER_CHANGED;
            } 
            break;
        }
        case APPHMI_STATE_WAIT_DELAY_MESSAGE_PARAMETER_CHANGED:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay) > _2000ms)
            {
                goParametersMenu();  
            }
            break;
        }
        /**********************************************************************/
        /** The parameters that have been changed are displayed graphically. **/ 
        case APPHMI_STATE_VERIFY_PARAMETERS_HAVE_LOGICAL_VALUES:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                switch (apphmiData.messagePointer)
                {
                    case 0x00:  
                    {
                        apphmiData.messagePointer++;
                        LCDClear(); 
                        break;
                    }
                    case 0x01:
                    {
                        apphmiData.adelay = RTC_Timer32CounterGet();
                        if (returnParameterTempTime(0) > returnParameterTempTime(2))
                        {
                            LCDStr(0x01,(uint8_t *)"ERRORª Preheat temp higher than than flux temp",false,true);
                            apphmiData.messagePointer++;
                            return;
                        }
                        if (returnParameterTempTime(2) > returnParameterTempTime(4))
                        {
                            LCDStr(0x01,(uint8_t *)"ERRORª Flux temp higher than than reflow temp",false,true);
                            apphmiData.messagePointer++;
                            return;
                        }
                        if (returnParameterTempTime(6) > returnParameterTempTime(4))
                        {
                            LCDStr(0x01,(uint8_t *)"ERRORª Cooling temp higher than than reflow temp",false,true);
                            apphmiData.messagePointer++;
                            return;
                        }
                        apphmiData.messagePointer = 0x00; //I'm going to use messagePointer to do multiple things in a single state machine.
                        apphmiData.state = APPHMI_STATE_GRAPHICALLY_DISPLAY_CHANGED_PARAMETERS;
                        break;
                    }
                    default: 
                    {
                        if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay) > _3000ms)
                        {
                            goParametersMenu();
                        }
                    }
                }
            }
            break;
        }
        case APPHMI_STATE_GRAPHICALLY_DISPLAY_CHANGED_PARAMETERS:
        {
            if (IsGLCDTaskIdle()) // I wait until the GLCD task is idle
            {
                switch (apphmiData.messagePointer)
                {
                    case 0x00:  LCDClear();         break;
                    case 0x01:  LCDLine (0,0,0,47); break;
                    case 0x02:  LCDLine (0,47,83,47); break;
                    case 0x03:
                    {
                        uint8_t i = 0x01; //to add up all the times that are on the X axis
                        apphmiData.backupParameterTemperatureTime = 0x0000;
                        do
                        {
                            apphmiData.backupParameterTemperatureTime += returnParameterTempTime(i);//Here I will save the sum of all the times
                            i += 0x02;
                        }while(i < 0x09);
                        apphmiData.backupParameterTemperatureTime += 30;
                        apphmiData.selectedOption = 0x00; //I'm going to use it as a pointer.
                        apphmiData.x = 0x0000;
                        apphmiData.y = 47;
                        break;
                    }
                    case 0x04:
                    {
                        uint32_t xPrevious = apphmiData.x;
                        uint32_t yPrevious = apphmiData.y;
                        if (apphmiData.selectedOption < 0x08)
                        {
                            apphmiData.x += 83U*(uint32_t)(returnParameterTempTime(apphmiData.selectedOption + 0x01))/((uint32_t)apphmiData.backupParameterTemperatureTime);
                            apphmiData.y = 49U - 47U*returnParameterTempTime(apphmiData.selectedOption)/270U;
                        }
                        else
                        {
                            apphmiData.x = 83;
                            apphmiData.y = 23;
                            apphmiData.state = APPHMI_STATE_WAIT_USER_PRESS_ANY_BUTTON;
                        }
                        LCDLine (xPrevious,yPrevious,apphmiData.x,apphmiData.y); 
                        break;
                    }
                    case 0x05:
                    {
                        sprintf(apphmiData.bufferForStrings,"%uC",returnParameterTempTime(apphmiData.selectedOption));
                        LCDTinyStr(apphmiData.x - 10U,apphmiData.y + 5U,apphmiData.bufferForStrings,true);
                        break;
                    }
                    default:
                    {
                        sprintf(apphmiData.bufferForStrings,"%us",returnParameterTempTime(apphmiData.selectedOption + 0x01));
                        LCDTinyStr((apphmiData.x - 1U),40,apphmiData.bufferForStrings,true);
                        apphmiData.selectedOption += 0x02;
                        if (apphmiData.selectedOption < 0x09)
                        {
                            apphmiData.messagePointer = 0x03; //To repeat from step 4
                        }
                    }
                }
                apphmiData.messagePointer++;
            }
            break;
        }
        /***  Menu where the user must accept to cancel or edit the parameters ***/
        case APPHMI_STATE_WAIT_USER_PRESS_ANY_BUTTON:
        {
            if (NO_BUTTON_PRESSED != getPressedBtn() && IsGLCDTaskIdle())
            {
                LCDClear();
                apphmiData.messagePointer = 0x00;
                apphmiData.selectedOption = 0x02;
                apphmiData.state = APPHMI_STATE_DRAW_MENU_APPROVE_EDIT_CANCEL_MODIFIED_PARAMETERS;
            }
            break;
        }
        case APPHMI_STATE_DRAW_MENU_APPROVE_EDIT_CANCEL_MODIFIED_PARAMETERS:
        {
            if (IsGLCDTaskIdle())
            {
                if (apphmiData.messagePointer < 0x06)
                {
                    LCDStr(apphmiData.messagePointer, msgsMenuParametersChanged[apphmiData.messagePointer], (apphmiData.selectedOption == apphmiData.messagePointer), false);
                    apphmiData.messagePointer++;
                }
                else
                {
                    apphmiData.stateToReturn = APPHMI_STATE_WAIT_USER_SELECTION_MENU_APPROVE_EDIT_CANCEL_MODIFIED_PARAMETERS;
                    apphmiData.state = APPHMI_STATE_UPDATE_HOME_MENU;
                }
            }
            break;
        }
        case APPHMI_STATE_WAIT_USER_SELECTION_MENU_APPROVE_EDIT_CANCEL_MODIFIED_PARAMETERS:
        {
            uint8_t button = getPressedBtn();
            if (NO_BUTTON_PRESSED != button && IsGLCDTaskIdle())
            {
                switch (button)
                {
                    case OK_BUTTON_PRESSED:
                    {
                        switch(apphmiData.selectedOption)
                        {
                            case 0x02: 
                            {
                                apphmiData.parametersBeenChanged = false;
                                goParametersMenu();
                                break;
                            }
                            case 0x03:  
                            {
                                apphmiData.messagePointer = 0x00; //I'm going to use messagePointer to do multiple things in a single state machine.
                                apphmiData.state = APPHMI_STATE_GRAPHICALLY_DISPLAY_CHANGED_PARAMETERS;
                                break;
                            }
                            case 0x04:        
                            {
                                apphmiData.selectedOption = 0x02;
                                apphmiData.state = APPHMI_STATE_CLEAR_LCD; //return to home menu
                                break;
                            }
                            default: apphmiData.state = APPHMI_STATE_WAIT_EERAM_TASK_IDLE;//Save the parameters to EERAM and return to the home menu
                        }
                        break;
                    }
                    case UP_BUTTON_PRESSED:
                    case DOWN_BUTTON_PRESSED:
                    {
                        if (button == UP_BUTTON_PRESSED)
                        {
                            apphmiData.selectedOption--;
                            if (apphmiData.selectedOption < 2)
                            {
                                apphmiData.selectedOption = 5;
                            }
                        }
                        else // DOWN
                        {
                            apphmiData.selectedOption++;
                            if (apphmiData.selectedOption > 5)
                            {
                                apphmiData.selectedOption = 2;
                            }
                        }
                        apphmiData.messagePointer = 0x00;
                        apphmiData.state = APPHMI_STATE_DRAW_MENU_APPROVE_EDIT_CANCEL_MODIFIED_PARAMETERS;
                        break;
                    }
                    default: break; // case ESC
                }
            }
            break;
        }
        case APPHMI_STATE_WAIT_EERAM_TASK_IDLE:
        {
            if (IsEERAMMTaskIdle() && IsGLCDTaskIdle())
            {
                LCDClear();
                upDatetEERAMvalues(); 
                initializeWriteEERAM();
                apphmiData.state = APPHMI_STATE_WAIT_EERAM_UPDATE_COMPLETED;
            }
            break;
        }
        case APPHMI_STATE_WAIT_EERAM_UPDATE_COMPLETED:
        {
            if (IsEERAMMTaskIdle() && IsGLCDTaskIdle())
            {
                LCDStr(0x02,(unsigned char *)"Parameters    have been updated!",false, true);
                apphmiData.adelay = RTC_Timer32CounterGet();
                apphmiData.state = APPHMI_STATE_WAIT_UPDATED_PARAMETERS_MESSAGE ;
            }
            break;
        }
        case APPHMI_STATE_WAIT_UPDATED_PARAMETERS_MESSAGE:
        {
            if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay) > _2000ms)
            {
                returnHomeMenu();
            }
            break;
        }
        /********* Proceso PID **************/
        case APPHMI_STATE_GRAPH_X_AND_Y_AXES:
        {
            if (IsGLCDTaskIdle())
            {
                switch (apphmiData.messagePointer)
                {
                    case 0x00:  LCDClear();             break;
                    case 0x01:  LCDLine (0,0,0,47);     break;
                    case 0x02:  LCDLine (0,47,83,47);   break;
                    default: 
                    {
                        if (IsPIDTaskIdle())//For safety reasons, I hope the PID's task is idleness.
                        {
                            apphmiData.typeError = NO_ERROR_HMI;
                            apphmiData.messagePointer = 0x00; //I'm going to use messagePointer to do multiple things in a single state machine.
                            apphmiData.stepsOfTheTotalProcessingTime = getStepsTotalProcessingTime();
                            float time_pred = predictTimeForT(getPreHeatTemp()); //I get the time for that given temperature
                            apphmiData.timeProcessTakes = RTC_Timer32CounterGet(); 
                            if (getPreHeatTime() < time_pred - TOLERANCIA_SEC)         
                            {        
                                initializeTaskPID(getPreHeatTemp());
                                apphmiData.estimatedTimeProcessWouldTake = (uint32_t)(time_pred * ((float)_1000ms));
                                apphmiData.state = APPHMI_STATE_START_SIMPLE_PREHEATING_PROCESS;
                            }
                            else
                            {
                                apphmiData.state = APPHMI_STATE_START_PREHEATING_PROCESS;
                            }
                        }
                        return; // so that the pointer doesn't increment at the end
                    }
                }
                apphmiData.messagePointer++;
            }
            break;
        }
        case APPHMI_STATE_START_SIMPLE_PREHEATING_PROCESS:
        {
            if (IsGLCDTaskIdle())
            {
                switch (apphmiData.messagePointer)
                {
                    case 0x00:  LCDTinyStr(1,1,"Simple Preheat",true);   break;
                    case 0x01:
                    {
                        apphmiData.x = 0; //x represents time.
                        updateCoordinatesY(getLastMeasurement());//apphmiData.y = (uint32_t)(1410.0f/29.0f - getLastMeasurement()*47.0f/290.0f); //y represents temperature.
                        break;
                    }
                    case 0x02:
                    {
                        plotTemperatureLine();
                        if (getLastMeasurement() >=  getPreHeatTemp())
                        {
                            apphmiData.messagePointer = 0x00; 
                            apphmiData.state = APPHMI_STATE_FLUX_ACTIVATION; //If the preheating temperature is reached, the flux should enter its activation state.
                            return;
                        }
                        if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.timeProcessTakes) > apphmiData.estimatedTimeProcessWouldTake)
                        {
                            apphmiData.typeError = ERROR_SIMPLE_PREHEAT;
                            apphmiData.state = APPHMI_STATE_ERROR; //If the preheating temperature is reached, the flux should enter its activation state.
                            return;
                        }
                        break;
                    }
                    case 0x03: displayTemperatureOnScreen(); break;
                    default:
                    {
                        if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay) > apphmiData.stepsOfTheTotalProcessingTime)
                        {
                            apphmiData.messagePointer = 0x02; //To return to state 2
                        }
                        return; //To prevent the increase at the end the switch
                    }
                }
                apphmiData.messagePointer++;
            }
            break;
        }
        case APPHMI_STATE_START_PREHEATING_PROCESS:
        {
            if (IsGLCDTaskIdle())
            {
                switch (apphmiData.messagePointer)
                {
                    case 0x00:
                    {
                        if (IsMaxTaskIdle())
                        {
                            startTemperatureReading();
                            LCDTinyStr(1,1,"Complex Preheat",true); break;
                            if (getPreHeatTemp() <= getLastMeasurement())
                            {
                                setReferenceTaskPID(getPreHeatTemp());//It would be a rare case where the oven temperature is above the desired temperature from the start.
                                apphmiData.messagePointer = 0x05; //so that it goes into default
                            }
                        }
                        else
                        {
                            return; //If the max is occupied, we prevent the pointer from incrementing.
                        }
                        break;
                    }
                    case 0x01:
                    {
                        if (IsMaxTaskIdle())
                        {
                            float currentTempMeasurement = getThermocoupleAverageTemp();
                            apphmiData.x = 0; //x represents time.
                            updateCoordinatesY(currentTempMeasurement);//apphmiData.y = (uint32_t)(1410.0f/29.0f - currentTempMeasurement*47.0f/290.0f); //y represents temperature.
                            initializeTaskPID(currentTempMeasurement);//It's just to start the PID
                            apphmiData.adelay = RTC_Timer32CounterGet();//Here I use this variable to gradually increase the PID every 500ms
                            apphmiData.ramp = (getPreHeatTemp() - currentTempMeasurement)/getPreHeatTime(); // rampa ºC/s
                            apphmiData.initialTemperature = currentTempMeasurement;
                            apphmiData.doNotRecalculateSetPoint = false;
                            apphmiData.elapsed_s = 0.0f;
                        }
                        else
                        {
                            return; //If the max is occupied, we prevent the pointer from incrementing.
                        }
                        break;
                    }
                    case 0x02: plotTemperatureLine(); break;
                    case 0x03:
                    {
                        displayTemperatureOnScreen();
                        if (apphmiData.doNotRecalculateSetPoint)
                        {
                            apphmiData.doNotRecalculateSetPoint = false;
                            apphmiData.messagePointer = 0x04; //to skip the set point calculation
                        }
                        break;
                    }
                    case 0x04:
                    {
                        apphmiData.elapsed_s += (float)(abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay))/((float)(_1000ms));
                        // calcula setpoint deseado en este instante (no pasar de Tmp objetivo (getPreHeatTemp))
                        float setpoint = apphmiData.initialTemperature + apphmiData.ramp * apphmiData.elapsed_s; // ramp*0.5s = ºC/s *s = ºC
                        if (apphmiData.ramp > 0.0f && setpoint > getPreHeatTemp()) setpoint = getPreHeatTemp();
                        if (apphmiData.ramp < 0.0f && setpoint < getPreHeatTemp()) setpoint = getPreHeatTemp();
                        // envía la referencia al PID
                        setReferenceTaskPID(setpoint);
                        // si llegamos al objetivo temporal o de temperatura, salimos
                        if (setpoint >= getPreHeatTemp()) 
                        {
                            setReferenceTaskPID(getPreHeatTemp());//For safety, we set the preheating temperature; there is a possibility that the setpoint may be slightly higher.
                            apphmiData.state = APPHMI_STATE_FLUX_ACTIVATION; //If the preheating temperature is reached, the flux should enter its activation state.
                            return;
                        }
                        if (apphmiData.elapsed_s >= getPreHeatTime())
                        {
                            apphmiData.typeError = ERROR_COMPLEX_PREHEAT;
                            apphmiData.state = APPHMI_STATE_ERROR; //If the preheating temperature is reached, the flux should enter its activation state.
                        }
                        apphmiData.adelay = RTC_Timer32CounterGet();
                        break;
                    }
                    case 0x05:
                    {
                       if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelay) > _500ms)
                       {
                           apphmiData.messagePointer = 0x03; //so that it returns to calibrate the setpoint
                       }
                       else if ( abs_diff_uint32(RTC_Timer32CounterGet(), apphmiData.adelayGraph) > apphmiData.stepsOfTheTotalProcessingTime)
                       {
                            apphmiData.messagePointer = 0x01; //so that it returns to graphing the temp
                            apphmiData.doNotRecalculateSetPoint = true;
                       }
                       else
                       {
                            return; //If the max is occupied, we prevent the pointer from incrementing.
                       }
                       break;
                    }
                    default:
                    {
                        if (getLastMeasurement() <=  getPreHeatTemp())//Wait until the temperature drops to preHeat, this doesn't make much sense.
                        {
                            apphmiData.messagePointer = 0x00; 
                            apphmiData.state = APPHMI_STATE_FLUX_ACTIVATION; //If the preheating temperature is reached, the flux should enter its activation state.
                            return; 
                        }
                    }
                }
                apphmiData.messagePointer++;
            }
            break;
        }
        case APPHMI_STATE_FLUX_ACTIVATION:
        {
            if (IsGLCDTaskIdle())
            {
                switch (apphmiData.messagePointer)
                {
                    case 0x00:  LCDTinyStr(1,1,"Flux Activation",true);   break;
                }
                apphmiData.messagePointer++;
            }
            break;
        }
        case APPHMI_STATE_ERROR:
        {
            stopTaskPID();
            apphmiData.messagePointer = 0x00; //I'm going to use messagePointer to do multiple things in a single state machine.
            apphmiData.state = APPHMI_STATE_SHOW_ERROR;
            break;
        }
        case APPHMI_STATE_SHOW_ERROR:
        {
            if (IsGLCDTaskIdle())
            {
                switch (apphmiData.messagePointer)
                {
                    case 0x00: LCDClear();  break;
                    case 0x01:
                    {
                        switch (apphmiData.typeError)
                        {
                            case ERROR_SIMPLE_PREHEAT:  
                            case ERROR_COMPLEX_PREHEAT: LCDStr(0x02,(unsigned char *)"Temp wasn't reached in time",false, true);    break;
                            default:                    LCDStr(0x02,(unsigned char *)"Unknown error",false, true);                      
                        }
                        break;
                    }
                    default:
                    {
                        if (ESC_BUTTON_PRESSED == getPressedBtn())
                        {
                            returnHomeMenu();
                        }
                        return; //so that the pointer doesn't increase
                    }
                }
                apphmiData.messagePointer++;
            }
            break;
        }
        /**************************************************************/
        /* The default state should never be executed. */
        default: break; /* TODO: Handle error in application's state machine. */
    }
}


/*******************************************************************************
 End of File
 */
