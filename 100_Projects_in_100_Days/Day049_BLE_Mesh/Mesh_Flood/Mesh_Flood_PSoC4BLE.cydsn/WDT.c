/*******************************************************************************
* File Name: WDT.c
*
* Version: 1.0
*
* Description:
* This file contains the functions to initialize Watchdog (WDT) counter and 
* record times for various timing operation in application.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <main.h>

extern uint8 ble_gap_state;
extern volatile uint16 RGBdataReceivedCounter;
extern uint8 switch_Role;
/*******************************************************************************
* Function Name: InitializeWatchdog
********************************************************************************
* Summary:
*        Start watchdog timer with given period
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitializeWatchdog(uint32 match_val)
{
	/* Unlock the WDT registers for modification */
	CySysWdtUnlock(); 
	
	/* Write Mode for Counter 0 as Interrupt on Match */
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT);
	
	/* Set Clear on Match for Counter 0*/
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, TRUE);
    
	/* Write the match value equal to 1 second in Counter 0 */
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, match_val);
    CyDelayUs(WATCHDOG_REG_UPDATE_WAIT_TIME);
	
	/* Enable Counter 0 */
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
	
	/* Lock Watchdog to prevent further changes */
    CySysWdtLock();
	
	isr_WDT_StartEx(WDT_INT_Handler);
}

/*******************************************************************************
* Function Name: WDT_INT_Handler
********************************************************************************
* Summary:
*        Watchdog interrupt routine for counting operation
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void WDT_INT_Handler(void)
{
	/* Clear Pending Interrupt */
	isr_WDT_ClearPending();
	
	/* If the Interrupt source is Counter 0 match, then process */
	if(CySysWdtGetInterruptSource() & CY_SYS_WDT_COUNTER0_INT)
	{
		/* Clear Watchdog Interrupt from Counter 0 */
		CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);
		
		/* Increment global counter variable */
		current_watchdog_counter++;
	}
}
/* [] END OF FILE */
