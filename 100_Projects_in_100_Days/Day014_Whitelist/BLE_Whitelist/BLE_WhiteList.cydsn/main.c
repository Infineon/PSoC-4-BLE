/******************************************************************************
* Project Name		: BLE_Whitelist
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Hardware          : CY8CKIT-042-BLE
* Software Used		: PSoC Creator 3.1 CP1
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
*******************************************************************************
*Whitelist functionality in Bluetooth allows a device to filter out other devices 
*trying to scan or connect with the device. Only the devices added to whitelist 
*will be given the privilege of scanning or connection or both (based on the filter
*policy selected). In this example, the BLE Component is programmed as a Peripheral
*and a Server, which has the Whitelist setting as “Scan Request Whitelist; Connect
*Request Whitelist”. This means that, only the central devices that are added to our
*peripheral’s Whitelist will be able to receive the scan response packet and establish
*a connection with it. UART Terminal (Baud Rate: 115200) is used to provide inputs in 
*this example. The user can manually add any device to the whitelist and can remove a 
*device from the whitelist.
******************************************************************************/
#include <project.h>
#include "stdio.h"


#include "LED.h"

void StackEventHandler(uint32 event,void *eventParam);
//Structure containing address of device 
CYBLE_GAP_BD_ADDR_T     whitelistdeviceaddress;
//Array which is used to store the list of devices added.
CYBLE_GAP_BD_ADDR_T     whitelistdeviceaddressBackup[8];
uint8                   Index;         //Index of devices added to Whitelist

uint32                  UartRxDataSim;//Character Input from the UART Terminal
uint8                   Count;        /*Count of the Characters from UART while 
                                        receiving Address, for adding to whitelist*/
uint8                   RemoveIndex;  //Index of the device to be removed from whitelist
uint8                   AddrNibble;     //Actual value derived from INPUT ASCII value

uint8                   AddRequest;   /*FLag which is set when requeust to add a
                                        device to whitelist is received */   
uint8                   DelRequest;   /* FLag which is set when requeust to remove
                                         a device from whitelist is received */
CYBLE_API_RESULT_T 		apiResult;

