/*******************************************************************************
* File Name: low_power.c
*
* Version: 1.0
*
* Description:
* This file contains the definiton for funtion to allow low power mode
* operation.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <main.h>

uint8 shut_down_led;
/*******************************************************************************
* Function Name: HandleLowPowerMode
********************************************************************************
* Summary:
*        This function puts the BLESS in deep sleep mode and CPU to sleep mode 
* or deep sleep mode. System will resume from here when it wakes from any interrupt.
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
		
		/* Process Pending BLE Events */
		CyBle_ProcessEvents();
		
		/* Put BLESS into Deep Sleep and check the return status */
		sleepMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
		
		/* Disable global interrupt to prevent changes from any other interrupt ISR */
		CyGlobalIntDisable;
	
		/* Check the Status of BLESS */
		blessState = CyBle_GetBleSsState();

		if(sleepMode == CYBLE_BLESS_DEEPSLEEP)
		{
		    /* If the ECO has started or the BLESS can go to Deep Sleep, then place CPU 
			* to Deep Sleep */
			if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
		    {
				if(shut_down_led)
				{
					/* If PrISM is not acive, put PrISM components to sleep */
					PrISM_1_Sleep();
					PrISM_2_Sleep();
						
					#if (DEBUG_ENABLED == 1)
						/* Put UART component to sleep */
						UART_Sleep();
					#endif
					
			        CySysPmDeepSleep();
					
					#if (DEBUG_ENABLED == 1)
						/* Wakeup UART components after system wakeup */
						UART_Wakeup();
					#endif
					
					/* After system wakes up, wake up the PrISM components */
					PrISM_1_Wakeup();
					PrISM_2_Wakeup();
				}
				else
				{
					/* If the system is controlling RGB LED, then PrISM needs to be running. 
					* Put CPU to sleep only */
					#if 0
					CySysClkWriteImoFreq(IMO_FREQUENCY_3MHZ);
					#endif
					CySysPmSleep();
					#if 0
					CySysClkWriteImoFreq(IMO_FREQUENCY_12MHZ);
					#endif
				}
		 	}
		}
		else
		{
		    if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
		    {
				/* If the BLESS hardware block cannot go to Deep Sleep and BLE Event has not 
				* closed yet, then place CPU to Sleep */
		        CySysPmSleep();
		    }
		}
		
		/* Re-enable global interrupt mask after wakeup */
		CyGlobalIntEnable;
		
	#endif
}
/* [] END OF FILE */
