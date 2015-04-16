/*******************************************************************************
* File Name: hids.c
*
* Version: 1.0
*
* Description:
*  This file contains HIDS callback handler function.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
* 
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "hids.h"

extern uint8 Joystick_Activity_Flag ;
extern uint16 JoyStick_Activity_Timer;
extern uint8 Joystick_Activity_Prev_State;
uint16 Joystick_Simulation;
uint8 protocol = CYBLE_HIDS_PROTOCOL_MODE_REPORT;   /* Boot or Report protocol mode */
uint8 suspend = CYBLE_HIDS_CP_EXIT_SUSPEND;         /* Suspend to enter into deep sleep mode */

void ReadJoystick (void);   /* Function prototype for reading and altering data for joystick information */
void ReadButtons (void);    /* Function prototype for reading and altering data for button information */

static uint16 X_Axis=0, Y_Axis=0;
static int16 X_Data, Y_Data;
static uint8 Joystick_Data[3] = {0, 0, 0}; /*[0] = X-Axis, [1] = Y-Axis, [2] = Buttons */
static unsigned char Buttons;

#define Button_A_Read 0x01
#define Button_B_Read 0x02
#define Button_C_Read 0x04
#define Button_D_Read 0x08
#define Thumb_Button_Read 0x10
/*******************************************************************************
* Function Name: HidsCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   HID Service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*  None.
*
********************************************************************************/
void HidsCallBack(uint32 event, void *eventParam)
{
    CYBLE_HIDS_CHAR_VALUE_T *locEventParam = (CYBLE_HIDS_CHAR_VALUE_T *)eventParam;

    #ifdef DEBUG_ENABLED
    printf("HIDS event: %lx, ", event);
    #endif
    switch(event)
    {
        case CYBLE_EVT_HIDSS_NOTIFICATION_ENABLED:
            #ifdef DEBUG_ENABLED
            printf("EVT_HIDSS_NOTIFICATION_ENABLED: serv=%x, char=%x\r\n",            
                locEventParam->serviceIndex,
                locEventParam->charIndex);
             #endif
            if(CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX == locEventParam->serviceIndex)
            {
                Joystick_Simulation = ENABLED;
            }
            break;
        case CYBLE_EVT_HIDSS_NOTIFICATION_DISABLED:
            #ifdef DEBUG_ENABLED
            printf("EVT_HIDSS_NOTIFICATION_DISABLED: serv=%x, char=%x\r\n", 
                locEventParam->serviceIndex,
                locEventParam->charIndex);
             #endif
            if(CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX == locEventParam->serviceIndex)
            {
                Joystick_Simulation = DISABLED;
            }
            break;
        case CYBLE_EVT_HIDSS_BOOT_MODE_ENTER:
            #ifdef DEBUG_ENABLED
            printf("EVT_HIDSS_BOOT_MODE_ENTER \r\n");
            #endif
            protocol = CYBLE_HIDS_PROTOCOL_MODE_BOOT;
            break;
        case CYBLE_EVT_HIDSS_REPORT_MODE_ENTER:
            #ifdef DEBUG_ENABLED
            printf("EVT_HIDSS_REPORT_MODE_ENTER \r\n");
            #endif
            protocol = CYBLE_HIDS_PROTOCOL_MODE_REPORT;
            break;
        case CYBLE_EVT_HIDSS_SUSPEND:
            #ifdef DEBUG_ENABLED
            printf("EVT_HIDSS_SUSPEND \r\n");
            #endif
            suspend = CYBLE_HIDS_CP_SUSPEND;
            /* Reduce power consumption, power down logic that is not required to wake up the system */
            #ifdef DEBUG_ENABLED
            UART_DEB_Stop();
            #endif
            break;
        case CYBLE_EVT_HIDSS_EXIT_SUSPEND:
            /* Power up all circuitry previously shut down */
            #ifdef DEBUG_ENABLED
            UART_DEB_Start();
            printf("EVT_HIDSS_EXIT_SUSPEND \r\n");
            #endif
            suspend = CYBLE_HIDS_CP_EXIT_SUSPEND;
            break;
        case CYBLE_EVT_HIDSS_REPORT_CHAR_WRITE:
            #ifdef DEBUG_ENABLED
            printf("EVT_HIDSS_REPORT_CHAR_WRITE: serv=%x, char=%x, value=", 
            locEventParam->serviceIndex,
            locEventParam->charIndex);
             #endif
            ShowValue(locEventParam->value);
            break;
        case CYBLE_EVT_HIDSC_NOTIFICATION:
            break;
        case CYBLE_EVT_HIDSC_READ_CHAR_RESPONSE:
            break;
        case CYBLE_EVT_HIDSC_WRITE_CHAR_RESPONSE:
            break;
        case CYBLE_EVT_HIDSC_READ_DESCR_RESPONSE:
            break;
        case CYBLE_EVT_HIDSC_WRITE_DESCR_RESPONSE:           
            break;
		default:
            #ifdef DEBUG_ENABLED
            printf("Not supported event\r\n");
            #endif
			break;
    }
}


