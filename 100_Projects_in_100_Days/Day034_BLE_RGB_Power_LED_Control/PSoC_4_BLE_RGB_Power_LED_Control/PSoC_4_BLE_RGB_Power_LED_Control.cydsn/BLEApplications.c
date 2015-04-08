/******************************************************************************
* Project Name		: PSoC_4_BLE_RGB_Power_LED_Control
* File Name			: BLEApplications.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : ROIT
*
********************************************************************************
* Copyright (2014-15), Cypress Semiconductor Corporation. All Rights Reserved.
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
#include <main.h>

/**************************Variable Declarations*****************************/
/* 'connectionHandle' stores connection parameters */
CYBLE_CONN_HANDLE_T  connectionHandle;

/* 'rgbHandle' stores RGB control data parameters */
CYBLE_GATT_HANDLE_VALUE_PAIR_T		rgbHandle;	

/* Array to store the present RGB LED control data. The 4 bytes 
* of the array represents {R,G,B,Intensity} */
uint8 RGBledData[RGB_CHAR_DATA_LEN];

/* This flag is used by application to know whether a Central 
* device has been connected. This is updated in BLE event callback 
* function*/
uint8 deviceConnected = FALSE;

/* 'restartAdvertisement' flag provided the present state of power mode in firmware */
uint8 restartAdvertisement = FALSE;

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

/* This flag is used to switch between CPU Deep Sleep and Sleep, depending on
* the status of RGB LED color values received. This method allows to conserve 
* more power while LED usage */
uint8 shut_down_led = TRUE;

/* Counter to keep the LED ON for a selected period before shuting the LEDs down */
uint8 led_timer = FALSE;

/* Flag to switch of the LED after connection */
uint8 switch_off_status_led = FALSE;

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
*  eventParam:	link to value of the events returned
*
* Return:
*  void
*
*******************************************************************************/
void CustomEventHandler(uint32 event, void * eventParam)
{
	/* Local variable to store the data received as part of the Write request 
	* events */
	CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;

    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
			/* This event is received when component is Started */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = TRUE;
			
			break;
			
		case CYBLE_EVT_TIMEOUT:
			/* Event Handling for Timeout  */
	
			break;
        
		/**********************************************************
        *                       GAP Events
        ***********************************************************/
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
			
			/* If the current BLE state is Disconnected, then the Advertisement
			* Start Stop event implies that advertisement has stopped */
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
			
			/* Set flag to allow system to go to Deep Sleep */
			shut_down_led = TRUE;
			break;
        
		/**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
			/* This event is received when device is connected over GATT level */
			
			/* Update attribute handle on GATT Connection*/
            connectionHandle = *(CYBLE_CONN_HANDLE_T  *)eventParam;
			
			/* This flag is used in application to check connection status */
			deviceConnected = TRUE;
			break;
        
        case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* This event is received when device is disconnected */
			
			/* Update deviceConnected flag*/
			deviceConnected = FALSE;
		
			/* Reset the color coordinates */
			RGBledData[RED_INDEX] = ZERO;
            RGBledData[GREEN_INDEX] = ZERO;
            RGBledData[BLUE_INDEX] = ZERO;
            RGBledData[INTENSITY_INDEX] = ZERO;
			UpdateRGBled();

			/* Reset the isConnectionUpdateRequested flag to allow sending
			* connection parameter update request in next connection */
			isConnectionUpdateRequested = TRUE;
			
			/* Set the flag to allow system to go to Deep Sleep */
			shut_down_led = TRUE;
			
			break;
            
        case CYBLE_EVT_GATTS_WRITE_REQ:
			/* This event is received when Central device sends a Write command on an Attribute */
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
            
			/* When this event is triggered, the peripheral has received a write command on the custom characteristic */
			/* Check if the returned handle is matching to RGB LED Control Write Attribute and extract the RGB data*/
            if(CYBLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
				/* Extract the Write value sent by the Client for RGB LED Color characteristic */
                RGBledData[RED_INDEX] = wrReqParam->handleValPair.value.val[RED_INDEX];
                RGBledData[GREEN_INDEX] = wrReqParam->handleValPair.value.val[GREEN_INDEX];
                RGBledData[BLUE_INDEX] = wrReqParam->handleValPair.value.val[BLUE_INDEX];
                RGBledData[INTENSITY_INDEX] = wrReqParam->handleValPair.value.val[INTENSITY_INDEX];
                
				/* Update the PrISM components and the attribute for RGB LED read characteristics*/
                UpdateRGBled();
            }
			
			/* Send the response to the write request received. */
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
    }   	/* switch(event) */
}

