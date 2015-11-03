/*******************************************************************************
* File Name: Common.c
*
* Version: 1.30
*
* Description:
*  Provides an API that implement various functionality in the example project.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/
#include "main.h"
#include "Common.h"


uint8 packetRX[BLE_PACKET_SIZE_MAX];
uint32 packetRXSize;
uint32 packetRXFlag;

uint8 packetTX[BLE_PACKET_SIZE_MAX];
uint32 packetTXSize;

uint8 bootloadingMode;
uint8 bootloadingModeReq;


/*******************************************************************************
* Function Name: PrintProjectHeader()
********************************************************************************
* Summary:
*  Prints project header to UART.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void PrintProjectHeader()
{
    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("===============================================================================\r\n");
    DBG_PRINT_TEXT("=              BLE_External_Memory_Bootloadable Application Started\r\n");
    DBG_PRINT_TEXT("=              Version: 1.30\r\n");
    #if (LED_ADV_COLOR == LED_GREEN)
        DBG_PRINT_TEXT("=              Code: LED_GREEN\r\n");
    #else
        DBG_PRINT_TEXT("=              Code: LED_BLUE\r\n");
    #endif /*LED_ADV_COLOR == LED_GREEN*/
    DBG_PRINTF    ("=              Compile Date and Time : %s %s\r\n", __DATE__,__TIME__);
    #if (ENCRYPTION_ENABLED == YES)
        DBG_PRINT_TEXT("=              ENCRYPTION OPTION : ENABLED\r\n");
    #else
        DBG_PRINT_TEXT("=              ENCRYPTION OPTION : DISABLED\r\n");
    #endif /*LED_ADV_COLOR == LED_GREEN*/
    #if (CI_PACKET_CHECKSUM_CRC == YES)
        DBG_PRINT_TEXT("=              PACKET CHECKSUM TYPE: CRC-16-CCITT\r\n");
    #else
        DBG_PRINT_TEXT("=              PACKET CHECKSUM TYPE: BASIC SUMMATION\r\n");
    #endif /*LED_ADV_COLOR == LED_GREEN*/
    
    DBG_PRINT_TEXT("===============================================================================\r\n");
    DBG_PRINT_TEXT("\r\n"); 
    DBG_PRINT_TEXT("\r\n"); 
    
    return;
}


/*******************************************************************************
* Function Name: ChangeBootloaderServiceState()
********************************************************************************
* Summary:
*  Changes Bootloader Service visibility.
*
* Parameters:
*  uint32 enabledState:
*       1 - Enable bootloader service
*       0 - Disable (hide) bootloader service
*
* Return:
*  None
*
*******************************************************************************/
void ChangeBootloaderServiceState(uint32 enabledState)
{
    if (0u == enabledState)
    {
        /* Disable bootloader service */
        CyBle_GattsDisableAttribute(cyBle_btss.btServiceHandle);
    }
    else if (1u == enabledState)
    {
        /* Enable bootloader service */
        CyBle_GattsEnableAttribute(cyBle_btss.btServiceHandle);
    }
    else
    {
        /* Wrong argument - do nothing */
    }
    
    /* Force client to rediscover services in range of bootloader service */
    WriteAttrServChanged();
    
    return;
}


/*******************************************************************************
* Function Name: HandleLeds()
********************************************************************************
*
* Summary:
*   This function handles LEDs operation depending on the project operation
*   mode.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void HandleLeds(void)
{
    static uint32 ledTimer = LED_TIMEOUT;
    static uint8 advLed = LED_OFF;

    /* Blink blue LED to indicate that device advertises */
    if(connHandle.bdHandle == 0u)
    {
        if(--ledTimer == 0u)
        {
            ledTimer = LED_TIMEOUT;
            advLed ^= LED_OFF;
            if (bootloadingMode == 0u)
            {
                LED_WRITE_MACRO(advLed);
            }
            else
            {
                Bootloading_LED_Write(advLed);
            }
        }
    }
    else
    {
        if (bootloadingMode == 0u)
        {
            LED_WRITE_MACRO(LED_ON);
        }
        else
        {
            Bootloading_LED_Write(LED_ON);
        }
    }
}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the WDT timer.
*
*******************************************************************************/
CY_ISR(Timer_Interrupt)
{
    if(CySysWdtGetInterruptSource() & WDT_INTERRUPT_SOURCE)
    {
        HandleLeds();
        
        /* Clears interrupt request  */
        CySysWdtClearInterrupt(WDT_INTERRUPT_SOURCE);
    }
}


