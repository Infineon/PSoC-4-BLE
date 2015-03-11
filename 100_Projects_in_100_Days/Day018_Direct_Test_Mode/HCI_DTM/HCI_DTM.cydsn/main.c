/******************************************************************************
* Project Name		: HCI_DTM
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 CP1
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
**********************************************************************************/

/*********************************************************************************
*                           THEORY OF OPERATION
**********************************************************************************
* This project operates in BLE specification defined HCI mode and executes the Direct
* Test Mode (DTM) cases. There is no application level activity except processing
* BLE events. This project requires the external CBT tester to operate.
* Refer CY8CKIT-042-BLE Pioneer Kit user guide for more details. 
**********************************************************************************
* Hardware connection required for testing -
* UART RX 	- P0[0] (Connect this pin from J3 header on BLE Pioneer Kit to UART TX
					 line of external RS232 converter)
* UART TX	- P0[1] (Connect this pin from J3 header on BLE Pioneer Kit to UART RX
					 line of external RS232 converter)
* RTS 		- P0[2] (Connect this pin from J3 header on BLE Pioneer Kit to RTS
					 line of external RS232 converter; can be left unconnected)
* CTS		- P0[3] (Connect this pin from J3 header on BLE Pioneer Kit to CTS
					 line of external RS232 converter; to be connected to GND if not
					 used)
**********************************************************************************/
#include <project.h>

/* BLE Callback Event Handler Function */
void GenericEventHandler(uint32 event, void * eventparam);

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        System entrance point. This calls the BLE start and processes BLE Events
*
* Parameters:
*  void
*
* Return:
*  int
*

*******************************************************************************/
int main()
{
	/* Enable global interrupts. */
    CyGlobalIntEnable; 
	
	/* Start BLE Component and register the event callback function */
	CyBle_Start(GenericEventHandler);
	
    for(;;)
    {
        /* Process BLE events continuously */
		CyBle_ProcessEvents();
    }
}

/*******************************************************************************
* Function Name: GenericEventHandler
********************************************************************************
* Summary:
*        Call back event function to handle various events from BLE stack. Not 
* used for HCI mode as BLE stack handles all events.
*
* Parameters:
*  event:		event returned
*  eventParam:	link to value of the events returned
*
* Return:
*  void
*
*******************************************************************************/
void GenericEventHandler(uint32 event, void * eventparam)
{
	switch(event)
	{
		/* No application event is generated in HCI Mode. All commands
		* are processed by BLE stack */

		default:
			/* To prevent compiler warning */
			eventparam = eventparam;
		break;
	}
}
/* [] END OF FILE */
