/******************************************************************************
* Project Name		: PSoC_4_BLE_CapSense_Proximity
* File Name			: BLEApplications.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: ROIT
*
********************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/

/******************************************************************************
* Contains BLE application functions definitions such as EventHandler and Macro
* definitions
******************************************************************************/
#include "main.h"

/**************************Variable Declarations*****************************/
/*'deviceConnected' flag is used by application to know whether a Central device  
* has been connected. This is updated in BLE event callback */
uint8 deviceConnected = FALSE;	

/*'startNotification' flag is set when the central device writes to CCC (Client 
* Characteristic Configuration) of the CapSense proximity characteristic to 
*enable notifications */
uint8 startNotification = FALSE;		

/* 'connectionHandle' is handle to store BLE connection parameters */
CYBLE_CONN_HANDLE_T  				connectionHandle;	

/* 'restartAdvertisement' flag provided the present state of power mode in firmware */
uint8 restartAdvertisement = FALSE;

/* This flag is used to let application update the CCCD value for correct read 
* operation by connected Central device */
uint8 updateNotificationCCCAttribute = FALSE;

/* Status flag for the Stack Busy state. This flag is used to notify the application 
* whether there is stack buffer free to push more data or not */
uint8 busyStatus = 0;
/****************************************************************************/

/*******************************************************************************
* Function Name: CustomEventHandler
********************************************************************************
* Summary:
*        Call back event function to handle various events from BLE stack
*
* Parameters:
*  event:		event returned
*  eventParam:	link to value of the event parameter returned
*
* Return:
*  void
*
*******************************************************************************/
void CustomEventHandler(uint32 event, void *eventParam)
{
    CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;
	
    switch(event)
	{
        /**********************************************************
        *                       General Events
        ***********************************************************/
		case CYBLE_EVT_STACK_ON:
			/* This event is received when component is Started */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = TRUE;
			
			break;
		
		case CYBLE_EVT_TIMEOUT:
			/* Event Handling for Timeout  */
		    restartAdvertisement = TRUE;
			break;
			
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
		
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
			
			if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
			{
				/* Set restartAdvertisement flag to allow calling Advertisement 
				* API from main function */
				restartAdvertisement = TRUE;
			}
			break;
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED:		
			/* This event is received when device is connected over GAP layer */

			break;
			
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:		
			/* This event is received when device is disconnected */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = TRUE;

            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
			/* This event is received when device is connected over GATT level */
			
			/* Update attribute handle on GATT Connection*/
            connectionHandle = *(CYBLE_CONN_HANDLE_T *)eventParam;	
			
			/* This flag is used in application to check connection status */
			deviceConnected = TRUE;	
	
            break;
			
        case CYBLE_EVT_GATT_DISCONNECT_IND:	
			/*This event is received when device is disconnected. 
			* Update connection flag accordingly */
			deviceConnected = FALSE;
			
			/* Reset notification flag to prevent further notifications
			 * being sent to Central device after next connection. */
			startNotification = FALSE;
			
			/* Reset the CCCD value to disable notifications */
			updateNotificationCCCAttribute = TRUE;
			UpdateNotificationCCCD();
			
			break;    
            
        case CYBLE_EVT_GATTS_WRITE_REQ:				
			/*When this event is triggered, the peripheral has received 
			* a write command on the custom characteristic */
			
			/* Extract the write value from the event parameter */
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
			
			/* Check if the returned handle is matching to CapSense proximity Client custom configuration*/
            if(CYBLE_CAPSENSE_CAPSENSE_PROXIMITY_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
                /* Set flag so that application can start  sending notifications.*/
				startNotification = wrReqParam->handleValPair.value.val[CYBLE_CAPSENSE_CAPSENSE_PROXIMITY_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX];
				
				/* Set flag to allow CCCD to be updated for next read operation */
				updateNotificationCCCAttribute = TRUE;
			}
            
			/* Send the response to the write command received. */
			CyBle_GattsWriteRsp(connectionHandle);
			
            break;
			
		case CYBLE_EVT_STACK_BUSY_STATUS:
			/* This event is generated when the internal stack buffer is full and no more
			* data can be accepted or the stack has buffer available and can accept data.
			* This event is used by application to prevent pushing lot of data to stack. */
			
			/* Extract the present stack status */
            busyStatus = * (uint8*)eventParam;
            break;
			
        default:
                        
            break;
	} /* switch(event)*/
}

