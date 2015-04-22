/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates the operation of the Heart Rate Profile
*  in Collector (Central) role.
*
* Related Document:
*  HEART RATE PROFILE SPECIFICATION v1.0
*  HEART RATE SERVICE SPECIFICATION v1.0
*
*******************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "hrsc.h"
#include "basc.h"

CYBLE_API_RESULT_T apiResult;
CYBLE_CONN_HANDLE_T hrsConnHandle;

void AppCallBack(uint32 event, void* eventParam)
{
#ifdef DEBUG_OUT    
    DebugOut(event, eventParam);
#endif
    
    switch(event)
    {
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            Disconnect_LED_Write(LED_ON);
        case CYBLE_EVT_STACK_ON:
            StartScan(CYBLE_UUID_HEART_RATE_SERVICE);
            break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            /* Send authorization request. */
            apiResult = CyBle_GapAuthReq(cyBle_connHandle.bdHandle, &cyBle_authInfo);
            
            if(CYBLE_ERROR_OK != apiResult)
        	{
        		printf("CyBle_GapAuthReq API Error: %x \r\n", apiResult);
        	}
        	else
        	{
        		printf("Authentification request is sent \r\n");
        	}

            Disconnect_LED_Write(LED_OFF);
            Scanning_LED_Write(LED_OFF);
            break;
            
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            hrsConnHandle.bdHandle = cyBle_connHandle.bdHandle;
            StartDiscovery();
            break;

        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
            /* Send request to read the body sensor location char. */
            apiResult = HrscReadBodySensorLocation();

            if(apiResult != CYBLE_ERROR_OK)
            {
                printf("HrscReadBodySensorLocation API Error: %x \r\n", apiResult);
            }
            else
            {
                printf("Body Sensor Location Read Request is sent \r\n");
            }
            break;
            
        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
            printf("CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ: %x, %x, %x, %x \r\n", 
                ((CYBLE_GAP_CONN_UPDATE_PARAM_T *)eventParam)->connIntvMax,
                ((CYBLE_GAP_CONN_UPDATE_PARAM_T *)eventParam)->connIntvMin,
                ((CYBLE_GAP_CONN_UPDATE_PARAM_T *)eventParam)->connLatency,
                ((CYBLE_GAP_CONN_UPDATE_PARAM_T *)eventParam)->supervisionTO
            );
            /* Accepted = 0x0000, Rejected  = 0x0001 */
            CyBle_L2capLeConnectionParamUpdateResponse(cyBle_connHandle.bdHandle, 0u); 
            break;

        default:
            break;
    }
}

/*******************************************************************************
* Function Name: HandleLeds
********************************************************************************
*
* Summary:
*  Blinking Green LED during scanning process 
*  and Blue LED on notification received event.
*
*******************************************************************************/

void HandleLeds(void)
{
    static uint32 ledTimer = LED_TIMEOUT;
    static uint8 greenLed = LED_OFF;
    
    /* Blink green LED to indicate that device is scanning */
    if(CyBle_GetState() == CYBLE_STATE_SCANNING)
    {
        if(--ledTimer == 0u) 
        {
            ledTimer = LED_TIMEOUT;
            greenLed ^= LED_OFF;
            Scanning_LED_Write(greenLed);
        }
    }
    /* Blink blue LED to indicate that device has received a notification */
    else if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
    {
        if((LED_ON == Notification_LED_Read()) && (--ledTimer == 0u))
        {
            ledTimer = LED_TIMEOUT;
            Notification_LED_Write(LED_OFF);
        }
    }
    else
    {
        /* nothing else */
    }
}


int main()
{
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
    CYBLE_STACK_LIB_VERSION_T stackVersion;

    CyGlobalIntEnable;
    
    UART_DEB_Start();               /* Start communication component */
    printf("BLE Heart Rate Collector Example Project \r\n");
    
    Disconnect_LED_Write(LED_OFF);
    Scanning_LED_Write(LED_OFF);
    Notification_LED_Write(LED_OFF);

    apiResult = CyBle_Start(AppCallBack);
    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("CyBle_Start API Error: 0x%x \r\n", apiResult);
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

    CyBle_BasRegisterAttrCallback(BasCallBack);
    HrsInit();

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
                    /* Put the device into the DeepSleep mode only when all debug information has been sent */
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
            
            /* Handle advertising led blinking */
            HandleLeds();
        }
        
        /* Store bonding data to flash only when all debug information has been sent */
        if((cyBle_pendingFlashWrite != 0u) &&
           ((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) == 0u))
        {
            apiResult = CyBle_StoreBondingData(0u);
            printf("Store bonding data, status: %x \r\n", apiResult);
        }
        
        
        /*******************************************************************
        *  Processes all pending BLE events in the stack
        *******************************************************************/        
        CyBle_ProcessEvents();
    }
}

/* [] END OF FILE */
