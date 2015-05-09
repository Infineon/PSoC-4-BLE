/*******************************************************************************
* File Name: ble_process.c
*
* Version: 1.0
*
* Description:
* This file contains the definiton for BLE related functions, such as event handler,
* GAP role switching, central scanning and connection to peripherals.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <main.h>

uint8 RGBData[4];
uint8 dataADVCounter = 1;
uint8 switch_Role = FALSE;

#ifdef ENABLE_ADV_DATA_COUNTER
CYBLE_GAPP_DISC_DATA_T  new_advData;
uint8 potential_node_found = 0;
uint8 potential_node_bdAddr[6];
uint8 potential_node_bdAddrType = 0;
#endif

uint8 scan_tag[SCAN_TAG_DATA_LEN] = {0x13, 0x21, 0x31, 0x01, 0x9B, 0x5F, 0x80, 0x00,
									0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xBB, 0xCB, 
									0x03, 0x00, 0xBB, 0xCB};

CYBLE_GAP_BD_ADDR_T				peripAddr;
uint8 clientConnectToDevice = FALSE;
uint8 ble_gap_state = BLE_PERIPHERAL;

uint8 deviceConnected = FALSE;
uint8 restartScanning = FALSE;
volatile uint16 centralStartedTime = 0;

/*******************************************************************************
* Function Name: GenericEventHandler
********************************************************************************
* Summary:
*        Event handler function for the BLE stack. All the events by BLE stack
* are received by application through this function. For this, CyBle_ProcessEvents()
* should be called continuously in main loop.
*
* Parameters:
*  event: 		event value
*  eventParame: pointer to the location where relevant event data is stored
*
* Return:
*  void
*
*******************************************************************************/
void GenericEventHandler(uint32 event, void * eventParam)
{
	/* Local variables and data structures*/
	CYBLE_GATTS_WRITE_REQ_PARAM_T 		writeReqData;
	CYBLE_GATTC_WRITE_REQ_T				writeADVcounterdata;
	CYBLE_GAPC_ADV_REPORT_T				scan_report;
	CYBLE_GATTS_WRITE_CMD_REQ_PARAM_T	writeCmdData;
	CYBLE_API_RESULT_T					apiResult;
	CYBLE_GATTC_WRITE_REQ_T 			writeRGBdata;
	
	switch(event)
	{
		case CYBLE_EVT_STACK_ON:
			#ifdef DEBUG_ENABLED
			UART_UartPutString("CYBLE_EVT_STACK_ON ");
			UART_UartPutCRLF(' ');
			#endif
			/* At the start of the BLE stack, start advertisement */
			CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
		break;
			
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
			/* This event is received at every start or stop of peripheral advertisement*/
			#ifdef DEBUG_ENABLED
			UART_UartPutString("CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP ");
			SendBLEStatetoUART(CyBle_GetState());
			UART_UartPutCRLF(' ');
			#endif
			
			if((CYBLE_STATE_DISCONNECTED == CyBle_GetState()) && (switch_Role == FALSE))
			{
				/* If the current state of the BLE is Disconnected, then restart advertisement.
				* Note that the advertisement should only be restarted if the switch flag is not
				* TRUE. If switch role flag is TRUE, then there is no need to start advertisement
				* as the GAP role has to be switched*/
				CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);	
				
				if(apiResult == CYBLE_ERROR_OK)
				{
					#ifdef DEBUG_ENABLED
					UART_UartPutString("Restart Advertisement ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
					#endif
				}
			}
		break;
			
		case CYBLE_EVT_GAPC_SCAN_START_STOP:
			/* This event is received at every start or stop of central scanning*/
			#ifdef DEBUG_ENABLED
			UART_UartPutString("CYBLE_EVT_GAPC_SCAN_START_STOP ");
			SendBLEStatetoUART(CyBle_GetState());
			UART_UartPutCRLF(' ');
			#endif
		break;
			
		case CYBLE_EVT_GATT_CONNECT_IND:
			/* This event is received at GATT connection with a device. This event
			* is received for both Client or Server role */
			#ifdef DEBUG_ENABLED
			UART_UartPutString("CYBLE_EVT_GATT_CONNECT_IND ");
			UART_UartPutCRLF(' ');
			#endif
		break;
		
		case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* This event is received at GATT disconnection with a device. This event
			* is received for both Client or Server role */
			#ifdef DEBUG_ENABLED
			UART_UartPutString("CYBLE_EVT_GATT_DISCONNECT_IND ");
			UART_UartPutCRLF(' ');
			#endif
		break;
			
		case CYBLE_EVT_GATTS_WRITE_REQ:
			/* This event is received at when Server receives a Write request from
			* connected Client device */
			/* Save the associated event parameter in local variable */
			writeReqData = *(CYBLE_GATTS_WRITE_REQ_PARAM_T*)eventParam;

			if(writeReqData.handleValPair.attrHandle == CYBLE_RGB_LED_CONTROL_CHAR_HANDLE)
			{
				/* If the Write request is on RGB LED Control Characteristic, then Client is 
				* trying to set a new color to the device. */
				if(writeReqData.handleValPair.value.len == RGB_LED_DATA_LEN)
				{
					#ifdef DEBUG_ENABLED
						UART_UartPutString("RGB CYBLE_EVT_GATTS_WRITE_REQ ");
						UART_UartPutCRLF(' ');
					#endif
					
					/* Extract the four bytes containing the color value and store it */
					RGBData[RGB_RED_INDEX] = writeReqData.handleValPair.value.val[0];
					RGBData[RGB_GREEN_INDEX] = writeReqData.handleValPair.value.val[1];
					RGBData[RGB_BLUE_INDEX] = writeReqData.handleValPair.value.val[2];
					RGBData[RGB_INTENSITY_INDEX] = writeReqData.handleValPair.value.val[3];
					
					/* Modify RGB Color my configuring the PrISM components with new density 
					* value*/
					UpdateRGBled(RGBData, RGB_LED_DATA_LEN);
					
					/* Update the RGB LED Control characteristic in GATT DB  to allow
					* Client to read the latest RGB LED color value set */
					CyBle_GattsWriteAttributeValue(&writeReqData.handleValPair,0,&cyBle_connHandle,CYBLE_GATT_DB_LOCALLY_INITIATED);
							
					#ifdef ENABLE_ADV_DATA_COUNTER
					/* Increment the ADV data counter so that scanning Central device knows
					* if this device has updated RGB LED data or not */
					dataADVCounter++;
					#endif
					
					#ifdef DEBUG_ENABLED
					UART_UartPutString("incremented dataADVCounter value in CYBLE_EVT_GATTS_WRITE_REQ= ");
					PrintNum(dataADVCounter);
					UART_UartPutCRLF(' ');
					#endif
					
					/* After receiveing the color value, set the switch role flag to allow the system
					* to switch role to Central role */
					switch_Role = TRUE;
					
					#ifdef DEBUG_ENABLED
					UART_UartPutString("switchRole to Central");
					UART_UartPutCRLF(' ');
					#endif
				}
				else
				{
					/* Send the error code for invalid attribute length packet */
					SendErrorCode(CYBLE_GATT_WRITE_REQ, 
									writeReqData.handleValPair.attrHandle, 
									ERR_INVALID_ATT_LEN);
					
					return;
				}
			}

			/* As part of every write request, the server needs to send a write response. Note
			* that this will be sent only if all the application layer conditions are met on a 
			* write request. Else, an appropriate error code is sent. */
	        CyBle_GattsWriteRsp(cyBle_connHandle);
		break;
			
		case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
			/* This event is generated whenever a Client device sends a Write Command (Write 
			* without response) to a connected Server. Save the associated event parameter in
			* local variable. */
			writeCmdData = *(CYBLE_GATTS_WRITE_CMD_REQ_PARAM_T*)eventParam;
			
			/* Check if the Write command is for ADV Data counter characteristic */
			if(writeCmdData.handleValPair.attrHandle == CYBLE_RGB_DATA_COUNT_CHAR_HANDLE)
			{
				/* If the data sent is of one byte, then proceed. */
				if(writeCmdData.handleValPair.value.len == 1)
				{
					/* Extract and save the set ADV data counter value */
					dataADVCounter = *(writeCmdData.handleValPair.value.val);	
					
					/* This increment is done to balance the ++ done as part of CYBLE_EVT_GATTS_WRITE_REQ */
					dataADVCounter--;
					
					/* Update the ADV data counter characteristic in GATT DB  to allow
					* Client to read the latest ADV data counter value */
					CyBle_GattsWriteAttributeValue(&writeCmdData.handleValPair,
													0,
													&cyBle_connHandle,
													CYBLE_GATT_DB_LOCALLY_INITIATED);

					#ifdef DEBUG_ENABLED
					UART_UartPutString("dataADVCounter from CYBLE_EVT_GATTS_WRITE_CMD_REQ = ");
					PrintNum(dataADVCounter);
					UART_UartPutCRLF(' ');
					#endif
				}	/* if(writeCmdData.handleValPair.value.len == 1) */
			}
			break;
		
		case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
			/* This event is generated whenever there is a peripheral device found by 
			* while scanning */
			if(CYBLE_STATE_CONNECTED != CyBle_GetState())	
			{
				/* If we are not connected to any peripheral device, then save the new device  
				* information so to add it to our list */
				scan_report = *(CYBLE_GAPC_ADV_REPORT_T*)eventParam;
				
				#ifdef DEBUG_ENABLED
					UART_UartPutString("CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT ");
					UART_UartPutCRLF(' ');
				#endif
				
				#ifdef ENABLE_ADV_DATA_COUNTER
				/* If ADV DATA COUNTER is enabled, then the central device would check
				* if the counter in ADV packet of peripheral is less than its own counter
				* or not. If yes, then it will consider the peripheral as a potential 
				* device to connect to.*/
				if(scan_report.eventType == CYBLE_GAPC_CONN_UNDIRECTED_ADV)
				{
					/* If the scan report received is of advertising nature and the data 
					* length is as expected... */
					if(scan_report.dataLen == new_advData.advDataLen)
					{
						/* If the second last value of the advertising data matches the custom 
						* marker, then the peripheral is a node of the network */
						if(scan_report.data[scan_report.dataLen-2] == CUSTOM_ADV_DATA_MARKER)
						{
							/* If the ADV counter data in Advertising data is less than that of
							* the value in this scanning device, then the node is a potential node 
							* whose color has to be updated. */
							if((scan_report.data[scan_report.dataLen-1] < dataADVCounter) ||
							((scan_report.data[scan_report.dataLen-1] == 255) && (dataADVCounter == 0)))
							{
								/* Potential node found*/
								potential_node_found = TRUE;
								/* Save the advertising peripheral address and type*/
								memcpy(potential_node_bdAddr, scan_report.peerBdAddr, 6);
								potential_node_bdAddrType = scan_report.peerAddrType;
								
								#ifdef DEBUG_ENABLED
								UART_UartPutString("potential_node_found ");
								UART_UartPutCRLF(' ');
								#endif
							}
							else
							{
								/* If the ADV data counter is equal or more than the data counter
								* in this scanning device, then the node has latest RGB LED data
								* and does not need to be connected to. Reset the potential node 
								* address */
								potential_node_found = FALSE;
								
								potential_node_bdAddrType = 0;
								
								potential_node_bdAddr[0] = 0x00;
								potential_node_bdAddr[1] = 0x00;
								potential_node_bdAddr[2] = 0x00;
								potential_node_bdAddr[3] = 0x00;
								potential_node_bdAddr[4] = 0x00;
								potential_node_bdAddr[5] = 0x00;
							}
						}
					}
				}
				#endif
				
				/* If the received scan data is part of scan response from a peripheral... */
				if(scan_report.eventType == CYBLE_GAPC_SCAN_RSP)
				{
					/* If the data lenght of the scan reponse packet is equal to expected
					* scan response data lenght...*/
					if(scan_report.dataLen == SCAN_TAG_DATA_LEN)
					{
						#ifdef ENABLE_ADV_DATA_COUNTER
						/* If a potential node had been found earlier as part of received 
						* advertising data, then compare the address of stored potential 
						* node and received address of the scan response */
						if(potential_node_found)
						{
							/* Compare the two addresses and type */
							if((!memcmp(scan_report.peerBdAddr, potential_node_bdAddr, 6))  
								&& (potential_node_bdAddrType == scan_report.peerAddrType))
							{
						#endif
								/* If the scan report data matches the expected data (scan_tag),
								* then it is our desired node */
								if(!memcmp(scan_report.data, scan_tag, scan_report.dataLen))
								{
									#ifdef DEBUG_ENABLED
									UART_UartPutString("Titan Found ");
									UART_UartPutCRLF(' ');
									#endif
									/* Stop existing scan */
									CyBle_GapcStopScan();
									#ifdef DEBUG_ENABLED
									UART_UartPutString("Stop Scan called ");
									UART_UartPutCRLF(' ');
									#endif
									
									/* Save the peripheral BD address and type*/
									peripAddr.type = scan_report.peerAddrType;
									peripAddr.bdAddr[0] = scan_report.peerBdAddr[0];
									peripAddr.bdAddr[1] = scan_report.peerBdAddr[1];
									peripAddr.bdAddr[2] = scan_report.peerBdAddr[2];
									peripAddr.bdAddr[3] = scan_report.peerBdAddr[3];
									peripAddr.bdAddr[4] = scan_report.peerBdAddr[4];
									peripAddr.bdAddr[5] = scan_report.peerBdAddr[5];

									/* Set the flag to allow application to connect to the
									* peripheral found */
									clientConnectToDevice = TRUE;
									
									#ifdef ENABLE_ADV_DATA_COUNTER
									/* Reset the potential node flag*/
									potential_node_found = FALSE;
									#endif
								}
						#ifdef ENABLE_ADV_DATA_COUNTER
							}
						}
						#endif
					}
				}
			}

		break;
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED:
			/* This event is received whenever the device connect on GAP layer */
			if(ble_gap_state == BLE_CENTRAL)
			{
				#ifdef ENABLE_CENTRAL_DISCOVERY
					/* The Device is connected now. Start Attributes discovery process.*/
					CyBle_GattcStartDiscovery(cyBle_connHandle);
					#ifdef DEBUG_ENABLED
						UART_UartPutString("CYBLE_EVT_GAP_DEVICE_CONNECTED ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
					#endif
				#else
					/* If this system is currently acting in Central role and has connected
					* to a peripheral device, then write directly the ADV counter data and
					* RGB LED control data using attribute handles */
					
					/* Set the device connected flag */
					deviceConnected = TRUE;
					
					#ifdef DEBUG_ENABLED
						UART_UartPutString("Directly write RGB using Attr handle ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
					#endif
					
					/* Write the Data Counter value */
					writeADVcounterdata.attrHandle = CYBLE_RGB_DATA_COUNT_CHAR_HANDLE;
					writeADVcounterdata.value.val = &dataADVCounter;
					writeADVcounterdata.value.len = 1;
					CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &writeADVcounterdata);
				
					/* Write the RGB LED Value */
					writeRGBdata.attrHandle = CYBLE_RGB_LED_CONTROL_CHAR_HANDLE;
					writeRGBdata.value.val = RGBData;
					writeRGBdata.value.len = RGB_LED_DATA_LEN;
					CyBle_GattcWriteCharacteristicValue(cyBle_connHandle, &writeRGBdata);
				#endif
			}	
        break;
			
		case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
			/* This event is generated whenever the discovery procedure is complete*/
			
			#ifdef ENABLE_CENTRAL_DISCOVERY
			deviceConnected = TRUE;
			#ifdef DEBUG_ENABLED
				UART_UartPutString("CYBLE_EVT_GATTC_DISCOVERY_COMPLETE ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');
			#endif
			
			/* Write the Data Counter value */
			writeADVcounterdata.attrHandle = CYBLE_RGB_DATA_COUNT_CHAR_HANDLE;
			writeADVcounterdata.value.val = &dataADVCounter;
			writeADVcounterdata.value.len = 1;
			CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &writeADVcounterdata);
		
			/* Write the RGB LED Value */
			writeRGBdata.attrHandle = CYBLE_RGB_LED_CONTROL_CHAR_HANDLE;
			writeRGBdata.value.val = RGBData;
			writeRGBdata.value.len = RGB_LED_DATA_LEN;
			CyBle_GattcWriteCharacteristicValue(cyBle_connHandle, &writeRGBdata);
			#endif
		break;
			
		case CYBLE_EVT_GATTC_WRITE_RSP:
			/* This event is generated when the Client device receives a response
			* as part of the Write request sent earlier. This indicates that
			* the RGB LED data was written successfully */
			#ifdef DEBUG_ENABLED
				UART_UartPutString("CYBLE_EVT_GATTC_WRITE_RSP ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');
			#endif
			
			/* Disconnect the existing connection and restart scanning */
			if((cyBle_connHandle.bdHandle != 0))
			{
				CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
				
				restartScanning = TRUE;
				#ifdef DEBUG_ENABLED
				UART_UartPutString("Disconnect from CYBLE_EVT_GATTC_WRITE_RSP ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');
				#endif
			}
		break;

		case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			/* This event is generated when the device disconnects from an 
			* existing connection */
			deviceConnected = FALSE;
			
			#ifdef DEBUG_ENABLED
				UART_UartPutString("CYBLE_EVT_GAP_DEVICE_DISCONNECTED ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');
			#endif
			
			if((ble_gap_state == BLE_PERIPHERAL) && (switch_Role != TRUE))
			{
				/* If the current role of this system was Peripheral and the role
				* is not to be switched, then restart advertisement */
				if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
				{
					CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);	
					#ifdef DEBUG_ENABLED
					UART_UartPutString("Restart Advertisement ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
					#endif
				}
			}
			else if((ble_gap_state == BLE_CENTRAL) && (switch_Role != TRUE))
			{
				/* If the current role of this system was Central and the role
				* is not to be switched, then restart scanning */
				if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
				{
					CyBle_GapcStartScan(CYBLE_SCANNING_FAST);	
					#ifdef DEBUG_ENABLED
					UART_UartPutString("Restart Scanning ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
					#endif
				}	
			}
		break;
			
		default:
			eventParam = eventParam;
		break;
	}
}

/*******************************************************************************
* Function Name: SwitchRole
********************************************************************************
* Summary:
*        This function switches the role between Central and Peripheral. If device 
* is connected while switching role, then it is first disconnected.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void SwitchRole(void)
{
	CYBLE_API_RESULT_T  apiResult;
	
	/* if the switch role flag is set... */
	if(switch_Role == TRUE)
	{	
		/* Process pending BLE events */
		CyBle_ProcessEvents();
		
		/* If there is an existing connection, then disconnect before switching
		* role. */
		if((cyBle_connHandle.bdHandle != 0))
		{
			/* Disconnect the device and process the event */
			CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
			CyBle_ProcessEvents();
			#ifdef DEBUG_ENABLED
			UART_UartPutString("Peripheral closed connection ");
			SendBLEStatetoUART(CyBle_GetState());
			UART_UartPutCRLF(' ');
			#endif
		}		
		
		switch(ble_gap_state)
		{
			case BLE_PERIPHERAL:
				/* If the current role is Peripheral and system is advertising,
				* then stop advertisement before switching role */
				if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
				{
					CyBle_GappStopAdvertisement();
					CyBle_ProcessEvents();

					#ifdef DEBUG_ENABLED
						UART_UartPutString("Peripheral Advertisment Stopped ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
					#endif
				}
				
				if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
				{
					/* Switch BLE role by starting scan. This way, the system is set
					* to Central role */
					apiResult = CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
					
					if(CYBLE_ERROR_OK == apiResult)
					{
						#ifdef DEBUG_ENABLED
						UART_UartPutString("Start Scan API called ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif
						
						/* Record the time at which Central role was started. This will be 
						* used for timeout and switching to Peripheral operation*/
						centralStartedTime = WatchDog_CurrentCount();
						
						/* Update the current BLE role to Central */
						ble_gap_state = BLE_CENTRAL;
						
						/* Reset the switch role flag*/
						switch_Role = FALSE;
					}
					else
					{
						/* If scanning did not start, maintain the current role and retry later */
						ble_gap_state = BLE_PERIPHERAL;
						
						#ifdef DEBUG_ENABLED
						UART_UartPutString("Start Scan API failed ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif
					}
				}
				
				/* Process Pending BLE Events */
				CyBle_ProcessEvents();
				
			break;
			
			case BLE_CENTRAL:
				/* If the current role is Central and system is scanning,
				* then stop scanning before switching role */
				if(CyBle_GetState() == CYBLE_STATE_SCANNING)
				{
					CyBle_GapcStopScan();
					CyBle_ProcessEvents();
					
					#ifdef DEBUG_ENABLED
						UART_UartPutString("Central Scan stopped ");
						UART_UartPutCRLF(' ');
					#endif
				}
				
				if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
				{
					#ifdef ENABLE_ADV_DATA_COUNTER
					/* Increment data counter */
					new_advData.advData[new_advData.advDataLen - 1] = dataADVCounter;
					
					cyBle_discoveryModeInfo.advData = &new_advData;
					
					#ifdef DEBUG_ENABLED
						UART_UartPutString("Updated ADV data = ");
						PrintNum(dataADVCounter);
						UART_UartPutCRLF(' ');
					#endif
					#endif
					
					/* Switch BLE role by starting advertisement. This way, the system is 
					* set to Peripheral role */
					apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
					
					if(apiResult == CYBLE_ERROR_OK)
					{
						/* If advertisement started successfully, set the BLE state and
						* reset the switch role flag*/
						ble_gap_state = BLE_PERIPHERAL;
						clientConnectToDevice = FALSE;	
						switch_Role = FALSE;
						
						#ifdef DEBUG_ENABLED
						UART_UartPutString("Peripheral Advertisment called ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif
					}
					else
					{
						/* If advertisement did not start, maintain the current role and retry later */
						ble_gap_state = BLE_CENTRAL;
						
						#ifdef DEBUG_ENABLED
						UART_UartPutString("Start Peripheral Advertisment Failed ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif					
					}
				}
				
				/* Process Pending BLE Events */
				CyBle_ProcessEvents();
			break;
			
			default:
			
			break;
			
		}
	}
}

/*******************************************************************************
* Function Name: ConnectToPeripheralDevice
********************************************************************************
* Summary:
*        Connects to Peripheral device with given BD Address.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConnectToPeripheralDevice(void)
{
	CYBLE_API_RESULT_T  apiResult;
	
	/* If flag has been set to connect to a device... */
	if(clientConnectToDevice)
	{
		/* Process pending BLE events */
		CyBle_ProcessEvents();
			
		#if 0
		if(CYBLE_STATE_SCANNING == CyBle_GetState())
		{
			CyBle_GapcStopScan();
			#ifdef DEBUG_ENABLED
				UART_UartPutString("Stop Scan called from clientConnectToDevice ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');
			#endif
			CyBle_ProcessEvents();
		}
		#endif
			
		if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
		{
			/* Reset the flag to connect to a device*/
			clientConnectToDevice = FALSE;

			/* Connect to the device whose address has been saved as part of 
			* potential node search */
			apiResult = CyBle_GapcConnectDevice(&peripAddr);
			
			if(apiResult != CYBLE_ERROR_OK)
			{
				#ifdef DEBUG_ENABLED
					UART_UartPutString("Connect Request failed ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
				#endif
			}
			else
			{
				#ifdef DEBUG_ENABLED
					UART_UartPutString("Connect Request Sent ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
				#endif	
			}
		
			/* Process pending BLE events */
			CyBle_ProcessEvents();
		}
	}
}

/*******************************************************************************
* Function Name: RestartCentralScanning
********************************************************************************
* Summary:
*        Restarts Central scanning. Also, if the time that the device has remained
* in Central role exceeds pre-determined value, then the switch role flag is set.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void RestartCentralScanning(void)
{
	/* If the current role is Central and the Central time has exceeded the preset time,
	* then set the flag to switch role to Peripheral */
	if((BLE_CENTRAL == ble_gap_state) && 
		(WatchDog_CurrentCount() - centralStartedTime > CENTRAL_STATE_SPAN) &&
		((CYBLE_STATE_DISCONNECTED == CyBle_GetState()) || (CYBLE_STATE_SCANNING == CyBle_GetState())))
	{
		/* Switch role flag set */
		switch_Role = TRUE;
		#ifdef DEBUG_ENABLED
		UART_UartPutString("switchRole from restartScanning loop ");
		UART_UartPutCRLF(' ');
		#endif
		
		return;
	}
			
	/* If restart scanning flag is set, the restart the Central scanning */
	if(restartScanning)
	{
		/* Process pending BLE events */
		CyBle_ProcessEvents();
		if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
		{
			/* Reset the restart scanning flag */
			restartScanning = FALSE;
			#ifdef DEBUG_ENABLED
			UART_UartPutString("restartScanning loop ");
			UART_UartPutCRLF(' ');
			#endif
			
			/* Start Central scan and process the event */
			CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
			CyBle_ProcessEvents();
			
			#ifdef DEBUG_ENABLED
			UART_UartPutString("Start Scan from restartScanning loop ");
			UART_UartPutCRLF(' ');
			#endif
		}
	}
}	

/*******************************************************************************
* Function Name: SendErrorCode
********************************************************************************
* Summary:
*        Send Error code at GATT Layer
*
* Parameters:
*  uint8 opCode, 
*  CYBLE_GATT_DB_ATTR_HANDLE_T  attrHandle, 
*  CYBLE_GATT_ERR_CODE_T errorCode
*
* Return:
*  void
*
*******************************************************************************/
void SendErrorCode(uint8 opCode, CYBLE_GATT_DB_ATTR_HANDLE_T  attrHandle, CYBLE_GATT_ERR_CODE_T errorCode)
{
	CYBLE_GATTS_ERR_PARAM_T err_param;
		            
    err_param.opcode = opCode;
    err_param.attrHandle = attrHandle;
    err_param.errorCode = errorCode;

    /* Send Error Response */
    (void)CyBle_GattsErrorRsp(cyBle_connHandle, &err_param);
}
/* [] END OF FILE */
