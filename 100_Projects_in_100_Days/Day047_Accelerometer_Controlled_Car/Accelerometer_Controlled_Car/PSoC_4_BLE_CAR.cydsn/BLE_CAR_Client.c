/*******************************************************************************
* File Name: BLE_CAR_Client.c
*
* Description:
*  This is the source file to handle the BLE CAR Client States and functions
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

#include "BLE_CAR_Client.h"
#include "LED.h"
#include "Motor_Control.h"


/* 'connHandle' is a varibale of type 'CYBLE_CONN_HANDLE_T' (defined in 
* BLE_StackGatt.h) and is used to store the connection handle parameters after
* connecting with the peripheral device. */
CYBLE_CONN_HANDLE_T			connHandle;

	/* 'apiResult' is a varibale of type 'CYBLE_API_RESULT_T' (defined in 
* BLE_StackTypes.h) and is used to store the return value from BLE APIs. */
	
CYBLE_API_RESULT_T 		apiResult;
	
/* 'connectPeriphDevice' is a varibale of type 'CYBLE_GAP_BD_ADDR_T' (defined in 
* BLE_StackGap.h) and is used to store address of the connected device. */
CYBLE_GAP_BD_ADDR_T connectPeriphDevice;

/*******************************************************************************
* Function Name: FilterScanResponsePackets
********************************************************************************
* Summary:
*        Function that meets the requirements of the Server (Accelerometer). This
*		function ensures that it connects only to the Server Project
*
* Parameters:
*  scanReport: Advertisement report received by GAP Central
*
* Return:
*  void
*
*******************************************************************************/

void FilterScanResponsePackets(CYBLE_GAPC_ADV_REPORT_T* scanReport)
{

	if(scanReport->data[1] == MANUFACTURER_SPECIFIC_DATA && 
	scanReport->data[2] == COMPANY_LSB && 
	scanReport->data[3] == COMPANY_MSB && 
	scanReport->data[4] == MAN_SPEC_DATA_LSB && scanReport->data[5] == MAN_SPEC_DATA_MSB)
	{
		CyBle_GapcStopScan();
		
		memcpy(connectPeriphDevice.bdAddr, scanReport->peerBdAddr,
               sizeof(connectPeriphDevice.bdAddr));
				
		ble_CAR_Server_found = TRUE;
						
	}		
}

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
	
	switch(event)
	{
		case CYBLE_EVT_STACK_ON:
			break;
		
		case CYBLE_EVT_GAPC_SCAN_START_STOP:
		break;
			
		case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
                /*Check if device meets the Server requirements*/
                FilterScanResponsePackets((CYBLE_GAPC_ADV_REPORT_T *)eventparam);
		break;
			
 		case CYBLE_EVT_GATT_CONNECT_IND:
			/* When the peripheral device is connected, store the connection handle.*/
			connHandle = *(CYBLE_CONN_HANDLE_T *)eventparam;
			break;
			
		case CYBLE_EVT_GATT_DISCONNECT_IND:
			RED_LED_ON();
			CyDelay(3000);
			break;
			
		case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			RED_LED_ON();
			PWM_Motor_1_Stop();
			PWM_Motor_2_Stop();
			Clock_PWM_Stop();
			break;		
		
		case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
			/*Got data from server */
			{
	            CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *UART_RX_data = (CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *)eventparam;
				Motor_Command=*UART_RX_data->handleValPair.value.val;
				UpdateMotor(Motor_Command);
				break;
			}
			
		default:
			break;
	}
}

/*******************************************************************************
* Function Name: handle_ble_CAR_Client_State
********************************************************************************
* Summary:
*        Handles the various States of the Client
*
* Parameters:
*  None
*
* Return:
*  void
*
*******************************************************************************/

void handle_ble_CAR_Client_State(void)
{
	
	if (CYBLE_STACK_STATE_BUSY == CyBle_GattGetBusStatus())
    {
        /*If stack is busy wait for ready signal*/
        return;
    }
	
	switch(ble_CAR_Client_State)
	{
		case BLE_CAR_CLIENT_STATE_NOT_READY:
		
			if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
			{
				if(ble_CAR_Server_found)
				{
					ble_CAR_Server_found = FALSE;
					apiResult = CyBle_GapcConnectDevice(&connectPeriphDevice);
					if(CYBLE_ERROR_OK != apiResult )
					{
						
					}
				}
				else
				{
					/*Start scanning*/    
				    if(CYBLE_STATE_DISCONNECTED == CyBle_GetState()) 
				    {
						BLUE_LED_ON(); /*Indicates scanning*/
				        if(CYBLE_ERROR_OK != CyBle_GapcStartScan(CYBLE_SCANNING_FAST))
				        {
				        }
				    }
				}
			}
			else if(CYBLE_STATE_SCANNING == CyBle_GetState())
			{
				
			}
			if(CYBLE_CLIENT_STATE_CONNECTED == CyBle_GetClientState())
            {
                /*Discover all server DB items*/
                if(CYBLE_ERROR_OK != CyBle_GattcStartDiscovery(cyBle_connHandle))
                {
                }
				
				/* Enable notifications on the server side */
				uint8 value = CYBLE_CCCD_NOTIFICATION;
				CYBLE_GATTC_WRITE_CMD_REQ_T write_server;
				
				write_server.attrHandle = BLE_CAR_CCCD_HANDLE;
				write_server.value.len = 1;
				write_server.value.val = &value;
				
				CyBle_GattcWriteWithoutResponse(connHandle,&write_server);
				
            }
			
			else if(CYBLE_CLIENT_STATE_DISCOVERED == CyBle_GetClientState())
            {
                /*Configure the device on each connection*/
                ble_CAR_Client_State = BLE_CAR_CLIENT_STATE_CONFIG;
            }
			
			break;
			
		        /*BLE component is ready to process new commands*/
			
        case BLE_CAR_CLIENT_STATE_ACTIVE:
			GREEN_LED_ON();
            /*Reset the application state, if BLE is not in discovered state*/
            if(CYBLE_CLIENT_STATE_DISCOVERED != CyBle_GetClientState())
            {
                ble_CAR_Client_State = BLE_CAR_CLIENT_STATE_NOT_READY;
            }
            
            break;	
		
		        /*BLE component is processing current command*/
        case BLE_CAR_CLIENT_STATE_BUSY:
            /*BLE component is processing current command*/

            /*Reset the application state, if BLE is not in discovered state*/
            if(CYBLE_CLIENT_STATE_DISCOVERED != CyBle_GetClientState())
            {
                ble_CAR_Client_State = BLE_CAR_CLIENT_STATE_NOT_READY;
            }
            break;	
		
		        /*Connected device being configured for operation*/
        case BLE_CAR_CLIENT_STATE_CONFIG:
            /*Reset the application state, if BLE is not in discovered state*/
            if(CYBLE_CLIENT_STATE_DISCOVERED != CyBle_GetClientState())
            {
                ble_CAR_Client_State = BLE_CAR_CLIENT_STATE_NOT_READY;
            }
            else
            {	
                /*Exit the state if all the configurations are complete*/
                ble_CAR_Client_State = BLE_CAR_CLIENT_STATE_ACTIVE;
            }
           break;	
			
		default:
			break;
	}
	
}

/* [] END OF FILE */