/*******************************************************************************
* Function Name: StackEventHandler
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

void StackEventHandler(uint32 event,void *eventParam)
{
    
    switch(event)
	{
        
		case CYBLE_EVT_STACK_ON:
            //Starting Advertisement as soon as Stack is ON
             apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if (apiResult == CYBLE_ERROR_OK)
            {
                printf ("Starting Advertisement\r\n");
            }
            else
            {
                printf ("Error Start Adv %d\r\n",apiResult);
            }
            break;
            
        case CYBLE_EVT_TIMEOUT:
         if( CYBLE_GAP_ADV_MODE_TO ==*(uint16*) eventParam)
            {
                printf ("Advertisement TimedOut\r\n");
                apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
                if (apiResult != CYBLE_ERROR_OK)
                {
                    printf ("\nRestarting  Advertisement\r\n");
                }
            }
		
		case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* Red LED Glows when device is disconnected */
            RED_LED_ON ();
            break;
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            printf ("Disconnected \r\n");
            // Starting Advertisent again when there is disconnection
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if (apiResult != CYBLE_ERROR_OK)
            {
                printf ("\nRestarting  Advertisement\r\n");
            }
            break;
            
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            printf ("\n\r Connection Established \r\n");
            //Blue LED glows when device is connected
            BLUE_LED_ON ();
            break;

		
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
         // This Event is received when advertisement is started or stopped.
            if (CyBle_GetState() == CYBLE_STATE_ADVERTISING)
            {
                printf("Advertising...\r\n");
                //Green LED Indicates that Advertisement is going on.
                GREEN_LED_ON();
            }
            else
            {
                RED_LED_ON();
                printf ("Advertisement Stopped \r\n");
                if (AddRequest == 1)
                {
                    AddRequest = 0;
                    // Adding the Device to whitelist
                    apiResult = CyBle_GapAddDeviceToWhiteList(&whitelistdeviceaddress); 
                                
                    if(apiResult == CYBLE_ERROR_INVALID_PARAMETER)
					{
						printf ("Adding to Whitelist Failed. Invalid parameter \r\n");
                    }
                    else if   (apiResult ==CYBLE_ERROR_INVALID_OPERATION)
                    {
                        printf ("Invalid Operation \r\n");
                    }
                    else if (apiResult ==CYBLE_ERROR_INSUFFICIENT_RESOURCES)
                    {
                      printf ("Adding to Whitelist Failed. List already full \r\n");
                    }
                    else if (apiResult ==CYBLE_ERROR_DEVICE_ALREADY_EXISTS)
                    {
                        printf ("Device Already exists \r\n");
                    }
                    else if (apiResult == CYBLE_ERROR_OK)
                    {
                        //Copying the address to the BackUp Array
                        whitelistdeviceaddressBackup[Index] = whitelistdeviceaddress; 
                        Index++;
                        printf ("Device Added to WhiteList\r\n");
                        printf ("Press A to Add or R to remove a from Whitelist \r\n");
                    }
                    
                    // Restarting the advertisement
                    apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
                    if (apiResult == CYBLE_ERROR_OK)
                    {
                        printf ("Restarting Advertisement \r\n");
                        GREEN_LED_ON ();
                    }
                    else
                    {
                        printf ("Error Start Adv %d \r\n",apiResult);
                    }
                }
                if (DelRequest == 1)
                {
                    DelRequest = 0;
                    //Removing the Device from Whitelist
                    apiResult = CyBle_GapRemoveDeviceFromWhiteList 
                    (&whitelistdeviceaddressBackup[RemoveIndex]);
                    if (apiResult == CYBLE_ERROR_OK)
                    {
                        
                        uint8 j;
                        for (j = RemoveIndex; j<Index - 1; j++)
                        {
                            whitelistdeviceaddressBackup[j] = 
                            whitelistdeviceaddressBackup[j+1];
                        }
                        Index--;
                                                                                                                      
                        printf ("Device %d Removed from Whitelist\r\n",RemoveIndex + 1);
                        printf ("Press A to Add a Device or R to remove a device\r\n");
                    }
                    else if (apiResult == CYBLE_ERROR_NO_DEVICE_ENTITY)
                    {
                        printf ("No Such Device Exists. Press A to Add a Device or R to remove\r\n");
                    }
                    else 
                    {
                        printf ("Error: Operation cannot be performed");
                        printf ("Press A to Add a Device or R to remove\r\n");
                    }
                    //re-starting Advertisement
                    apiResult = CyBle_GappStartAdvertisement (CYBLE_ADVERTISING_FAST);
                    if (apiResult == CYBLE_ERROR_OK)
                    {
                        printf ("Restarting Advertisement\r\n");
                    }
                    else
                    {
                        printf ("Restarting Advertisement Failed\r\n");
                    }
                }
            }
            
		default:
			break;
	}
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  System entry and execution point for application
*
* Parameters:  
*  None
*
* Return: 
*  int

********************************************************************************/

