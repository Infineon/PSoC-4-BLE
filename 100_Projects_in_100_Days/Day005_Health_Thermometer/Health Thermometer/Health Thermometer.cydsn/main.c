/****************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
* This file implements the system functions.
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
#include <project.h>
#include "CommonFunctions.h"
#include "Temperature.h"
#include "BLE_HTSS.h"

/*****************************************************************************
* Function Name: main()
******************************************************************************
* Summary:
* Central function which controls the application flow
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Central function which is entered on power up. This function controls
* the complete application which includes BLE processing, temperature 
* measurement and power mode transitions.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
int main()
{
    /* Powerstate indicators of BLESS */
    CYBLE_LP_MODE_T lpMode = CYBLE_BLESS_DEEPSLEEP;
    CYBLE_BLESS_STATE_T blessState = CYBLE_BLESS_STATE_DEEPSLEEP;
    
    /* Local variable used to store temperature information */
    uint32 temperature;
    
    /* Enable global interrupt, required for BLESS operation */
    CyGlobalIntEnable; 
    
    /* Initialize the complete system */
    InitializeSystem();
    
    /* Infinite loop */
    for(;;)
    {
        /* Process all pending BLE events in the stack */
        CyBle_ProcessEvents();
        
        /***********************************************************************
        * Wait for connection established with Central device
        ***********************************************************************/
        if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            /* Process all pending BLE events in the stack */
            CyBle_ProcessEvents();
            /* Periodically measure temperature and send results to the Client */
            if(SystemFlag & WAKEUP_SOURCE_WDT)
            {
                /* Clear WDT wakeup source flag */
                SystemFlag &= ~WAKEUP_SOURCE_WDT;
                /* Wakeup the system from deep sleep */
                WakeupFromDeepSleep();
                /* Measure temperature */
                temperature = ProcessTemperature();
                /* Send temperature information over BLE */
                ProcessBLE(temperature);
            }
        }
        
        /* If BLE is successfully initialized, then execute the low power routine */
        if(CyBle_GetState() != CYBLE_STATE_INITIALIZING)
        {
            /* Disable all hardware blocks */
            PrepareForDeepSleep();
            /* Enter Deep Sleep mode between connection intervals */
            lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
            
            /* Disable global interrupts to avoid race condition */
            CyGlobalIntDisable;
            blessState = CyBle_GetBleSsState();

            /* Try putting system in DeepSleep mode if BLESS succesfully entered Deep Sleep mode*/
            if(lpMode == CYBLE_BLESS_DEEPSLEEP) 
            {   
                /* If BLESS is in Deep Sleep or is in the process of waking up from Deep Sleep,
                 * put system in Deep Sleep mode */
                if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
                {
                    CySysPmDeepSleep();
                }
            }
            /* If BLESS is in Active state */
            else
            {
                /* If BLESS Tx/Rx Event is not complete, reduce IMO and put CPU to Sleep */
                if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
                {
                    CySysClkWriteImoFreq(IMO_FREQUENCY_3MHZ);
                    CySysPmSleep();
                    CySysClkWriteImoFreq(IMO_FREQUENCY_12MHZ);
                }
            }
            /* Enable interrupts for normal operation */
            CyGlobalIntEnable;
        }
        
        /* If Hibernate is enabled then put system in hibernate state */
        if(SystemFlag & ENABLE_HIBERNATE)
        {
            /* Enable ISR for the pin to wakeup the device from hibernate */
            Wakeup_ISR_Start();
            
            /* Put sytem to hibernate */
            CySysPmHibernate();
        }
    }
}

/* [] END OF FILE */
