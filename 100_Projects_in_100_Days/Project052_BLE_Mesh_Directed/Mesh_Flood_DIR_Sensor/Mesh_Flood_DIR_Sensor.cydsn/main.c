/******************************************************************************
* Project Name		: Mesh_Flood_DIR_Sensor
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.3
* Compiler    		: ARM GCC 4.8.4, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : roit@cypress.com
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

extern uint8 switch_Role;
extern uint8 ble_gap_state;
extern uint8 deviceConnected;

extern uint8 restartScanning;

extern uint8 dataADVCounter;

#ifdef ENABLE_ADV_DATA_COUNTER
extern CYBLE_GAPP_DISC_DATA_T  new_advData;
extern uint8 potential_node_found;
extern uint8 potential_node_bdAddr[6];
extern uint8 potential_node_bdAddrType;
#endif

extern volatile uint8 clientConnectToDevice;
extern CYBLE_GAP_BD_ADDR_T				peripAddr;
extern uint8 shut_down_led;
extern uint16 node_address;

CY_ISR_PROTO(Button_ISR);

#ifdef RESTART_BLE_STACK
uint8 stackRestartIssued = FALSE;
#endif
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*       Entry point for the firmware, Starts the system and continuously processes
* Application and BLE events.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main()
{
	#ifdef RESTART_BLE_STACK
		uint16	hwRegConnected;	
	#endif
	
	/* Initialize system */	
	InitializeSystem();
	
    for(;;)
    {
		/* Process BLE Events. This function generates the respective 
		* events to application layer */
		CyBle_ProcessEvents();
		
		/* If flag is set, then switch the GAP role between Peripheral and 
		* Central. This function disconnects existing connection, if any, 
		* before switching role. Depending on switched role, the system will
		* either start scanning or start advertisement */
		SwitchRole();
		
		/* If a valid node is found, then this function will initiate a connection 
		* to that node. */
		ConnectToPeripheralDevice();
		
		/* This function will restart scanning when earlier connection has been
		* disconnected. This function also checks internal counter and sets flag to
		* switch role from Central to Peripheral when the count is above set value. */
		RestartCentralScanning();
		
		#ifdef SENSOR_DATA_ROUTING
		/* Application API to collect the new sensor data and send it over to all nodes
		* using mesh network. CUrrently, the sensor data is the new RGB data selected on 
		* each button press. The content in the API can be changed to collect and data 
		* and forward it to the mesh network. */
		CheckSensorStatus();
		#endif
		
		#ifdef RESTART_BLE_STACK
			/* Check the LL Config Status resgister for hardware connection and compare 
			* it with BLE state. If they mismatch, then assume the stack disconnection issue 
			* and restart the stack */
			CyBLE_Bless_LlRegRead((uint32*)CYREG_BLE_BLELL_CE_CNFG_STS_REGISTER,&hwRegConnected); 
 
	        if((CyBle_GetState()==CYBLE_STATE_CONNECTED) && (!(0x400 & hwRegConnected))) 
	        { 
	            /* Restart BLE STACK */ 
				#if (DEBUG_ENABLED == 1)
					UART_UartPutString("     \\//\\//\\//\\// Restart BLE Stack \\//\\//\\//\\// ");
					UART_UartPutCRLF(' ');
				#endif

	            CyBle_Stop();
	            CyBle_Start(GenericEventHandler);
			
				stackRestartIssued = TRUE;
				
				/* Reset the BLE gap state flag */
				ble_gap_state = BLE_PERIPHERAL;
	        }
		#endif
		
		#ifdef ENABLE_LOW_POWER_MODE
		/* This function puts the system to Sleep/Deep Sleep, depending on BLESS 
		* state */
		HandleLowPowerMode();
		#endif

    }
}

