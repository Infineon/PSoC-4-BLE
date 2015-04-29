/*****************************************************************************
* File Name: BleProcessing.c
*
* Version: 1.0
*
* Description:
* This file handles the BLE callbacks and notifications.
*
* Note:
* 
* Owner:
* UDYG
*
* Related Document:
* PSoC 4 BLE Heart Rate Monitor Solution Demo IROS: 001-92353
*
* Hardware Dependency:
* PSoC 4 BLE HRM Solution Demo Board
*
* Code Tested With:
* 1. PSoC Creator 3.1
* 2. ARM-GCC Compiler
*
******************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
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
*****************************************************************************/

/*****************************************************************************
* Included headers
*****************************************************************************/
#include "CompileTimeOptions.h"
#include <cytypes.h>
#include <stdbool.h>
#include "BleProcessing.h"
#include "BLE.h"
#include "BLE_eventHandler.h"
#include "WatchdogTimer.h"

#if (HRM_EN)
    #include "HeartRateDetection.h"
#endif  /* #if (HRM_EN) */

#if (BATTERY_EN)
    #include "BatteryLevel.h"
#endif  /* #if (BATTERY_EN) */


/*****************************************************************************
* Global variables
*****************************************************************************/
static bool deviceConnected = false;
static bool isConnectionUpdateRequested = true;
static bool hrsNotification = false;
static bool basNotification = false;
static uint32 timestampWhenConnected = 0;

static CYBLE_GAP_CONN_UPDATE_PARAM_T hrmConnectionParam =
{
    792,        /* Minimum connection interval of 990 ms */
    800,        /* Maximum connection interval of 1000 ms */
    0,          /* Slave latency */
    500         /* Supervision timeout of 5 seconds */
};



/*****************************************************************************
* Function Definitions
*****************************************************************************/

/*****************************************************************************
* Function Name: BleProcessing_SendNotifications()
******************************************************************************
* Summary:
* Function to send heart rate and battery notifications
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* If enabled, the corresponding notifications are sent.
* Heart rate notification is sent every second.
* Battery notification is sent if the battery level has changed by more 
* than 10%.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
static void BleProcessing_SendNotifications(void)
{
#if (HRM_EN)
    static uint32 previousTimestamp = 0;
    static uint32 currentTimestamp = 0;
    uint8 heartRatePacket[] = {0x01, heartRateFiltered};
#endif  /* #if (HRM_EN) */

#if (BATTERY_EN)
    static uint8 previousBatteryLevel = 100;
#endif  /* #if (BATTERY_EN) */
    
    if(deviceConnected)
    {
    #if (HRM_EN) 
        if(hrsNotification)
        {
            currentTimestamp = WatchdogTimer_GetTimestamp();
            if(((uint32)1000) <= (currentTimestamp - previousTimestamp))
            {
                previousTimestamp = currentTimestamp;
                CyBle_HrssSendNotification(cyBle_connHandle, CYBLE_HRS_HRM, sizeof(heartRatePacket), heartRatePacket);
            }
        }
    #endif  /* #if (HRM_EN) */

    #if (BATTERY_EN)
        if(basNotification)
        {
            if((batteryLevel >= (previousBatteryLevel + 10)) || (previousBatteryLevel >= (batteryLevel + 10)))
            {
                previousBatteryLevel = batteryLevel;
                CyBle_BassSendNotification(cyBle_connHandle, 0, CYBLE_BAS_BATTERY_LEVEL, sizeof(batteryLevel), &batteryLevel);
            }
        }
    #endif  /* #if (BATTERY_EN) */
    }
}


/*****************************************************************************
* Function Name: BleProcessing_HeartRateCallback()
******************************************************************************
* Summary:
* Callback function to handle heart rate service events.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function enables or disables notifications as per the event.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void BleProcessing_HeartRateCallback(uint32 event, void* eventParam)
{
    switch(event)
    {
        case CYBLE_EVT_HRSS_NOTIFICATION_ENABLED:
            hrsNotification = true;
            break;
                
        case CYBLE_EVT_HRSS_NOTIFICATION_DISABLED:
            hrsNotification = false;
            break;
            
		default:
			break;
    }
}


/*****************************************************************************
* Function Name: BleProcessing_BatteryCallback()
******************************************************************************
* Summary:
* Callback function to handle battery service events.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function enables or disables notifications as per the event.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void BleProcessing_BatteryCallback(uint32 event, void* eventParam)
{
    switch(event)
    {
        case CYBLE_EVT_BASS_NOTIFICATION_ENABLED:
            basNotification = true;
            break;
                
        case CYBLE_EVT_BASS_NOTIFICATION_DISABLED:
            basNotification = false;
            break;
            
		default:
			break;
    }
}


/*****************************************************************************
* Function Name: BleProcessing_GeneralCallBack()
******************************************************************************
* Summary:
* Callback function to handle general events.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Different events related to the BLE stack can be handled here.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void BleProcessing_GeneralCallBack(uint32 event, void* eventParam)
{
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
            /* Start advertisement */
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
        
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {
                /* Advertisement finished. Put the device to stop mode. */
                stopDeviceFlag = true;
            }
            break;
            
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            timestampWhenConnected = WatchdogTimer_GetTimestamp();
            break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            deviceConnected = false;

            /* Put the device to stop mode */
            stopDeviceFlag = true;
            break;
            
        case CYBLE_EVT_GATT_CONNECT_IND:
            deviceConnected = true;
            break;
            
        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
            isConnectionUpdateRequested = false;
            break;
            
        default:
			break;
    }
}


/*****************************************************************************
* Function Name: BleProcessing_Main()
******************************************************************************
* Summary:
* BLE processing main firmware
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Sends required notifications, Calls process events, and sends connection
* parameter update request on connection.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void BleProcessing_Main(void)
{
    BleProcessing_SendNotifications();
    CyBle_ProcessEvents();
    if(deviceConnected && isConnectionUpdateRequested)
    {
        if((WatchdogTimer_GetTimestamp() - timestampWhenConnected) > 5000)
        {
            CyBle_L2capLeConnectionParamUpdateRequest(cyBle_connHandle.bdHandle, &hrmConnectionParam);
        }
    }
}


/* [] END OF FILE */
