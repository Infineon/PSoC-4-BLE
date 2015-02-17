/******************************************************************************
* Project Name		: PSoC_4_BLE_Central_IAS
* File Name			: BLEclient.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: ROIT
*
********************************************************************************
* Copyright (2014-15), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
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
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/
#include <main.h>

/* 'list_of_devices' is an array of type 'CYBLE_GAPC_ADV_REPORT_T' (defined in 
 * BLE_StackGap.h) that is used to store the reports of the peripheral devices
* being scanned by this Central device*/
CYBLE_GAPC_ADV_REPORT_T 	list_of_devices[CYBLE_MAX_ADV_DEVICES];

/* 'connectPeriphDevice' is a variable of type 'CYBLE_GAP_BD_ADDR_T' (defined in 
* BLE_StackGap.h) and is used to store address of the connected device. */
CYBLE_GAP_BD_ADDR_T 		connectPeriphDevice;

/* 'connHandle' is a variable of type 'CYBLE_CONN_HANDLE_T' (defined in 
* BLE_StackGatt.h) and is used to store the connection handle parameters after
* connecting with the peripheral device. */
CYBLE_CONN_HANDLE_T			connHandle;

/* 'deviceConnected' flag tells the status of connection with BLE peripheral Device */
uint8 deviceConnected = FALSE;

/* 'ble_state' stores the state of connection which is used for updating LEDs */
uint8 ble_state = BLE_DISCONNECTED;

/* 'address_store' stores the addresses returned by Scanning results */
uint8 address_store[10][6];

/* 'peripheralAddress' stores the addresses of device presently connected to */
uint8 peripheralAddress[6];

/* 'peripheralFound' flag tells whether the peripheral device with the required Address
 * has been found during scanning or not. */
uint8 peripheralFound = FALSE;

/* 'addedDevices' tells the number of devices that has been added to the list till now. */
uint8 addedDevices = FALSE;

/* 'iasLevel' stores the current alert level as set by Central device */
extern uint8 iasLevel;

/* 'restartScanning' flag indicates to application whether starting scan API has to be
* called or not */
uint8 restartScanning = FALSE;

