/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This is source code for the CYBLE Proximity Profile example project. In this
*  example project the CYBLE component is configured for Proximity Reporter
*  profile role.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"


/***************************************
*          API Constants
***************************************/
#define DISCONNECTED                        (0u)
#define ADVERTISING                         (1u)
#define CONNECTED                           (2u)

#define ALERT_TO                            (20u)
#define BLINK_DELAY                         (15u)


/***************************************
*        Global Variables
***************************************/
CYBLE_CONN_HANDLE_T  	connectionHandle;
uint8                	ledState = LED_OFF;
uint8                	advLedState = LED_OFF;
uint8                	alertLedState = LED_OFF;
uint8                	state = DISCONNECTED;
uint16               	alertBlinkDelayCount;
uint16               	advBlinkDelayCount;
uint8                	displayAlertMessage = YES;
uint8                	buttonState = BUTTON_IS_NOT_PRESSED;
extern volatile uint8 	tps_notification_enabled;

/*******************************************************************************
* Function Name: GenericAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the CYBLE Component.
*
* Parameters:
*  uint8 event:       Event from the CYBLE component.
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of events structure is described in the component
*                     datasheet.
*
* Return:
*  None
*
*******************************************************************************/
void GenericAppEventHandler(uint32 event, void *eventParam)
{
    CYBLE_API_RESULT_T apiResult;
    CYBLE_GAP_BD_ADDR_T localAddr;
    uint8 i;

    switch(event)
    {
    /**********************************************************
    *                       General Events
    ***********************************************************/
    case CYBLE_EVT_STACK_ON: /* This event is received when the component is Started */
        printf("Bluetooth On. Device address is: ");
        CyBle_GetDeviceAddress(&localAddr);
        for(i = CYBLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
        {
            printf("%2.2x", localAddr.bdAddr[i-1]);
        }
        printf("\r\n");

        /* Enter discoverable mode so that the remote Client could find the device. */
        apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);

        if(apiResult != CYBLE_ERROR_OK)
        {
            printf("StartAdvertisement API Error: %d \r\n", apiResult);
        }
        break;

    case CYBLE_EVT_TIMEOUT:
        /* Possible timeout event parameter values:
        * CYBLE_GAP_ADV_MODE_TO -> GAP limited discoverable mode timeout;
        * CYBLE_GAP_AUTH_TO -> GAP pairing process timeout.
        */
        if(CYBLE_GAP_ADV_MODE_TO == *(uint8 *) eventParam)
        {
            printf("Advertisement timeout occurred. Advertisement will be disabled.\r\n");
        }
        else
        {
            printf("Timeout occurred.\r\n");
        }
        break;

    case CYBLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
        printf("Hardware Error \r\n");
        break;

    case CYBLE_EVT_HCI_STATUS:
        printf("HCI Error. Error code is %x.\r\n", *(uint8 *) eventParam);
        break;

    /**********************************************************
    *                       GAP Events
    ***********************************************************/
    case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
        if(0u == connectionHandle.attId)
        {
            if(ADVERTISING == state)
            {
                printf("Advertisement is disabled \r\n");
                state = DISCONNECTED;
            }
            else
            {
                printf("Advertisement is enabled \r\n");
                /* Device now is in Advertising state */
                state = ADVERTISING;
            }
        }
        else
        {
            /* Error occurred in the BLE Stack */
            if(ADVERTISING == state)
            {
                printf("Failed to stop advertising \r\n");
            }
            else
            {
                printf("Failed to start advertising \r\n");
            }
        }
        break;

    case CYBLE_EVT_GAP_DEVICE_CONNECTED:
        printf("CYBLE_EVT_GAP_DEVICE_CONNECTED: %d \r\n", connectionHandle.bdHandle);
        state = CONNECTED;
        break;

    case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
        printf("CYBLE_EVT_GAP_DEVICE_DISCONNECTED\r\n");
        /* Enter discoverable mode so that remote Client could find the device. */
        apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        connectionHandle.bdHandle = 0u;
        if(apiResult != CYBLE_ERROR_OK)
        {
            printf("StartAdvertisement API Error: %d\r\n", apiResult);
        }
        break;

    /**********************************************************
    *                       GATT Events
    ***********************************************************/
    case CYBLE_EVT_GATT_CONNECT_IND:
        /* GATT connection was established */
        connectionHandle = *(CYBLE_CONN_HANDLE_T *) eventParam;
        printf("CYBLE_EVT_GATT_CONNECT_IND: %x\r\n", connectionHandle.attId);
        break;

    case CYBLE_EVT_GATT_DISCONNECT_IND:
        /* GATT connection was disabled */
        printf("CYBLE_EVT_GATT_DISCONNECT_IND:\r\n");
        connectionHandle.attId = 0u;

		tps_notification_enabled = 0;
        /* Get the LLS Alert Level from the GATT database */
        (void) CyBle_LlssGetCharacteristicValue(CYBLE_LLS_ALERT_LEVEL, CYBLE_LLS_ALERT_LEVEL_SIZE, &alertLevel);
        break;

    case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
        break;

    case CYBLE_EVT_GATTS_INDICATION_ENABLED:
        break;

    default:
    #if (0)
        printf("Other event - %x \r\n", LO16(event));
    #endif
        break;
    }
}


