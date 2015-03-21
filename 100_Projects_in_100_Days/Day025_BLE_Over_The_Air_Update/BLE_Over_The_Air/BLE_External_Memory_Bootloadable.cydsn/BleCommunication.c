/*******************************************************************************
* File Name: BleCommunication.c
*
* Version: 1.0
*
* Description:
*
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include "BleCommunication.h"

#include <stdio.h>
#include <string.h>
#include "debug.h"

#define BLE_PACKET_SIZE_MAX                 (144u)

extern uint8 packetRX[BLE_PACKET_SIZE_MAX];
extern uint32 packetRXSize;
extern uint32 packetRXFlag;

extern uint8 packetTX[BLE_PACKET_SIZE_MAX];
extern uint32 packetTXSize;

extern CYBLE_CONN_HANDLE_T connHandle;


/*******************************************************************************
* Function Name: CyBtldrCommStart
********************************************************************************
*
* Summary:
*  Enables the communication component for the FRAM memory.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void CyBtldrCommStart(void)
{
    EMI_I2CM_Start();
}


/*******************************************************************************
* Function Name: CyBtldrCommRead
********************************************************************************
*
* Summary:
*  Reads data recieved by BLE component and passes it to Bootloader emulator.
*
* Parameters:
*  pData:     pointer to buffer where data from BLE is to be stored
*  size:      size in bytes of buffer for data from BLE
*  count:     size in bytes of actually received data
*  timeOut:   timeout of operation, currently not used, left for consistancy
*             with bootloader component
*
* Return:
*  None
*
*******************************************************************************/
cystatus CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut)
{
    cystatus status;
    #if (DEBUG_UART_ENABLED == YES)
        uint32 i;
    #endif /* (DEBUG_UART_ENABLED == YES) */

    /* Process BLE events */
    CyBle_ProcessEvents();

    /* If flag is set - this means that BLE component received come data */
    if (packetRXFlag == 1u)
    {
        /*
        Make sure that lenght of data does not exceed buffer, allocated
        in bootloader packet parser.
         */
        packetRXFlag = 0u;
        if (packetRXSize < size)
        {
            (void)memcpy((void *)pData, (const void *)&packetRX[0], packetRXSize);
            *count = packetRXSize;
            
            status = CYRET_SUCCESS;

            #if (DEBUG_UART_ENABLED == YES)
                DBG_PRINTF("->>\r\n");
                for(i = 0; i < packetRXSize; i++)
                {
                    DBG_PRINTF("%02x ", pData[i]);
                }
                DBG_PRINTF(" (%lu bytes)\r\n",packetRXSize);
            #endif /* (DEBUG_UART_ENABLED == YES) */
        }
        else
        {
            status = CYRET_BAD_DATA;
            DBG_PRINTF("Wrong packet size!\r\n\r\n");
        }
        packetRXFlag = 0;
    }
    else
    {
        status = CYRET_TIMEOUT;
    }

    return (status);
}


/*******************************************************************************
* Function Name: CyBtldrCommWrite
********************************************************************************
*
* Summary:
*  Writes data to bootloader host tool via BLE notification, if notifications
*  are enabled from the client-side(Bootloader Host Tool).
*
* Parameters:
*  pData:     pointer to buffer with data
*  size:      size in bytes of data to be sent
*  count:     size in bytes of actually sent data
*  timeOut:   timeout of operation, currently not used, left for consistancy
*             with bootloader component
*
* Return:
*  None
*
*******************************************************************************/
cystatus CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut)
{
    CYBLE_GATTS_HANDLE_VALUE_NTF_T ntfParam;
    cystatus status = CYRET_INVALID_STATE;
    #if (DEBUG_UART_ENABLED == YES)
        uint32 i;
    #endif /* (DEBUG_UART_ENABLED == YES) */

    if (CYBLE_IS_NOTIFICATION_ENABLED(cyBle_customs[0].customServiceInfo[0].customServiceCharDescriptors[0]))
    {
        /*
        Prepeare and send notification with bootloader packet emulator
        response if notifications are enabled
        */
        (void)memcpy((void *)&packetTX[0], (const void *)&pData[0], size);
        packetTXSize = size;
        ntfParam.value.len = size;
        ntfParam.value.val = (uint8 *)&packetTX;
        ntfParam.attrHandle = cyBle_customs[0].customServiceInfo[0].customServiceCharHandle;
        status = CyBle_GattsNotification(connHandle, &ntfParam);

       #if (DEBUG_UART_ENABLED == YES)
            DBG_PRINTF("<<-\r\n");
            for(i = 0; i < size; i++)
            {
                DBG_PRINTF("%02x ", pData[i]);
            }
            DBG_PRINTF(" (%hu bytes)\r\n\r\n",size);
        #endif /* (DEBUG_UART_ENABLED == YES) */

        if (status == CYRET_SUCCESS)
        {
            *count = size;
        }
        else
        {
            status = CYRET_TIMEOUT;
        }
    }

    return (status);
}


/* [] END OF FILE */
