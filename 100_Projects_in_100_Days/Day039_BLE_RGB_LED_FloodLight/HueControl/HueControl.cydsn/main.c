/******************************************************************************
* Project Name		: HueControl
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : PRIA
*
********************************************************************************
* Copyright (2015-16), Cypress Semiconductor Corporation. All Rights Reserved.
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
* This Project showcases the use of BLE to control a 10W RGB Flood Light. 
* The flood light used with this project comes with an IR Remote Control.This 
* project replaces the IR Remote control with a BLE control, thus allowing
* the use of CySmart on laptop to control the color and intensity of the RGB 
* flood light.
* Note that this project just replaces communication over IR to BLE but still
* controls the flood light by sending same NEC commands as were sent by the 
* IR Remote. This was done in order to retain the same LED drivers that came 
* with the flood light; however, it is also possible to replace the driver
* circuit completely with PSoC4BLE and external current amplifiers.
*
*******************************************************************************
* Hardware connection required for testing -
* NEC pin   	- P0[0] (to be connected to NEC data pin on RGB Flood Light)
* Blue LED 	    - P3[7] (hard-wired on the BLE Pioneer kit)
******************************************************************************/
#include <main.h>

/* This flag is used by application to know whether a Central 
* device has been connected. This is updated in BLE event callback 
* function 
*/
extern uint8 deviceConnected;

/* 'restartAdvertisement' flag is used to restart advertisement */
extern uint8 restartAdvertisement;

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        System entrance point. This calls the initializing function and
* continuously process BLE events.
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

    /* This function will initialize the system resources such as BLE and 
     * NEC pulse timer 
     */
    InitializeSystem();
	
    for(;;)
    {
        /* Process event callback to handle BLE events. The events generated and 
		 * used for this application are inside the 'CustomEventHandler' routine
         */
        CyBle_ProcessEvents();
		
		/* Update LED for status during BLE active states */
		HandleStatusLED();
		
		if(TRUE == deviceConnected)
		{
			/* After the connection, send new connection parameter to the Client device 
			* to run the BLE communication on desired interval. This affects the data rate 
			* and power consumption. High connection interval will have lower data rate but 
			* lower power consumption. Low connection interval will have higher data rate at
			* expense of higher power. This function is called only once per connection. 
            */
			UpdateConnectionParam();
		}
		
		if(restartAdvertisement)
		{
			/* Reset 'restartAdvertisement' flag*/
			restartAdvertisement = FALSE;
			
			/* Start Advertisement and enter Discoverable mode*/
			CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);	
		}
    }	/* End of for(;;) */
}

/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
* Summary:
*        Start the components and initialize system 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitializeSystem(void)
{
	/* Enable global interrupt mask */
	CyGlobalIntEnable; 
			    
	/* Start BLE component and register the CustomEventHandler function. This 
	* function exposes the events from BLE component for application use */
    CyBle_Start(CustomEventHandler);
	
	/* Initialize NEC pin to high state */
    NecOutPin_Write(TRUE);

}

/* [] END OF FILE */