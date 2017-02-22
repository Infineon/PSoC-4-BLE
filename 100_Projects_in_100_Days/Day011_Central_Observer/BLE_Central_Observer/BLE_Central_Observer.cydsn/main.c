/******************************************************************************
* Project Name		: BLE_Central_Observer
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
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
*A BLE Central is a device that initiates connections to Peripherals and will 
*therefore become a master when connected. BLE Observer is one which scans for 
*Broadcasters (devices that send advertising events over BLE) and reports the
*received information to an application. The BLE Observer role does not allow
*transmissions. In this example the BLE Component is configured to work as both 
*a BLE Central and BLE Observer. It keeps scanning for the Peripherals and 
*Broadcasters and displays the information from the Advertisement and Scan 
*response packets received. The user can initiate a connection with any of
*the detected devices any time or disconnect an existing connection. The user 
*enters the input via UART terminal. This example can handle upto 10 peripherals 
*or broadcastres advetising at the same time.
******************************************************************************/

#include <project.h>
#include "stdio.h"
#include "BLE_Central_Observer.h"
#include "main.h"

uint32  UartRxDataSim;  // Character Input Received from the UART Terminal for Initiating Connection / Disconnection
uint8 devIndex;         // Index of the devices detected in Scanning
uint8 Periph_Selected;  // The Index of the Peropheral which the user wants to connect the Central 
uint8 IsSelected;       // Whether user has given Periph_Selected
uint8 IsDetected;       // Whether any device has been detected in Scanning
uint8 IsConnected;      // Whether the Central is in connected state or not.
CYBLE_CONN_HANDLE_T			connHandle; // Handle of the device connected.
CYBLE_API_RESULT_T 		    apiResult;

/*******************************************************************************
* Function Name: BLE_Central_Observer_init
********************************************************************************
*
* Summary:
* Starts BLE and UART Components and enables Global Interrupt
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/

void BLE_Central_Observer_init(void)
{
	ALL_LED_OFF();
    
	CyGlobalIntEnable; /* Enabling Global interrupts */
    
    /* Start BLE component and register Event handler function */
	CyBle_Start (StackEventHandler);	
	
	/* Start UART Component which is used for receiving inputs and Debugging */
    UART_Start();
	printf("BLE Central + Observer Example \r\n");
	
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
    IsSelected = 0;
    Periph_Selected = 0;
    IsConnected = 0;
    devIndex = 0;
    
    /* Initialize the system */
	BLE_Central_Observer_init();
	
    /* Continuous loop scans for inputs from UART Terminal and accordingly 
       handles conneciton, disconnection and scanning. Also processes
        BLE events */
    for(;;)
    {
		//Checks the internal task queue in the BLE Stack
	    CyBle_ProcessEvents();
        
        if(UART_SpiUartGetRxBufferSize())
		{
            UartRxDataSim = UART_UartGetChar();
            // The user has to Enter D for disconnection  
            if (UartRxDataSim == 'D' || UartRxDataSim == 'd')  // The user has to Enter D for disconnection 
            {
                printf ("Attempting Disconnection\r\n");
                CyBle_GapDisconnect(connHandle.bdHandle);
                IsConnected = 0;
                devIndex= 0;
            }
            
            else
            {
                // Check if a Valid Device index has been received
                if ((UartRxDataSim - '0' < devIndex) && (IsDetected))  
                {
                    Periph_Selected = (uint8)(UartRxDataSim - '0');
                    IsSelected = 1;
                    //Stop the scanning before connecting to the preferred peripheral
                    if(CyBle_GetState() != CYBLE_STATE_CONNECTING)
                    CyBle_GapcStopScan();
                    else
                        printf("Trying to connect to previous device.\r\n");
                    
                    if (IsConnected)
                    {
                      // Disconnect if another connection already exists
                        apiResult =    CyBle_GapDisconnect(connHandle.bdHandle); 
                        if(CYBLE_ERROR_OK != apiResult )
    				    {
    				       printf ("Error. Cannot Disconnect a previous Connection \r\n"); 
                        }
                        else
                        {
                            printf ("Disconnecting Previous Connection and Trying to connect to Device No %d \r\n",Periph_Selected);
                        }
                    } 
                }
                
             else
                {
                    printf ("Invalid Input \r\n");
                }
            }
		}
        //Function to handle connection and disconnecton
		Handle_ble_Central_Observer_State();
    }
}

/* [] END OF FILE */
