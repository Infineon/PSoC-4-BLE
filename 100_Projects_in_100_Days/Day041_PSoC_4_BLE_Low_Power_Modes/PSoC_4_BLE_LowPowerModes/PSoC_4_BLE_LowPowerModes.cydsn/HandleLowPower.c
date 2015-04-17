/******************************************************************************
* Project Name		: PSoC_4_BLE_LowPowerModes
* File Name			: HandleLowPower.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : ANKU
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

extern uint8 shutDownLed;
/*******************************************************************************
* Function Name: HandleLowPowerMode
********************************************************************************
* Summary:
*        This function puts the BLESS in deep sleep mode and CPU to sleep mode. 
* System will resume from here when it wakes from user button press.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleLowPowerMode(uint8 lpmSel)
{
	#ifdef ENABLE_LOW_POWER_MODE
		/* Local variable to store the status of BLESS Hardware block */
		CYBLE_LP_MODE_T sleepMode;
		CYBLE_BLESS_STATE_T blessState;

        if (lpmSel == DEEPSLEEP)
        {
            /* Leave chip in Deep Sleep mode */
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
    				/* Place CPU to Deep sleep */
                    #ifdef ENABLE_LED_NOTIFICATION
                    GREEN_SetDriveMode(GREEN_DM_STRONG);
                    GREEN_Write(0);
                    #endif
                    
    		        CySysPmDeepSleep();
                    
                    #ifdef ENABLE_LED_NOTIFICATION
                    GREEN_SetDriveMode(GREEN_DM_STRONG);
                    GREEN_Write(1);
                    #endif
    		 	}
    		}
    		
    		/* Re-enable global interrupt mask after wakeup */
    		CyGlobalIntEnable;        
        }
        
        else if (lpmSel == SLEEP)
        {
            /* Leave chip in Sleep mode */
            /* Leave chip in Deep Sleep mode */
    		/* Put BLESS into Deep Sleep and check the return status */
    		sleepMode = CyBle_EnterLPM(CYBLE_BLESS_SLEEP);
    		
    		/* Disable global interrupt to prevent changes from any other interrupt ISR */
    		CyGlobalIntDisable;
    	
    		/* Check the Status of BLESS */
    		blessState = CyBle_GetBleSsState();

    		if(sleepMode == CYBLE_BLESS_SLEEP)
    		{
    		    if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
    		    {
    				/* If BLE Event has not 
    				* closed yet, then place CPU to Sleep */
                    #ifdef ENABLE_LED_NOTIFICATION                   
                    BLUE_Write(0);
                    BLUE_SetDriveMode(BLUE_DM_STRONG);
                    #endif
                    
    		        CySysPmSleep();
                    
                    #ifdef ENABLE_LED_NOTIFICATION
                    BLUE_SetDriveMode(BLUE_DM_STRONG);
                    BLUE_Write(1);
                    #endif
    		    }
    		}
    		
    		/* Re-enable global interrupt mask after wakeup */
    		CyGlobalIntEnable;        
        }
        else if (lpmSel == ACTIVE)
        {
            /* Leave chip in Active mode */
            #ifdef ENABLE_LED_NOTIFICATION
            RED_Write(0);
            RED_SetDriveMode(RED_DM_STRONG);
            #endif
        }
	#endif
}

/*******************************************************************************
* Function Name: MyISR
********************************************************************************
* Summary:
*        ISR routine for isr_button. System enters here after CPU wakeup.
* Clears pending interrupt.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
CY_ISR(MyISR)
{
	/* Clear Pending interrupts */
	isr_button_ClearPending();
	
	User_Button_ClearInterrupt();
}
/* [] END OF FILE */
