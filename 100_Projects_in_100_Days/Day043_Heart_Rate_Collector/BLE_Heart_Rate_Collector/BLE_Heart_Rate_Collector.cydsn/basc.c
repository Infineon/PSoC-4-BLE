/*******************************************************************************
* File Name: basc.c
*
* Version 1.0
*
* Description:
*  This file contains BAS callback handler function.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
* 
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"
#include "basc.h"
#include "hrsc.h"

uint8 basNotification = 3u;

/*******************************************************************************
* Function Name: BasCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   Battery Service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*  None.
*
*******************************************************************************/
void BasCallBack(uint32 event, void* eventParam)
{
    uint8 batteryLevel;
    
    printf("BAS event: %lx, ", event);

    switch(event)
    {
        case CYBLE_EVT_BASC_WRITE_DESCR_RESPONSE:
            if(0u == basNotification)
            {
                printf("Battery Level Notification is Disabled  \r\n");
            }
            else
            {
                printf("Battery Level Notification is Enabled  \r\n");
            }
            break;

        case CYBLE_EVT_BASC_NOTIFICATION:
            batteryLevel = *((CYBLE_BAS_CHAR_VALUE_T*)eventParam)->value->val;
            printf("Battery Level Notification: %d  \r\n", batteryLevel);

            if(2u == basNotification)
            {
                basNotification = 1u;
                apiResult = CyBle_BascGetCharacteristicDescriptor(cyBle_connHandle, 0, CYBLE_BAS_BATTERY_LEVEL,
                    CYBLE_BAS_BATTERY_LEVEL_CCCD);
                if(apiResult != CYBLE_ERROR_OK)
                {
                    printf("CyBle_BascGetCharacteristicDescriptor API Error: %xd \r\n", apiResult);
                }
                else
                {
                    printf("BL CCCD Read Request is sent \r\n");
                }
            }
            
            Notification_LED_Write(LED_ON);
            break;

        case CYBLE_EVT_BASC_READ_CHAR_RESPONSE:
            printf("BAS CHAR Read Response: %d  \r\n", *((CYBLE_BAS_CHAR_VALUE_T*)eventParam)->value->val);
            break;

        case CYBLE_EVT_BASC_READ_DESCR_RESPONSE:
            printf("BAS descriptor read rsp: %4.4x  \r\n", CyBle_Get16ByPtr(((CYBLE_BAS_DESCR_VALUE_T*)eventParam)->value->val));
            break;

        default:
            printf("Not supported event\r\n");
            break;
    }
}


/* [] END OF FILE */
