/*******************************************************************************
* File Name: BLE_CAR_Client.h
*
* Description:
*  This is the header file to handle the BLE CAR Client States and functions
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#ifndef _BLE_CAR_CLIENT_H_
#define _BLE_CAR_CLIENT_H_
	
#include "project.h"	
#include "stdio.h"
	
#define FALSE						0
#define TRUE						1

#define MANUFACTURER_SPECIFIC_DATA  0xFF
#define COMPANY_LSB					0x31
#define COMPANY_MSB                 0x01
#define MAN_SPEC_DATA_LSB 			0x34
#define MAN_SPEC_DATA_MSB			0x12

#define BLE_CAR_CCCD_HANDLE 		0x000F

uint8 					ble_CAR_Server_found;
uint8 					Motor_Command;
	
typedef enum
{
	BLE_CAR_CLIENT_STATE_NOT_READY,
	BLE_CAR_CLIENT_STATE_CONFIG,
	BLE_CAR_CLIENT_STATE_ACTIVE,
	BLE_CAR_CLIENT_STATE_BUSY
}BLE_CAR_CLIENT_STATES_T;

BLE_CAR_CLIENT_STATES_T ble_CAR_Client_State;
	
void ApplicationEventHandler(uint32 event, void *eventparam);	

void handle_ble_CAR_Client_State(void);
	
#endif


/* [] END OF FILE */
