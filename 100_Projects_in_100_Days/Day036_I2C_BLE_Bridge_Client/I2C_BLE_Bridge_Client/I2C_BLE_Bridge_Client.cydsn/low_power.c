/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "low_power.h"

#ifdef LOW_POWER_MODE
	
/*******************************************************************************
* Function Name: handleLowPowerMode
********************************************************************************
* Summary:
*        This functions puts the BLESS and the MCU core to the lowest possible
*		power state
*	     
* Parameters:
*	void
*
* Return:
*  	void
*
*******************************************************************************/	
inline void handleLowPowerMode(void)
{
	CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
    uint8 interruptStatus;
	
	if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
    {
		while((I2C_I2CSlaveStatus() & (I2C_I2C_SSTAT_RD_BUSY | I2C_I2C_SSTAT_WR_BUSY)))
		{
			/* Wait till I2C read/write to complete */
		}
        /* Put BLE sub system in DeepSleep mode when it is idle */
        lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
        
       /* Disable global interrupts to avoid any other tasks from interrupting this section of code*/
        interruptStatus = CyEnterCriticalSection();
        
        /* Get current state of BLE sub system to check if it has successfully entered deep sleep state */
        blessState = CyBle_GetBleSsState();

        /* If BLE sub system has entered deep sleep, put chip into deep sleep for reducing power consumption */
        if(lpMode == CYBLE_BLESS_DEEPSLEEP)
        {   
            if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
            {
               /* Put the chip into the deep sleep state as there are no pending tasks and BLE has also 
               ** successfully entered BLE DEEP SLEEP mode */
#ifdef 	LED_INDICATION				
				ALL_LED_OFF();
#endif	/* LED_INDICATION */			
				I2C_Sleep();
               	CySysPmDeepSleep();
				I2C_Wakeup();
#ifdef 	LED_INDICATION	
			if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
			{
				CONNECT_LED_ON();
			}
			else if(CyBle_GetState() == CYBLE_STATE_SCANNING)
			{
				SCAN_LED_ON();
			}
#endif	/* LED_INDICATION */
            }
        }
        /* BLE sub system has not entered deep sleep, wait for completion of radio operations */
        else if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
        {
            /* change HF clock source from IMO to ECO, as IMO can be stopped to save power */
            CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO); 
            
			/* stop IMO for reducing power consumption */
            CySysClkImoStop(); 
            
			/* put the CPU to sleep */
            CySysPmSleep();
            
			/* starts execution after waking up, start IMO */
            CySysClkImoStart();
            
			/* change HF clock source back to IMO */
            CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_IMO);
        }
        /* Enable interrupts back */
        CyExitCriticalSection(interruptStatus);
    }
}

#endif /* LOW_POWER_MODE */
/* [] END OF FILE */
