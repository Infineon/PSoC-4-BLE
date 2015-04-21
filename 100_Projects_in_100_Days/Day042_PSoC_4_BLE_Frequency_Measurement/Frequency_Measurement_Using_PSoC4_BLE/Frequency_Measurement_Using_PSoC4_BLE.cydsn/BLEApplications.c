/***************************************************************************//**
* @file    BLEApplications.c
* @version 1.0
* @author  Chethan (dche@cypress.com)
*
* @par Description
*     This file contains the source code for the APIs used for BLE communication.
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 BLE Pioneer Kit
*
* @par References
*    1. 001-32359: AN2283 "PSoC1: Frequency Measurement"
*
* @par Code Tested With
*    1. PSoC Creator  3.1 SP2 (3.1.0.2177)
*    2. GCC 4.8.4
*
**//****************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
********************************************************************************
* All rights reserved. 
* This software, including source code, documentation and related 
* materials (“Software”), is owned by Cypress Semiconductor 
* Corporation (“Cypress”) and is protected by and subject to worldwide 
* patent protection (United States and foreign), United States copyright 
* laws and international treaty provisions. Therefore, you may use this 
* Software only as provided in the license agreement accompanying the 
* software package from which you obtained this Software (“EULA”). 
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive, 
* non-transferable license to copy, modify and compile the Software source code
* solely for your use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.

* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use 
* of the Software or any product or circuit described in the Software. Cypress
* does not authorize its products for use in any products where a malfunction
* or failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such  
* system or application assumes all risk of such use and in doing so agrees to  
* indemnify Cypress against all liability. 
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
* Characteristic Configuration) of the Custom characteristic to 
* enable notifications */
uint8 startNotification = FALSE;		

/* 'connectionHandle' is handle to store BLE connection parameters */
CYBLE_CONN_HANDLE_T  connectionHandle;	

/* 'restartAdvertisement' flag provided the present state of power mode in firmware */
uint8 restartAdvertisement = FALSE;

/* This flag is used to let application update the CCCD value for correct read 
* operation by connected Central device */
uint8 updateNotificationCCCAttribute = FALSE;

/* This flag is used to let application send a L2CAP connection update request
* to Central device */
static uint8 isConnectionUpdateRequested = TRUE;

/* Connection Parameter update values. This values are used by the BLE component
* to update the connector parameter, including connection interval, to desired 
* value */
static CYBLE_GAP_CONN_UPDATE_PARAM_T ConnectionParam =
{
    CONN_PARAM_UPDATE_MIN_CONN_INTERVAL,  		      
    CONN_PARAM_UPDATE_MAX_CONN_INTERVAL,		       
    CONN_PARAM_UPDATE_SLAVE_LATENCY,			    
    CONN_PARAM_UPDATE_SUPRV_TIMEOUT 			         	
};

/* Status flag for the Stack Busy state. This flag is used to notify the application 
* whether there is stack buffer free to push more data or not */
uint8 busyStatus = 0;
/****************************************************************************/

/*******************************************************************************
* Function Name: InitializeBLESystem
********************************************************************************
* Summary:
*  Starts the BLE component and wakeup_button_ISR
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitializeBLESystem(void)
{
	/* Start BLE component and register the CustomEventHandler function. This 
	* function exposes the events from BLE component for application use */
	CyBle_Start(CustomEventHandler);					
	
	/* Set drive mode of the status LED pin to Strong*/
	Status_LED_SetDriveMode(Status_LED_DM_STRONG);	
}

