/*******************************************************************************
* File Name: server.c
*
* Version 1.0
*
* Description:
*  Common BLE application code for server devices.
*
*******************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include <stdio.h>
#include "common.h"


/*******************************************************************************
* Function Name: StartAdvertisement
********************************************************************************
*
* Summary:
*   Initiates the advertisement procedure.
*   Prints the Device Address.
*
* Parameters:
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void StartAdvertisement(void)
{
    uint16 i;
    CYBLE_GAP_BD_ADDR_T localAddr;
    apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("StartAdvertisement API Error: %x \r\n", (int) apiResult);
    }
    else
    {
        printf("Start Advertisement with addr: ");
        CyBle_GetDeviceAddress(&localAddr);
        for(i = CYBLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
        {
            printf("%2.2x", localAddr.bdAddr[i-1]);
        }
        printf("\r\n");
    }
}

/*******************************************************************************
* Function Name: ServerDebugOut
********************************************************************************
*
* Summary:
*   GATT server specific event handler.
*
* Parameters:
*  event - the event code.
*  *eventParam - the event parameters.
*
* Return:
*   None.
*
*******************************************************************************/
void ServerDebugOut(uint32 event, void* eventParam)
{
    uint16 i;
    switch(event)
    {
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            printf("CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP, state: %x\r\n", CyBle_GetState());
            if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {   
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate mode) and wait for an external
                 * user event to wake up the device again */
                printf("Hibernate \r\n");
                Advertising_LED_Write(LED_OFF);
                Disconnect_LED_Write(LED_ON);
                while((UART_SpiUartGetTxBufferSize() + UART_GET_TX_FIFO_SR_VALID) != 0);
                SW2_ClearInterrupt();
                Wakeup_Interrupt_ClearPending();
                Wakeup_Interrupt_Start();
                CySysPmHibernate();
            }
            break;

        case CYBLE_EVT_GATTS_WRITE_REQ:
            printf("EVT_GATT_WRITE_REQ: %x = ",((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->handleValPair.attrHandle);
            for(i = 0; i < ((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->handleValPair.value.len; i++)
            {
                printf("%2.2x ", ((CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam)->handleValPair.value.val[i]);
            }
            printf("\r\n");
            break;

        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
            printf("EVT_GATTS_XCNHG_MTU_REQ \r\n");
            break;

        case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
            printf("EVT_GATTS_HANDLE_VALUE_CNF \r\n");
            break;

        default:
        #if (0)
            printf("unknown event: %lx \r\n", event);
        #endif
            break;
    }
}


/* [] END OF FILE */
