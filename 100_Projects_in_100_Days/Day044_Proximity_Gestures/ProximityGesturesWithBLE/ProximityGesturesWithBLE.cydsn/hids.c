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
#include "gestures.h"

/*******************************************************************************
*   Variable and Function Declarations
*******************************************************************************/

uint16 keyboardSimulation;
uint8 protocol = CYBLE_HIDS_PROTOCOL_MODE_REPORT;   /* Boot or Report protocol mode */
uint8 suspend = CYBLE_HIDS_CP_EXIT_SUSPEND;         /* Suspend to enter into deep sleep mode */

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
* @par Theory of Operation
*    Service specific events are handled here.
*
* @par Notes
*    None
*
**//***************************************************************************/
void HidsCallBack(uint32 event, void *eventParam)
{
    CYBLE_HIDS_CHAR_VALUE_T *locEventParam = (CYBLE_HIDS_CHAR_VALUE_T *)eventParam;

    switch(event)
    {
        case CYBLE_EVT_HIDSS_NOTIFICATION_ENABLED:
            if(CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX == locEventParam->serviceIndex)
            {
                keyboardSimulation = ENABLED;
            }
            break;
        case CYBLE_EVT_HIDSS_NOTIFICATION_DISABLED:
            if(CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX == locEventParam->serviceIndex)
            {
                keyboardSimulation = DISABLED;
            }
            break;
        case CYBLE_EVT_HIDSS_REPORT_MODE_ENTER:
            protocol = CYBLE_HIDS_PROTOCOL_MODE_REPORT;
            break;
        case CYBLE_EVT_HIDSS_SUSPEND:
            suspend = CYBLE_HIDS_CP_SUSPEND;
            break;
        case CYBLE_EVT_HIDSS_EXIT_SUSPEND:
            /* Power up all circuitry previously shut down */
            suspend = CYBLE_HIDS_CP_EXIT_SUSPEND;
            break;
		default:
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
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*   Registers the HID attribute call back function. 
*   Enables the keyboard simulation operation upon checking CCCD configurations from 
*   flash.
*
* @par Notes
*    None
*
**//***************************************************************************/
void HidsInit(void)
{
    CYBLE_API_RESULT_T apiResult;
    uint16 cccdValue;
    
    /* Register service specific callback function */
    CyBle_HidsRegisterAttrCallback(HidsCallBack);
    keyboardSimulation = DISABLED;
    /* Read CCCD configurations from flash */
    apiResult = CyBle_HidssGetCharacteristicDescriptor(CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
        CYBLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, CYBLE_HIDS_REPORT_CCCD, CYBLE_CCCD_LEN, (uint8 *)&cccdValue);
    if((apiResult == CYBLE_ERROR_OK) && (cccdValue != 0u))
    {
        keyboardSimulation |= ENABLED;
    }
    if((apiResult == CYBLE_ERROR_OK) && (cccdValue != 0u))
    {
        keyboardSimulation |= ENABLED;
    }
}

    
/*******************************************************************************
* Function Name: SimulateKeyboard()
********************************************************************************
*
* Summary:
*   The custom function to simulate Left and Right arrow key pressing
*
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*   keyboard_data[3u] contains the keyboard data. Following a valid key,
*   zero is sent to indicate key lift.
*
* @par Notes
*    None
*
**//***************************************************************************/
void SimulateKeyboard(void)
{
    static uint8 keyboard_data[KEYBOARD_DATA_SIZE]={0,0,0,0,0,0,0,0};
    CYBLE_API_RESULT_T apiResult;
   
    if(CyBle_GattGetBusStatus() == CYBLE_STACK_STATE_FREE)
    {

        
        keyboard_data[3u] = SendKeyForDetectedGesture();
            
        apiResult = CyBle_HidssGetCharacteristicValue(CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
            CYBLE_HIDS_PROTOCOL_MODE, sizeof(protocol), &protocol);
        if(apiResult == CYBLE_ERROR_OK)
        {
            /* Send notification only when a valid gesture is detected */
            if(0 != keyboard_data[3u])
            {
                apiResult = CyBle_HidssSendNotification(cyBle_connHandle, CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                    CYBLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, KEYBOARD_DATA_SIZE, keyboard_data);
            }
            if(apiResult == CYBLE_ERROR_OK)
            {
                keyboard_data[2u] = 0u;                       /* Set up keyboard data*/
                keyboard_data[3u] = 0u;                       /* Set up keyboard data*/

                apiResult = CyBle_HidssSendNotification(cyBle_connHandle, CYBLE_HUMAN_INTERFACE_DEVICE_SERVICE_INDEX, 
                    CYBLE_HUMAN_INTERFACE_DEVICE_REPORT_IN, KEYBOARD_DATA_SIZE, keyboard_data);

            }
            if(apiResult != CYBLE_ERROR_OK)
            {
                keyboardSimulation = DISABLED;
            }
        }
    }
}

/*******************************************************************************
* Function Name: SendKeyForDetectedGesture()
********************************************************************************
*
* Summary:
*   This function sends appropriate key based on the gesture detected.
*
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*   For left to right gesture- Right arrow key is sent
*   For right to left gesture- Left arrow key is sent
*
* @par Notes
*    None
*
**//***************************************************************************/
uint8 SendKeyForDetectedGesture(void)
{
    uint8 simKey = 0x00;
    
    if((TURN_ALL_LEDS_OFF == previousLEDDriveSequence) && (LEFT_TO_RIGHT == LEDDriveSequence))
    {
        simKey =  0x4F;/* Key corrresponding to RIGHT_ARROW */
    }
    else if((TURN_ALL_LEDS_OFF == previousLEDDriveSequence) && (RIGHT_TO_LEFT == LEDDriveSequence))
    {
        simKey = 0x50; /* Key corresponding to LEFT_ARROW */
    }     
    
    previousLEDDriveSequence = LEDDriveSequence;
    
    return simKey;
}

/* [] END OF FILE */
