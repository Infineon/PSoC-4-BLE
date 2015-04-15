/******************************************************************************
* Project Name		: Low_Power_Startup
* File Name			: Application.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP2
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
*******************************************************************************/

/********************************************************************************
* Description - Contains your application code and APIs for power management 
* of peripherals used in your application
********************************************************************************/

#include <Application.h>
#include <LowPower.h>

/******************************************************************************
*                           Global variables
*******************************************************************************/
POWER_MODE_T applicationPowerState = ACTIVE;

/*******************************************************************************
* Function Name: Application_ManagePower
********************************************************************************
*
* Summary:
* This function manages the different power modes and transition between the
* power modes based on the state of the application and peripherals
*
* Parameters:  
*  None
*
* Return: 
*  None
*******************************************************************************/
void Application_ManagePower(void)
{
    switch(applicationPowerState)
    {
        case ACTIVE: 
        case SLEEP: 
        case DEEPSLEEP:
            /* No application layer tasks for this example project */
        break;
        
        case WAKEUP_SLEEP: 
        case WAKEUP_DEEPSLEEP: 
            applicationPowerState = ACTIVE; /* After wakeup, set the application state to active */
        break;
        
        default:
        break;
    }
}

/*******************************************************************************
* Function Name: Application_Run
********************************************************************************
*
* Summary:
* This is the main application run loop. Most of the non-BLE part of the
* application must be done within this function. A template implementation for 
* your application is provided for now.
*
* Parameters:  
*  None
*
* Return: 
*  None
*******************************************************************************/
void Application_Run(void)
{
    /***********************************************************************
    * There's no application specific code for AltBeacon. It's a simple
    * broadcaster and hence this routine is almost empty.
    ************************************************************************/  
   
#if(CONSOLE_LOG_ENABLE)
    /* Put the device into the DeeSleep mode only when all the console information has been sent */
    if((Console_SpiUartGetTxBufferSize() + Console_GET_TX_FIFO_SR_VALID) == 0u)    
    {
        applicationPowerState = DEEPSLEEP;
    }
    else
    {
        applicationPowerState = SLEEP;
    }
#else  
    applicationPowerState = DEEPSLEEP;
#endif /* End of #if(CONSOLE_LOG_ENABLE) */

}

/*******************************************************************************
* Function Name: Application_GetPowerState
********************************************************************************
*
* Summary:
* Returns if the application is active, waiting to be or in Sleep or DeepSleep
* mode
*
* Parameters:  
*  None
*
* Return: 
*  Application power state
*******************************************************************************/
POWER_MODE_T Application_GetPowerState(void)
{
    return applicationPowerState;
}

/*******************************************************************************
* Function Name: Application_SetPowerState
********************************************************************************
*
* Summary:
* Sets the state of the application power mode. This function will be used by
* the peripherals or application run loop or system power manager to indicate
* a power state change
*
* Parameters:  
*  power state to be set
*
* Return: 
*  None
*******************************************************************************/
void Application_SetPowerState(POWER_MODE_T pwrState)
{
    applicationPowerState = pwrState;
}

/* [] END OF FILE */
