/*******************************************************************************
* File Name: common.h
*
* Version 1.30
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "cytypes.h"
#include <project.h>

#if !defined(Common_H)
#define Common_H
    
extern CYBLE_CONN_HANDLE_T connHandle;
    
#define BLE_PACKET_SIZE_MAX                 (300u)

void PrintProjectHeader(void);
void ChangeBootloaderServiceState(uint32 enabledState);

/* Buffer for received data */
extern uint8 packetRX[BLE_PACKET_SIZE_MAX];
extern uint32 packetRXSize;
extern uint32 packetRXFlag;

extern uint8 packetTX[BLE_PACKET_SIZE_MAX];
extern uint32 packetTXSize;

extern uint8 bootloadingMode;
extern uint8 bootloadingModeReq;

extern void generateKey(uint8 * key);
void AppCallBack(uint32 event, void* eventParam);
void HandleLeds(void);
void WriteAttrServChanged(void);
CY_ISR(Timer_Interrupt);
void AppCallBack(uint32 event, void* eventParam);
void WDT_Start(void);
void WDT_Stop(void);
CY_ISR(Wakeup_Interrupt_Bootloader);
void LowPowerImplementation(void);

#endif /* Common_H */


/* [] END OF FILE */
