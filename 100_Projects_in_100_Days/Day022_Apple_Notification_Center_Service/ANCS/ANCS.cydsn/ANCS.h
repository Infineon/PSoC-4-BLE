/*******************************************************************************
* File Name: ANCS.h
*
* Version: 1.0
*
* Description:
*  This is the header file for the ANCS implementation source code.
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

#if !defined (_ANCS_H)
#define _ANCS_H
    
/*******************************************************************************
* Included headers
*******************************************************************************/
#include <project.h>
#include <stdbool.h>


/*******************************************************************************
* Structures for discovery procedure
*******************************************************************************/
typedef struct
{
    CYBLE_GATT_ATTR_HANDLE_RANGE_T  range;
    CYBLE_UUID128_T   	            uuid;
} SERVICE_DATA_STRUCT;

typedef struct
{
    CYBLE_GATT_DB_ATTR_HANDLE_T charDeclHandle;
    uint8                       properties;
    CYBLE_GATT_DB_ATTR_HANDLE_T valueHandle;
    CYBLE_UUID128_T             uuid;
} CHAR_DATA_STRUCT;

typedef struct
{
    CYBLE_GATT_DB_ATTR_HANDLE_T     descHandle;
    CYBLE_UUID16   	                uuid;
} DESC_DATA_STRUCT;


/*******************************************************************************
* Enums 
*******************************************************************************/
typedef enum
{
    ANCS_DISC_NONE_DISCOVERED,
    ANCS_DISC_SERVICE_DISCOVERED,
    ANCS_DISC_CHAR_DISCOVERED,
    ANCS_DISC_DESC_DISCOVERED,
    ANCS_DISC_NOTIF_SOURCE_CCCD_SET,
    ANCS_DISC_DATA_SOURCE_CCCD_SET,
    ANCS_DISC_WRITE_COMPLETE
} ANCS_DISCOVERY_STATUS;

typedef enum
{
    ANCS_USAGE_IDLE,
    ANCS_USAGE_INCOMING_CALL_WAITING_FOR_INPUT
} ANCS_USAGE_STATUS;

typedef enum
{
    PRINT_SAME_LINE,
    PRINT_NEW_LINE
} PRINT_STATUS;

typedef enum
{
    SERVICE_CHANGED_CCCD_WRITE_REQ_NOT_SENT,
    SERVICE_CHANGED_CCCD_WRITE_REQ_SENT,
    SERVICE_CHANGED_CCCD_WRITE_RESP_RECEIVED
} SERVICE_CHANGED_CCCD_WRITE_STATUS;

typedef enum
{
    ANCS_NOTIFICATION_SOURCE_NOTIF,
    ANCS_DATA_SOURCE_NOTIF
} ANCS_CHAR_NOTIF;


/*******************************************************************************
* External variables 
*******************************************************************************/
extern SERVICE_CHANGED_CCCD_WRITE_STATUS serviceChangedCccdWriteStatus;
extern CYBLE_GATTC_WRITE_REQ_T serviceChangedCccdPacket;

extern CYBLE_GATT_VALUE_T cccdIndFlagSetStruct;

extern ANCS_DISCOVERY_STATUS ancsDiscoveryStatus;
extern ANCS_USAGE_STATUS ancsUsageState;
extern CYBLE_GATT_ATTR_HANDLE_RANGE_T ancsServiceRange;
extern CYBLE_GATTC_FIND_INFO_REQ_T descriptorHandleRange;

extern CYBLE_GATT_DB_ATTR_HANDLE_T ancsNotifSourceCharHandle;
extern CYBLE_GATT_DB_ATTR_HANDLE_T ancsDataSourceCharHandle;


/*******************************************************************************
* External functions 
*******************************************************************************/
extern void Ancs_Reset(void);
extern void Ancs_EventHandler(uint32 eventCode, void * eventParam);
extern void Ancs_HandleNotifications(uint8 * value);
extern void Ancs_HandleData(uint8 * value);
extern void Ancs_StateMachine(void);

#endif  /* #if !defined (_ANCS_H) */

/* [] END OF FILE */