/*******************************************************************************
* Function Name: SendDataOverCapSenseNotification
********************************************************************************
* Summary:
*        Update the data handle for notification and report it to BLE so that
*	data can be sent over notifications
*
* Parameters:
*  proximityValue: Proximity range data; value between 1-100
*
* Return:
*  void
*
*******************************************************************************/
void SendDataOverCapSenseNotification(uint8 proximityValue)
{
	/* 'notificationHandle' is handle to store notification data parameters */
	CYBLE_GATTS_HANDLE_VALUE_NTF_T		notificationHandle; 
	
	/* If stack is not busy, then send the notification */
	if(busyStatus == CYBLE_STACK_STATE_FREE)
	{
		/* Update Notification handle with proximity data*/
		notificationHandle.attrHandle = CYBLE_CAPSENSE_CAPSENSE_PROXIMITY_CHAR_HANDLE;				
		notificationHandle.value.val = &proximityValue;
		notificationHandle.value.len = CAPSENSE_NOTIFICATION_DATA_LEN;
		
		/* Report data to BLE component for sending data by notifications*/
		CyBle_GattsNotification(connectionHandle,&notificationHandle);
	}
}

/*******************************************************************************
* Function Name: UpdateNotificationCCCD
********************************************************************************
* Summary:
*        Update the data handle for notification status and report it to BLE 
*	component so that it can be read by Central device.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void UpdateNotificationCCCD(void)
{
	/* Local variable to store the current CCCD value */
	uint8 CCCDvalue[2];
	
	/* Handle value to update the CCCD */
	CYBLE_GATT_HANDLE_VALUE_PAIR_T notificationCCCDhandle;

	/* Update notification attribute only when there has been change */
	if(updateNotificationCCCAttribute)
	{
		updateNotificationCCCAttribute = FALSE;
	
		/* Write the present notification status to the local variable */
		CCCDvalue[0] = startNotification;
		CCCDvalue[1] = 0x00;
		
		/* Update CCCD handle with notification status data*/
		notificationCCCDhandle.attrHandle = CYBLE_CAPSENSE_CAPSENSE_PROXIMITY_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
		notificationCCCDhandle.value.val = CCCDvalue;
		notificationCCCDhandle.value.len = CCC_DATA_LEN;
		
		/* Report data to BLE component for sending data when read by Central device */
		CyBle_GattsWriteAttributeValue(&notificationCCCDhandle, ZERO, &connectionHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);
	}	
}

/*******************************************************************************
* Function Name: HandleStatusLED
********************************************************************************
* Summary:
*        Handle LED status, such as blinking, ON or OFF depending on BLE state.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleStatusLED(void)
{
	/* Local static counter to handle the periodic toggling of LED or keeping LED ON
	* for some time. */
	static uint32 led_counter = TRUE;
	
	/* Local static variable that stores the last BLE state in which firmware was */
	static uint8 state = 0xFF;
	
	/* Flag to indicate that the state of BLE has changed from the last known value */
	uint8 state_changed = FALSE;
	
	if(state != CyBle_GetState())
	{
		/* If the present BLE state is different from the new BLE state, set the 
		* state_changed flag and reset the local counter */
		state_changed = TRUE;
				
		if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
		{
			led_counter = TRUE;
		}
	}
	
	/* Store the new BLE state into the present state variable */
	state = CyBle_GetState();
		
	switch(state)
	{
		case CYBLE_STATE_CONNECTED:
			/* If the present BLE state is connected, keep the LED ON for
			* pre-determined time and then switch it OFF in WDT ISR */
			if(state_changed)
			{
				/* Reset the flag for state change */
				state_changed = FALSE;
				
				/* Set the drive mode of LED to Strong to allow driving the LED */
				Status_LED_SetDriveMode(Status_LED_DM_ALG_HIZ);
			}
		break;
		
		case CYBLE_STATE_ADVERTISING:
			/* If the present BLE state is advertising, toggle the LED
			* at pre-determined period to indicate advertising. */
			if((--led_counter) == FALSE)
			{
				Status_LED_SetDriveMode(Status_LED_DM_STRONG);
				
				if(Status_LED_Read() == LED_OFF)
				{
					Status_LED_Write(LED_ON);
					led_counter	= LED_ADV_BLINK_PERIOD_ON;
				}
				else
				{
					Status_LED_Write(LED_OFF);
					led_counter	= LED_ADV_BLINK_PERIOD_OFF;
				}
			}
		break;
		
		case CYBLE_STATE_DISCONNECTED:
			/* If the present BLE state is disconnected, switch off LED
			* and set the drive mode of LED to Hi-Z (Analog)*/
			Status_LED_Write(LED_OFF);
			Status_LED_SetDriveMode(Status_LED_DM_ALG_HIZ);
		break;
		
		default:

		break;
	}
	
	/* Reset the state changed flag. */
	state_changed = FALSE;
}

/* [] END OF FILE */
