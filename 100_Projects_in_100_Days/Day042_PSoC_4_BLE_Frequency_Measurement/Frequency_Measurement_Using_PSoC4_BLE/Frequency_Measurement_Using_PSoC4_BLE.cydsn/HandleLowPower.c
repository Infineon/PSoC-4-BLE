/***************************************************************************//**
* @file   HandleLowPower.c
* @version 1.0
* @author  Chethan (dche@cypress.com)
*
* @par Description
*   This file contains the source code to the APIs for implementing low power modes.
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 BLE Pioneer Kit
*
* @par References
*    1. 001-32359: AN2283 "PSoC 1 Measuring Frequency"
*
* @par Code Tested With
*    1. PSoC Creator  3.1 SP1 (3.1.0.1722)
*    2. GCC 4.7.3
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
* Contains the function for handling Low power mode (Deep Sleep/Sleep) in 
* firmware.
******************************************************************************/
#include <main.h>

/*******************************************************************************
* Function Name: HandleLowPowerMode
********************************************************************************
* Summary:
*        This function puts the BLESS in deep sleep mode and CPU to sleep mode. 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleLowPowerMode(void)
{
	#ifdef ENABLE_LOW_POWER_MODE
		/* Local variable to store the status of BLESS Hardware block */
		CYBLE_LP_MODE_T sleepMode;
		CYBLE_BLESS_STATE_T blessState;
		
		/* Put BLESS into Deep Sleep and check the return status */
		sleepMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
		
		CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO);		
		/* Disable global interrupt to prevent changes from any other interrupt ISR */
		//CyGlobalIntDisable;
	
		/* Check the Status of BLESS */
		blessState = CyBle_GetBleSsState();

//		if(sleepMode == CYBLE_BLESS_DEEPSLEEP)
//		{
//		    /* If the ECO has started or the BLESS can go to Deep Sleep, then place CPU 
//			* to Deep Sleep */
//			if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
//		    {
//		        CySysPmDeepSleep();
//		 	}
//		}
//		else
//		{
//		    if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
//		    {
//				/* If the BLESS hardware block cannot go to Deep Sleep and BLE Event has not 
//				* closed yet, then place CPU to Sleep */
//		        CySysPmSleep();
//		    }
//		}
		
		/* Re-enable global interrupt mask after wakeup */
		//CyGlobalIntEnable;
	#endif
}
/* [] END OF FILE */
