/*****************************************************************************************
* File Name: app_UART.c
*
* Description:
*  Common BLE application code for client devices.
*
*****************************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*****************************************************************************************/

#include "app_UART.h"
#include "app_Ble.h"


/*****************************************************************************************
* Function Name: HandleUartTxTraffic
******************************************************************************************
*
* Summary:
*  This function takes data from UART RX buffer and pushes it to the server 
*  as Notifications.
*
* Parameters:
*  uint16 - CCCD for checking if notifications are enabled  
*
* Return:
*   None.
*
*****************************************************************************************/
void HandleUartTxTraffic(uint16 txDataClientConfigDesc)
{
    uint8   index;
    uint8   uartTxData[mtuSize - 3];
    uint16  uartTxDataLength;
    
    static uint16 uartIdleCount = UART_IDLE_TIMEOUT;
    
    CYBLE_API_RESULT_T                  bleApiResult;
    CYBLE_GATTS_HANDLE_VALUE_NTF_T      uartTxDataNtf;
    
    uartTxDataLength = UART_SpiUartGetRxBufferSize();
    
    #ifdef FLOW_CONTROL
        if(uartTxDataLength >= (UART_UART_RX_BUFFER_SIZE - (UART_UART_RX_BUFFER_SIZE/2)))
        {
            DisableUartRxInt();
        }
        else
        {
            EnableUartRxInt();
        }
    #endif
    
    if((0 != uartTxDataLength) && (NOTIFICATON_ENABLED == txDataClientConfigDesc))
    {
        if(uartTxDataLength >= (mtuSize - 3))
        {
            uartIdleCount       = UART_IDLE_TIMEOUT;
            uartTxDataLength    = mtuSize - 3;
        }
        else
        {
            if(--uartIdleCount == 0)
            {
                /*uartTxDataLength remains unchanged */;
            }
            else
            {
                uartTxDataLength = 0;
            }
        }
        
        if(0 != uartTxDataLength)
        {
            uartIdleCount       = UART_IDLE_TIMEOUT;
            
            for(index = 0; index < uartTxDataLength; index++)
            {
                uartTxData[index] = (uint8) UART_UartGetByte();
            }
            
            uartTxDataNtf.value.val  = uartTxData;
            uartTxDataNtf.value.len  = uartTxDataLength;
            uartTxDataNtf.attrHandle = CYBLE_SERVER_UART_SERVER_UART_TX_DATA_CHAR_HANDLE;
            
            #ifdef FLOW_CONTROL
                DisableUartRxInt();
            #endif
            
            do
            {
                bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &uartTxDataNtf);
                CyBle_ProcessEvents();
            }
            while((CYBLE_ERROR_OK != bleApiResult)  && (CYBLE_STATE_CONNECTED == cyBle_state));
        }
    }
}

/*****************************************************************************************
* Function Name: HandleUartRxTraffic
******************************************************************************************
*
* Summary:
*  This function takes data from received "write without response" command from
*  server and, pushes it to the UART TX buffer. 
*
* Parameters:
*  CYBLE_GATTS_WRITE_REQ_PARAM_T * - the "write without response" param as
*                                    recieved by the BLE stack
*
* Return:
*   None.
*
*****************************************************************************************/
void HandleUartRxTraffic(CYBLE_GATTS_WRITE_REQ_PARAM_T * uartRxDataWrReq)
{
    if(uartRxDataWrReq->handleValPair.attrHandle == CYBLE_SERVER_UART_SERVER_UART_RX_DATA_CHAR_HANDLE)
    {
        UART_SpiUartPutArray(uartRxDataWrReq->handleValPair.value.val, \
                                    (uint32) uartRxDataWrReq->handleValPair.value.len);
    }
}

/*****************************************************************************************
* Function Name: DisableUartRxInt
******************************************************************************************
*
* Summary:
*  This function disables the UART RX interrupt.
*
* Parameters:
*   None.
*
* Return:
*   None.
*
*****************************************************************************************/
void DisableUartRxInt(void)
{
    UART_INTR_RX_MASK_REG &= ~UART_RX_INTR_MASK;  
}

/*****************************************************************************************
* Function Name: EnableUartRxInt
******************************************************************************************
*
* Summary:
*  This function enables the UART RX interrupt.
*
* Parameters:
*   None.
*
* Return:
*   None.
*
*****************************************************************************************/
void EnableUartRxInt(void)
{
    UART_INTR_RX_MASK_REG |= UART_RX_INTR_MASK;  
}

/* [] END OF FILE */
