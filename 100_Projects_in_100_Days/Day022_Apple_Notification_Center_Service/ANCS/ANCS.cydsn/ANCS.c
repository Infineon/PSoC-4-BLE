/*******************************************************************************
* File Name: ANCS.c
*
* Version: 1.0
*
* Description:
*  This file implements the ANCS functionality and also is responsible for the
*  discovery of ANCS service, characteristics and descriptors.
*
* Hardware Dependency:
*  CY8CKIT-042-BLE
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
#include <ANCS.h>
#include <common.h>


/*******************************************************************************
* Macros
*******************************************************************************/
#define ANCS_NAME_MAX_LENGTH_MSB            (0)
#define ANCS_NAME_MAX_LENGTH_LSB            (20)


/*******************************************************************************
* Enums for ANCS Service
*******************************************************************************/

typedef enum
{
    ANCS_CATEGORY_ID_OTHER                  = 0,
    ANCS_CATEGORY_ID_INCOMING_CALL,
    ANCS_CATEGORY_ID_MISSED_CALL,
    ANCS_CATEGORY_ID_VOICEMAIL,
    ANCS_CATEGORY_ID_SOCIAL,
    ANCS_CATEGORY_ID_SCHEDULE               = 5,
    ANCS_CATEGORY_ID_EMAIL,
    ANCS_CATEGORY_ID_NEWS,
    ANCS_CATEGORY_ID_HEALTH_AND_FITNESS,
    ANCS_CATEGORY_ID_BUSINESS_AND_FINANCE,
    ANCS_CATEGORY_ID_LOCATION,
    ANCS_CATEGORY_ID_ENTERTAINMENT          = 11
} ANCS_CATEGORY_ID_VALUES;

typedef enum
{
    ANCS_EVENT_ID_NOTIFICATION_ADDED        = 0,
    ANCS_EVENT_ID_NOTIFICATION_MODIFIED,
    ANCS_EVENT_ID_NOTIFICATION_REMOVED
} ANCS_EVENT_ID_VALUES;

typedef enum
{
    ANCS_EVENT_FLAG_SILENT          = (1 << 0),
    ANCS_EVENT_FLAG_IMPORTANT       = (1 << 1),
    ANCS_EVENT_FLAG_PRE_EXISTING    = (1 << 2),
    ANCS_EVENT_FLAG_POSITIVE_ACTION = (1 << 3),
    ANCS_EVENT_FLAG_NEGATIVE_ACTION = (1 << 4)
} ANCS_EVENT_FLAGS;


typedef enum
{
    ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES = 0,
    ANCS_COMMAND_ID_GET_APP_ATTRIBUTES,
    ANCS_COMMAND_ID_PERFORM_NOTIFICATION_ACTION
} ANCS_COMMAND_ID_VALUES;


typedef enum
{
    ANCS_NOTIFICATION_ATTRIBUTE_ID_APP_IDENTIFIER        = 0,
    ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE,
    ANCS_NOTIFICATION_ATTRIBUTE_ID_SUBTITLE,
    ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE,
    ANCS_NOTIFICATION_ATTRIBUTE_ID_MESSAGE_SIZE,
    ANCS_NOTIFICATION_ATTRIBUTE_ID_DATE                  = 5,
    ANCS_NOTIFICATION_ATTRIBUTE_ID_POSITIVE_ACTION_LABEL,
    ANCS_NOTIFICATION_ATTRIBUTE_ID_NEGATIVE_ACTION_LABEL
} ANCS_NOTIFICATION_ATTRIBUTE_ID_VALUES;


typedef enum
{
    ANCS_ACTION_ID_POSITIVE = 0,
    ANCS_ACTION_ID_NEGATIVE
} ANCS_ACTION_ID_VALUES;


typedef enum
{
    ANCS_APP_ATTRIBUTE_ID_DISPLAY_NAME = 0
} ANCS_APP_ATTRIBUTE_ID_VALUES;


