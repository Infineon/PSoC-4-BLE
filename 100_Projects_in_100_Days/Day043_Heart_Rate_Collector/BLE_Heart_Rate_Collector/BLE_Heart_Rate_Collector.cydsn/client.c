/*******************************************************************************
* File Name: client.c
*
* Version: 1.0
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

#include <project.h>
#include <stdio.h>
#include "common.h"

#define CONNECT 0x01
uint8 flag = 0;
uint8 advDevices = 0u;
uint8 deviceN = 0u;
CYBLE_UUID16 serviceUuid = 0x0000u;
CYBLE_GAP_BD_ADDR_T peerAddr[CYBLE_MAX_ADV_DEVICES];


void StartScan(CYBLE_UUID16 uuid)
{
    serviceUuid = uuid;
    apiResult = CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
	if(apiResult != CYBLE_ERROR_OK)
    {
        printf("StartScan API Error: %xd \r\n", apiResult);
    }
    else
    {
        printf("Start Scan \r\n");
    }
}

void StartDiscovery(void)
{
    apiResult = CyBle_GattcStartDiscovery(cyBle_connHandle);

    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("CyBle_GattcStartDiscovery API Error: %x \r\n", apiResult);
    }
    else
    {
        printf("Start Discovery \r\n");
    }
}

void ScanProgressEventHandler(CYBLE_GAPC_ADV_REPORT_T* eventParam)
{
    uint8 newDevice = 0u, device = 0u;
    uint8 i;
    uint8 adStructPtr = 0u;
    uint8 adTypePtr = 0u;
    uint8 nextPtr = 0u;

    printf("SCAN_PROGRESS_RESULT: peerAddrType - %d, ", eventParam->peerAddrType);
    printf("peerBdAddr - ");
    for(newDevice = 1u, i = 0u; i < advDevices; i++)
    {
        if((memcmp(peerAddr[i].bdAddr, eventParam->peerBdAddr, CYBLE_GAP_BD_ADDR_SIZE) == 0)) /* same address */
        {
            device = i;
            printf("%x: ", device);
            newDevice = 0u;
            break;
        }
    }
    if(newDevice != 0u)
    {
        if(advDevices < CYBLE_MAX_ADV_DEVICES)
        {
            memcpy(peerAddr[advDevices].bdAddr, eventParam->peerBdAddr, CYBLE_GAP_BD_ADDR_SIZE);
            peerAddr[advDevices].type = eventParam->peerAddrType;
            device = advDevices;
            advDevices++;
            printf("%x: ", device);

        }
    }
    for(i = CYBLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
    {
        printf("%2.2x", eventParam->peerBdAddr[i-1]);
    }
    printf(", rssi - %d dBm, data - ", eventParam->rssi);

    /* Print and parse advertisement data and connect to device which has HRM */
    adStructPtr = 0u;
    for(i = 0; i < eventParam->dataLen; i++)
    {
        printf("%2.2x ", eventParam->data[i]);

        if(i == adStructPtr)
        {
            adTypePtr = i + 1;
            adStructPtr += eventParam->data[i] + 1;
            nextPtr = 1;
        }
        else if(i == (adTypePtr + nextPtr))
        {
            switch(eventParam->data[adTypePtr])
            {
                case CYBLE_GAP_ADV_FLAGS:
                    break;

                case CYBLE_GAP_ADV_INCOMPL_16UUID:
                case CYBLE_GAP_ADV_COMPL_16UUID:
                    if(serviceUuid == CyBle_Get16ByPtr(&(eventParam->data[i])))
                    {
                        newDevice = 2; /* temporary use newDevice as a flag */
                    }
                    else
                    {
                        nextPtr += 2;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    if(2 == newDevice)
    {
        deviceN = device;
        printf("          This device contains ");
        switch(serviceUuid)
        {
            case CYBLE_UUID_HEART_RATE_SERVICE:
                printf("Heart Rate Service \r\n");
                printf("Stop Scanning, waiting for Scanning event \r\n");
                flag |= CONNECT;
                CyBle_GapcStopScan();
                break;
                
            case CYBLE_UUID_GLUCOSE_SERVICE:
                printf("Glucose Service \r\n");
                break;
                
			case CYBLE_UUID_BLOOD_PRESSURE_SERVICE:
				printf("Blood Pressure Service \r\n");
                break;
				
                /* other services */
                
            default:
                printf("%x \r\n", serviceUuid);
                break;
        }
    }
    else
    {
        printf("\r\n");
    }
}

void ClientDebugOut(uint32 event, void* eventParam)
{
    uint16 length, i;
    CYBLE_GATTC_GRP_ATTR_DATA_LIST_T *locAttrData;
    uint8 type, *locHndlUuidList;

    switch(event)
    {
        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
            ScanProgressEventHandler((CYBLE_GAPC_ADV_REPORT_T *)eventParam);
            break;

        case CYBLE_EVT_GAPC_SCAN_START_STOP:
            printf("EVT_GAPC_SCAN_START_STOP, state: %x\r\n", CyBle_GetState());
            if(0u != (flag & CONNECT))
            {
                printf("Connect to the Device: %x \r\n", deviceN);
                /* Connect to selected device */
                apiResult = CyBle_GapcConnectDevice(&peerAddr[deviceN]);
                if(apiResult != CYBLE_ERROR_OK)
                {
                    printf("ConnectDevice API Error: %x \r\n", apiResult);
                }

                flag &= ~CONNECT;
            }
            else if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {   
                /* Scanning period complete, go to low power  
                 * mode (Hibernate mode) and wait for an external
                 * user event to wake up the device again */
                printf("Hibernate \r\n");
                Scanning_LED_Write(LED_OFF);
                Disconnect_LED_Write(LED_ON);
                Notification_LED_Write(LED_OFF);
                while((UART_DEB_SpiUartGetTxBufferSize() + UART_DEB_GET_TX_FIFO_SR_VALID) != 0);
                SW2_ClearInterrupt();
                Wakeup_Interrupt_ClearPending();
                Wakeup_Interrupt_Start();
                CySysPmHibernate();
            }
            break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            CyBle_GapAddDeviceToWhiteList(&peerAddr[deviceN]);
            break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/

        case CYBLE_EVT_GATTC_ERROR_RSP:
            printf("EVT_GATTC_ERROR_RSP: opcode: ");
            switch(((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->opCode)
            {
                case CYBLE_GATT_FIND_INFO_REQ:
                    printf("FIND_INFO_REQ");
                    break;

                case CYBLE_GATT_READ_BY_TYPE_REQ:
                    printf("READ_BY_TYPE_REQ");
                    break;

                case CYBLE_GATT_READ_BY_GROUP_REQ:
                    printf("READ_BY_GROUP_REQ");
                    break;

                default:
                    printf("%x", ((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->opCode);
                    break;
            }
            printf(",  handle: %x,  ", ((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->attrHandle);
            printf("errorcode: ");
            switch(((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->errorCode)
            {
                case CYBLE_GATT_ERR_ATTRIBUTE_NOT_FOUND:
                    printf("ATTRIBUTE_NOT_FOUND");
                    break;

                case CYBLE_GATT_ERR_READ_NOT_PERMITTED:
                    printf("READ_NOT_PERMITTED");
                    break;

                default:
                    printf("%x", ((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->errorCode);
                    break;
            }
            printf("\r\n");
            break;

        case CYBLE_EVT_GATTC_READ_RSP:
            printf("EVT_GATTC_READ_RSP: ");
            length = ((CYBLE_GATTC_READ_RSP_PARAM_T *)eventParam)->value.len;
            for(i = 0; i < length; i++)
            {
                printf("%2.2x ", ((CYBLE_GATTC_READ_RSP_PARAM_T *)eventParam)->value.val[i]);
            }
            printf("\r\n");
            break;

        case CYBLE_EVT_GATTC_WRITE_RSP:
            printf("EVT_GATTC_WRITE_RSP: ");
            printf("bdHandle: %x \r\n", ((CYBLE_CONN_HANDLE_T *)eventParam)->bdHandle);
            break;

        case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
            printf("EVT_GATTC_XCHNG_MTU_RSP \r\n");
            break;

        case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP: /* Response to CYBLE_DiscoverAllPrimServices() */
            printf("EVT_GATTC_READ_BY_GROUP_TYPE_RSP: ");
            locAttrData = &(*(CYBLE_GATTC_READ_BY_GRP_RSP_PARAM_T *)eventParam).attrData;
            for(i = 0u; i < locAttrData -> attrLen; i ++)
            {
                printf("%2.2x ",*(uint8 *)(locAttrData->attrValue + i));
            }
            printf("\r\n");
            break;

        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:      /* Response to CYBLE_DiscoverAllCharacteristicsOfService() */
            printf("EVT_GATTC_READ_BY_TYPE_RSP: ");
            locAttrData = &(*(CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T *)eventParam).attrData;
            for(i = 0u; i < locAttrData -> attrLen; i ++)
            {
                printf("%2.2x ",*(uint8 *)(locAttrData->attrValue + i));
            }
            printf("\r\n");
            break;

        case CYBLE_EVT_GATTC_FIND_INFO_RSP:
            printf("EVT_GATTC_FIND_INFO_RSP: ");
            type = (*(CYBLE_GATTC_FIND_INFO_RSP_PARAM_T *)eventParam).uuidFormat;
            locHndlUuidList = (*(CYBLE_GATTC_FIND_INFO_RSP_PARAM_T *)eventParam).handleValueList.list;
            if(type == CYBLE_GATT_16_BIT_UUID_FORMAT)
            {
                length = CYBLE_ATTR_HANDLE_LEN + CYBLE_GATT_16_BIT_UUID_SIZE;
            }
            else
            {
                length = CYBLE_ATTR_HANDLE_LEN + CYBLE_GATT_128_BIT_UUID_SIZE;
            }

            for(i = 0u; i < (*(CYBLE_GATTC_FIND_INFO_RSP_PARAM_T *)eventParam).handleValueList.byteCount; i += length)
            {
                if(type == CYBLE_GATT_16_BIT_UUID_FORMAT)
                {
                    printf("%2.2x ",CyBle_Get16ByPtr(locHndlUuidList + i + CYBLE_ATTR_HANDLE_LEN));
                }
                else
                {
                    printf("UUID128");
                }
            }
            printf("\r\n");
            break;

        case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
            printf("EVT_GATTC_FIND_BY_TYPE_VALUE_RSP \r\n");
            break;

        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
            printf("EVT_GATTC_HANDLE_VALUE_NTF \r\n");
            break;

        /**********************************************************
        *                       Discovery Events
        ***********************************************************/
        case CYBLE_EVT_GATTC_SRVC_DUPLICATION:
            printf("EVT_GATTC_SRVC_DUPLICATION, UUID: %x \r\n", *(uint16 *)eventParam);
            break;

        case CYBLE_EVT_GATTC_CHAR_DUPLICATION:
            printf("EVT_GATTC_CHAR_DUPLICATION, UUID: %x \r\n", *(uint16 *)eventParam);
            break;

        case CYBLE_EVT_GATTC_DESCR_DUPLICATION:
            printf("EVT_GATTC_DESCR_DUPLICATION, UUID: %x \r\n", *(uint16 *)eventParam);
            break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_FAILED:
            printf("EVT_GATTC_DISCOVERY_FAILED \r\n");
            break;

        case CYBLE_EVT_GATTC_SRVC_DISCOVERY_COMPLETE:
            printf("EVT_GATTC_SRVC_DISCOVERY_COMPLETE \r\n");
            break;

        case CYBLE_EVT_GATTC_INCL_DISCOVERY_COMPLETE:
            printf("EVT_GATTC_INCL_DISCOVERY_COMPLETE \r\n");
            break;

        case CYBLE_EVT_GATTC_CHAR_DISCOVERY_COMPLETE:
            printf("EVT_GATTC_CHAR_DISCOVERY_COMPLETE \r\n");
            break;

        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
            printf("EVT_GATTC_DISCOVERY_COMPLETE \r\n");
            break;


        default:
        #if (0)
            printf("unknown event: %lx \r\n", event);
        #endif
            break;
    }
}


/* [] END OF FILE */
