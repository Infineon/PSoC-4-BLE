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
* PMAD
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
#include "CommonFunctions.h"
#include "BLE_HTSS.h"

/*****************************************************************************
* Macros and constants
*****************************************************************************/
#define WDT_INTERRUPT_NUM           (8)
#define WATCHDOG_DEFAULT_PERIOD_MS  (20)
#define WATCHDOG_TICKS_PER_MS       (32)
#define CLK_ILO_CONFIG_ILO_ENABLE   (1u << 31)
#define WDT_CONFIG_WDT_MODE0_INT    (1u << 0)    
#define WDT_CONTROL_WDT_ENABLE0     (1u << 0)
#define WDT_CONTROL_WDT_ENABLED0    (1u << 1)
#define WDT_CONTROL_WDT_INT0        (1u << 2)
#define CLK_SELECT_WDT_LOCK_SET01   ((1u << 14) | (1u << 15))
#define CLK_SELECT_WDT_LOCK_CLR0    (1u << 14)
#define CLK_SELECT_WDT_LOCK_CLR1    (1u << 15)
#define UPDATE_WDT_MATCH(value)     CY_SET_REG32(CYREG_WDT_MATCH, \
                                    (CY_GET_REG32(CYREG_WDT_MATCH) & 0xFFFF0000) + (uint16)value)


/*****************************************************************************
* Global variables
*****************************************************************************/
volatile static uint32 watchdogPeriodMs = WATCHDOG_DEFAULT_PERIOD_MS;
volatile static uint32 nextTicks = WATCHDOG_DEFAULT_PERIOD_MS * WATCHDOG_TICKS_PER_MS;

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

    /* Update CLK_SELECT register to lock watchdog timer */
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

    /* Update CLK_SELECT register to unlock watchdog timer */
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
* The ISR checks if the WDT interrupt source is timer 0. If yes, then it 
* updates the WDT_MATCH register to prepare for the next interrupt and
* updates the flags for measurement interval. 
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
CY_ISR(WatchdogTimer_Isr)
{
    /* Initializes the interval timer with default measurement interval */
    static uint32 intervalTimer = DEFAULT_MEASUREMENT_INTERVAL;
    
    /* Unlock watchdog timer to enable configuration changes*/
    WatchdogTimer_Unlock();

    /* Update WDT match register for next interrupt */
    UPDATE_WDT_MATCH((uint16)(CY_GET_REG32(CYREG_WDT_MATCH) + nextTicks));
    
    /* Clear WDT pending interrupt */
    CY_SET_REG32(CYREG_WDT_CONTROL, CY_GET_REG32(CYREG_WDT_CONTROL) | WDT_CONTROL_WDT_INT0);

    /* Lock watchdog timer to prevent configuration changes */
    WatchdogTimer_Lock();
    
    /* Decrement interval timer */
    intervalTimer--;    
    
    /* If interval timer has elapsed then enable MEASUREMENT_INTERVAL flag */
    if(intervalTimer == 0)
    {
        /* Initialize interval timer and set MEASUREMENT_INTERVAL flag */
        intervalTimer = htssInterval;
        SystemFlag |= MEASUREMENT_INTERVAL;
    }
    
    /* Set WDT flag for application processing */
    SystemFlag |= WAKEUP_SOURCE_WDT;        
}


/*****************************************************************************
* Function Name: WatchdogTimer_Start()
******************************************************************************
* Summary:
* Initializes the watchdog timer 0.
*
* Parameters:
* wdtPeriodMs - The watchdog timer period to set, in milliseconds.
*
* Return:
* None
*
* Theory:
* Writes to the SRSS registers to start the low frequency clock and configure 
* the watchdog timer 0. The watchdog timer is configured for 16-bit timing, 
* and to generate an interrupt on match. 
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void WatchdogTimer_Start(uint32 wdtPeriodMs)
{
    CyIntSetVector(WDT_INTERRUPT_NUM, &WatchdogTimer_Isr);
    
    WatchdogTimer_Unlock();

    /* Set the WDT period */
    watchdogPeriodMs = wdtPeriodMs;
    nextTicks = watchdogPeriodMs * WATCHDOG_TICKS_PER_MS;
    UPDATE_WDT_MATCH(nextTicks);
        
    /* Set WDT_CONFIG register */
    CY_SET_REG32(CYREG_WDT_CONFIG, CY_GET_REG32(CYREG_WDT_CONFIG) | WDT_CONFIG_WDT_MODE0_INT);

    /* Enable WDT interrupt for the NVIC */
    CyIntEnable(WDT_INTERRUPT_NUM);

    /* Set WDT_CONTROL register */
    CY_SET_REG32(CYREG_WDT_CONTROL, CY_GET_REG32(CYREG_WDT_CONTROL) | WDT_CONTROL_WDT_ENABLE0);

    /* Wait for the WDT enable to complete */
    while((CY_GET_REG32(CYREG_WDT_CONTROL) & WDT_CONTROL_WDT_ENABLED0) == 0);
    
    WatchdogTimer_Lock();
}

/*****************************************************************************
* Function Name: WatchdogTimer_Stop()
******************************************************************************
* Summary:
* Disables the watchdog timer.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Disables the watchdog timer 0.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void WatchdogTimer_Stop (void)
{
    WatchdogTimer_Unlock();

    /* Clear WDT_CONFIG register */
    CY_SET_REG32(CYREG_WDT_CONFIG, CY_GET_REG32(CYREG_WDT_CONFIG) & ~WDT_CONFIG_WDT_MODE0_INT);

    /* Clear WDT pending interrupt */
    CY_SET_REG32(CYREG_WDT_CONTROL, CY_GET_REG32(CYREG_WDT_CONTROL) | WDT_CONTROL_WDT_INT0);

    WatchdogTimer_Lock();
}

/* [] END OF FILE */