/*******************************************************************************
* ANCS Service, Characteristic and Descriptor UUIDs
*******************************************************************************/
const uint8 ancsServiceUuid[] = {0xD0, 0x00, 0x2D, 0x12,
                                 0x1E, 0x4B, 0x0F, 0xA4, 
                                 0x99, 0x4E, 0xCE, 0xB5, 
                                 0x31, 0xF4, 0x05, 0x79
                                };

const uint8 ancsNotifSourceCharUuid[] = {0xBD, 0x1D, 0xA2, 0x99,
                                         0xE6, 0x25, 0x58, 0x8C,
                                         0xD9, 0x42, 0x01, 0x63,
                                         0x0D, 0x12, 0xBF, 0x9F
                                        };

const uint8 ancsControlPointCharUuid[] = {0xD9, 0xD9, 0xAA, 0xFD,
                                          0xBD, 0x9B, 0x21, 0x98,
                                          0xA8, 0x49, 0xE1, 0x45,
                                          0xF3, 0xD8, 0xD1, 0x69
                                        };

const uint8 ancsDataSourceCharUuid[] = {0xFB, 0x7B, 0x7C, 0xCE, 
                                        0x6A, 0xB3, 0x44, 0xBE,
                                        0xB5, 0x4B, 0xD6, 0x24,
                                        0xE9, 0xC6, 0xEA, 0x22
                                       };

#define CCCD_UUID_16BIT   (0x2902)


/*******************************************************************************
* ANCS Notification Source characteristic notification data structure
*******************************************************************************/
typedef struct 
{
    uint8 eventId;
    uint8 eventFlags;
    uint8 categoryId;
    uint8 categoryCount;
    uint8 notificationUid[4];
} ANCS_NOTIF_SOURCE_PACKET;


/*******************************************************************************
* Global variables
*******************************************************************************/

CYBLE_GATT_DB_ATTR_HANDLE_T ancsNotifSourceCharHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;
CYBLE_GATT_DB_ATTR_HANDLE_T ancsNotifSourceCccdHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;

CYBLE_GATT_DB_ATTR_HANDLE_T ancsControlPointCharHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;

CYBLE_GATT_DB_ATTR_HANDLE_T ancsDataSourceCharHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;
CYBLE_GATT_DB_ATTR_HANDLE_T ancsDataSourceCccdHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;


CYBLE_GATT_ATTR_HANDLE_RANGE_T ancsServiceRange = {CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE, CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE};
CYBLE_GATTC_FIND_INFO_REQ_T descriptorHandleRange = {CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE, CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE};


CYBLE_GATTC_WRITE_REQ_T serviceChangedCccdPacket;
CYBLE_GATTC_WRITE_REQ_T ancsNotificationSourceCccdPacket;
CYBLE_GATTC_WRITE_REQ_T ancsDataSourceCccdPacket;


SERVICE_CHANGED_CCCD_WRITE_STATUS serviceChangedCccdWriteStatus;
ANCS_DISCOVERY_STATUS ancsDiscoveryStatus = ANCS_DISC_NONE_DISCOVERED;
ANCS_USAGE_STATUS ancsUsageState = ANCS_USAGE_IDLE;
PRINT_STATUS printStatus = PRINT_NEW_LINE;


uint8 cccdNotifEnabledValue[] = {0x01, 0x00};
uint8 cccdIndEnabledValue[] = {0x02, 0x00};

CYBLE_GATT_VALUE_T cccdNotifFlagSetStruct =
{
    cccdNotifEnabledValue,
    2,
    0
};

CYBLE_GATT_VALUE_T cccdIndFlagSetStruct =
{
    cccdIndEnabledValue,
    2,
    0
};

ANCS_NOTIF_SOURCE_PACKET ancsNotification;

uint8 missedCallCount = 0;
uint8 voiceMailCount = 0;
uint8 emailCount = 0;


/*******************************************************************************
* Function definitions
*******************************************************************************/


