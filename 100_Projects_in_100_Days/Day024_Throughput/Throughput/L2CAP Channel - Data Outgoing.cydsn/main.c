/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This is the source code for the project which demonstrates BLE data transfer
*  via an L2CAP connection-oriented channel.
*  This project sends data on the L2CAP channel to be received by a peer device.
*
* Hardware Dependency:
*  CY8CKIT-042-BLE Bluetooth Low Energy (BLE) Pioneer Kit
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
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
*******************************************************************************/

/*******************************************************************************
* Included headers
*******************************************************************************/
#include <project.h>
#include <stdbool.h>
#include "common.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define PEER_DEVICE_PSM             (73)
#define PEER_DEVICE_TX_CREDITS      (0)

#define LOCAL_DEVICE_PSM            (43)
#define LOCAL_DEVICE_MTU            (512)
#define LOCAL_DEVICE_MPS            (512)

#define MAX_MTU_SIZE                (512)


/*******************************************************************************
* Enumerations
*******************************************************************************/
typedef enum 
{
    CHANNEL_PSM_NOT_REGISTERED,
    CHANNEL_PSM_REGISTERED,
    CHANNEL_SENT_REQUEST,
    CHANNEL_CREATED
} CHANNEL_STATE;
    

/*******************************************************************************
* Variables
*******************************************************************************/
/* This variable holds the channel ID for the L2CAP channel */
uint16 l2capCid;

/* Buffer to hold data */
uint8 buffer[MAX_MTU_SIZE];
    
/* L2CAP Channel parameters for the local device */
CYBLE_L2CAP_CBFC_CONNECT_PARAM_T cbfcLocalParameters = 
{
    LOCAL_DEVICE_MTU,         /* MTU size of this device */
    LOCAL_DEVICE_MPS,         /* MPS size of this device */
    PEER_DEVICE_TX_CREDITS    /* Initial Credits given to peer device for Tx */
};

/* L2CAP Channel parameters for the peer device - to be updated on the 
 * L2CAP connection response event.
 */
CYBLE_L2CAP_CBFC_CONNECT_PARAM_T cbfcPeerParameters;

/* Variable to track the state of L2CAP channel creation */
CHANNEL_STATE channelState = CHANNEL_PSM_NOT_REGISTERED;

/* Variable to track sending the next data only when previous data is 
 * transmitted completely 
 */
bool previousDataTransmitted = true;

/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: StackEventHandler()
********************************************************************************
* Summary:
* Event handler function for the BLE events processing.
*
* Parameters:
* uint32 eventCode: The event to be processed
* void * eventParam: Pointer to hold the additional information associated 
*                    with an event
*
* Return:
* None
*
* Theory:
* The function is responsible for handling the events generated by the stack.
* In addition to handling general events for BLE advertisement, connection, 
* and disconnection, this function handles the events related to L2CAP CBFC 
* connection-oriented channel connection and disconnection.
*
* For details on L2CAP connection-oriented channels, refer to Bluetooth 4.1 
* specification, Volume 3, Part A, section 3.4.
*
* Side Effects:
* None
*
*******************************************************************************/
void StackEventHandler(uint32 eventCode, void * eventParam)
{
    CYBLE_L2CAP_CBFC_CONN_CNF_PARAM_T cbfcResponse;
    uint8 counter; 
    
    switch(eventCode)
    {
        /* Stack initialized; ready for advertisement */
        case CYBLE_EVT_STACK_ON:
            UART_UartPutString("\n\rAdvertising with Address: ");
            for(counter = 6; counter > 0; counter--)
            {
                UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 1));
                UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 0));
                UART_UartPutChar(' ');
            }
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
        
        /* Advertisement timed out; Restart advertisement */
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
            {
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            }
            break;
        
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            UART_UartPutString("\n\rConnected. ");
            break;
            
        /* Device disconnected */
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            /* The L2CAP channel is disconnected but the PSM is already 
             * registered. Update the state machine.
             */
            channelState = CHANNEL_PSM_REGISTERED;
            previousDataTransmitted = true;
            
            /* Restart advertisement */
            UART_UartPutString("\n\n\rDisconnected. ");
            UART_UartPutString("\n\rAdvertising again. ");
            UART_UartPutString("Address: ");
            for(counter = 6; counter > 0; counter--)
            {
                UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 1));
                UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 0));
                UART_UartPutChar(' ');
            }
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
        
        /* CBFC connection response is received */
        case CYBLE_EVT_L2CAP_CBFC_CONN_CNF:
            cbfcResponse = *(CYBLE_L2CAP_CBFC_CONN_CNF_PARAM_T *)eventParam;
            
            /* If the connection request was accepted */
            if(cbfcResponse.response == CYBLE_L2CAP_CONNECTION_SUCCESSFUL)
            {
                UART_UartPutString("\n\rL2CAP channel connection request accepted. Sending data. ");
                
                /* Cache the connection parameters and channel ID */
                cbfcPeerParameters = cbfcResponse.connParam;
                l2capCid = cbfcResponse.lCid;

                /* Update the state machine to indicate that the channel 
                 * is created.
                 */
                channelState = CHANNEL_CREATED;
            }
            break;
        
        /* Peer device requested for CBFC channel disconnection */
        case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:
            if(*(uint16 *)eventParam == l2capCid)
            {
                /* L2CAP channel disconnected but the PSM is still registered */
                channelState = CHANNEL_PSM_REGISTERED;
                previousDataTransmitted = true;
            }
            break;
            
        /* Invalid credits received from peer device; initiate disconnect */
        case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
            if(((CYBLE_L2CAP_CBFC_LOW_TX_CREDIT_PARAM_T *)eventParam)->result != CYBLE_L2CAP_RESULT_SUCCESS)
            {
                CyBle_L2capDisconnectReq(l2capCid);
                channelState = CHANNEL_PSM_REGISTERED;
            }
            break;

        /* Previous data transmission completed */
        case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
            previousDataTransmitted = true;
            break;
            
        default:
            break;
    }
}


