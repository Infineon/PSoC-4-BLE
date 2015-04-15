/******************************************************************************
* Project Name		: Low_Power_Startup
* File Name			: WatchdogTimer.c
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
*	Description - Watchdog timer initialization and interrupt handler source code.
*   Watchdog timer is clocked by LFCLK and is typically used in most of the 
*   application as a source of wakeup for PSoC 4 BLE device from Sleep or DeepSleep
*   modes.
********************************************************************************/
#include <Configuration.h>
#include <Project.h>
#include <WatchdogTimer.h>

#if LOW_POWER_STARTUP_ENABLE
/*******************************************************************************
* Function Name: WDT_Handler
********************************************************************************
*
* Summary:
*  Watchdog timer(WDT) interrupt handler routine. WDT is only used to wakeup 
*  the device after WCO or ECO has started
*
* Parameters:  
*  None
*
* Return: 
*  None
*******************************************************************************/    
CY_ISR(WDT_Handler)
{
    /* WDT interrupt is only used as a source of wakeup for PSoC 4 BLE device 
     * in this example project. Clearing the WDT interrupt status is all that is 
     * required in this example project */
    
    if(CySysWdtGetInterruptSource() & WDT_INTERRUPT_SOURCE)
    {
        CySysWdtClearInterrupt(WDT_INTERRUPT_SOURCE);
    }
    
    if(CySysWdtGetInterruptSource() & ECO_INTERRUPT_SOURCE)
    {
        CySysWdtClearInterrupt(ECO_INTERRUPT_SOURCE);
    }
}

/*******************************************************************************
* Function Name: WDT_WcoEcoLpStartSetup
********************************************************************************
*
* Summary:
*  Configures the watchdog counter 0 and 1 count and interrupt period as per the 
*  PSoC 4 BLE WCO and ECO startup time.
*
* Parameters:  
*  None
*
* Return: 
*  None
*******************************************************************************/ 
void WDT_WcoEcoLpStartSetup(void)
{
    CySysWdtUnlock(); /* Unlock the WDT registers for modification. Note that WDT is running from ILO now */
    
    CySysWdtWriteMode(WCO_COUNTER, CY_SYS_WDT_MODE_INT); /* configure counter0 & 1 in interrupt mode */
    CySysWdtWriteMode(ECO_COUNTER, CY_SYS_WDT_MODE_INT);
    
    CySysWdtWriteClearOnMatch(WCO_COUNTER, COUNTER_ENABLE);
    CySysWdtWriteClearOnMatch(ECO_COUNTER, COUNTER_ENABLE);
    
    CySysWdtWriteMatch(WCO_COUNTER, WCO_COUNT_PERIOD); /* counter 0 = 500ms, counter 1 = 1ms period */
    CySysWdtWriteMatch(ECO_COUNTER, ECO_COUNT_PERIOD);
    
    CySysWdtLock();
}

/*******************************************************************************
* Function Name: WDT_EnableWcoCounter
********************************************************************************
*
* Summary:
*  Enables WDT counter 0
*
* Parameters:  
*  None
*
* Return: 
*  None
*******************************************************************************/
void WDT_EnableWcoCounter(void)
{
    CySysWdtUnlock();
    
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    
    CySysWdtLock();
}

/*******************************************************************************
* Function Name: WDT_EnableEcoCounter
********************************************************************************
*
* Summary:
*  Enables WDT counter 1
*
* Parameters:  
*  None
*
* Return: 
*  None
*******************************************************************************/
void WDT_EnableEcoCounter(void)
{
    CySysWdtUnlock();
    
    CySysWdtEnable(CY_SYS_WDT_COUNTER1_MASK);
    
    CySysWdtLock();
}

/*******************************************************************************
* Function Name: WDT_DisableWcoEcoCounters
********************************************************************************
*
* Summary:
*  Disable WDT counter 0 & 1
*
* Parameters:  
*  None
*
* Return: 
*  None
*******************************************************************************/
void WDT_DisableWcoEcoCounters(void)
{
    CySysWdtUnlock();
    
    CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK); 
    CySysWdtDisable(CY_SYS_WDT_COUNTER1_MASK);
    
    CySysWdtLock();
}

#endif /* End of #if LOW_POWER_STARTUP_ENABLE */

/* [] END OF FILE */
