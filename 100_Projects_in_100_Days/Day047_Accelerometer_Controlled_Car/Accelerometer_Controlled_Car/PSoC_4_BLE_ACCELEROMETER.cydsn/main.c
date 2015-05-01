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
#include <project.h>
#include "main.h"

uint8 applicationPower;

/*******************************************************************************
* Function Name: ManageApplicationPower
********************************************************************************
*
* Summary:
*   Handle ADC sleep and wake up and also switch between the various low power states
*   during different states of BLESS.
*
* Parameters:
*   None.
*
* Return:
*   None.
*
*******************************************************************************/

void ManageApplicationPower()
{
 	switch(applicationPower)
 	{
 	case ACTIVE: // don’t need to do anything
 	break;

 	case WAKEUP_SLEEP: // do whatever wakeup needs to be done
		Sensor_ADC_Wakeup();
 		applicationPower = ACTIVE;
 	break;

 	case WAKEUP_DEEPSLEEP: // do whatever wakeup needs to be done.
		Sensor_ADC_Wakeup();
 		applicationPower = ACTIVE;
 	break;

 	case SLEEP:
 		Sensor_ADC_Sleep();
 	break;
	
	case DEEPSLEEP:
 		Sensor_ADC_Sleep();
	 break;
 	}
}

/*******************************************************************************
* Function Name: ManageSystemPower
********************************************************************************
*
* Summary:
*   Checks the current low power stste, the BLESS status and put the BLESS and the 
*	system into apropriate lowpower stste.
*
* Parameters:
*   None.
*
* Return:
*   None.
*
*******************************************************************************/


void ManageSystemPower()
{
	CYBLE_BLESS_STATE_T blePower;
 	uint8 interruptStatus ;
	interruptStatus = CyEnterCriticalSection();
	blePower = CyBle_GetBleSsState();
	if((blePower == CYBLE_BLESS_STATE_DEEPSLEEP || blePower == CYBLE_BLESS_STATE_ECO_ON) && applicationPower == DEEPSLEEP)
 	{
 		applicationPower = WAKEUP_DEEPSLEEP;
 		/*Put system into Deep-Sleep mode*/
 		CySysPmDeepSleep();
 	}
 	/* BLESS is not in Deep Sleep mode. Check if it can enter Sleep mode */
 	else if((blePower != CYBLE_BLESS_STATE_EVENT_CLOSE))
 	{
 	/* Application is in Deep Sleep. IMO is not required */
 		if(applicationPower == DEEPSLEEP)
 		{
 			applicationPower = WAKEUP_DEEPSLEEP;
			/*Change HF clock source from IMO to ECO*/
			CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO);
			 /* Stop IMO for reducing power consumption */
			CySysClkImoStop();
			 /*Put the CPU to sleep */
			CySysPmSleep();
			 /* Starts execution after waking up, start IMO */
			CySysClkImoStart();
			 /* Change HF clock source back to IMO */
			CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_IMO);
 		}
 		/* Application components need IMO clock */
 		else if(applicationPower == SLEEP )
 		{
 			/* Put the system into Sleep mode*/
 			applicationPower = WAKEUP_SLEEP;
 			CySysPmSleep();
 		}
 }
	/* Enable interrupts */
 	CyExitCriticalSection(interruptStatus );
}
	


int main()
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	CyGlobalIntEnable;
	CySysClkIloStop();
	CySysClkWriteEcoDiv(CY_SYS_CLK_ECO_DIV8);

   	
	/* Start UART and BLE component and display project information */
    CyBle_Start(AppCallBack); 
	while (CyBle_GetState() == CYBLE_STATE_INITIALIZING)
 	{
 	CyBle_ProcessEvents();
 	}
	
    state=NORMAL; /*The default state is 'Normal'*/
	applicationPower = ACTIVE;
	/*Starting ADC and the interrupt connected to eoc of ADC. This interrupt will wake up the device from sleep */
    Sensor_ADC_Start();
	WakeUpISR_Start();
	Sensor_ADC_StartConvert();
	
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {              
    
		/*******************************************************************
        *  Process all pending BLE events in the stack
        *******************************************************************/  
		CyBle_ProcessEvents();
		CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP); 
		
        if(applicationPower == ACTIVE)
 		{
 		HandleBleProcessing();/* The main function which BLE based tasks are done. When the tasks are done the system will be put ito low power mode */
		}
		ManageApplicationPower();
		ManageSystemPower(); 
		
	}
}




/* [] END OF FILE */