/*******************************************************************************
* Function Name: HidsInit()
********************************************************************************
*
* Summary:
*   Initializes the HID service.
*
*******************************************************************************/
void HidsInit(void)
{
    CYBLE_API_RESULT_T apiResult;
    uint16 cccdValue;
    
    /* Register service specific callback function */
    CyBle_HidsRegisterAttrCallback(HidsCallBack);
    Joystick_Simulation = DISABLED;
    /* Read CCCD configurations from flash */
    apiResult = CyBle_HidssGetCharacteristicDescriptor(CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
        CYBLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, CYBLE_HIDS_REPORT_CCCD, CYBLE_CCCD_LEN, (uint8 *)&cccdValue);
    if((apiResult == CYBLE_ERROR_OK) && (cccdValue != 0u))
    {
        Joystick_Simulation |= ENABLED;
    }
   
}

    
/*******************************************************************************
* Function Name: SimulateKeyboard()
********************************************************************************
*
* Summary:
*   The custom function to simulate CapsLock key pressing
*
*******************************************************************************/
void SimulateJoystick(void)
{
    
    CYBLE_API_RESULT_T apiResult;
    
    
    
    /* Scan SW2 key each connection interval */
    ReadJoystick();						/* Calls function to read joystick movement */
	ReadButtons();						/* Calls function to monitor button presses */
	
	Joystick_Data[0] = X_Data;		
	Joystick_Data[1] = Y_Data;
	Joystick_Data[2] = Buttons;
     
    if(CyBle_GattGetBusStatus() == CYBLE_STACK_STATE_FREE)
    {
        if((Joystick_Activity_Flag ==CyTrue)||(Joystick_Activity_Prev_State == CyTrue))
        {
            apiResult = CyBle_HidssSendNotification(cyBle_connHandle, CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                    CYBLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, JOYSTICK_DATA_SIZE, Joystick_Data);
            
            
            if(apiResult==CYBLE_ERROR_OK)
             {
                #ifdef DEBUG_ENABLED
                printf("success");
                #endif
            }
            else
            {
                #ifdef DEBUG_ENABLED
                printf("failed");
                #endif
            }
            
            if(apiResult != CYBLE_ERROR_OK)
            {
                #ifdef DEBUG_ENABLED
                printf("HID notification API Error: %x \r\n", apiResult);
                #endif
                Joystick_Simulation = DISABLED;
            }
        }
    }
    
    
     if(Joystick_Activity_Flag == CyFalse)
    {
        JoyStick_Activity_Timer --;
        Joystick_Activity_Prev_State = CyFalse;
    }
    else
        Joystick_Activity_Prev_State = CyTrue;
    
}

void ReadJoystick (void)
{
    Joystick_Activity_Flag = CyTrue;
												/* Connects AMUX to channel 0 to connect ADC to joystick X-axis */  		
	SAR_ADC_StartConvert();								/* End ADC conversion */
    
	SAR_ADC_IsEndConversion(SAR_ADC_WAIT_FOR_RESULT); /* Wait for ADC reading to complete */
    SAR_ADC_StopConvert();									/* End ADC conversions */
	X_Axis = SAR_ADC_GetResult16(0) *0xFF /0x7FF;						/* Get ADC reading and store in variable X_Axis */
	
    if( (X_Axis >110) && (X_Axis < 134))
    {
        Joystick_Activity_Flag = CyFalse;
    }
    else
    {
        Joystick_Activity_Flag = CyTrue;
    }
    
					/* End ADC conversions */
	Y_Axis = SAR_ADC_GetResult16(1) *0xFF /0x7FF;						/* Get ADC reading and store in variable Y_Axis */
	if( (Y_Axis >110) && (Y_Axis < 134))
    {
        Joystick_Activity_Flag = CyFalse;
    }
    else
    {
        Joystick_Activity_Flag = CyTrue;
    }
	X_Data = X_Axis - 127;										/* Adjust axis to center joystick */
	Y_Data = Y_Axis - 127;
	
	if(X_Data > 127)										
	X_Data = 127;
	if(Y_Data > 127)
	Y_Data = 127;
	if(X_Data < -127)
	X_Data = -127;
	if(Y_Data< -127)
	Y_Data = -127;
	
	Y_Data = Y_Data * -1;										/* Inverts Y-Axis for PC direction formatting */
}

void ReadButtons (void)
{
    
	Buttons = 0x00;
    
	if((Button_A_Read &Button_Read()) == 0)
	Buttons |= 0x01;
	else
	Buttons &= ~0x01;
	
	if((Button_B_Read &Button_Read()) == 0)
	Buttons |= 0x02;
	else
	Buttons &= ~0x02;
    
    if(Buttons != 0x00)
    {   
        Joystick_Activity_Flag = CyTrue;
        
    }
    
	
}




/* [] END OF FILE */
