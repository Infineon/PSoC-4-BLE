/******************************************************************************
* Project Name		: BLE_Bonding
* File Name			: main.cl
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Hardware          : CY8CKIT-042-BLE
* Software Used		: PSoC Creator
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Owner				: MADY
*
********************************************************************************
* Copyright (2014-15), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivativeworks of,
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
*This example demonstrates the concept of Bonding. Bonding is a process which
*enables two "Secure" BLE devices to  connect to each other without User intervention.
*In this example, the BLE Component requires its pair to enter PassKey for establishing
*a connection for the first time. After this, the bonding informtion is stored. From
*then on, no passkey is required to be entered, until the Bonding data is removed
*manually by the user. Press 'R' to clear all the bonding information.
*******************************************************************************/

#include <project.h>
#include <stdio.h>
#include "led.h"


/***************************************
*        Global Variables
***************************************/
char8                command;           //Input from user via UART Terminal
uint8                RemoveDevice;      // Flag set when user sends 'R' to remove bonding data
uint8                Remove_Ad_Stopped;  /* Flag indicating Advertisement stopped for removing 
                                           bonding data */

/* All zeros passed as  argument passed to CyBle_GapRemoveDeviceFromWhiteList for 
removing all the bonding data stored */
CYBLE_GAP_BD_ADDR_T clearAllDevices = {{0,0,0,0,0,0},0};

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
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    switch(event)
    {
    /**********************************************************
    *                       General Events
    ***********************************************************/
        case CYBLE_EVT_STACK_ON: /* This event received when BLE component is started */
        
            printf ("BLE_Bonding Device ON\r\n");
        /* Starts advertisement */
             if(!(CYBLE_ERROR_OK == CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST)))
                {
                    printf ("Starting Advertisement Failed, status =0x%x",apiResult);
                }
                else
                {
                    printf ("Starting to advertise\r\n");
                }
             break;


    /**********************************************************
    *                       GAP Events
    ***********************************************************/

        case CYBLE_EVT_GAP_AUTH_REQ:
            //Request to Initiate Authentication received
            printf("\r\n");
            printf("CYBLE_EVT_GAP_AUTH_REQ: security=0x%x, bonding=0x%x, ekeySize=0x%x, err=0x%x \r\n",
                    (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).security,
                    (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).bonding,
                    (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).ekeySize,
                    (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).authErr);
            break;
            
       
        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
           printf("\r\n");
            printf("CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST. Passkey is: %lu \r\n", *(uint32 *)eventParam);
          
            printf("Please enter the passkey on your Server device.\r\n");
            break;

        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            //Received when authentication is complete
            BLUE_LED_ON();
            printf("\r\n");
            printf("Authentication Complete\r\n");
                 
            break;

        case CYBLE_EVT_GAP_AUTH_FAILED:
            // Received when Authentication failed
            printf("\r\n");
            printf("CYBLE_EVT_GAP_AUTH_FAILED: %d\r\n", *(uint8 *) eventParam);
            break;

        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            //Event received when advertisement is Started ot Stopped
            printf("\r\n");
            if (CyBle_GetState() != CYBLE_STATE_ADVERTISING)
                {
                    RED_LED_ON();
                    printf("Advertisement is disabled\r\n");
                    if (RemoveDevice)
                    {
                       /*Set the flag indicating that advertisement has been stopped for
                        //removing bonding data */
                        Remove_Ad_Stopped = 1;
                        RemoveDevice = 0;  
                    }               
                }
                else
                {
                    GREEN_LED_ON();
                    printf("Advertisement is enabled \r\n");
                    /* Device is now in Advertising state */                }
            break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            // event received when connection is established
            printf("Device  Connected\r\n");
            apiResult = CyBle_GattcStartDiscovery(cyBle_connHandle);
            printf("Starting Discovery \r\n");
            if(apiResult != CYBLE_ERROR_OK)
            {
                printf("StartDiscovery API Error: %x \r\n", apiResult);
            }
            break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            //Event received when connection is disconnected
            RED_LED_ON ();
            printf("\r\n");
            printf("Device Disconnected\r\n");
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if (apiResult != CYBLE_ERROR_OK)
            {
                printf ("Restarting Advertisement Failed, status =0x%x",apiResult);
            }
            else
            {
                printf ("Restarting Advertisement..\r\n");
            }
            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/

        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
            printf("\r\n");
            printf("Discovery complete.\r\n");
            /* Server is now discovered */
            /* Send authentication request to peer device */
            (void) CyBle_GapAuthReq(cyBle_connHandle.bdHandle, &cyBle_authInfo);
            break;

            /**********************************************************
            *                       Other Events
            ***********************************************************/
        default:
            break;
    }
}


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
    RemoveDevice = 0;
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    // Enable the Global Interrupts
    CyGlobalIntEnable;

    /* Start CYBLE component and register generic event handler */
    CyBle_Start(StackEventHandler);
    
    //Start the UART Component
    UART_Start();
    while(1)
    {
        /* CyBle_ProcessEvents() allows BLE stack to process pending events */
        CyBle_ProcessEvents();
        if((command = UART_UartGetChar()) != 0u)
        {
            if ((command == 'R') || (command == 'r'))
            {
                /* Request to clear Bonding information received */
                RemoveDevice = 1; 
                if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
                {
                    /* Stop the advertisement before removing the bonding 
                    information */
                    CyBle_GappStopAdvertisement ();
                }
            }
        }
        
        if((cyBle_pendingFlashWrite != 0u) &&
           ((UART_SpiUartGetTxBufferSize() + UART_GET_TX_FIFO_SR_VALID) == 0u))
        {
            // Store Bonding informtation to flash 
            apiResult = CyBle_StoreBondingData(0u);
            if ( apiResult == CYBLE_ERROR_OK)
            {
                printf("Bonding data stored\r\n");
            }
            else
            {
                printf ("Bonding data storing pending\r\n");
            }
        }
        
        if (Remove_Ad_Stopped)
        {
            Remove_Ad_Stopped = 0;
            apiResult = CyBle_GapRemoveDeviceFromWhiteList(&clearAllDevices);
            /* CyBle_StoreBondingData should be called to clear the Bonding info
                from flash */
            while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
            printf("Cleared the list of bonded devices. \n\n\r");
            /* Restart the advertisement */
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if (apiResult != CYBLE_ERROR_OK)
            {
                printf ("Restarting Advertisement Failed, status =0x%x",apiResult);
            }
            else
            {
                printf ("Restarting Advertisement..\r\n");
            } 
        }
    }
}

/* [] END OF FILE */
