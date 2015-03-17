/******************************************************************************
* Project Name		: BLE_Dynamic_GATT_Service_Configuration
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Hardware          : CY8CKIT-042-BLE
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Owner				: mady@cypress.com
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
/******************************************************************************
*                           THEORY OF OPERATION
* This example demonstrates dynamic configuration of GATT Services, especially
* enabling and disabling a service in firmware. This example uses a Custom Profile 
* for controlling the color of RGB Led. This custom service can be dynamically 
* enabled or disabled based on the input from the UART terminal
*******************************************************************************/

#include <project.h>
#include <stdio.h>

#define FALSE                               (0)
#define ALL_OFF                             (0)

/***************************************
*        Global variables
***************************************/
uint8 *RGBledData;

/***************************************
*        Function declarations
***************************************/
void NotifyServChanged(void);
void StackEventHandler(uint32 event, void *eventParam);

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    CYBLE_GATT_ERR_CODE_T GattErrCode = CYBLE_GATT_ERR_NONE;

    /* Enable the Global Interrupts */
    CyGlobalIntEnable;

    /* Start CYBLE component and register generic event handler */
    CyBle_Start(StackEventHandler);
    
    /* Start the UART Component for Debugging and Entering Input */
    UART_Start();
    
    while(1)
    {
        char8 command;       /* Input from user via UART terminal */
        
        /* CyBle_ProcessEvents() allows BLE stack to process pending events */
        CyBle_ProcessEvents();
        
        if((command = UART_UartGetChar()) != 0u)
        {
            /* Enter D for disabling the custom RGB LED control service */
            if ((command == 'D') || (command == 'd'))
            {
                GattErrCode = CyBle_GattsDisableAttribute (CYBLE_RGB_LED_SERVICE_HANDLE);
                
                if (GattErrCode == CYBLE_GATT_ERR_NONE)
                {
                    UART_UartPutString ("LED service disabled\r\n");
                    NotifyServChanged();
                }
                else
                {
                    UART_UartPutString ("Attribute handle is not valid\r\n");
                }
            }
        
            /* Enter E for enabling the custom RGB LED control service */
            if ((command == 'E') || (command == 'e'))
            {
                GattErrCode = CyBle_GattsEnableAttribute(CYBLE_RGB_LED_SERVICE_HANDLE);
                if (GattErrCode == CYBLE_GATT_ERR_NONE)
                {
                    UART_UartPutString ("LED service enabled\r\n");
                    NotifyServChanged();
                }
                else
                {
                    UART_UartPutString ("Attribute handle is not valid\r\n");
                }
            }
        }
    }
}

/*******************************************************************************
* Function Name: NotifyServChanged()
********************************************************************************
*
* Summary:
*   Sends a service changed notification to the Master
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void NotifyServChanged(void)
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    CYBLE_GATT_ERR_CODE_T apiGattErrCode = 0;
    uint32 value;
    CYBLE_GATT_HANDLE_VALUE_PAIR_T    handleValuePair;
    
    /* The Handle of the 1st and the last service affected are sent in the parameter 'value' */
    value = (CYBLE_RGB_LED_SERVICE_HANDLE) << 16u | 
            (CYBLE_RGB_LED_RGB_LED_CONTROL_CHARACTERISTIC_USER_DESCRIPTION_DESC_HANDLE);
    handleValuePair.value.val = (uint8 *)&value;
    handleValuePair.value.len = sizeof(value);
    handleValuePair.attrHandle = cyBle_gatts.serviceChangedHandle;
      
    apiResult =CyBle_GattsNotification(cyBle_connHandle, &handleValuePair);
    
    if (apiResult != CYBLE_ERROR_OK)
    {
        printf ("Sending Service Changed Notification failed\r\n");
    }
    else
    {
        printf ("Service Changed Notification sent\r\n");
    }
    
    /* To register the service change in the Database of the GATT Server */
    apiGattErrCode = CyBle_GattsWriteAttributeValue(&handleValuePair, 0u, NULL,CYBLE_GATT_DB_LOCALLY_INITIATED);
    
    if (apiGattErrCode != 0)
    {
        printf ("Service Changed Attribute DB write failed\r\n");
    }
     else
    {
        printf ("Service Changed Attribute DB write success\r\n\n");
    }
}

