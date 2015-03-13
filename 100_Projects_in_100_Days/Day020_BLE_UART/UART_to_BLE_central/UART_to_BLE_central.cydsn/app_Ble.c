/*******************************************************************************
* File Name: app_Ble.c
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

#include "app_Ble.h"

uint16 txCharHandle             = 0;                /* Handle for the TX data characteristic */
uint16 rxCharHandle             = 0;                /* Handle for the RX data characteristic */
uint16 txCharDescHandle         = 0;                /* Handle for the TX data characteristic descriptor */
uint16 bleUartServiceHandle     = 0;                /* Handle for the BLE UART service */
uint16 bleUartServiceEndHandle  = 0;                /* End handle for the BLE UART service */
uint16 mtuSize                  = CYBLE_GATT_MTU;   /* MTU size to be used by Client and Server after MTU exchange */

const uint8 enableNotificationParam[2] = {0x01, 0x00};

volatile static bool peerDeviceFound         = false;
volatile static bool notificationEnabled     = false;

static CYBLE_GAP_BD_ADDR_T      peerAddr;           /* BD address of the peer device */
static INFO_EXCHANGE_STATE_T    infoExchangeState   = INFO_EXCHANGE_START;

CYBLE_GATT_ATTR_HANDLE_RANGE_T  attrHandleRange;
CYBLE_GATTC_FIND_INFO_REQ_T     charDescHandleRange;

/* UUID of the custom BLE UART service */
const uint8 bleUartServiceUuid[16]    = {
                                            0x31, 0x01, 0x9b, 0x5f, 0x80, 0x00, 0x00,0x80, \
                                            0x00, 0x10, 0x00, 0x00, 0xd0, 0xcd, 0x03, 0x00 \
                                        };

/* UUID of the TX attribute of the custom BLE UART service */
const uint8 uartTxAttrUuid[16]        = {
                                            0x31, 0x01, 0x9b, 0x5f, 0x80, 0x00, 0x00,0x80, \
                                            0x00, 0x10, 0x00, 0x00, 0xd1, 0xcd, 0x03, 0x00 \
                                        };

/* UUID of the RX attribute of the custom BLE UART service */
const uint8 uartRxAttrUuid[16]        = {
                                            0x31, 0x01, 0x9b, 0x5f, 0x80, 0x00, 0x00,0x80, \
                                            0x00, 0x10, 0x00, 0x00, 0xd2, 0xcd, 0x03, 0x00 \
                                        };

/* structure to be passed for discovering service by UUID */
const CYBLE_GATT_VALUE_T    bleUartServiceUuidInfo = { 
                                                        (uint8 *) bleUartServiceUuid, \
                                                        CYBLE_GATT_128_BIT_UUID_SIZE,\
                                                        CYBLE_GATT_128_BIT_UUID_SIZE \
                                                      };

/* structure to be passed for the enable notification request */
CYBLE_GATTC_WRITE_REQ_T     enableNotificationReqParam   = {
                                                                {(uint8*)enableNotificationParam, 2, 2},
                                                                0
                                                            };


/*******************************************************************************
* Function Name: HandleBleProcessing
********************************************************************************
*
* Summary:
*   Handles the BLE state machine for intiating different procedures
*   during different states of BLESS.
*
* Parameters:
*   None.
*
* Return:
*   None.
*
*******************************************************************************/
void HandleBleProcessing(void)
{    
    CYBLE_API_RESULT_T      cyble_api_result;
    
    switch (cyBle_state)
    {
        case CYBLE_STATE_SCANNING:
            if(peerDeviceFound)
            {
                CyBle_GapcStopScan();
            }
            break;
    
        case CYBLE_STATE_CONNECTED:
            
            /* if Client does not has all the information about attribute handles 
             * call procedure for getting it */
            if((INFO_EXCHANGE_COMPLETE != infoExchangeState))
            {
                attrHandleInit();
            }
            
            /* enable notifications if not enabled already */
            else if(false == notificationEnabled)
            {
                enableNotifications();
            }
            
            /* if client has all required info and stack is free, handle UART traffic */
            else if(CyBle_GattGetBusStatus() != CYBLE_STACK_STATE_BUSY)
            {
                HandleUartTxTraffic();
            }
            
            break;
                
        case CYBLE_STATE_DISCONNECTED:
        {
            if(peerDeviceFound)
            {
                cyble_api_result = CyBle_GapcConnectDevice(&peerAddr);
                
			    if(CYBLE_ERROR_OK == cyble_api_result)
			    {
				    peerDeviceFound = false;
			    }
            }
            else
            {
                CyBle_GapcStartScan(CYBLE_SCANNING_FAST);  
            }
            break;
        }
        
        default:
            break;       
    }
}


