/*******************************************************************************
* File Name: app_UART.c
*
* Description:
*  Common BLE application code for client devices.
*
*******************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "app_UART.h"

/*******************************************************************************
* Function Name: HandleUartRxTraffic
********************************************************************************
*
* Summary:
*  This function takes data from received notfications and pushes it to the 
*  UART TX buffer. 
*
* Parameters:
*  CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T * - the notification parameter as  
*                                           recieved by the BLE stack
*
* Return:
*   None.
*
*******************************************************************************/
void HandleUartRxTraffic(CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *uartRxDataNotification)
{
    if(uartRxDataNotification->handleValPair.attrHandle == txCharHandle)
    {
        UART_SpiUartPutArray(uartRxDataNotification->handleValPair.value.val, \
            (uint32) uartRxDataNotification->handleValPair.value.len);
    }
}


/*******************************************************************************
* Function Name: HandleUartTxTraffic
********************************************************************************
*
* Summary:
*  This function takes data from UART RX buffer and pushes it to the server 
*  as Write Without Response command.
*
* Parameters:
*  None.
*
* Return:
*   None.
*
*******************************************************************************/
void HandleUartTxTraffic(void)
{
    uint8   index;
    uint8   uartTxData[mtuSize - 3];
    uint16  uartTxDataLength;
    
    static uint16 uartIdleCount = UART_IDLE_TIMEOUT;
    
    CYBLE_API_RESULT_T              bleApiResult;
    CYBLE_GATTC_WRITE_CMD_REQ_T     uartTxDataWriteCmd;
    
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
    
    if((uartTxDataLength != 0))
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
            
            uartTxDataWriteCmd.attrHandle = rxCharHandle;
            uartTxDataWriteCmd.value.len  = uartTxDataLength;
            uartTxDataWriteCmd.value.val  = uartTxData;           
            
            #ifdef FLOW_CONTROL
                DisableUartRxInt();
            #endif
            
            do
            {
                bleApiResult = CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &uartTxDataWriteCmd);
                CyBle_ProcessEvents();
            }
            while((CYBLE_ERROR_OK != bleApiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
            
        }
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