/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
* The top-level application function for the project.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* This is the main function for the application. It does the following -
* 1. Initializes the BLE component
* 2. Initializes a buffer from which data is sent out over BLE
* 3. Registers a Protocol Service Multiplexer (PSM) for the L2CAP channel
* 4. Sends L2CAP channel request to the peer device once BLE connection is made
* 5. Once the L2CAP channel is made, starts sending data until credits exhaust
*    and waits for more credits
* 6. The data is sent only when the previous data is transmitted completely, 
*    to avoid packet loss.
*
* Refer Bluetooth 4.1 specification, Volume 3, Part A, section 3.4 for details.
*
* Side Effects:
* None
*
*******************************************************************************/
int main()
{
    uint32 counter = 0; 
    
    CyGlobalIntEnable; 
    
    CyBle_Start(StackEventHandler);
    UART_Start();

    /* Clear screen and put a welcome message */
    UART_UartPutChar(12);
    UART_UartPutString("========= BLE L2CAP Throughput Measurement - Sender side =========\n\n\r");

    /* Initialize entire buffer; the amount of data sent will 
     * depend on the peer device's MTU and MPS */
    for(counter = 0; counter < MAX_MTU_SIZE; counter++)
    {
        buffer[counter] = counter;
    }

    for(;;)
    {
        if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
        {
            /* State machine for L2CAP channel maintenance */
            switch(channelState)
            {
                case CHANNEL_PSM_NOT_REGISTERED:
                    /* Register a new PSM */
                    if(CYBLE_ERROR_OK == CyBle_L2capCbfcRegisterPsm(LOCAL_DEVICE_PSM, 0))
                    {
                        channelState = CHANNEL_PSM_REGISTERED;
                    }
                    break;
                
                case CHANNEL_PSM_REGISTERED:
                    /* Create a CBFC channel - send a request to the peer device.
                     * Once the peer responds, CYBLE_EVT_L2CAP_CBFC_CONN_CNF 
                     * event will come up on this device.
                     */
                    CyBle_L2capCbfcConnectReq(cyBle_connHandle.bdHandle, PEER_DEVICE_PSM, 
                                              LOCAL_DEVICE_PSM, &cbfcLocalParameters);
                    UART_UartPutString("\n\rL2CAP channel connection request sent. ");
                    channelState = CHANNEL_SENT_REQUEST;
                    break;
                    
                case CHANNEL_CREATED:
                    /* Keep sending data as long as credits are available.
                     * Maximum of (peer device's MTU - 2) bytes can be sent in 
                     * one LE-frame.
                     * New data is sent only when the previous data is 
                     * transmitted completely.
                     */
                    if(previousDataTransmitted == true)
                    {
                        CyBle_L2capChannelDataWrite(cyBle_connHandle.bdHandle, l2capCid, buffer, cbfcPeerParameters.mtu - 2);
                        previousDataTransmitted = false;
                    }
                    break;
                    
                default:
                    break;
            }
        }
        CyBle_ProcessEvents();
    }
}

/* [] END OF FILE */
