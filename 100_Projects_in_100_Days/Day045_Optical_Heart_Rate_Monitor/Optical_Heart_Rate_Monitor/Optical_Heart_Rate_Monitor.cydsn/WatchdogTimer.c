/*****************************************************************************
* File Name: WatchdogTimer.c
*
* Version: 1.0
*
* Description:
* This file implements the watchdog timer for a deep-sleep wakeup source, 
* and also to keep the timestamp.
*
* Note:
* 
* Owner:
* UDYG
*
* Related Document:
* PSoC 4 BLE Heart Rate Monitor Solution Demo IROS: 001-92353
*
* Hardware Dependency:
* PSoC 4 BLE HRM Solution Demo Board
*
* Code Tested With:
* 1. PSoC Creator 3.1
* 2. ARM-GCC Compiler
*
******************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
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
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/


/*****************************************************************************
* Included headers
*****************************************************************************/
#include <cytypes.h>
#include <cyfitter.h>
#include "CyLib.h"


/*****************************************************************************
* Macros and constants
*****************************************************************************/
#define INTERRUPT_PRIORITY_3                        (3)
#define WATCHDOG_DEFAULT_PERIOD_MS                  (20)

/* Default for WCO (PSoC4A-BLE), Nominal for ILO (PSoC4A) */
#define WATCHDOG_TICKS_PER_MS                       (32)

#define CLK_ILO_CONFIG_ILO_ENABLE                   (1u << 31)
#define WDT_INTERRUPT_NUM                           (8) /* For PSoC4A-BLE */
#define WDT_CONFIG_WDT_MODE0_INT                    ((uint32)1u << 0)
#define WDT_CONTROL_WDT_ENABLE0                     (1u << 0)
#define WDT_CONTROL_WDT_ENABLED0                    (1u << 1)
#define WDT_CONTROL_WDT_RESET0                      (1u << 3)
#define CLK_SELECT_WDT_LOCK_SET01                   ((1u << 14) | (1u << 15))
#define CLK_SELECT_WDT_LOCK_CLR0                    (1u << 14)
#define CLK_SELECT_WDT_LOCK_CLR1                    (1u << 15)
#define WDT_CONTROL_WDT_INT0                        (1u << 2)
#define WATCHDOG_COUNT_INIT                         (3)
#define UPDATE_WDT_MATCH(value)     CY_SET_REG32(CYREG_WDT_MATCH, \
                                                 (CY_GET_REG32(CYREG_WDT_MATCH) & 0xFFFF0000) + (uint16)value)


/*****************************************************************************
* Global variables
*****************************************************************************/
volatile static uint32 watchdogPeriodMs = WATCHDOG_DEFAULT_PERIOD_MS;
volatile static uint32 nextTicks = WATCHDOG_DEFAULT_PERIOD_MS * WATCHDOG_TICKS_PER_MS;
volatile static uint32 watchdogTimestamp = 0;


/*****************************************************************************
* Function Definitions
*****************************************************************************/

/*****************************************************************************
* Static Functions
*****************************************************************************/

/*****************************************************************************
* Function Name: WatchdogTimer_Lock()
******************************************************************************
* Summary:
* Locks the watchdog timer to prevent configuration changes.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The CLK_SELECT register is written to, such that watchdog timer is now 
* locked. Any further changes to watchdog timer registers are then ignored.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
static void WatchdogTimer_Lock(void)
{
    uint32 ClkSelectValue;

    ClkSelectValue = CY_GET_REG32(CYREG_CLK_SELECT) | CLK_SELECT_WDT_LOCK_SET01;
    CY_SET_REG32(CYREG_CLK_SELECT, ClkSelectValue);
}


/*****************************************************************************
* Function Name: WatchdogTimer_Unlock()
******************************************************************************
* Summary:
* Unlocks the watchdog timer to allow configuration changes.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The CLK_SELECT register is written to, such that watchdog timer is now 
* unlocked. The watchdog timer registers can then be modified.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
static void WatchdogTimer_Unlock(void)
{
    uint32 ClkSelectValue;

    ClkSelectValue = CY_GET_REG32(CYREG_CLK_SELECT) & ~CLK_SELECT_WDT_LOCK_SET01;
    CY_SET_REG32(CYREG_CLK_SELECT, ClkSelectValue | CLK_SELECT_WDT_LOCK_CLR0);
    CY_SET_REG32(CYREG_CLK_SELECT, ClkSelectValue | CLK_SELECT_WDT_LOCK_CLR1);
}


/*****************************************************************************
* Public Functions
*****************************************************************************/

/*****************************************************************************
* Function Name: WatchdogTimer_Isr()
******************************************************************************
* Summary:
* The ISR for the watchdog timer. 
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The ISR increments the timestamp by the watchdog period, measured in ms. 
* Also updates the WDT_MATCH register to prepare for the next interrupt.
* Third, the ISR checks for a watchdog reset. 
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void WatchdogTimer_Isr(void)
{
    WatchdogTimer_Unlock();

    /* Update the timestamp */
    watchdogTimestamp += watchdogPeriodMs;
    
    /* Update WDT match register for next interrupt */
    UPDATE_WDT_MATCH((uint16)(CY_GET_REG32(CYREG_WDT_MATCH) + nextTicks));
    
    /* Clear WDT pending interrupt */
    CY_SET_REG32(CYREG_WDT_CONTROL, CY_GET_REG32(CYREG_WDT_CONTROL) | WDT_CONTROL_WDT_INT0);

    WatchdogTimer_Lock();
}