int main()
{
  
    /* Initializing all the Flags and Indexes to 0 */
    ALL_LED_OFF ();
    Count = 0;
    Index = 0;
    AddRequest = 0;
    DelRequest = 0;

    CyGlobalIntEnable;  /* Comment this line to disable global interrupts. */
    
    /* Start BLE component and register Event handler function */	
    CyBle_Start(StackEventHandler);
	

    /* Start UART Component which is used for receiving inputs and Debugging */
    UART_Start();

	printf("BLE WhiteList Example \r\n");
    printf("Press A to add a Device to WhiteList. R to remove the Device from Whitelist \r\n");

    /* Continuous loop scans for inputs from UART Terminal and accordingly 
    handles Addition to and Removal from Whitelist. Also processes
    BLE events */
    
    for(;;)
    {
        //Checks the internal task queue in the BLE Stack
        CyBle_ProcessEvents();
        
        if(UART_SpiUartGetRxBufferSize())
		{
		   	UartRxDataSim = UART_UartGetChar();
            if (UartRxDataSim == 'A' || UartRxDataSim == 'a')  // The user has to Enter D for disconnection 
            {
                printf ("Enter the Address of the Device. Press Z to Go Back \r\n");
                for (;;)
                {
                    if (Count ==12)
                    {
                        //If the user had entered the full address, stop advertisement
                        //for addition process
                        CyBle_GappStopAdvertisement ();
                        /*Once We stop advertisement, the 
                        CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP event is invoked.
                         After this, the API for adding the device to whitelist is invoked
                        in the StackEventHandler*/
                        RED_LED_ON ();
                        AddRequest = 1;
                        printf ("\r\n");
                        printf ("Address is 0x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",
                                whitelistdeviceaddress.bdAddr[5],
                                whitelistdeviceaddress.bdAddr[4],
                                whitelistdeviceaddress.bdAddr[3],
                                whitelistdeviceaddress.bdAddr[2],
                                whitelistdeviceaddress.bdAddr[1],
                                whitelistdeviceaddress.bdAddr[0]);
                        printf ("Attempting to Add to whitelist \r \n");
                        Count = 0; 
                        break;
                    }
                    
                    if(UART_SpiUartGetRxBufferSize())
                    {
                        UartRxDataSim = UART_UartGetChar();
                        if (UartRxDataSim == 'Z' || UartRxDataSim == 'z')  
                        {
                            Count = 0;
                            printf("Press A to add a Device to WhiteList \r\n");
                            break;
                        }
                       
                        else
                        {
                            if ((UartRxDataSim >= '0') && (UartRxDataSim <= '9' ))
                            {
                                AddrNibble = UartRxDataSim - '0';
                                UART_UartPutChar (UartRxDataSim);
                            }
                            else if ((UartRxDataSim >= 'A') && (UartRxDataSim <= 'F' ))
                            {
                                AddrNibble = UartRxDataSim - 'A' + 0xA;
                                UART_UartPutChar (UartRxDataSim);
                            }
                             else if ((UartRxDataSim >= 'a') && (UartRxDataSim <= 'f' ))
                            {
                                AddrNibble = UartRxDataSim - 'a' + 0xA;
                                UART_UartPutChar (UartRxDataSim);
                            }
                            else 
                            {
                                printf ("\nplease Enter a Valid Address. Press A to Enter a New Address. R ro remove the Device\r\n");
                                Count = 0;
                                break;
                            }
                            
                            //Receiving the addresss Nibble by Nibble
                            whitelistdeviceaddress.bdAddr[5 - (Count/2)] =
                            (whitelistdeviceaddress.bdAddr[5 - (Count/2)]<<4)|AddrNibble;
                            Count ++;
                        }
                    }
                }
            }
            
            else if (UartRxDataSim == 'R' || UartRxDataSim == 'r')
            {
                if (Index == 0)
                {
                    printf ("No Devices in WhiteList. press A to Add \r\n");
                   
                }
                else
                {
                    printf (" The List of Devices are given below \4\n");
                    uint8 i = 0;
                    // Retrieving the list of added devices for user to choose
                    for (i = 0; i< Index; i++)
                    {
                        printf ("Device %d 0x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",i + 1,
                            whitelistdeviceaddressBackup[i].bdAddr[5],
                            whitelistdeviceaddressBackup[i].bdAddr[4],
                            whitelistdeviceaddressBackup[i].bdAddr[3],
                            whitelistdeviceaddressBackup[i].bdAddr[2],
                            whitelistdeviceaddressBackup[i].bdAddr[1],
                            whitelistdeviceaddressBackup[i].bdAddr[0]);
                    }
                    printf ("Enter the Index of the device to be removed. Press Z to go back \r\n");
                    
                    for (;;)
                    {
                        if(UART_SpiUartGetRxBufferSize())
                        {
                            UartRxDataSim = UART_UartGetChar();
                            if (UartRxDataSim == 'Z' || UartRxDataSim == 'z')  
                            {
                                printf("Press A to add a Device to WhiteList. R to remove \r\n");
                                break;
                            }
                            else if (UartRxDataSim >= '1' || UartRxDataSim <= '0' + Index)
                            {
                                RemoveIndex = UartRxDataSim - '1';
                                if(RemoveIndex < Index)
                                {
                                    CyBle_GappStopAdvertisement ();
                                /*Once We stop advertisement, the 
                                CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP event is invoked.
                                After this, the API for removing the device from whitelist 
                                is invoked in the StackEventHandler*/
                                    DelRequest = 1;
                                    break;
                                }
                                else
                                {
                                    printf("There is no device with that number.\r\n");
                                }
                            }
                            else 
                            {
                                printf ("Invaid Index. Press A to Add and R to remove a Device");
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

/* [] END OF FILE */
