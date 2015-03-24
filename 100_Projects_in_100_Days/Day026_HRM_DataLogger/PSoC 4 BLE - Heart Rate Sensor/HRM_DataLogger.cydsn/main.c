/*******************************************************************************
* File Name: main.c
*
* Version 1.0
*
* Description:
*  This project demonstrates the operation of the Heart Rate Profile
*  in Server (Peripheral) role.
*
* Note:
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "hrss.h"

volatile uint32 mainTimer = 0;
CYBLE_API_RESULT_T apiResult;


/*******************************************************************************
* Function Name: AppCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
*******************************************************************************/
void AppCallBack(uint32 event, void* eventParam)
{
#ifdef DEBUG_OUT    
    DebugOut(event, eventParam);
#endif

    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            heartRateSimulation = DISABLED;
            /* Put the device into discoverable mode so that remote can search it. */
            StartAdvertisement();
            break;
            
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            Advertising_LED_Write(LED_OFF);
            break;

        default:
            break;
    }
}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the WDT timer.
*  It is called from common WDT ISR located in BLE component. 
*
*******************************************************************************/
void Timer_Interrupt(void)
{
    if(CySysWdtGetInterruptSource() & WDT_INTERRUPT_SOURCE)
    {
        static uint8 led = LED_OFF;
        
        /* Blink LED to indicate that device advertises */
        if(CYBLE_STATE_ADVERTISING == CyBle_GetState())
        {
            led ^= LED_OFF;
            Advertising_LED_Write(led);
        }
        
        /* Indicate that timer is raised to the main loop */
        mainTimer++;
        
        /* Clears interrupt request  */
        CySysWdtClearInterrupt(WDT_INTERRUPT_SOURCE);
    }
}

/*******************************************************************************
* Function Name: WDT_Start
********************************************************************************
*
* Summary:
*  Configures WDT(counter 2) to trigger an interrupt every second.
*
*******************************************************************************/

void WDT_Start(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    /* Setup ISR callback */
    WdtIsr_StartEx(Timer_Interrupt);
    /* Write the mode to generate interrupt on match */
    CySysWdtWriteMode(WDT_COUNTER, CY_SYS_WDT_MODE_INT);
    /* Configure the WDT counter clear on a match setting */
    CySysWdtWriteClearOnMatch(WDT_COUNTER, WDT_COUNTER_ENABLE);
    /* Configure the WDT counter match comparison value */
    CySysWdtWriteMatch(WDT_COUNTER, WDT_1SEC);
    /* Reset WDT counter */
    CySysWdtResetCounters(WDT_COUNTER);
    /* Enable the specified WDT counter */
    CySysWdtEnable(WDT_COUNTER_MASK);
    /* Lock out configuration changes to the Watchdog timer registers */
    CySysWdtLock();    
}


int main()
{
    CYBLE_STACK_LIB_VERSION_T stackVersion;

    CyGlobalIntEnable;
    
    I2C_M_Start();
    UART_Start();               /* Start communication component */
    printf("BLE Heart Rate Sensor Example Project \r\n");
    
    Disconnect_LED_Write(LED_OFF);
    Advertising_LED_Write(LED_OFF);

    /* Start CYBLE component and register generic event handler */
    apiResult = CyBle_Start(AppCallBack);
    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("CyBle_Start API Error: %x \r\n", apiResult);
    }
    
    apiResult = CyBle_GetStackLibraryVersion(&stackVersion);
    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("CyBle_GetStackLibraryVersion API Error: 0x%x \r\n", apiResult);
    }
    else
    {
        printf("Stack Version: %d.%d.%d.%d \r\n", stackVersion.majorVersion, 
            stackVersion.minorVersion, stackVersion.patch, stackVersion.buildNumber);
    }
    
    /* Services initialization */
    HrsInit();
    
    WDT_Start();
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {        
        /***********************************************************************
        * Wait for connection established with Central device
        ***********************************************************************/
        if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            /*******************************************************************
            *  Periodically simulate heart rate and measure a battery level 
            *  and send results to the Client
            *******************************************************************/        
            if(mainTimer != 0u)
            {
                mainTimer = 0u;
                if(heartRateSimulation == ENABLED)
                {
                    SimulateHeartRate();
                }
            }
        }
        
        /*******************************************************************
        *  Process all pending BLE events in the stack
        *******************************************************************/
        CyBle_ProcessEvents();
    }
}


/* [] END OF FILE */
