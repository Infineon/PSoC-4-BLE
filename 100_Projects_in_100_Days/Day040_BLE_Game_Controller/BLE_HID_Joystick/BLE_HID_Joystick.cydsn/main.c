/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  BLE HID keyboard example project that supports both input and output reports
*  in boot and protocol mode. The example also demonstrates handling suspend 
*  event from the central device and enters low power mode when suspended.
*
* References:
*  BLUETOOTH SPECIFICATION Version 4.1
*  HID Usage Tables spec ver 1.12
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

#include "common.h"
#include "hids.h"
#include "bas.h"


volatile uint32 mainTimer = 0;
uint16 JoyStick_Activity_Timer = 500u;
uint16 connIntv = CYBLE_GAPP_CONNECTION_INTERVAL_MIN;
uint8 Joystick_Activity_Flag = CyFalse;
uint8 Joystick_Activity_Prev_State = CyFalse;
uint8 Is_WDT_Button_Wakeup = CyFalse;
//#define DEBUG_ENABLED 
/*******************************************************************************
* Function Name: AppCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the BLE Component.
*
*******************************************************************************/
void AppCallBack(uint32 event, void* eventParam)
{
    CYBLE_API_RESULT_T apiResult;
    CYBLE_GAP_BD_ADDR_T localAddr;
    CYBLE_GAP_AUTH_INFO_T *authInfo;
    uint8 i;
    
    switch (event)
	{
        /**********************************************************
        *                       General Events
        ***********************************************************/
		case CYBLE_EVT_STACK_ON: /* This event is received when the component is Started */
            /* Enter into discoverable mode so that remote can search it. */
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if(apiResult != CYBLE_ERROR_OK)
            {
                #ifdef DEBUG_ENABLED
                printf("StartAdvertisement API Error: %d \r\n", apiResult);
                #endif
            }
            #ifdef DEBUG_ENABLED
            printf("Bluetooth On, StartAdvertisement with addr: ");
            #endif
            
            CyBle_GetDeviceAddress(&localAddr);
            for(i = CYBLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
            {
                #ifdef DEBUG_ENABLED
                printf("%2.2x", localAddr.bdAddr[i-1]);
                #endif
            }
            
            #ifdef DEBUG_ENABLED
            printf("\r\n");
            #endif
            
            break;
		case CYBLE_EVT_TIMEOUT: 
            break;
		case CYBLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            
            #ifdef DEBUG_ENABLED
            printf("CYBLE_EVT_HARDWARE_ERROR \r\n");
            #endif
            
			break;
            
    	/* This event will be triggered by host stack if BLE stack is busy or not busy.
    	 *  Parameter corresponding to this event will be the state of BLE stack.
    	 *  BLE stack busy = CYBLE_STACK_STATE_BUSY,
    	 *  BLE stack not busy = CYBLE_STACK_STATE_FREE 
         */
    	case CYBLE_EVT_STACK_BUSY_STATUS:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_STACK_BUSY_STATUS: %x\r\n", *(uint8 *)eventParam);
            #endif
            
            break;
        case CYBLE_EVT_HCI_STATUS:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_HCI_STATUS: %x \r\n", *(uint8 *)eventParam);
            #endif
            
			break;
            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CYBLE_EVT_GAP_AUTH_REQ:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_AUTH_REQ: security=%x, bonding=%x, ekeySize=%x, err=%x \r\n", 
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).security, 
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).bonding, 
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).ekeySize, 
                (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).authErr);
            #endif
            
            break;
        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_PASSKEY_ENTRY_REQUEST press 'p' to enter passkey \r\n");
            #endif
            
            break;
        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_PASSKEY_DISPLAY_REQUEST %6.6ld \r\n", *(uint32 *)eventParam);
            #endif
            
            break;
        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
            #endif
            
            break;
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            authInfo = (CYBLE_GAP_AUTH_INFO_T *)eventParam;
            
            #ifdef DEBUG_ENABLED
            printf("AUTH_COMPLETE: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n", 
                                    authInfo->security, authInfo->bonding, authInfo->ekeySize, authInfo->authErr);
            #endif
            
            break;
        case CYBLE_EVT_GAP_AUTH_FAILED:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_AUTH_FAILED: %x \r\n", *(uint8 *)eventParam);
            #endif
            
            break;
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_ADVERTISING, state: %x \r\n", CyBle_GetState());
            #endif
            
            if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {   
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate mode) and wait for an external
                 * user event to wake up the device again */
                #ifdef DEBUG_ENABLED
                printf("Hibernate \r\n");
                #endif
                
                Advertising_LED_Write(LED_OFF);
                Disconnect_LED_Write(LED_ON);

                Button_ClearInterrupt();
                Wakeup_Interrupt_ClearPending();
                Wakeup_Interrupt_Start();
                /* Wait until debug info is sent */
                #ifdef DEBUG_ENABLED
                while((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) != 0);
                #endif
                
                CySysPmHibernate();
            }
            break;
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            Advertising_LED_Write(LED_OFF);
            connIntv = ((CYBLE_GAP_CONN_PARAM_UPDATED_IN_CONTROLLER_T *)eventParam)->connIntv * 5u /4u;
            connIntv = ((CYBLE_GAP_CONN_PARAM_UPDATED_IN_CONTROLLER_T *)eventParam)->connLatency; 
            
            #ifdef DEBUG_ENABLED
            printf("EVT_GAP_DEVICE_CONNECTED: connIntv = %d ms \r\n", connIntv);
            #endif
            
            Advertising_LED_Write(LED_OFF);            
            break;
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            
            #ifdef DEBUG_ENABLED
            printf("EVT_GAP_DEVICE_DISCONNECTED\r\n");
            #endif
            
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if(apiResult != CYBLE_ERROR_OK)
            {
                #ifdef DEBUG_ENABLED
                printf("StartAdvertisement API Error: %d \r\n", apiResult);
                #endif
            }
            break;
        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
            { 
                uint16 mtu;
                CyBle_GattGetMtuSize(&mtu);
                
                #ifdef DEBUG_ENABLED
                printf("CYBLE_EVT_GATTS_XCNHG_MTU_REQ, final mtu= %d \r\n", mtu);
                #endif
            }
            break;
        case CYBLE_EVT_GATTS_WRITE_REQ:
            #ifdef DEBUG_ENABLED
            printf("EVT_GATT_WRITE_REQ: %x = ",((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->handleValPair.attrHandle);
            #endif
            ShowValue(&((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->handleValPair.value);
            (void)CyBle_GattsWriteRsp(((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->connHandle);
            break;
        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
            #ifdef DEBUG_ENABLED
            printf("EVT_GAP_ENCRYPT_CHANGE: %x \r\n", *(uint8 *)eventParam);
            #endif
            break;
        case CYBLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE:
            #ifdef DEBUG_ENABLED
            printf("EVT_CONNECTION_UPDATE_COMPLETE: %x \r\n", *(uint8 *)eventParam);
            #endif
            break;
            
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
            #ifdef DEBUG_ENABLED
            printf("EVT_GATT_CONNECT_IND: %x, %x \r\n", cyBle_connHandle.attId, cyBle_connHandle.bdHandle);
            #endif
            /* Register service specific callback functions */
            HidsInit();
            BasInit();
            break;
        case CYBLE_EVT_GATT_DISCONNECT_IND:
            #ifdef DEBUG_ENABLED
            printf("EVT_GATT_DISCONNECT_IND \r\n");
            #endif
            break;
            
        /**********************************************************
        *                       Other Events
        ***********************************************************/
		default:
            #ifdef DEBUG_ENABLED
            printf("OTHER event: %lx \r\n", event);
            #endif
			break;
	}

}

CY_ISR(Button_Press_Interrupt)
{
    Is_WDT_Button_Wakeup = CyTrue;
    Wakeup_Interrupt_ClearPending();
    Button_ClearInterrupt();
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
       
        Is_WDT_Button_Wakeup= CyTrue;
              
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

void WDT_Modify(uint16 activity)
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
    
    //CySysWdtWriteMatch(WDT_COUNTER, WDT_ACTIVITY_TIMEOUT);
    CySysWdtWriteMatch(WDT_COUNTER, activity);
         
   
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

void Initialize_Module(void)
{
    SAR_ADC_Start();
    SAR_ADC_IRQ_Enable();
    Wakeup_Interrupt_StartEx(&Button_Press_Interrupt);
    Wakeup_Interrupt_Enable();
    
    #ifdef DEBUG_ENABLED
    UART_DEB_Start(); 
    printf("BLE HID Keyboard Example Project \r\n");
    #endif
    
    Disconnect_LED_Write(LED_OFF);
    Advertising_LED_Write(LED_OFF);
    
    CySysClkIloStop();

    /* Start CYBLE component and register generic event handler */
    CyBle_Start(AppCallBack);
    WDT_Modify(WDT_NO_ACTIVITY_TIMEOUT);
    
    CySysClkWriteEcoDiv(CY_SYS_CLK_ECO_DIV8);
}


int main()
{
    uint8 led;
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
 
    CyGlobalIntEnable;  

    Initialize_Module();

	while(1) 
    {           

        if(CyBle_GetState() != CYBLE_STATE_INITIALIZING)
        {
            /* Enter DeepSleep mode between connection intervals */
            lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
            CyGlobalIntDisable;
            blessState = CyBle_GetBleSsState();

            if(lpMode == CYBLE_BLESS_DEEPSLEEP) 
            {   
                if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
                {
                    #ifdef DEBUG_ENABLED
                    /* Put the device into the Deep Sleep mode only when all debug information has been sent */
                    if((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) == 0u)
                    {
                        CySysPmDeepSleep();
                    }
                    else
                    {
                        CySysPmSleep();
                    }
                    #else
                        SAR_ADC_IRQ_Disable();
                    
                        SAR_ADC_Stop();
                    
                        CySysPmDeepSleep();
                        
                        SAR_ADC_Start();
                    
                        SAR_ADC_IRQ_Enable();
                    #endif
                }
            }
            else
            {
                if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
                {
                    SAR_ADC_IRQ_Disable();
                    
                    SAR_ADC_Stop();
                                        
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
                    
                    SAR_ADC_Start();
                    
                    SAR_ADC_IRQ_Enable();
                }
            }
            CyGlobalIntEnable;
        }
        
        
        if((CyBle_GetState() == CYBLE_STATE_CONNECTED) && (suspend != CYBLE_HIDS_CP_SUSPEND)&&(Is_WDT_Button_Wakeup==CyTrue))
        {
            Is_WDT_Button_Wakeup =CyFalse;            
            if(batteryMeasure == ENABLED)
            {
                MeasureBattery();

            }
            if(Joystick_Simulation == ENABLED)
            {
                SimulateJoystick();
            }
            
            
            if(JoyStick_Activity_Timer == 0x00)
            {
                JoyStick_Activity_Timer =500u;
                if(CySysWdtReadMatch(WDT_COUNTER)==WDT_ACTIVITY_TIMEOUT)
                { 
                    WDT_Modify(WDT_NO_ACTIVITY_TIMEOUT);
                    
                }
                
            }
            else if(Joystick_Activity_Flag ==CyTrue &&
                (CySysWdtReadMatch(WDT_COUNTER)!=WDT_ACTIVITY_TIMEOUT))
            {
                WDT_Modify(WDT_ACTIVITY_TIMEOUT);
                
                
            }
            
            #ifdef DEBUG_ENABLED
            /* Store bonding data to flash only when all debug information has been sent */
            if((cyBle_pendingFlashWrite != 0u) &&
               ((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) == 0u))
            {
                CYBLE_API_RESULT_T apiResult;
                
                apiResult = CyBle_StoreBondingData(0u);
                printf("Store bonding data, status: %x \r\n", apiResult);
            }
            #else
                if(cyBle_pendingFlashWrite != 0u)
                {
                    CYBLE_API_RESULT_T apiResult;
                    apiResult = CyBle_StoreBondingData(0u);
                }
            #endif
        }
        
       
        CyBle_ProcessEvents();
	}   
}  


/* [] END OF FILE */