/*******************************************************************************
* Function Name: SW_Interrupt
********************************************************************************
*
* Summary:
*   Handles the mechanical button press.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(ButtonPressInt)
{
    buttonState = BUTTON_IS_PRESSED;

    SW2_ClearInterrupt();
}


/*******************************************************************************
* Function Name: WDT_Start
********************************************************************************
*
* Summary:
*  Configures and starts Watchdog timer to trigger an interrupt every second.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void WDT_Start(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock();
    /* Setup ISR callback */
    WDT_Interrupt_StartEx(&Timer_Interrupt);
    /* Write the mode to generate interrupt on match */
    CySysWdtWriteMode(WDT_COUNTER, CY_SYS_WDT_MODE_INT);
    /* Configure the WDT counter clear on a match setting */
    CySysWdtWriteClearOnMatch(WDT_COUNTER, WDT_COUNTER_ENABLE);
    /* Configure the WDT counter match comparison value */
    CySysWdtWriteMatch(WDT_COUNTER, WDT_COUNT_PERIOD);
    /* Reset WDT counter */
    CySysWdtResetCounters(WDT_COUNTER);
    /* Enable the specified WDT counter */
    CySysWdtEnable(WDT_COUNTER_MASK);
    /* Lock out configuration changes to the Watchdog timer registers */
    CySysWdtLock();
}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the WDT timer.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(Timer_Interrupt)
{
    if(CySysWdtGetInterruptSource() & WDT_INTERRUPT_SOURCE)
    {
        if(CONNECTED != state)
        {
            if((llsAlertTOCounter != ALERT_TO) && (alertLevel != CYBLE_NO_ALERT))
            {
                /* Update alert timeout */
                llsAlertTOCounter++;
            }
            else
            {
                alertLevel = CYBLE_NO_ALERT;
                
                /* Clear alert timeout */
                llsAlertTOCounter = 0u;
            }
        }
        /* Clear interrupt request */
        CySysWdtClearInterrupt(WDT_INTERRUPT_SOURCE);
    }
}


