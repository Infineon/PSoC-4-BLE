/*******************************************************************************
* File Name: client.h
*
* Version: 1.0
*
* Description:
*  Common BLE application header for client devices.
*
*******************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>

#define CYBLE_MAX_ADV_DEVICES        (10u)
#define APP_MAX_SUPPOTED_SERVICES    (10u)


extern CYBLE_GAP_BD_ADDR_T peerAddr[CYBLE_MAX_ADV_DEVICES];

void StartScan(CYBLE_UUID16 uuid);
void StartDiscovery(void);
void ScanProgressEventHandler(CYBLE_GAPC_ADV_REPORT_T* eventParam);
void ClientDebugOut(uint32 event, void* eventParam);


/* [] END OF FILE */
