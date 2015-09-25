/*******************************************************************************
* File Name: main.c
* 
* Version 1.0
*
* Author - udyg@cypress.com
* 
* Description:
*  Application for Eddystone beacon.
*
********************************************************************************
* Copyright 2010-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <project.h>
#include <stdbool.h>
#include "Configuration.h"
#include "WatchdogTimer.h"
#include "Eddystone.h"

void WCO_ECO_LowPowerStart(void)
{
    WDT_WcoEcoLpStartSetup();   /* Setup WDT counters to enable low power WCO & ECO startup */

    CySysClkWcoStart();         /* Start the WCO clock */
    
    WDT_EnableWcoCounter();     /* Enable WDT's WCO counter (counter 0) */
    
    CySysPmDeepSleep(); /* Wait for the WDT counter 0 interrupt to wake up the device. On wakeup WCO is up & running */

    (void)CySysClkWcoSetPowerMode(CY_SYS_CLK_WCO_LPM);      /* Switch WCO to the low power mode after startup */
    CySysClkSetLfclkSource(CY_SYS_CLK_LFCLK_SRC_WCO);       /* LFCLK is now driven by WCO */
    CySysClkIloStop();                                      /* WCO is running, shut down the ILO */

    (void)CySysClkEcoStart(0);  /* It's time to start ECO */

    WDT_EnableEcoCounter();     /* Enable WDT's ECO counter (counter 1) */
    
    CySysPmDeepSleep();  /* Wait for the WDT counter 1 interrupt to wake up the device. On wakeup ECO is up & running */

    WDT_DisableWcoEcoCounters();
}


void Initialization(void)
{
    /* Set the divider for ECO, ECO will be used as source when IMO is switched off to save power, to drive the HFCLK */
    CySysClkWriteEcoDiv(CY_SYS_CLK_ECO_DIV8);
    
    /* Do the following for achieving lowest possible WCO & ECO startup current:
     * 1. Shut down the ECO (to reduce power consumption while WCO is starting)
     * 2. Enable WDT counter 0 to wakeup the system after 500ms (500ms = WCO startup time)
     * 3. Configure PSoC 4 BLE device in DeepSleep mode for the 500ms WCO startup time
     * 4. After WCO is enabled, restart the ECO so that BLESS interface can function
     * 5. Enable WDT counter 1 to wakeup the system after 1ms (1ms = ECO startup time)
     * 5. Configure PSoC 4 BLE device in DeepSleep mode for the 1ms ECO startup time */
    
    CySysClkEcoStop();      /* Shutdown the ECO and later re-start in low power mode after WCO is turned on */
    
    WDT_Interrupt_StartEx(WDT_Handler); /* Initialize WDT interrupt */
    
    WCO_ECO_LowPowerStart();    /* Enable WCO & ECO in low power mode using WDT counter 0/1 as system wakeup sources respectively */
    
    CyBle_Start(BLE_AppEventHandler);
    
    while (CyBle_GetState() == CYBLE_STATE_INITIALIZING)
    {
        CyBle_ProcessEvents(); 
    }
    
}

void LowPower(void)
{
    CYBLE_LP_MODE_T pwrState;
    CYBLE_BLESS_STATE_T blessState;
    uint8 intStatus = 0;

    pwrState  = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP); /* Configure BLESS in Deep-Sleep mode */

    intStatus = CyEnterCriticalSection(); /* No interrupts allowed while entering system low power modes */
        blessState = CyBle_GetBleSsState();

        if(pwrState == CYBLE_BLESS_DEEPSLEEP) /* Make sure BLESS is in Deep-Sleep before configuring system in Deep-Sleep */
        {
            if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
            {
                CySysPmDeepSleep(); /* System Deep-Sleep. 1.3uA mode */
            }
        }
        else if (blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
        {
             /* Change HF clock source from IMO to ECO, as IMO can be stopped to save power */
            CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO); 
            
            /* Stop IMO for reducing power consumption */
            CySysClkImoStop(); 
            
            /* Put the CPU to Sleep. 1.1mA mode */
            CySysPmSleep();
            
            /* Starts execution after waking up, start IMO */
            CySysClkImoStart();
            
            /* Change HF clock source back to IMO */
            CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_IMO);
        }
    CyExitCriticalSection(intStatus);
}

/* Main loop */
int main()
{
    CYBLE_API_RESULT_T apiResult;
    
    CyGlobalIntEnable;
    Initialization();
    
    for(;;)
    {
        /* Delayed start of advertisement */
        if(initCounter == 6)
        {
            initCounter = 7;
            WDT_DisableWcoEcoCounters();
            
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_CUSTOM);
            if(apiResult != CYBLE_ERROR_OK)
            {
                CYASSERT(0);
            }
        }
        
        CyBle_ProcessEvents(); /* BLE stack processing state machine interface */
        
        LowPower();
    }
}

/* [] END OF FILE */