/*******************************************************************************
* Function Name: CustomEventHandler
********************************************************************************
* Summary:
*  Call back event function to handle various events from BLE stack
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
			
			/* Reset the isConnectionUpdateRequested flag to allow sending
			* connection parameter update request in next connection */
			isConnectionUpdateRequested = TRUE;
			break;    
            
        case CYBLE_EVT_GATTS_WRITE_REQ:				
			/*When this event is triggered, the peripheral has received 
			* a write command on the custom characteristic */
			
			/* Extract the write value from the event parameter */
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
			
			/* Check if the returned handle is matching to Frequency Measurement Client custom configuration*/
            if(CYBLE_PSOC4_FREQ_MEASUREMENT_FREQ_MEASUREMENT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
                /* Set flag so that application can start  sending notifications.*/
				startNotification = wrReqParam->handleValPair.value.val[CYBLE_PSOC4_FREQ_MEASUREMENT_FREQ_MEASUREMENT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX];
				
				if(startNotification == 1)
				{
					/* After the connection, send new connection parameter to the Client device 
					* to run the BLE communication on desired interval. This affects the data rate 
					* and power consumption. High connection interval will have lower data rate but 
					* lower power consumption. Low connection interval will have higher data rate at
					* expense of higher power. This function is called only once per connection. */
					UpdateConnectionParam();
				}
				/* Set flag to allow CCCD to be updated for next read operation */
				updateNotificationCCCAttribute = TRUE;
			}
           
			/* Send the response to the write command received. */
			CyBle_GattsWriteRsp(connectionHandle);
			
            break;
			
		case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
				/* If L2CAP connection parameter update response received, reset application flag */
            	isConnectionUpdateRequested = FALSE;
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
* Function Name: SendDataOverFreqCounterNotification
********************************************************************************
* Summary:
*   Update the data handle for notification and report it to BLE so that
*	data can be sent over notifications
*
* Parameters:
*  Frequency_Ptr: Pointer to the first byte of the array where frequency value 
*  is stored.
*
* Return:
*  void
*
*******************************************************************************/
void SendDataOverFreqCounterNotification(uint8* Frequency_Ptr)
{
	/* 'notificationHandle' is handle to store notification data parameters */
	CYBLE_GATTS_HANDLE_VALUE_NTF_T		notificationHandle; 
	
	/* If stack is not busy, then send the notification */
	if(busyStatus == CYBLE_STACK_STATE_FREE)
	{
		/* Update Notification handle with frequency data*/
		notificationHandle.attrHandle = CYBLE_PSOC4_FREQ_MEASUREMENT_FREQ_MEASUREMENT_CHAR_HANDLE;				
		notificationHandle.value.val = Frequency_Ptr;
		notificationHandle.value.len = FREQUENCY_NOTIFICATION_DATA_LEN;
		
		/* Report data to BLE component for sending data by notifications*/
		CyBle_GattsNotification(connectionHandle,&notificationHandle);
	}
}

/*******************************************************************************
* Function Name: UpdateConnectionParam
********************************************************************************
* Summary:
*  Send the Connection Update Request to Client device after connection 
*  and modify the connection interval for low power operation.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void UpdateConnectionParam(void)
{
	/* If device is connected and Update connection parameter not updated yet,
	* then send the Connection Parameter Update request to Client. */
    if(deviceConnected && isConnectionUpdateRequested)
	{
		/* Reset the flag to indicate that connection Update request has been sent */
		isConnectionUpdateRequested = FALSE;
		
		/* Send Connection Update request with set Parameter */
		CyBle_L2capLeConnectionParamUpdateRequest(connectionHandle.bdHandle, &ConnectionParam);
	}
}

/*******************************************************************************
* Function Name: UpdateNotificationCCCD
********************************************************************************
* Summary:
*  Update the data handle for notification status and report it to BLE 
*  component so that it can be read by Central device.
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
		notificationCCCDhandle.attrHandle = CYBLE_PSOC4_FREQ_MEASUREMENT_FREQ_MEASUREMENT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
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
*  Handle LED status, such as blinking, ON or OFF depending on BLE state.
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
	
	if(state != CyBle_GetState())
	{
		/* If the present BLE state is different from the new BLE state reset the local counter */
				
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
				/* Set the drive mode of LED to Strong to allow driving the LED */
				Status_LED_SetDriveMode(Status_LED_DM_ALG_HIZ);

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
}
/* [] END OF FILE */