/*******************************************************************************
* Function Name: WDT_Start
********************************************************************************
*
* Summary:
*  Configures WDT to trigger an interrupt.
*
*******************************************************************************/
void WDT_Start(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    /* Setup ISR */
    WDT_Interrupt_StartEx(&Timer_Interrupt);
    /* Write the mode to generate interrupt on match */
    CySysWdtWriteMode(WDT_COUNTER, CY_SYS_WDT_MODE_INT);
    /* Configure the WDT counter clear on a match setting */
    CySysWdtWriteClearOnMatch(WDT_COUNTER, WDT_COUNTER_ENABLE);
    /* Configure the WDT counter match comparison value */
    CySysWdtWriteMatch(WDT_COUNTER, WDT_TIMEOUT);
    /* Reset WDT counter */
    CySysWdtResetCounters(WDT_COUNTER);
    /* Enable the specified WDT counter */
    CySysWdtEnable(WDT_COUNTER_MASK);
    /* Lock out configuration changes to the Watchdog timer registers */
    CySysWdtLock();    
}


/*******************************************************************************
* Function Name: WDT_Stop
********************************************************************************
*
* Summary:
*  This API stops the WDT timer.
*
*******************************************************************************/
void WDT_Stop(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    /* Disable the specified WDT counter */
    CySysWdtDisable(WDT_COUNTER_MASK);
    /* Locks out configuration changes to the Watchdog timer registers */
    CySysWdtLock();    
}


/*******************************************************************************
* Function Name: Wakeup_Interrupt_Bootloader()
********************************************************************************
*
* Summary:
*   Interrupt handler for the Bootloader Service activation.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(Wakeup_Interrupt_Bootloader)
{
    bootloadingModeReq = 1u;
    Bootloader_Service_Activation_ClearInterrupt();
    Wakeup_Interrupt_ClearPending();
}


/*******************************************************************************
* Function Name: WriteAttrServChanged()
********************************************************************************
*
* Summary:
*   Sets serviceChangedHandle for enabling or disabling hidden service.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void WriteAttrServChanged(void)
{
    uint32 value;
    CYBLE_GATT_HANDLE_VALUE_PAIR_T    handleValuePair;
    
    /* Force client to rediscover services in range of bootloader service */
    value = ((uint32)(((uint32) cyBle_btss.btServiceHandle) << 16u)) | 
        ((uint32) (cyBle_btss.btServiceInfo[0u].btServiceCharDescriptors[0u]));

    handleValuePair.value.val = (uint8 *)&value;
    handleValuePair.value.len = sizeof(value);

    handleValuePair.attrHandle = cyBle_gatts.serviceChangedHandle;
    CyBle_GattsWriteAttributeValue(&handleValuePair, 0u, NULL,CYBLE_GATT_DB_LOCALLY_INITIATED);
}


/*******************************************************************************
* Function Name: LowPowerImplementation()
********************************************************************************
* Summary:
* Implements low power in the project.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function tries to enter deep sleep as much as possible - whenever the 
* BLE is idle and the UART transmission/reception is not happening. At all other
* times, the function tries to enter CPU sleep.
*
*******************************************************************************/
void LowPowerImplementation(void)
{
    CYBLE_LP_MODE_T bleMode;
    uint8 interruptStatus;
    
    /* For advertising and connected states, implement deep sleep 
     * functionality to achieve low power in the system. For more details
     * on the low power implementation, refer to the Low Power Application 
     * Note.
     */
    if((CyBle_GetState() == CYBLE_STATE_ADVERTISING) || 
       (CyBle_GetState() == CYBLE_STATE_CONNECTED))
    {
        /* Request BLE subsystem to enter into Deep-Sleep mode between connection and advertising intervals */
        bleMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
        /* Disable global interrupts */
        interruptStatus = CyEnterCriticalSection();
        /* When BLE subsystem has been put into Deep-Sleep mode */
        if(bleMode == CYBLE_BLESS_DEEPSLEEP)
        {
            /* And it is still there or ECO is on */
            if((CyBle_GetBleSsState() == CYBLE_BLESS_STATE_ECO_ON) || 
               (CyBle_GetBleSsState() == CYBLE_BLESS_STATE_DEEPSLEEP))
            {
                /* Put the CPU into the Deep-Sleep mode. */
                CySysPmSleep();
            }
        }
        else /* When BLE subsystem has been put into Sleep mode or is active */
        {
            /* And hardware doesn't finish Tx/Rx opeation - put the CPU into Sleep mode */
            if(CyBle_GetBleSsState() != CYBLE_BLESS_STATE_EVENT_CLOSE)
            {
                CySysPmSleep();
            }
        }
        /* Enable global interrupt */
        CyExitCriticalSection(interruptStatus);
    }
}

/* [] END OF FILE */