/*******************************************************************************
* Function Name: AppCallBack
********************************************************************************
*
* Summary:
*   Call back function for BLE stack to handle BLESS events
*
* Parameters:
*   event       - the event generated by stack
*   eventParam  - the parameters related to the corresponding event
*
* Return:
*   None.
*
*******************************************************************************/
void AppCallBack(uint32 event, void *eventParam)
{
    CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T    *readResponse;
    CYBLE_GAPC_ADV_REPORT_T		            *advReport;
    CYBLE_GATTC_FIND_BY_TYPE_RSP_PARAM_T    *findResponse;
    CYBLE_GATTC_FIND_INFO_RSP_PARAM_T       *findInfoResponse;
    
    switch (event)
    {
        case CYBLE_EVT_STACK_ON:
      
            break;
        
        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
            
            advReport = (CYBLE_GAPC_ADV_REPORT_T *) eventParam;
            
            /* check if report has manfacturing data corresponding to the intended matching peer */
            if((advReport->eventType == CYBLE_GAPC_SCAN_RSP) && (advReport->dataLen == 0x06) \
                    && (advReport->data[1] == 0xff) && (advReport->data[2] == 0x31)  \
                    && (advReport->data[3] == 0x01) && (advReport->data[4] == 0x3b) \
                    && (advReport->data[5] == 0x04))
            {
                peerDeviceFound = true;
                
                memcpy(peerAddr.bdAddr, advReport->peerBdAddr, sizeof(peerAddr.bdAddr));
                peerAddr.type = advReport->peerAddrType;
                
                #ifdef PRINT_MESSAGE_LOG   
                    UART_UartPutString("\n\r\n\rServer with matching custom service discovered...");
                #endif
            }           
            
            break;    
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            
            /* RESET all flags */
            peerDeviceFound         = false;
            notificationEnabled     = false;
            infoExchangeState       = INFO_EXCHANGE_START;
            
            #ifdef PRINT_MESSAGE_LOG   
                UART_UartPutString("\n\r DISCONNECTED!!! \n\r ");
                while(0 != (UART_SpiUartGetTxBufferSize() + UART_GET_TX_FIFO_SR_VALID));
            #endif
            
            /* RESET Uart and flush all buffers */
            UART_Stop();
            UART_SpiUartClearTxBuffer();
            UART_SpiUartClearRxBuffer();
            UART_Start();
            
            break;
        
        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
            
            readResponse = (CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T *) eventParam;
            
            if(0 == memcmp((uint8 *)&(readResponse->attrData.attrValue[5]), (uint8 *)uartTxAttrUuid, 16))
            {
                txCharHandle = readResponse->attrData.attrValue[3];
                txCharHandle |= (readResponse->attrData.attrValue[4] << 8);
                
                infoExchangeState |= TX_ATTR_HANDLE_FOUND;
            }
            else if(0 == memcmp((uint8 *)&(readResponse->attrData.attrValue[5]), (uint8 *)uartRxAttrUuid, 16))
            {
                rxCharHandle = readResponse->attrData.attrValue[3];
                rxCharHandle |= (readResponse->attrData.attrValue[4] << 8);
                
                infoExchangeState |= RX_ATTR_HANDLE_FOUND;
               
            }
            
            break;
            
        case CYBLE_EVT_GATTC_FIND_INFO_RSP:
            
            findInfoResponse = (CYBLE_GATTC_FIND_INFO_RSP_PARAM_T *) eventParam;
            
            if((0x29 == findInfoResponse->handleValueList.list[3]) && \
                                (0x02 == findInfoResponse->handleValueList.list[2]))
            {
                txCharDescHandle = findInfoResponse->handleValueList.list[0];
                txCharDescHandle |= findInfoResponse->handleValueList.list[1] << 8;
            
                infoExchangeState |= TX_CCCD_HANDLE_FOUND;
            }
           
            break;
            
        case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:   
            
            /*set the 'mtuSize' variable based on the minimum MTU supported by both devices */
            if(CYBLE_GATT_MTU > ((CYBLE_GATT_XCHG_MTU_PARAM_T *)eventParam)->mtu)
            {
                mtuSize = ((CYBLE_GATT_XCHG_MTU_PARAM_T *)eventParam)->mtu;
            }
            else
            {
                mtuSize = CYBLE_GATT_MTU;
            }
            
            infoExchangeState |= MTU_XCHNG_COMPLETE;
            
            break;
            
        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
            
            HandleUartRxTraffic((CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *)eventParam);
			
            break;
        
        case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
            
            findResponse            = (CYBLE_GATTC_FIND_BY_TYPE_RSP_PARAM_T *) eventParam;
            
            bleUartServiceHandle    = findResponse->range->startHandle;
            bleUartServiceEndHandle = findResponse->range->endHandle;
            
            infoExchangeState |= BLE_UART_SERVICE_HANDLE_FOUND;
            
            break;
        
        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
            
            /*set the 'mtuSize' variable based on the minimum MTU supported by both devices */
            if(CYBLE_GATT_MTU > ((CYBLE_GATT_XCHG_MTU_PARAM_T *)eventParam)->mtu)
            {
                mtuSize = ((CYBLE_GATT_XCHG_MTU_PARAM_T *)eventParam)->mtu;
            }
            else
            {
                mtuSize = CYBLE_GATT_MTU;
            }
            
            break;    
        
        case CYBLE_EVT_GATTC_WRITE_RSP:
            
            notificationEnabled = true;
            
            #ifdef PRINT_MESSAGE_LOG   
                UART_UartPutString("\n\rNotifications enabled\n\r");
                UART_UartPutString("\n\rStart entering data:\n\r");
            #endif
            
            break;
        
        case CYBLE_EVT_GATT_CONNECT_IND:
            
            #ifdef PRINT_MESSAGE_LOG   
                UART_UartPutString("\n\rConnection established");             
            #endif
            
            break;
            
        default:            
            break;
    }
}