/*******************************************************************************
* Function Name: UpdateRGBled
********************************************************************************
* Summary:
* Receive the new RGB data and change the color of the RGB LED. Also, update the
* read characteristic handle so that the next read from the BLE central device
* gives present RGB color.
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
    CYBLE_GATT_HANDLE_VALUE_PAIR_T rgbHandle; /* stores RGB control data parameters */
    
	printf ("Updating RGB LEDs : %d\r\n",*RGBledData);
    
    RED_LED_Write(~ (*RGBledData & 1));
    GREEN_LED_Write(~(( *RGBledData & 2) >> 1));
    BLUE_LED_Write(~(( *RGBledData & 4) >>2));
    
	/* Update RGB control handle with new values */
	rgbHandle.attrHandle = CYBLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE;
	rgbHandle.value.val = RGBledData;
	rgbHandle.value.len = 1;
	
	/* Send updated RGB control handle as attribute for read by central device, so that
	 * Client reads the new RGB color data */
	CyBle_GattsWriteAttributeValue(&rgbHandle, FALSE, &cyBle_connHandle, FALSE);
}

/*******************************************************************************
* Function Name: StackEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the CYBLE Component.
*
* Parameters:
*  uint8 event:       Event from the CYBLE component.
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of event structure is described in the component
*                     datasheet.
*
* Return:
*  None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void *eventParam)
{
    CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    switch(event)
    {
        /**********************************************************
        *                       General Events
        ***********************************************************/
        case CYBLE_EVT_STACK_ON: /* This event received when BLE component is started */
            printf ("BLE Component ON\r\n");
            /* Starts advertisement */
            if(!(CYBLE_ERROR_OK == CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST)))
            {
                printf ("Starting advertisement failed, status =0x%x",apiResult);
            }
            else
            {
                printf ("Starting to advertise\r\n");
            }
            break;

        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            /* Event received when advertisement is Started ot Stopped */
            printf("\r\n");
            if (CyBle_GetState() != CYBLE_STATE_ADVERTISING)
            {
               printf("Advertisement is disabled\r\n");
            }
            else
            {
                printf("Advertisement is enabled \r\n");
            }
            break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            /* event received when connection is established */
            printf("Device connected\r\n\n");
            break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            /* Restarting the advertisement */
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if (apiResult != CYBLE_ERROR_OK)
            {
                printf ("\r\nRestarting advertisement failed, status =0x%x",apiResult);
            }
            else
            {
                printf ("\r\nRestarting advertisement..\r\n");
            }
            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/
  
        case CYBLE_EVT_GATTS_WRITE_REQ:
			/* This event is received when Central device sends a Write command on an Attribute */
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
          
			/* Check if command is for correct attribute and update the flag for sending Notifications */
           	/* Check if the returned handle is matching to RGB LED Control Write Attribute and extract the RGB data*/
            if(CYBLE_RGB_LED_RGB_LED_CONTROL_CHAR_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
				/* Extract the Write value sent by the Client for RGB LED Color characteristic */
                RGBledData = wrReqParam->handleValPair.value.val;
               /* Update the RGB Color and the attribute for RGB LED read characteristics*/
                UpdateRGBled();
            }
            
			/* Send the response to the write request received. */
			CyBle_GattsWriteRsp(cyBle_connHandle);
			break;
            
        case CYBLE_EVT_GATT_DISCONNECT_IND:
            {
                uint8 ledValue = ALL_OFF;
        		/* This event is received when the device is disconnected */
        		printf ("Device disconnected\r\n");
                RGBledData = &ledValue;
                /* Turn Off the LED */   
                UpdateRGBled();
            }
            break;
            
            /**********************************************************
            *                       Other Events
            ***********************************************************/
        default:
            break;
    }
}

/* [] END OF FILE */