/*****************************************************************************
* Function Name: WatchdogTimer_Start()
******************************************************************************
* Summary:
* Initializes the watchdog timer.
*
* Parameters:
* wdtPeriodMs - The watchdog timer period to set, in milliseconds.
*
* Return:
* None
*
* Theory:
* Writes to the SRSS registers to start the low frequency clock and configure 
* the watchdog timer. The watchdog timer is configured for 16-bit timing, 
* and to generate an interrupt on match, plus an overflow on the third 
* unserviced interrupt. The interrupt vector is pointed to a custom ISR in 
* this file.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void WatchdogTimer_Start(uint32 wdtPeriodMs)
{
    watchdogTimestamp = 0;
    
    CyIntSetVector(WDT_INTERRUPT_NUM, &WatchdogTimer_Isr);
    
    WatchdogTimer_Unlock();

    /* Set the WDT period */
    watchdogPeriodMs = wdtPeriodMs;
    nextTicks = watchdogPeriodMs * WATCHDOG_TICKS_PER_MS;
    UPDATE_WDT_MATCH(nextTicks);
    
#if (CY_PSOC4_4200)
    /* For PSoC4A - Start ILO */
    CY_SET_REG32(CYREG_CLK_ILO_CONFIG, CY_GET_REG32(CYREG_CLK_ILO_CONFIG) | CLK_ILO_CONFIG_ILO_ENABLE);
#elif (CY_PSOC4_4200BL)
    /* 
     * For PSoC 4 BLE - Assume that WCO is started as part of CyBoot 
     * since WCO is selected as LFCLK source in the CYDWR.
     */
#endif  /* #if (CY_PSOC4_4200) */    
    
    /* Enable WDT interrupt for the NVIC */
    CyIntEnable(WDT_INTERRUPT_NUM);
    
    /* Set WDT_CONFIG register */
    CY_SET_REG32(CYREG_WDT_CONFIG, CY_GET_REG32(CYREG_WDT_CONFIG) | WDT_CONFIG_WDT_MODE0_INT);

    /* Set WDT_CONTROL register */
    CY_SET_REG32(CYREG_WDT_CONTROL, WDT_CONTROL_WDT_ENABLE0);

    /* Wait for the WDT enable to complete */
    while((CY_GET_REG32(CYREG_WDT_CONTROL) & WDT_CONTROL_WDT_ENABLED0) == 0);
    
    WatchdogTimer_Lock();
}


/*****************************************************************************
* Function Name: WatchdogTimer_Sync()
******************************************************************************
* Summary:
* Synchronizes the watchdog timestamp to the current system time.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Updates the watchdog timestamp to the current value.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void WatchdogTimer_Sync(void)
{
    uint32 wdtIntState;
    uint16 wdtCounter;
    uint16 wdtPreviousMatch;
    uint16 wdtCurrentMatch;
    uint16 ticksElapsed;
    
    /* Capture the WDT interrupt enable and disable the WDT interrupt. */
    wdtIntState = CyIntGetState(WDT_INTERRUPT_NUM);
    CyIntDisable(WDT_INTERRUPT_NUM);
    
    wdtCounter = CY_GET_REG32(CYREG_WDT_CTRLOW);
    wdtCurrentMatch = CY_GET_REG32(CYREG_WDT_MATCH);
    
    /* Find time elapsed since last WDT interrupt */
    wdtPreviousMatch = wdtCurrentMatch - nextTicks;
    ticksElapsed = wdtCounter - wdtPreviousMatch;
    watchdogTimestamp += ticksElapsed / WATCHDOG_TICKS_PER_MS;
    
    /* Add slow timer period to match register for next interrupt */
    WatchdogTimer_Unlock();
    
    UPDATE_WDT_MATCH((uint16)(wdtCounter + nextTicks - 
                                  (ticksElapsed % WATCHDOG_TICKS_PER_MS)));

    /* Clear pending WDT interrupt */
    CY_SET_REG32(CYREG_WDT_CONTROL, CY_GET_REG32(CYREG_WDT_CONTROL) | WDT_CONTROL_WDT_INT0);
    CyIntClearPending(WDT_INTERRUPT_NUM);

    WatchdogTimer_Lock();
    
    /* Restore WDT interrupt enable */
    if(wdtIntState)
    {
        CyIntEnable(WDT_INTERRUPT_NUM);
    }
    
}


/*****************************************************************************
* Function Name: WatchdogTimer_GetTimestamp()
******************************************************************************
* Summary:
* Gives the current time stamp of the system.
*
* Parameters:
* None
*
* Return:
* uint32 - The current system time stamp.
*
* Theory:
* Returns a variable set in the ISR.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
uint32 WatchdogTimer_GetTimestamp(void)
{
    return watchdogTimestamp;
}

/* [] END OF FILE */