/*******************************************************************************
* Function Name: Ancs_CmdGetNotificationAttributeTitle()
********************************************************************************
* Summary:
* The function asks for the Attribute ID title for a GATT notification.
*
* Parameters:
* uint8 * notificationUid: The GATT notification on Notification source 
*                          characteristic for which more information is asked
*
* Return:
* None
*
* Theory:
* The function issues a "Get Notification Attributes" command to the iOS by 
* sending a write request on the Control Point characteristic with this data:
* Byte 0 - Command ID - 0
* Byte 1 - Notification UID[0]
* Byte 2 - Notification UID[1]
* Byte 3 - Notification UID[2]
* Byte 4 - Notification UID[3]
* Byte 5 - Attribute ID 1 - Attribute ID Title
* Byte 6 - Title Maximum Length LSB
* Byte 7 - Title Maximum Length MSB
*
*******************************************************************************/
static void Ancs_CmdGetNotificationAttributeTitle(uint8 * notificationUid)
{
    CYBLE_GATTC_WRITE_REQ_T writeCommand;
    
    uint8 writeData[8];

    /* Create an array for holding the data as per Apple definitions */
    writeData[0] = ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES;
    writeData[1] = *(notificationUid + 0);
    writeData[2] = *(notificationUid + 1);
    writeData[3] = *(notificationUid + 2);
    writeData[4] = *(notificationUid + 3);
    writeData[5] = ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE;
    writeData[6] = ANCS_NAME_MAX_LENGTH_LSB;
    writeData[7] = ANCS_NAME_MAX_LENGTH_MSB;
    
    writeCommand.value.val = writeData;
    writeCommand.value.len = sizeof(writeData);
    writeCommand.attrHandle = ancsControlPointCharHandle;
    
    /* Write to the Control Point characteristic */
    CyBle_GattcWriteCharacteristicValue(cyBle_connHandle, &writeCommand);
}


/*******************************************************************************
* Function Name: Ancs_CmdPerformNotificationAction()
********************************************************************************
* Summary:
* The function performs a notification Action.
*
* Parameters:
* uint8 * notificationUid: The GATT notification on Notification source 
*                          characteristic
* ANCS_ACTION_ID_VALUES actionId: The action to be performed.
*
* Return:
* None
*
* Theory:
* The function issues a "Perform Notification Action" command to the iOS by 
* sending a write request on the Control Point characteristic with this data:
* Byte 0 - Command ID - 2
* Byte 1 - Notification UID[0]
* Byte 2 - Notification UID[1]
* Byte 3 - Notification UID[2]
* Byte 4 - Notification UID[3]
* Byte 5 - Action ID - 0(Positive) or 1(Negative)
*
*******************************************************************************/
static void Ancs_CmdPerformNotificationAction(uint8 * notificationUid, ANCS_ACTION_ID_VALUES actionId)
{
    CYBLE_GATTC_WRITE_REQ_T writeCommand;
    
    uint8 writeData[6];
    
    /* Create an array for holding the data as per Apple definitions */
    writeData[0] = ANCS_COMMAND_ID_PERFORM_NOTIFICATION_ACTION;
    writeData[1] = *(notificationUid + 0);
    writeData[2] = *(notificationUid + 1);
    writeData[3] = *(notificationUid + 2);
    writeData[4] = *(notificationUid + 3);
    writeData[5] = actionId;
    
    writeCommand.value.val = writeData;
    writeCommand.value.len = sizeof(writeData);
    writeCommand.attrHandle = ancsControlPointCharHandle;
    
    /* Write to the Control Point characteristic */
    CyBle_GattcWriteCharacteristicValue(cyBle_connHandle, &writeCommand);
}