/*******************************************************************************
* Function Name: ApplicationEventHandler
********************************************************************************
* Summary:
*        Call back event function to handle various events from BLE stack
*
* Parameters:
*  event:		event returned
*  eventParam:	link to value of the event parameter returned
*
* Return:
*  void
*
*******************************************************************************/
void ApplicationEventHandler(uint32 event, void *eventparam)
{
	/* 'apiResult' is a variable of type 'CYBLE_API_RESULT_T' (defined in 
	* BLE_StackTypes.h) and is used to store the return value from BLE APIs. */
	CYBLE_API_RESULT_T 			apiResult;
	
	/* 'scan_report' is a variable of type 'CYBLE_GAPC_ADV_REPORT_T' (defined in 
	* BLE_StackGap.h) and is used to store report retuned from Scan results. */
	CYBLE_GAPC_ADV_REPORT_T		scan_report;
	
	/* Local variable for Loop */
	uint16 i = FALSE;
	
	switch(event)
	{
		case CYBLE_EVT_STACK_ON:
			
			/* Set start scanning flag to allow calling the API in main loop */
			restartScanning = TRUE;
		break;
			
		case CYBLE_EVT_GAPC_SCAN_START_STOP:
			/* Add relevant code here pertaining to Starting/Stopping of Scan*/
			if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
			{
				ble_state = BLE_DISCONNECTED;
				
				if(!peripheralFound)
				{
					/* Restart Scanning */
					Status_LED_Write(1);
					restartScanning = TRUE;
				}
			}
			else
			{
				ble_state = BLE_SCANNING;
			}
		break;
			
		case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
			/* This event is generated whenever there is a device found*/
			if(CYBLE_STATE_CONNECTED != CyBle_GetState())	
			{
				/* If we are not connected to any peripheral device, then save the new device  
					information so to add it to our list */
				scan_report = *(CYBLE_GAPC_ADV_REPORT_T*) eventparam;
				
				/* Add the new device to existing list if not done yet, and compare the address with our
					required address.*/
				HandleScanDevices(&scan_report);
			}
			/* Update the LED status for BLE scanning mode*/
			ble_state = BLE_SCANNING;
		break;
			
 		case CYBLE_EVT_GATT_CONNECT_IND:

			/* When the peripheral device is connected, store the connection handle.*/
            connHandle = *(CYBLE_CONN_HANDLE_T *)eventparam;
            break;
			
		case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* When the peripheral device is disconnected, reset variables*/
			
			break;
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED:
			
			/* The Device is connected now. Start Attributes discovery process.*/
			apiResult = CyBle_GattcStartDiscovery(connHandle);
			
			if(apiResult != CYBLE_ERROR_OK)
			{

			}
			
			/* Update the LED status for BLE discovery mode*/
			ble_state = BLE_SERVICE_DISCOVERY;
        break;
			
		case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			/* Reset all saved peripheral Addresses */
			for(i=0;i<addedDevices;i++)
			{
				list_of_devices[i].peerBdAddr[0] = FALSE;
				list_of_devices[i].peerBdAddr[1] = FALSE;
				list_of_devices[i].peerBdAddr[2] = FALSE;
				list_of_devices[i].peerBdAddr[3] = FALSE;
				list_of_devices[i].peerBdAddr[4] = FALSE;
				list_of_devices[i].peerBdAddr[5] = FALSE;
			}
			
			/* Reset application Flags on BLE Disconnect */
			addedDevices = FALSE;
			peripheralFound = FALSE;
			deviceConnected = FALSE;
			iasLevel = FALSE;
			ble_state = BLE_DISCONNECTED;
			
			/* Set the flag for rescanning after wakeup */
			restartScanning = TRUE;
			
			/* Update LED Status for Disconnection */
			HandleLEDs(ble_state);
			if(apiResult != CYBLE_ERROR_OK)
			{
			}
			break;
			
	
		case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
			/* This event is generated whenever the discovery procedure is complete*/
			
			/*Set the Device connected flag*/
			deviceConnected = TRUE;
			
			/* Update the LED status for BLE discovery mode*/
			ble_state = BLE_CONNECTED;
		break;
			
		default:
			
		break;
	}
}
/*******************************************************************************
* Function Name: HandleScanDevices
********************************************************************************
* Summary:
*        This function checks for new devices that have been scanned and ads them
* to its internal list. Also, if one of the scanned peripherals is of CySmart
* USB Dongle, then it sets flag to allow connection with it.
*
* Parameters:
*  scanReport:		parameter of type CYBLE_GAPC_ADV_REPORT_T* returned by BLE
* 					event CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT.
*
* Return:
*  void
*
*******************************************************************************/
void HandleScanDevices(CYBLE_GAPC_ADV_REPORT_T* scanReport)
{
	uint16 i;
	uint8 newDevice = TRUE;
	
	if((addedDevices < CYBLE_MAX_ADV_DEVICES))
	{
		for(i = 0; i < addedDevices; i++)
		{
			/* Initialize the peerBdAddr element of our list.*/
			list_of_devices[i].peerBdAddr = &address_store[i][0];
			
			/* In this for loop, compare the new device address with the existing addresses in the list to 
				determine if the address is new or not. If the address exists in the list, then the device 
				is not new.*/
			if(FALSE == memcmp(list_of_devices[i].peerBdAddr, scanReport->peerBdAddr, ADV_ADDR_LEN))
			{
				newDevice = FALSE;
				break;
			}
		}
		
		if(newDevice)
		{
			/* If the device address is new, then add the device to our existing list and compare the address
				with our expected address to see if the desired peripheral is advertising or not.*/
			
			list_of_devices[addedDevices].peerBdAddr = &address_store[addedDevices][0];
			
			/* Store the data*/
			list_of_devices[addedDevices].dataLen = scanReport->dataLen;
			
			list_of_devices[addedDevices].data = scanReport->data;
			
			list_of_devices[addedDevices].eventType = scanReport->eventType;
			/* Record the address type, Public or Random, of the advertising peripheral.*/
			list_of_devices[addedDevices].peerAddrType = scanReport->peerAddrType;
			
			/* Save the BD addresses */
			list_of_devices[addedDevices].peerBdAddr[0] = scanReport->peerBdAddr[0];
			list_of_devices[addedDevices].peerBdAddr[1] = scanReport->peerBdAddr[1];
			list_of_devices[addedDevices].peerBdAddr[2] = scanReport->peerBdAddr[2];
			list_of_devices[addedDevices].peerBdAddr[3] = scanReport->peerBdAddr[3];
			list_of_devices[addedDevices].peerBdAddr[4] = scanReport->peerBdAddr[4];
			list_of_devices[addedDevices].peerBdAddr[5] = scanReport->peerBdAddr[5];
			
			list_of_devices[addedDevices].rssi = scanReport->rssi;
			
			/* If the new BD address found matches the desired BD address, the Dongle has been found*/
			if(FALSE == memcmp(peripheralAddress, scanReport->peerBdAddr, ADV_ADDR_LEN))
			{
				/* Save the connected device BD Address and Type*/
				connectPeriphDevice.bdAddr[0] = scanReport->peerBdAddr[0];
				connectPeriphDevice.bdAddr[1] = scanReport->peerBdAddr[1];
				connectPeriphDevice.bdAddr[2] = scanReport->peerBdAddr[2];
				connectPeriphDevice.bdAddr[3] = scanReport->peerBdAddr[3];
				connectPeriphDevice.bdAddr[4] = scanReport->peerBdAddr[4];
				connectPeriphDevice.bdAddr[5] = scanReport->peerBdAddr[5];
				
				connectPeriphDevice.type = list_of_devices[addedDevices].peerAddrType;
					
				/* Set the flag to notify application of a connected peripheral device */
				peripheralFound = TRUE;	
				
				/* Stop existing BLE Scan */
				CyBle_GapcStopScan();
			}
			
			addedDevices++;
		}
		
		newDevice = TRUE;
	}
}


/*******************************************************************************
* Function Name: LoadPeripheralDeviceData
********************************************************************************
* Summary:
*        This function stores the address of the peripheral device to which this
* Central device wishes to connect. 
*
* Parameters:
*  void
*
* Return:
*  void
*

*******************************************************************************/
void LoadPeripheralDeviceData(void)
{
	/* This stores the 6-byte BD address of peripheral device to which we have to connect.
	   In this project, we have peripheral (CySmart USB Dongle) with BD Address 0x00A050654325. 
	   We write this address to the 6-byte array 'peripheralAddress'. This array is then  
	   used to compare the addresses of the advertising peripherals. If one of the advertising 
	   peripherals has the same address, then we know it is the device which we want to connect  
	   and initiate a connection request to it.*/
		peripheralAddress[5] = 0x00;
		peripheralAddress[4] = 0xA0;
		peripheralAddress[3] = 0x50;
		peripheralAddress[2] = 0x65;
		peripheralAddress[1] = 0x43;
		peripheralAddress[0] = 0x25;
}
/* [] END OF FILE */
