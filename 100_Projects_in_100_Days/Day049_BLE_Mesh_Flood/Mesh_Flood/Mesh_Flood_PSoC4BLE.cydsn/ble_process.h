/*******************************************************************************
* File Name: ble_process.h
*
* Version: 1.0
*
* Description:
*  This file contains the headers and constants for BLE functions, such as
* event call back function, changing GAP roles and error codes.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(BLE_PROCESS_H)
#define BLE_PROCESS_H
#include "project.h"

/*****************************************************
*                  Enums and macros
*****************************************************/ 
#define RGB_LED_DATA_LEN					4
#define RGB_RED_INDEX						0
#define RGB_GREEN_INDEX						1
#define RGB_BLUE_INDEX						2
#define RGB_INTENSITY_INDEX					3
#define RGB_LED_MAX_VAL						255

#define CUSTOM_ADV_DATA_MARKER				0x01
#define SCAN_TAG_DATA_LEN					20

/*****************************************************
*                  GATT Error codes
*****************************************************/
#define ERR_INVALID_HANDLE				0x01
#define ERR_INVALID_PDU					0x04
#define ERR_REQ_NOT_SUPP				0x06
#define ERR_ATTR_NOT_FOUND				0x0A
#define ERR_UNLIKELY_ERR				0x0E
#define ERR_INVALID_ATT_LEN				0x0D
#define ERR_INSUFF_RESOURCE				0x11
#define ERR_APPL_ERR_1					0x80
	
/*****************************************************
*                  Function Declarations
*****************************************************/
void GenericEventHandler(uint32 event, void * eventParam);
void SwitchRole(void);
void ConnectToPeripheralDevice(void);
void RestartCentralScanning(void);
void SendErrorCode(uint8 opCode, CYBLE_GATT_DB_ATTR_HANDLE_T  attrHandle, CYBLE_GATT_ERR_CODE_T errorCode);

#endif
/* [] END OF FILE */