/*******************************************************************************
* Function Name: Ancs_UpdateOutputInformation()
********************************************************************************
* Summary:
* This function prints data onto the UART terminal.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function prints the number of Missed calls, Voicemails and Emails on the 
* screen. The same line on the terminal is updated with new data if no other
* notification came up.
*
*******************************************************************************/
static void Ancs_UpdateOutputInformation(void)
{
    /* Update the same line if no new notification since. */
    if(printStatus == PRINT_SAME_LINE)
    {
        /* Go back to the same line */
        UART_UartPutChar(13);
    }
    else
    {
        UART_UartPutString("\n\r");
    }
    
    printStatus = PRINT_SAME_LINE;
    
    UART_UartPutString("Missed calls: ");
    UART_UartPutChar(HexToDecimal(missedCallCount, 1));
    UART_UartPutChar(HexToDecimal(missedCallCount, 0));
    
    UART_UartPutString("   Voicemails: ");
    UART_UartPutChar(HexToDecimal(voiceMailCount, 1));
    UART_UartPutChar(HexToDecimal(voiceMailCount, 0));
    
    UART_UartPutString("   Emails: ");
    UART_UartPutChar(HexToDecimal(emailCount, 1));
    UART_UartPutChar(HexToDecimal(emailCount, 0));
}


/*******************************************************************************
* Function Name: Ancs_Reset()
********************************************************************************
* Summary:
* Resets all the information related to ANCS service.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function clears all the variables holding the ANCS data. Note that the 
* bonding information is not removed by this function.
*
*******************************************************************************/
void Ancs_Reset(void)
{
    /* Reset all the variables for ANCS */
    ancsNotifSourceCharHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    ancsNotifSourceCccdHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    ancsControlPointCharHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    ancsDataSourceCharHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    ancsDataSourceCccdHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    ancsServiceRange.startHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    ancsServiceRange.endHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    descriptorHandleRange.startHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    descriptorHandleRange.endHandle = CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    ancsDiscoveryStatus = ANCS_DISC_NONE_DISCOVERED;
    ancsUsageState = ANCS_USAGE_IDLE;
    printStatus = PRINT_NEW_LINE;

    serviceChangedCccdWriteStatus = SERVICE_CHANGED_CCCD_WRITE_REQ_NOT_SENT;
    
    missedCallCount = 0;
    voiceMailCount = 0;
    emailCount = 0;
}


