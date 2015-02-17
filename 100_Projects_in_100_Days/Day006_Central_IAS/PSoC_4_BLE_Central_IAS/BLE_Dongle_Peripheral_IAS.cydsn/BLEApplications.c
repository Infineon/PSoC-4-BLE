/******************************************************************************
* Project Name		: BLE_Dongle_Peripheral_IAS
* File Name			: BLEApplications.c
* Version 			: 1.0
* Device Used		: CYBL10162-56LQXI
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CySmart USB Dongle (part of CY8CKIT-042-BLE Bluetooth Low 
*                     Energy Pioneer Kit) 
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

/* This variable stores the connection handle value after connection is made */
CYBLE_CONN_HANDLE_T  connectionHandle;

/*******************************************************************************
* Function Name: GenericAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component.
*
* Parameters:  
*  uint8 event:       Event from the BLE component
*  void* eventParams: A structure instance for corresponding event type. The 
*                     list of event structure is described in the component 
*                     datasheet.
*
* Return: 
*  None
*
*******************************************************************************/
void GenericAppEventHandler(uint32 event, void *eventParam)
{
    switch(event)
	{
    /**********************************************************
    *                       General Events
    ***********************************************************/
	case CYBLE_EVT_STACK_ON: 
		/* Start BLE Advertisement after BLE Stack is ON */
		CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
		
		break;
        
	case CYBLE_EVT_TIMEOUT:
		/* Timeout has occured */
        if(CYBLE_GAP_ADV_MODE_TO == *(uint8 *) eventParam)
        {
            /* Advertisement timeout has occured - stop advertisement */
            CyBle_GappStopAdvertisement();
        }
	break;
     
    /**********************************************************
    *                       GAP Events
    ***********************************************************/
    case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
		/* Restart Advertisement if the state is disconnected */
		if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED )
		{
			CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
		}

        break;

    case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
        /* Put the device to discoverable mode so that remote can search it. */
        CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
		
		/* Stop PWM as no LED status is required*/
		LED_PWM_Stop();
		
		/* Set drive mode of Alert LED pin to High-Z to shutdown LED */
		Alert_LED_SetDriveMode(Alert_LED_DM_ALG_HIZ);
        break;
    /**********************************************************
    *                       GATT Events
    ***********************************************************/
    case CYBLE_EVT_GATT_CONNECT_IND:
		/* Extract connection handle */
        connectionHandle = *(CYBLE_CONN_HANDLE_T *)eventParam;	
		
		/* Start PWM for LED status control */
		LED_PWM_Start();
        break;

    default:
        break;
	}
}

/*******************************************************************************
* Function Name: IasServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Immediate Alert Service.
*
* Parameters:  
*  uint8 event:       Write Command event from the BLE component.
*  void* eventParams: parameter to the respective event
*
* Return: 
*  None
*
*******************************************************************************/
void iasEventHandler(uint32 event, void *eventParam)
{
	/* Local variables */
    uint8 alertLevel;
    CYBLE_IAS_CHAR_VALUE_T *iasWrCmdValueParam;
    
    if(event == CYBLE_EVT_IASS_WRITE_CHAR_CMD)
    {
        /* Check if the event received was for writing Alert level */
        iasWrCmdValueParam = (CYBLE_IAS_CHAR_VALUE_T *)eventParam;
            
		/* If size of received attribute is equal to size of Alert level data */
        if(CYBLE_IAS_ALERT_LEVEL_SIZE == iasWrCmdValueParam->value->len)
        {
			/* Extract the alert level value received from Client device*/
            alertLevel = *((iasWrCmdValueParam->value->val));
            
			/* Switch LED status depending on Alert level received */
            switch (alertLevel)
    		{
	    		case CYBLE_NO_ALERT:
						Alert_LED_SetDriveMode(Alert_LED_DM_ALG_HIZ);
						LED_PWM_WriteCompare(LED_NO_ALERT);
	    			break;
	    			
	    		case CYBLE_MILD_ALERT:
						Alert_LED_SetDriveMode(Alert_LED_DM_STRONG);
						LED_PWM_WriteCompare(LED_MILD_ALERT);
	    			break;
	    		
	    		case CYBLE_HIGH_ALERT:
						Alert_LED_SetDriveMode(Alert_LED_DM_STRONG);
						LED_PWM_WriteCompare(LED_HIGH_ALERT);
	    			break;
	    			
	    		default:
	    		    break;
    		}
        }
    }
}

/* [] END OF FILE */
