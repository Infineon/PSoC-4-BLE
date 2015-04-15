/******************************************************************************
* Project Name		: Low_Power_Startup
* File Name			: Init.c
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
*	Description - This file contains system initialization routine
********************************************************************************/

#include <BLE Interface.h>
#include <Configuration.h>
#include <LowPower.h>
#include <Project.h>
#include <WatchdogTimer.h>

#if LOW_POWER_STARTUP_ENABLE
/***************************************
*    Function declarations
***************************************/
void WCO_ECO_LowPowerStart(void);    
#endif /* End of #if LOW_POWER_STARTUP_ENABLE */

/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
*
* Summary:
*  Systm initialization function.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
void InitializeSystem(void)
{
    CyGlobalIntEnable;      /* Enable Global Interrupts*/
    
    /* Set the divider for ECO, ECO will be used as source when IMO is switched off to save power */
    CySysClkWriteEcoDiv(CY_SYS_CLK_ECO_DIV8);

#if LOW_POWER_STARTUP_ENABLE
        
    /* If LOW_POWER_STARTUP_ENABLE is set, then do the following for achieving lowest possible WCO & ECO startup current:
     * 1. Shut down the ECO (to reduce power consumption while WCO is starting)
     * 2. Enable WDT counter 0 to wakeup the system after 500ms (500ms = WCO startup time)
     * 3. Configure PSoC 4 BLE device in DeepSleep mode for the 500ms WCO startup time
     * 4. After WCO is enabled, restart the ECO so that BLESS interface can function
     * 5. Enable WDT counter 1 to wakeup the system after 1ms (1ms = ECO startup time)
     * 5. Configure PSoC 4 BLE device in DeepSleep mode for the 1ms ECO startup time */
    
    CySysClkEcoStop();      /* Shutdown the ECO and later re-start in low power mode after WCO is turned on */
    
    WDT_Interrupt_StartEx(WDT_Handler); /* Initialize WDT interrupt */
    
    WCO_ECO_LowPowerStart();    /* Enable WCO & ECO in low power mode using WDT counter 0/1 as system wakeup sources respectively */
    
#endif /* End of #if LOW_POWER_STARTUP_ENABLE */
    
#if (CONSOLE_LOG_ENABLE)
    Console_Start();  /* Console log interface */
    Console_UartPutString("Low power system startup complete\r\n"); 
#endif /* End of #if (CONSOLE_LOG_ENABLE) */

    BLE_Engine_Start();     /* start the BLE interface */
    
    /* Wait for BLE Component to Initialize */
    while (CyBle_GetState() == CYBLE_STATE_INITIALIZING)
    {
        CyBle_ProcessEvents(); 
    }
}

#if LOW_POWER_STARTUP_ENABLE
/*******************************************************************************
* Function Name: WCO_LowPowerStart
********************************************************************************
* Summary:
*  Start WCO in low power mode by configuring the system in DeepSleep mode during
*  WCO startup time(500ms)   
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
void WCO_ECO_LowPowerStart(void)
{
    WDT_WcoEcoLpStartSetup();   /* Setup WDT counters to enable low power WCO & ECO startup */

    CySysClkWcoStart();         /* Start the WCO clock */
    
    WDT_EnableWcoCounter();     /* Enable WDT's WCO counter (counter 0) */
    
#if DEBUG_ENABLE                    
    DeepSleep_Write(1);
#endif /* End of #if DEBUG_ENABLE */
    CySysPmDeepSleep(); /* Wait for the WDT counter 0 interrupt to wake up the device. On wakeup WCO is up & running */
#if DEBUG_ENABLE                    
    DeepSleep_Write(0);
#endif /* End of #if DEBUG_ENABLE */

    (void)CySysClkWcoSetPowerMode(CY_SYS_CLK_WCO_LPM);      /* Switch WCO to the low power mode after startup */
    CySysClkSetLfclkSource(CY_SYS_CLK_LFCLK_SRC_WCO);       /* LFCLK is now driven by WCO */
    CySysClkIloStop();                                      /* WCO is running, shut down the ILO */

    (void)CySysClkEcoStart(0);  /* It's time to start ECO */

    WDT_EnableEcoCounter();     /* Enable WDT's ECO counter (counter 1) */
    
#if TIMING_DEBUG_ENABLE                    
    DeepSleep_Write(1);
#endif    
    CySysPmDeepSleep();  /* Wait for the WDT counter 1 interrupt to wake up the device. On wakeup ECO is up & running */
#if TIMING_DEBUG_ENABLE                    
    DeepSleep_Write(0);
#endif

    WDT_DisableWcoEcoCounters();
}

#endif /*End of #if LOW_POWER_STARTUP_ENABLE */

/* [] END OF FILE */
