/******************************************************************************
* Project Name		: PSoC4_BLE_RTC
* File Name			: LowPower.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: kris@cypress.com
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
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
*******************************************************************************
******************************************************************************
*                           Description
*******************************************************************************
* This file contains routines for configuring the device in low power mode.
*
*******************************************************************************/

#include <Configuration.h>
#include <Project.h>

/*******************************************************************************
* Function Name: System_ManagePower
********************************************************************************
*
* Summary:
*  configures the peripherals used in the design and the device into possible
*  low power modes based on the state of the system.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
void System_ManagePower (void)
{
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
    uint32 intStatus;
    
    lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP); /* try configuring BLESS in DeepSleep mode */
        
    intStatus = CyEnterCriticalSection();
    
    blessState = CyBle_GetBleSsState(); /* Check the current state of the BLESS */

    if(lpMode == CYBLE_BLESS_DEEPSLEEP) /* BLESS had entered low power mode */
    {   
        /* and it continues to be in a state where BLESS can enter DeepSleep mode */
        if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
        {
            #if(CONSOLE_LOG_ENABLED)
                /* Put the device into the Dee Sleep mode only when all debug information has been sent */
            if((Debug_Console_SpiUartGetTxBufferSize() + Debug_Console_GET_TX_FIFO_SR_VALID) == 0u)
            {
                CySysPmDeepSleep();
            }
            else
            {
                /* Wait for UART interface to finish data transfer */
                CySysPmSleep();
            }
            #else
            CySysPmDeepSleep();    
            #endif    
        }
    }
    else
    {
        /* BLESS can't enter DeepSleep as next BLE connection interval is close by, go to Sleep for now */
        if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
        {
            CySysPmSleep();
        }
    }
    
    CyExitCriticalSection(intStatus);
}

/* [] END OF FILE */