/*******************************************************************************
* Function Name: Ancs_EventHandler()
********************************************************************************
* Summary:
* Event handler for the Apple Notification Center Service.
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
* This function is responsible for the discovery of the ANCS service, its 
* characteristics, and the characteristic descriptors. 
* Three characteristics are identified - 
*   1. Notification source
*   2. Control Point
*   3. Data source
* All other characteristics (if present) are ignored. 
* The CCCD of Notification Source and Data Source characteristics are identified.
* All other descriptors (if present) are ignored.
*
* Once the discovery procedure is complete, this function writes to the CCCDs 
* of the Notification Source and Data Source characteristics to enable GATT
* notifications on those characteristics.
* It also subscribes to the Service Changed indication on the GATT service for
* any future service change.
*
*******************************************************************************/
void Ancs_EventHandler(uint32 eventCode, void * eventParam)
{
    uint16 counter;
    uint16 dataLength;
    uint16 attributeLength;
    
    SERVICE_DATA_STRUCT * serviceData;
    CHAR_DATA_STRUCT characteristicData;
    DESC_DATA_STRUCT * descriptorData;
    
    CYBLE_GATTC_READ_BY_GRP_RSP_PARAM_T * readByGroupResponse;
    CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T * readByTypeResponse;
    CYBLE_GATTC_FIND_INFO_RSP_PARAM_T * findInfoResponse;
    CYBLE_GATTC_ERR_RSP_PARAM_T * errorResponse;
    
    switch(eventCode)
    {
        /* Case to discover services */
        case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:
            /* Typecast the event paramter for Read by Group Response data */ 
            readByGroupResponse = (CYBLE_GATTC_READ_BY_GRP_RSP_PARAM_T *)eventParam;
            dataLength = readByGroupResponse->attrData.length;
            attributeLength = readByGroupResponse->attrData.attrLen;
            
            /* Search for ANCS service within Services with 128-bit UUID only */
            if((dataLength == CYBLE_DISC_SRVC_INFO_128_LEN) && (ancsDiscoveryStatus == ANCS_DISC_NONE_DISCOVERED))
            {
                for(counter = 0; counter < attributeLength; counter += dataLength)
                {
                    /* Read by Group response data consists of rows of 
                     * {Service Handle range, Service UUID}.
                     * So typecast the relevant data into that structure.
                     */
                    serviceData = (SERVICE_DATA_STRUCT *)(readByGroupResponse->attrData.attrValue + counter);
                    
                    /* Check whether the Service UUID matches that of ANCS */
                    if(memcmp(serviceData->uuid.value, ancsServiceUuid, CYBLE_GATT_128_BIT_UUID_SIZE) == 0)
                    {
                        /* Match! ANCS Service found. */
                        ancsServiceRange.startHandle = serviceData->range.startHandle;
                        ancsServiceRange.endHandle = serviceData->range.endHandle;
                        ancsDiscoveryStatus = ANCS_DISC_SERVICE_DISCOVERED;
                        break;
                    }
                }
            }
            break;

            
        /* Case to discover characteristics */
        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
            if(ANCS_DISC_SERVICE_DISCOVERED == ancsDiscoveryStatus)
            {
                /* Typecast the event paramter for Read by Type Response data */ 
                readByTypeResponse = (CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T *)eventParam;
                dataLength = readByTypeResponse->attrData.length;
                attributeLength = readByTypeResponse->attrData.attrLen;
                
                /* Search for ANCS characteristics (128-bit UUID) in the data */
                if(dataLength == CYBLE_DISC_CHAR_INFO_128_LEN)
                {
                    for(counter = 0; counter < attributeLength; counter += dataLength)
                    {
                        /* Cache the characteristic UUID in a buffer */
                        characteristicData.valueHandle = CyBle_Get16ByPtr(readByTypeResponse->attrData.attrValue + counter + 3);
                        memcpy(characteristicData.uuid.value, (uint8 *)(readByTypeResponse->attrData.attrValue + counter + 5), CYBLE_GATT_128_BIT_UUID_SIZE);
                        
                        /* Compare the UUID with known UUIDs for ANCS */
                        if(memcmp(characteristicData.uuid.value, ancsNotifSourceCharUuid, CYBLE_GATT_128_BIT_UUID_SIZE) == 0)
                        {
                            /* Match! ANCS Notification Source characteristic */
                            ancsNotifSourceCharHandle = characteristicData.valueHandle;
                        }
                        else if(memcmp(characteristicData.uuid.value, ancsControlPointCharUuid, CYBLE_GATT_128_BIT_UUID_SIZE) == 0)
                        {
                            /* Match! ANCS Control Point characteristic */
                            ancsControlPointCharHandle = characteristicData.valueHandle;
                        }
                        else if(memcmp(characteristicData.uuid.value, ancsDataSourceCharUuid, CYBLE_GATT_128_BIT_UUID_SIZE) == 0)
                        {
                            /* Match! ANCS Data Source characteristic */
                            ancsDataSourceCharHandle = characteristicData.valueHandle;
                        }
                        else
                        {
                            /* Unknown characteristic */
                        }
                    }
                }
            }
            break;

            
        /* Case to discover characteristic descriptors */
        case CYBLE_EVT_GATTC_FIND_INFO_RSP:
            if(ANCS_DISC_CHAR_DISCOVERED == ancsDiscoveryStatus)
            {
                /* Typecast the event paramter for Find Info Response data */ 
                findInfoResponse = (CYBLE_GATTC_FIND_INFO_RSP_PARAM_T *)eventParam;
                dataLength = CYBLE_DB_ATTR_HANDLE_LEN + CYBLE_GATT_16_BIT_UUID_SIZE;
                attributeLength = findInfoResponse->handleValueList.byteCount;
                
                /* Search within the descriptors with 16-bit UUID */
                if(findInfoResponse->uuidFormat == CYBLE_GATT_16_BIT_UUID_FORMAT)
                {
                    for(counter = 0; counter < attributeLength; counter += dataLength)
                    {
                        /* Find Info Response data consists of rows of 
                         * {Handle, UUID} of descriptors. So typecast the 
                         * relevant data into that structure.
                         */
                        descriptorData = (DESC_DATA_STRUCT *)(findInfoResponse->handleValueList.list + counter);
                        
                        /* Search for CCCD descriptors in the data */
                        if(descriptorData->uuid == CCCD_UUID_16BIT)
                        {
                            /* Found the CCCD descriptor; now check which 
                             * characteristic it belongs to.
                             */
                            if((descriptorData->descHandle < ancsNotifSourceCharHandle) && 
                               (descriptorData->descHandle < ancsDataSourceCharHandle))
                            {
                                /* Unknown CCCD; ignore */
                            }
                            else if((descriptorData->descHandle > ancsNotifSourceCharHandle) && 
                                    (descriptorData->descHandle < ancsDataSourceCharHandle))
                            {
                                /* This belongs to the Notification source 
                                 * characteristic.
                                 */
                                ancsNotifSourceCccdHandle = descriptorData->descHandle;
                            }
                            else if((descriptorData->descHandle < ancsNotifSourceCharHandle) && 
                                    (descriptorData->descHandle > ancsDataSourceCharHandle))
                            {
                                /* This belongs to the Data source 
                                 * characteristic.
                                 */
                                ancsDataSourceCccdHandle = descriptorData->descHandle;
                            }
                            else if((descriptorData->descHandle > ancsNotifSourceCharHandle) && 
                                    (descriptorData->descHandle > ancsDataSourceCharHandle))
                            {
                                /* Check if the CCCD is within range of the 
                                 * ANCS service. 
                                 */
                                if(descriptorData->descHandle <= ancsServiceRange.endHandle)
                                {
                                    /* Need to know whether Notification source 
                                     * char handle is more or Data source char. 
                                     */
                                    if(ancsNotifSourceCharHandle < ancsDataSourceCharHandle)
                                    {
                                        /* Data source CCCD */
                                        ancsDataSourceCccdHandle = descriptorData->descHandle;
                                    }
                                    else if(ancsNotifSourceCharHandle > ancsDataSourceCharHandle)
                                    {
                                        /* Notification source CCCD */
                                        ancsNotifSourceCccdHandle = descriptorData->descHandle;
                                    }
                                }
                                else
                                {
                                    /* Unknown CCCD; ignore */
                                }
                            }
                            else
                            {
                                /* Invalid data; ignore */
                            }
                        }
                        
                        if(descriptorData->descHandle == descriptorHandleRange.endHandle)
                        {
                            /* Descriptor discovery is complete */
                            ancsDiscoveryStatus = ANCS_DISC_DESC_DISCOVERED;
                            
                            /* Service discovery procedure complete; subscribe to the GATT
                             * Service changed indication by writing 0x02 to its CCCD.
                             */
                            if((serviceChangedCccdWriteStatus == SERVICE_CHANGED_CCCD_WRITE_REQ_NOT_SENT) && 
                              (cyBle_gattc.serviceChanged.valueHandle != CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE))
                            {
                                serviceChangedCccdPacket.value = cccdIndFlagSetStruct;
                                serviceChangedCccdPacket.attrHandle = cyBle_gattc.cccdHandle;
                                CyBle_GattcWriteCharacteristicDescriptors(cyBle_connHandle, &serviceChangedCccdPacket);
                            }
                            serviceChangedCccdWriteStatus = SERVICE_CHANGED_CCCD_WRITE_REQ_SENT;
                            
                            break;
                        }
                    }
                }
            }
            break;

            
        case CYBLE_EVT_GATTC_WRITE_RSP:
            /* Service changed CCCD set to 0x02 to enable indications */
            if(serviceChangedCccdWriteStatus == SERVICE_CHANGED_CCCD_WRITE_REQ_SENT)
            {
                serviceChangedCccdWriteStatus = SERVICE_CHANGED_CCCD_WRITE_RESP_RECEIVED;
                
                /* Enable Notification source characteristic notifications */
                if(ancsDiscoveryStatus == ANCS_DISC_DESC_DISCOVERED)
                {
                    ancsNotificationSourceCccdPacket.value = cccdNotifFlagSetStruct;
                    ancsNotificationSourceCccdPacket.attrHandle = ancsNotifSourceCccdHandle;
                    CyBle_GattcWriteCharacteristicDescriptors(cyBle_connHandle, &ancsNotificationSourceCccdPacket);
                    ancsDiscoveryStatus = ANCS_DISC_NOTIF_SOURCE_CCCD_SET;
                }
            }
            /* ANCS Notification source characteristic CCCD set to 0x01
             * to enable notifications 
             */
            else if((ancsDiscoveryStatus == ANCS_DISC_NOTIF_SOURCE_CCCD_SET) &&
                    (ancsDataSourceCccdHandle != 0))
            {
                ancsDataSourceCccdPacket.value = cccdNotifFlagSetStruct;
                ancsDataSourceCccdPacket.attrHandle = ancsDataSourceCccdHandle;
                CyBle_GattcWriteCharacteristicDescriptors(cyBle_connHandle, &ancsDataSourceCccdPacket);
                ancsDiscoveryStatus = ANCS_DISC_DATA_SOURCE_CCCD_SET;
            }
            /* ANCS Data source characteristic CCCD set to 0x01 to enable
             * notifications.
             */
            else if(ancsDiscoveryStatus == ANCS_DISC_DATA_SOURCE_CCCD_SET)
            {
                ancsDiscoveryStatus = ANCS_DISC_WRITE_COMPLETE;
            }
            break;

            
        case CYBLE_EVT_GATTC_ERROR_RSP:
            errorResponse = (CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam;
            
            /* Check if the ANCS characteristic discovery is complete.
             * Initiate descriptor discovery if it is done.
             */
            if(ancsDiscoveryStatus == ANCS_DISC_SERVICE_DISCOVERED)
            {
                if((errorResponse->opCode == CYBLE_GATT_READ_BY_TYPE_REQ) && 
                   (errorResponse->errorCode == CYBLE_GATT_ERR_ATTRIBUTE_NOT_FOUND) &&
                   (errorResponse->attrHandle == ancsServiceRange.endHandle) &&
                   (cyBle_clientState != CYBLE_CLIENT_STATE_CHAR_DISCOVERING))
                {
                    ancsDiscoveryStatus = ANCS_DISC_CHAR_DISCOVERED;
                    descriptorHandleRange.startHandle = (ancsNotifSourceCharHandle < ancsDataSourceCharHandle) ? 
                                                        (ancsNotifSourceCharHandle + 1) : 
                                                        (ancsDataSourceCharHandle + 1);
                    descriptorHandleRange.endHandle = ancsServiceRange.endHandle;
                    CyBle_GattcDiscoverAllCharacteristicDescriptors(cyBle_connHandle, &descriptorHandleRange);
                }
            }
            break;
            
            
        default:
            break;
    }
}


/*******************************************************************************
* Function Name: Ancs_HandleNotifications()
********************************************************************************
* Summary:
* The function handles the GATT notifications on the Notification Source
* characteristic.
*
* Parameters:
* uint8 * value: Pointer to the data as part of the notification
*
* Return:
* None
*
* Theory:
* The function responds to the incoming GATT notifications on the Notification
* Source characteristic by displaying it to the user on the UART output.
* For some of the notification types, it asks iOS to provide more information.
* For example, in case of an incoming call, it asks iOS to provide the caller's 
* name (for which the response comes as a notification on the Data Source).
*
*******************************************************************************/
void Ancs_HandleNotifications(uint8 * value)
{
    ancsNotification.eventId = value[0];
    ancsNotification.eventFlags = value[1];
    ancsNotification.categoryId = value[2];
    ancsNotification.categoryCount = value[3];
    ancsNotification.notificationUid[0] = value[4];
    ancsNotification.notificationUid[1] = value[5];
    ancsNotification.notificationUid[2] = value[6];
    ancsNotification.notificationUid[3] = value[7];

    switch(ancsNotification.categoryId)
    {
        case ANCS_CATEGORY_ID_INCOMING_CALL:
            if(ancsNotification.eventId == ANCS_EVENT_ID_NOTIFICATION_ADDED)
            {
                printStatus = PRINT_NEW_LINE;
                UART_UartPutString("\n\n\rIncoming call ");
                
                /* Ask for the caller information */
                if((ancsControlPointCharHandle != CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE) &&
                   (ancsDataSourceCharHandle != CYBLE_GATT_INVALID_ATTR_HANDLE_VALUE))
                {
                    Ancs_CmdGetNotificationAttributeTitle(ancsNotification.notificationUid);
                }
            }
            break;
            
        case ANCS_CATEGORY_ID_MISSED_CALL:
            missedCallCount = ancsNotification.categoryCount;
            Ancs_UpdateOutputInformation();
            break;
        
        case ANCS_CATEGORY_ID_VOICEMAIL:
            voiceMailCount = ancsNotification.categoryCount;
            Ancs_UpdateOutputInformation();
            break;
        
        case ANCS_CATEGORY_ID_EMAIL:
            emailCount = ancsNotification.categoryCount;
            Ancs_UpdateOutputInformation();
            break;
        
        /* Other cases of ANCS notifications are not shown in this example. */
        
        default:
            break;
    }
}


/*******************************************************************************
* Function Name: Ancs_HandleData()
********************************************************************************
* Summary:
* The function handles the GATT notifications on the Data Source
* characteristic.
*
* Parameters:
* uint8 * value: Pointer to the data as part of the notification
*
* Return:
* None
*
* Theory:
* The function takes the GATT notification coming on the Data Source and 
* displays it to the user.
*
*******************************************************************************/
void Ancs_HandleData(uint8 * value)
{
    uint8 * uid = value + 1;
    uint16 counter;
    
    switch(value[0])
    {
        case ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES:
            if(memcmp(uid, ancsNotification.notificationUid, 4) == 0)
            {
                if(value[5] == ANCS_NOTIFICATION_ATTRIBUTE_ID_TITLE)
                {
                    uint16 length = value[6] + (value[7] << 8);
                    
                    UART_UartPutString("from ");
                    for(counter = 0; counter < length; counter++)
                    {
                        UART_UartPutChar(value[8 + counter]);
                    }
                }
            }
            
            /* If positive and negative actions can be taken up */
            if((ancsNotification.eventFlags & ANCS_EVENT_FLAG_POSITIVE_ACTION) &&
               (ancsNotification.eventFlags & ANCS_EVENT_FLAG_NEGATIVE_ACTION))
            {
                UART_UartPutString(". Accept (Y) or Decline (N)? ");
                ancsUsageState = ANCS_USAGE_INCOMING_CALL_WAITING_FOR_INPUT;
            }
            break;
            
            
        /* The case GET_APP_ATTRIBUTES is not shown in this example.
         * The case PERFORM_NOTIFICATION_ACTION is not a valid case.
         */
            
        default:
            break;
    }
}


/*******************************************************************************
* Function Name: Ancs_StateMachine()
********************************************************************************
* Summary:
* The function handles the ANCS state machine.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function decides the steps to do for each state of ANCS notifications.
*
*******************************************************************************/
void Ancs_StateMachine(void)
{
    uint8 command;
    
    switch(ancsUsageState)
    {
        case ANCS_USAGE_INCOMING_CALL_WAITING_FOR_INPUT:
            command = UART_UartGetChar();

            /* Check whether the user wants to accept the incoming 
             * call or decline it.
             */
            if(command != 0u)
            {
                if((command == 'y') || (command == 'Y'))
                {
                    /* User wants to accept the call. */
                    Ancs_CmdPerformNotificationAction(ancsNotification.notificationUid, ANCS_ACTION_ID_POSITIVE);
                    UART_UartPutString("Accepted. ");
                }
                else if((command == 'n') || (command == 'N'))
                {
                    /* User wants to decline the call. */
                    Ancs_CmdPerformNotificationAction(ancsNotification.notificationUid, ANCS_ACTION_ID_NEGATIVE);
                    UART_UartPutString("Declined. ");
                }
                
                ancsUsageState = ANCS_USAGE_IDLE;
            }
            break;
        
            
        default:
            break;
    }
}


/* [] END OF FILE */