/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
* Summary:
*       Starts the components in the project and registers BLE event handler 
* function. It also modifies the advertisement packet and appends data counter
* at its end.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitializeSystem(void)
{
	#ifdef STORE_SFLASH_NODE_ADDRESS
		uint8 *sflashPtr;
		uint8 readData[2];
		CYBLE_GATT_HANDLE_VALUE_PAIR_T	handleValPair;	
	#endif

	/* Enable global interrupts. */
	CyGlobalIntEnable; 
	
	/* Start BLE component and Register the generic Event callback function */
	CyBle_Start(GenericEventHandler);

	/* Start the PrISM component and configure drive mode of LED pins to be
	* initially OFF*/
	PrISM_1_Start();
	PrISM_2_Start();
	
	PrISM_1_WritePulse0(255);
	PrISM_1_WritePulse1(255);
	PrISM_2_WritePulse0(255);
	
	RED_SetDriveMode(RED_DM_STRONG);
	GREEN_SetDriveMode(GREEN_DM_STRONG);
	BLUE_SetDriveMode(BLUE_DM_STRONG);
	
	/* Configure the Watchdog (WDT) timer for 100 millisecond timing */
	InitializeWatchdog(WATCHDOG_COUNT_VAL);
	
	/* Provide a color pusle on RGB LED to indicate startup */
	PrISM_2_WritePulse0(128);
	CyDelay(20);
	PrISM_2_WritePulse0(255);

	
	#ifdef ENABLE_ADV_DATA_COUNTER
	new_advData = *cyBle_discoveryModeInfo.advData;
	
	if( cyBle_discoveryModeInfo.advData->advDataLen < 29)
	{
		/* Initialize the DataCounter data in advertisement packet. This is custom data in 
		* ADV packet and used to track whether the RGB LED data is latest or not */
		new_advData.advData[cyBle_discoveryModeInfo.advData->advDataLen] = CUSTOM_ADV_DATA_MARKER;	//length of next packet
		new_advData.advData[cyBle_discoveryModeInfo.advData->advDataLen+1] = dataADVCounter;
		new_advData.advDataLen = cyBle_discoveryModeInfo.advData->advDataLen+2;
	}
	
	/* Assign the new ADV data to stack */
	cyBle_discoveryModeInfo.advData = &new_advData;
	#endif
	
	#ifdef STORE_SFLASH_NODE_ADDRESS
		/* Read the index 0 in row 1 for node address */
		sflashPtr = (uint8 *)USER_SFLASH_BASE_ADDRESS 
							+ (NODE_ADDRESS_SFLASH_ROW * USER_SFLASH_ROW_SIZE) 
							+ SFLASH_NODE_ADDRESS_INDEX; 
		
		readData[NODE_ADDRESS_LSB_INDEX] = *sflashPtr++;
		readData[NODE_ADDRESS_MSB_INDEX] = *sflashPtr;

		if(((0xFF != readData[0]) || (0xFF != readData[1]))
			&& ((0x00 != readData[0]) || (0x00 != readData[1])))
		{
			/* If the read adress is not equal to 0xFFFF or 0x0000, then it is  
			* assumed that a valid node address has been already assigned. Rename 
			* the BLE device to Titan to indicate that the device can be connected
			* and RGB LED data transffered */
			node_address = (uint16)(((uint16)readData[NODE_ADDRESS_MSB_INDEX] << 8)
									| (uint16)readData[NODE_ADDRESS_LSB_INDEX]);
			
			new_advData.advData[ADV_DATA_NAME_START_INDEX] = 'T';
			new_advData.advData[ADV_DATA_NAME_START_INDEX+1] = 'i';
			new_advData.advData[ADV_DATA_NAME_START_INDEX+2] = 't';
			new_advData.advData[ADV_DATA_NAME_START_INDEX+3] = 'a';
			new_advData.advData[ADV_DATA_NAME_START_INDEX+4] = 'n';
			
			cyBle_discoveryModeInfo.advData = &new_advData;
			
			/* Update the GATT DB with the valid node address read from SFLASH */
			handleValPair.attrHandle = CYBLE_NODE_ADDRESS_CHAR_HANDLE;
			handleValPair.value.len = 2;
			handleValPair.value.val = readData;
			
			CyBle_GattsWriteAttributeValue(&handleValPair,0,&cyBle_connHandle,CYBLE_GATT_DB_LOCALLY_INITIATED);
		}
	#endif
	
	#ifdef SENSOR_DATA_ROUTING
	/* Clear pending interrupts and enable ISR to record Button press through Glitch Filter */
	isr_button_ClearPending();	
	isr_button_StartEx(Button_ISR);
	#endif

	#if (DEBUG_ENABLED == 1)
		UART_Start();
		UART_UartPutCRLF(' ');
		UART_UartPutString("-----------------------------Mesh Directed Flood-----------------------------------");
		UART_UartPutCRLF(' ');
	#endif
}

/*******************************************************************************
* Function Name: UpdateRGBled
********************************************************************************
* Summary:
*        Update the RGB LED color by reconfiguring PrISM component. This function 
* is valid for PSoC 4 BLE parts and not PROC BLE, as PRoC BLE does not have UDBs
* that makes up the PrISM component. To replace this with software PrISM, refer to
* CY8CKIT-042-BLE Pioneer Kit example project, PRoC_BLE_CapSense_Slider_LED, from
* http://www.cypress.com/CY8CKIT-042-BLE
*
* Parameters:
*  rgb_led_data: array storing the RGB and Intensity vlaue
*  len: length of the array.
*
* Return:
*  void
*
*******************************************************************************/
void UpdateRGBled(uint8 * rgb_led_data, uint8 len)
{
	uint8 calc_red, calc_green, calc_blue;
	
	if(len == RGB_PAYLOAD_LEN)
	{
		/* If a valid length packet has been sent, calculate the intensity of each of
		* R, G and B color and update the PrISM module */
		calc_red = (uint8)(((uint16)rgb_led_data[RGB_RED_INDEX]*rgb_led_data[RGB_INTENSITY_INDEX])/RGB_LED_MAX_VAL);
		calc_green = (uint8)(((uint16)rgb_led_data[RGB_GREEN_INDEX]*rgb_led_data[RGB_INTENSITY_INDEX])/RGB_LED_MAX_VAL);
		calc_blue = (uint8)(((uint16)rgb_led_data[RGB_BLUE_INDEX]*rgb_led_data[RGB_INTENSITY_INDEX])/RGB_LED_MAX_VAL);
		
		PrISM_1_WritePulse0(RGB_LED_MAX_VAL - calc_red);
		PrISM_1_WritePulse1(RGB_LED_MAX_VAL - calc_green);
		PrISM_2_WritePulse0(RGB_LED_MAX_VAL - calc_blue);
		
		#ifdef ENABLE_LOW_POWER_MODE
			/* Depending on whether there is color or not, set the shut_down_led flag
			* to allow the system to go to deep sleep. Else, reset the flag to 0 to 
			* allow the system to go to sleep only, as PrISM is active */
			if((calc_red<RGB_OFF_THRESHOLD) 
				&& (calc_green<RGB_OFF_THRESHOLD)
				&& (calc_blue<RGB_OFF_THRESHOLD))
			{
				shut_down_led = TRUE;
			}
			else
			{
				shut_down_led = FALSE;	
			}
		#endif
		
		#if (DEBUG_ENABLED == 1)
			UART_UartPutString("PrISM Updated ");
			UART_UartPutCRLF(' ');
		#endif
	}
}

/* [] END OF FILE */