/*******************************************************************************
* Function Name: UpdateConnectionParam
********************************************************************************
* Summary:
*        Send the Connection Update Request to Client device after connection 
* and modify theconnection interval for low power operation.
*
* Parameters:
*	void
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
* Function Name: UpdateRGBled
********************************************************************************
* Summary:
*        Receive the new RGB data and modify PrISM parameters. Also, update the
* read characteristic handle so that the next read from the BLE central device
* gives present RGB color and intensity data.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void UpdateRGBled(void)
{
	/* Local variables to calculate the color components from RGB received data*/
	uint8 debug_red;
	uint8 debug_green;
	uint8 debug_blue;
	uint8 intensity_divide_value = RGBledData[INTENSITY_INDEX];
	
	/* Calculate the intensity of each of the Red, Green and Blue component from the
	* received 4-byte data */
	debug_red = (uint8)(((uint16)RGBledData[RED_INDEX]*intensity_divide_value)/255);
	debug_green = (uint8)(((uint16)RGBledData[GREEN_INDEX]*intensity_divide_value)/255);
	debug_blue = (uint8)(((uint16)RGBledData[BLUE_INDEX]*intensity_divide_value)/255);
	
	/*If the Intensity value sent by client is below a set threshold, assume no intensity
	* and switch of the RGB colors on LED. Also, set the appropriate flag to allow the system 
	* to go to Deep Sleep */
	if(RGBledData[INTENSITY_INDEX] < LED_NO_COLOR_THRESHOLD)
	{
		/* Set the flag to allow the system to go to Deep Sleep */
		shut_down_led = TRUE;
		
		/* Set the RGB LED pin drive mode to HiZ to prevent leakage of current */
		RED_SetDriveMode(RED_DM_ALG_HIZ);
		GREEN_SetDriveMode(GREEN_DM_ALG_HIZ);
		BLUE_SetDriveMode(BLUE_DM_ALG_HIZ);
	}
	else
	{
		/* If the individual color value of Red, Green and Blue component is less than
		*  set threshold, then assume no color and shut down the LEDs. Also,set the 
		* appropriate flag to allow system to go to Deep Sleep */
		if((debug_red < LED_NO_COLOR_THRESHOLD) && \
			(debug_green < LED_NO_COLOR_THRESHOLD) && \
			(debug_blue < LED_NO_COLOR_THRESHOLD))
		{
			/* Set the flag to allow the system to go to Deep Sleep */
			shut_down_led = TRUE;
			
			/* Set the RGB LED pin drive mode to HiZ to prevent leakage of current */
			RED_SetDriveMode(RED_DM_ALG_HIZ);
			GREEN_SetDriveMode(GREEN_DM_ALG_HIZ);
			BLUE_SetDriveMode(BLUE_DM_ALG_HIZ);
		}
		else
		{
			/* If the color and intensity values received are within the acceptable 
			* range, then PrISM has to be enabled and written with correct density 
			* value to present the right color. For this, reset the shut_down_led 
			* flag to allow CPU to go to only Sleep, and not Deep Sleep. This is 
			* because CPU cannot be in Deep Sleep while PrISM is active */
			shut_down_led = FALSE;
			
			/* Set the time for RGB LED on Period. After this time (in seconds), the 
			* LED will be shutdown to prevent current usage */
			led_timer = LED_OFF_TIME_PERIOD;
			
			/* Set the drive mode for LED pins to Strong mode */
			RED_SetDriveMode(RED_DM_STRONG);
			GREEN_SetDriveMode(GREEN_DM_STRONG);
			BLUE_SetDriveMode(BLUE_DM_STRONG);
			
			/* Update the density value of the PrISM module for color control*/
			PrISM_1_WritePulse0(RGB_LED_MAX_VAL - debug_red);
		    PrISM_1_WritePulse1(RGB_LED_MAX_VAL - debug_green);
		    PrISM_2_WritePulse0(RGB_LED_MAX_VAL - debug_blue);
		}
	}
	
	/* Update RGB control handle with new values */
	rgbHandle.attrHandle = CYBLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE;
	rgbHandle.value.val = RGBledData;
	rgbHandle.value.len = RGB_CHAR_DATA_LEN;
	
	/* Send updated RGB control handle as attribute for read by central device, so that
	* Client reads the new RGB color data */
	CyBle_GattsWriteAttributeValue(&rgbHandle,FALSE,&connectionHandle,FALSE);
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
	
	if(state != CyBle_GetState())
	{
		/* Reset the local counter for advertisement state */
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

		break;
		
		case CYBLE_STATE_ADVERTISING:
			/* If the present BLE state is advertising, toggle the LED
			* at pre-determined period to indicate advertising. */
			if((--led_counter) == FALSE)
			{
				/* Toggle Status LED for indicating Advertisement */
				if(PrISM_1_ReadPulse0() == RGB_LED_OFF)
				{
					PrISM_1_WritePulse0(RGB_LED_ON);
					RED_SetDriveMode(RED_DM_STRONG);
					
					led_counter	= LED_ADV_BLINK_PERIOD_ON;
				}
				else
				{
					PrISM_1_WritePulse0(RGB_LED_OFF);
					RED_SetDriveMode(RED_DM_ALG_HIZ);
					
					led_counter	= LED_ADV_BLINK_PERIOD_OFF;
				}
			}
		break;
		
		case CYBLE_STATE_DISCONNECTED:
			/* If the present BLE state is disconnected, switch off LED
			* and set the drive mode of LED to Hi-Z (Analog)*/
			PrISM_1_WritePulse0(RGB_LED_OFF);
			RED_SetDriveMode(RED_DM_ALG_HIZ);
			GREEN_SetDriveMode(GREEN_DM_ALG_HIZ);
			BLUE_SetDriveMode(BLUE_DM_ALG_HIZ);
		break;
		
		default:

		break;
	}
}

/* [] END OF FILE */
