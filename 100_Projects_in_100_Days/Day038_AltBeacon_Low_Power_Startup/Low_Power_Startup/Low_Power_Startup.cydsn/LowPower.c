/******************************************************************************
* Project Name		: Low_Power_Startup
* File Name			: LowPower.c
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
*	Description - Routines to configure PSoC 4 BLE system in low power modes 
********************************************************************************/

#include <Application.h>
#include <Configuration.h>
#include <Project.h>

/*******************************************************************************
* Function Name: System_ManagePower()
********************************************************************************
*
* Summary:
*   This function puts the system in appropriate low power modes based on the 
*   state of BLESS and application power state.
*
*   Note that this API is designed to be reused as-is irrespective of what 
*   your end application is. Based on the application or peripheral requirements
*   in your end design, update the Application_GetPowerState APIs return value
*   to control system low power mode configuration.
*
* Parameters:
*  None
*
* Return: 
*  None
********************************************************************************/
void System_ManagePower()
{
    /* Variable declarations */
    CYBLE_BLESS_STATE_T blePower;
    uint8 interruptStatus ;
    
   /* Disable global interrupts to avoid any other tasks from interrupting this section of code*/
    interruptStatus  = CyEnterCriticalSection();
    
    /* Get current state of BLE sub system to check if it has successfully entered deep sleep state */
    blePower = CyBle_GetBleSsState();
    
    /* System can enter DeepSleep only when BLESS and rest of the application are in DeepSleep or equivalent
     * power modes */
    if((blePower == CYBLE_BLESS_STATE_DEEPSLEEP || blePower == CYBLE_BLESS_STATE_ECO_ON) && 
        Application_GetPowerState() == DEEPSLEEP)
    {
        Application_SetPowerState(WAKEUP_DEEPSLEEP);
        
#if DEBUG_ENABLE
        DeepSleep_Write(1);
#endif /* End of #if DEBUG_ENABLE */
        
        CySysPmDeepSleep();
        
#if DEBUG_ENABLE
        DeepSleep_Write(0);
#endif /* End of #if DEBUG_ENABLE */
    }
    else if((blePower != CYBLE_BLESS_STATE_EVENT_CLOSE))
    {
        if(Application_GetPowerState() == DEEPSLEEP)
        {
            Application_SetPowerState(WAKEUP_DEEPSLEEP);
            
            /* change HF clock source from IMO to ECO, as IMO can be stopped to save power */
            CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO); 
            /* stop IMO for reducing power consumption */
            CySysClkImoStop(); 
            /* put the CPU to sleep */
#if DEBUG_ENABLE
            Sleep_Write(1);
#endif /* End of #if DEBUG_ENABLE */      

            CySysPmSleep();
            
#if DEBUG_ENABLE
            Sleep_Write(0);
#endif /* End of #if DEBUG_ENABLE */           
            /* starts execution after waking up, start IMO */
            CySysClkImoStart();
            /* change HF clock source back to IMO */
            CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_IMO);
        }
        else if(Application_GetPowerState() == SLEEP )
        {
            /* If the application is using IMO for its operation, we shouldn't switch over the HFCLK source */
            Application_SetPowerState(WAKEUP_SLEEP);
            
#if DEBUG_ENABLE
            Sleep_Write(1);
#endif /* End of #if DEBUG_ENABLE */     

            CySysPmSleep();
            
#if DEBUG_ENABLE
            Sleep_Write(0);
#endif /* End of #if DEBUG_ENABLE */
        }
    }
    
    /* Enable interrupts */
    CyExitCriticalSection(interruptStatus );
}

/* [] END OF FILE */