/*******************************************************************************
* Function Name: HandleLeds
********************************************************************************
*
* Summary:
*  Handles indications on Advertising, Disconnection and Alert LEDs. Also it
*  implements timeout after which Alert LED is cleared if it was previously set.
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
    /* If in disconnected state ... */
    if(DISCONNECTED == state)
    {
        /* ... turn on disconnect indication LED and turn off rest of the LEDs. */
        Disconnect_LED_Write(LED_ON);
        Advertising_LED_Write(LED_OFF);
    }
    /* In advertising state ... */
    else if(ADVERTISING == state)
    {
        /* ... turn off disconnect indication and alert LEDs ... */
        Disconnect_LED_Write(LED_OFF);

        /* ... and blink advertisement indication LED. */
        if(BLINK_DELAY == advBlinkDelayCount)
        {
            if(LED_OFF == advLedState)
            {
                advLedState = LED_ON;
            }
            else
            {
                advLedState = LED_OFF;
            }
            advBlinkDelayCount = 0u;
        }

        advBlinkDelayCount++;

        Advertising_LED_Write(advLedState);
    }
    /* In connected State ... */
    else
    {
        /* ... turn off all LEDs. */
        Disconnect_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        Alert_LED_Write(LED_OFF);
    }

    if(CONNECTED != state)
    {
        /* If "Mild Alert" is set by the Client then blink alert LED */
        if((CYBLE_MILD_ALERT == alertLevel) && (ALERT_TO != llsAlertTOCounter))
        {
            if(YES == displayAlertMessage)
            {
                printf("Device started alerting with \"Mild Alert\"\r\n");
                displayAlertMessage = NO;
            }

            /* Switch state of alert LED to make blinking */
            if(BLINK_DELAY == alertBlinkDelayCount)
            {
                if(LED_OFF == alertLedState)
                {
                    alertLedState = LED_ON;
                }
                else
                {
                    alertLedState = LED_OFF;
                }
                alertBlinkDelayCount = 0u;
            }
            alertBlinkDelayCount++;
        }
        /* For "High Alert" turn alert LED on */
        else if((CYBLE_HIGH_ALERT == alertLevel) && (ALERT_TO != llsAlertTOCounter))
        {
            if(YES == displayAlertMessage)
            {
                printf("Device started alerting with \"High Alert\"\r\n");
                displayAlertMessage = NO;
            }

            alertLedState = LED_ON;
        }
        /* In case of "No Alert" turn alert LED off */
        else
        {
            displayAlertMessage = YES;

            alertLedState = LED_OFF;
        }

        Alert_LED_Write(alertLedState);
    }
}


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    CYBLE_API_RESULT_T apiResult;
    CYBLE_BLESS_PWR_IN_DB_T txPower;
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
    int8 intTxPowerLevel;

    CyGlobalIntEnable;

    /* Turn off all of the LEDs */
    Disconnect_LED_Write(LED_OFF);
    Advertising_LED_Write(LED_OFF);
    Alert_LED_Write(LED_OFF);

    /* Start CYBLE component and register generic event handler */
    CyBle_Start(GenericAppEventHandler);

    SW2_Interrupt_StartEx(&ButtonPressInt);

    /* Register the event handler for LLS specific events */
    CyBle_LlsRegisterAttrCallback(LlsServiceAppEventHandler);

    /* Register the event handler for TPS specific events */
    CyBle_TpsRegisterAttrCallback(TpsServiceAppEventHandler);

    /* Set Tx power level of the radio to 0 dBm */
    txPower.bleSsChId = CYBLE_LL_CONN_CH_TYPE;
    txPower.blePwrLevelInDbm = CYBLE_LL_PWR_LVL_0_DBM;

    apiResult = CyBle_SetTxPowerLevel(&txPower);

    WDT_Start();

    if(CYBLE_ERROR_OK == apiResult)
    {
        /* Convert power level to numeric int8 value */
        intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);

        /* Display new Tx Power Level value */
        printf("Tx power level is set to %d dBm\r\n", intTxPowerLevel);
    }

    UART_DEB_Start();

    while(1)
    {
        /* CyBle_ProcessEvents() allows BLE stack to process pending events */
        CyBle_ProcessEvents();

        if((CyBle_GetState() != CYBLE_STATE_CONNECTED) && (CyBle_GetState() != CYBLE_STATE_ADVERTISING))
        {
            if(BUTTON_IS_PRESSED == buttonState)
            {
                /* Start advertisement */
                if(CYBLE_ERROR_OK == CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST))
                {
                    printf("Device has entered Limited Discovery mode \r\n");
                }

                /* Reset button state */
                buttonState = BUTTON_IS_NOT_PRESSED;
            }
        }
        else
        {
            /* Decrease Tx power level of BLE radio if button is pressed */
            if(BUTTON_IS_PRESSED == buttonState)
            {
				if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
				{
					/* Specify connection channel for reading Tx power level */
                	txPower.bleSsChId = CYBLE_LL_CONN_CH_TYPE;
				}
				else if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
				{
					/* //roit: Specify adv channel for reading Tx power level */
                	txPower.bleSsChId = CYBLE_LL_ADV_CH_TYPE;
				}
				
                /* Get current Tx power level */
                CyBle_GetTxPowerLevel(&txPower);

                /* Decrease the Tx power level by one scale */
                DecreaseTxPowerLevelValue(&txPower.blePwrLevelInDbm);

                /* Set the new Tx power level */
                apiResult = CyBle_SetTxPowerLevel(&txPower);

                if(CYBLE_ERROR_OK == apiResult)
                {
					/* Get current TX power */
					CyBle_GetTxPowerLevel(&txPower);
					
                    /* Convert power level to numeric int8 value */
                    intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);

                    (void) CyBle_TpssSetCharacteristicValue(CYBLE_TPS_TX_POWER_LEVEL,
                                                            CYBLE_TPS_TX_POWER_LEVEL_SIZE,
                                                            &intTxPowerLevel);
					
					if(tps_notification_enabled)
					{
						/* If notifications have been enabled, send notification data */
						(void) CyBle_TpssSendNotification(cyBle_connHandle, 
															CYBLE_TPS_TX_POWER_LEVEL,
															CYBLE_TPS_TX_POWER_LEVEL_SIZE,
                                                            &intTxPowerLevel);
					}

                    /* Display new Tx Power Level value */
                    printf("Tx power level is set to %d dBm\r\n", intTxPowerLevel);
                }
            
                /* Reset button state */
                buttonState = BUTTON_IS_NOT_PRESSED;
            }
        }

        HandleLeds();
        
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
                    /* Put the device into the Deep Sleep mode only when all debug information has been sent */
                   if((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) == 0u)
                    {
                        CySysPmDeepSleep();
                    }
                    else
                    {
                        CySysPmSleep();
                    }
                }
            }
            else
            {
                if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
                {
                    CySysPmSleep();
                }
            }
            CyGlobalIntEnable;
        }
    }
}


/* [] END OF FILE */
