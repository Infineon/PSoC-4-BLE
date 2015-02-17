/******************************************************************************
* Project Name		: PSoC_4_BLE_Central_IAS
* File Name			: BLEApplications.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: ROIT
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
/* 'connHandle' contains the connection handle parameter and used for BLE API to
* send data to particular connected device */
extern CYBLE_CONN_HANDLE_T			connHandle;

/* 'ble_state' contains the present status of BLE, used for LED state update */
extern uint8 ble_state;

/* 'iasLevel' contains the alert level that is sent to the connected peripheral 
* device */
uint8 iasLevel = FALSE;

/*******************************************************************************
* Function Name: CheckButtonStatus
********************************************************************************
* Summary:
*        This function checks the press of the User button on BLE Pioneer Kit, 
* applies proper debounce and then initiates sending of the Alert notification.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void CheckButtonStatus(void)
{
	static uint8 switchDebounceTimer;
	
	static uint8 buttonStatus = FALSE;
	
	if(User_Button_Read() == FALSE)
    {
        /* Register the button press for the first time and check if button debouncing is completed */
        if(!(buttonStatus & (USER_BUTTON_PUSHED_MASK | USER_BUTTON_DETECTED_MASK)))
        {
           
            /* If button press not registered, register the User button press event*/
            buttonStatus = buttonStatus | USER_BUTTON_PUSHED_MASK;
            
            /* Initiate a debounce counter */
            switchDebounceTimer = SWITCH_DEBOUNCE_DELAY;
        }
        else if( (buttonStatus & USER_BUTTON_PUSHED_MASK) && (switchDebounceTimer == 0) )
        {
            /* If debounce counter has elapsed, then register a button press detect event */
            buttonStatus = buttonStatus & (~USER_BUTTON_PUSHED_MASK);
            buttonStatus = buttonStatus | USER_BUTTON_DETECTED_MASK;
        }
    }
    else
    {
        if((buttonStatus & USER_BUTTON_DETECTED_MASK))
        {
            /* Increment the Alert level, which will then be sent to peripheral. Limit the value till 0x02 */
			iasLevel = (iasLevel+1)%3;
            
			/* Send the updated Alert level to Peripheral over Immediate Alert Service*/
			SetAlertLevel(&iasLevel);
			
            buttonStatus = buttonStatus & ( ~(USER_BUTTON_DETECTED_MASK) );
        }
        /* If button press is not detected and the switch debounce timer has expired, clear the button press status
         * in the ButtonStatus register */
        else if((buttonStatus & USER_BUTTON_PUSHED_MASK) && switchDebounceTimer == 0)
        {
             buttonStatus = buttonStatus & (~USER_BUTTON_PUSHED_MASK);
        }
    }
	
	if(switchDebounceTimer)
	{
		switchDebounceTimer--;	
	}
}

/*******************************************************************************
* Function Name: HandleLEDs
********************************************************************************
* Summary:
*        This function handles the LED on the BLE Pioneer kit that signifies the 
* state of the BLE connection.
*
* Parameters:
*  state: the state of connection.
*
* Return:
*  void
*
*******************************************************************************/
void HandleLEDs(uint8 state)
{
	static uint16 led_scanning_counter = TRUE;
	static uint16 led_discovery_counter = TRUE;
	
	switch(state)
	{
		case BLE_DISCONNECTED:
			/* Set LED to OFF for Disconnected state*/
			Status_LED_Write(LED_OFF);
		break;
		
		case BLE_SCANNING:
			/* Run internal software timer to blink status LED, 
			* indicating ongoing BLE scan */
			if((--led_scanning_counter) == FALSE)
			{
				led_scanning_counter = LED_SCANNING_COUNTER_VALUE;
				Status_LED_Write(!Status_LED_Read());
			}
		break;
			
		case BLE_SERVICE_DISCOVERY:
			/* Run internal software timer to blink status LED, 
			* indicating BLE discovery procedure with connected device */
			if((--led_discovery_counter) == FALSE)
			{
				led_scanning_counter = LED_DISCOVERY_COUNTER_VALUE;
				Status_LED_Write(!Status_LED_Read());
			}
		break;
		
		case BLE_CONNECTED:
			/* Set the LED to ON to indicate BLE connected state */
			Status_LED_Write(LED_ON);
		break;
		
		default:
		
		break;
	}
}

/*******************************************************************************
* Function Name: SetAlertLevel
********************************************************************************
* Summary:
*        This function sends the appropriate Alert level to peripheral device.
*
* Parameters:
*  alertLevel: address of the variable containing the present state of BLE
*				connection
*
* Return:
*  void
*
*******************************************************************************/
void SetAlertLevel(uint8* alertLevel)
{
	CYBLE_API_RESULT_T 			apiResult;
	
	/* Set the IAS characteristic with provided alert level. Alert Level can be either
	* NO alert, MID alert or HIGH alert */
	apiResult = CyBle_IascSetCharacteristicValue(connHandle, CYBLE_IAS_ALERT_LEVEL ,IAS_ATTR_SIZE, alertLevel);
	
	if(apiResult != CYBLE_ERROR_OK)
	{
		
	}	
}

/* [] END OF FILE */