/*******************************************************************************
* Function Name: attrHandleInit
********************************************************************************
*
* Summary:
*  This function gathhers all the information like attribute handles and MTU size
*  from the server.
*
* Parameters:
*  None.
*
* Return:
*   None.
*
*******************************************************************************/
void attrHandleInit()
{
    switch(infoExchangeState)
    {
        case INFO_EXCHANGE_START:        
            CyBle_GattcDiscoverPrimaryServiceByUuid(cyBle_connHandle, bleUartServiceUuidInfo);
            break;
        
        case BLE_UART_SERVICE_HANDLE_FOUND:
            attrHandleRange.startHandle    = bleUartServiceHandle;
            attrHandleRange.endHandle      = bleUartServiceEndHandle;

            CyBle_GattcDiscoverAllCharacteristics(cyBle_connHandle, attrHandleRange);
            break;
        
        case (SERVICE_AND_CHAR_HANDLES_FOUND):
            charDescHandleRange.startHandle = txCharHandle + 1;
            charDescHandleRange.endHandle   = bleUartServiceEndHandle;

            CyBle_GattcDiscoverAllCharacteristicDescriptors(cyBle_connHandle, &charDescHandleRange);
            break;
        
        case (ALL_HANDLES_FOUND):
            CyBle_GattcExchangeMtuReq(cyBle_connHandle, CYBLE_GATT_MTU);
            break;    
            
        default:
            break;    
    }
    
    CyBle_ProcessEvents();
}

/*******************************************************************************
* Function Name: enableNotifications
********************************************************************************
*
* Summary:
*  This function enables notfications from servers. 
*
* Parameters:
*  None.
*
* Return:
*   None.
*
*******************************************************************************/
void enableNotifications()
{     
    enableNotificationReqParam.attrHandle = txCharDescHandle;   
    CyBle_GattcWriteCharacteristicDescriptors(cyBle_connHandle, (CYBLE_GATTC_WRITE_REQ_T *)(&enableNotificationReqParam));
}
/* [] END OF FILE */
