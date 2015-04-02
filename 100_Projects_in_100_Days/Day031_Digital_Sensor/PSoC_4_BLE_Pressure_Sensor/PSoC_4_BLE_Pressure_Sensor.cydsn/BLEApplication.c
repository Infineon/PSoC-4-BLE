/***********************************************************************************************************************
* Project Name		: PSoC_4_BLE_Pressure_Sensor
* File Name			: BLEApplication.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit
***********************************************************************************************************************/

/************************************************************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
************************************************************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
***********************************************************************************************************************/

#include <BLEApplication.h>

/*************************Variables Declaration*************************************************************************/
uint8 StartAdvertisement = FALSE; //This flag is used to start advertisement
uint8 DeviceConnected = FALSE; //This flag is set when a Central device is connected
uint8 BLEStackStatus = FALSE; //Variable store the BLE stack status
uint8 ConnectionParametersUpdateRequired = TRUE; //This flag is used to send the connection parameters update
uint8 TemperatureNotificationEnabled = FALSE; //This flag is set when the Central device writes to CCCD to enable temperature notification
uint8 PressureNotificationEnabled = FALSE; //This flag is set when the Central device writes to CCCD to enable pressure notification
uint8 AltitudeNotificationEnabled = FALSE; //This flag is set when the Central device writes to CCCD to enable altitude notification
uint8 UpdateTemperatureNotificationAttribute = FALSE; //This flags is used to update the respective temperature CCCD value
uint8 UpdatePressureNotificationAttribute = FALSE; //This flags is used to update the respective pressure CCCD value
uint8 UpdateAltitudeNotificationAttribute = FALSE; //This flags is used to update the respective altitude CCCD value
uint8 TemperatureNotificationCCCDValue[0x02]; //The temperature CCCD value is stored in this array
uint8 PressureNotificationCCCDValue[0x02]; //The pressure CCCD value is stored in this array
uint8 AltitudeNotificationCCCDValue[0x02]; //The altitude CCCD value is stored in this array
uint8 TemperatureNotificationData[0x04]; //The temperature notification value is stored in this array
uint8 PressureNotificationData[0x04]; //The pressure notification value is stored in this array
uint8 AltitudeNotificationData[0x04]; //The altitude notification value is stored in this array
extern uint8 ChangePowerPinDriveMode; //This flag is set when the GATT disconnect event occurs
extern long LastTemperatureData; //Previous temperature reading is stored in this variable
extern long LastPressureData; //Previous pressure reading is stored in this variable
extern long LastAltitudeData; //Previous altitude reading is stored in this variable
static CYBLE_CONN_HANDLE_T ConnectionHandle; //This handle stores the connection parameters
static CYBLE_GATTS_WRITE_REQ_PARAM_T *WriteRequestedParameter; //Variable to store the data received as part of the Write request event
static CYBLE_GATTS_HANDLE_VALUE_NTF_T TemperatureNotificationHandle; //This handle stores the temperature notification data parameters
static CYBLE_GATTS_HANDLE_VALUE_NTF_T PressureNotificationHandle; //This handle stores the pressure notification data parameters
static CYBLE_GATTS_HANDLE_VALUE_NTF_T AltitudeNotificationHandle; //This handle stores the altitude notification data parameters
static CYBLE_GATT_HANDLE_VALUE_PAIR_T TemperatureNotificationCCCDHandle; //This handle is used to update the temperature CCCD
static CYBLE_GATT_HANDLE_VALUE_PAIR_T PressureNotificationCCCDHandle; //This handle is used to update the pressure CCCD
static CYBLE_GATT_HANDLE_VALUE_PAIR_T AltitudeNotificationCCCDHandle; //This handle is used to update the altitude CCCD
static CYBLE_GAP_CONN_UPDATE_PARAM_T ConnectionParametersHandle = {CONN_PARAM_UPDATE_MIN_CONN_INTERVAL, CONN_PARAM_UPDATE_MAX_CONN_INTERVAL,
    CONN_PARAM_UPDATE_SLAVE_LATENCY, CONN_PARAM_UPDATE_SUPRV_TIMEOUT}; //Connection Parameter update values
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: CustomEventHandler
**************************************************************************************************************************
* Summary: This is a call back event function to handle various events from BLE stack.
*
* Parameters:
*  Event - Event returned
*  EventParameter - Link to value of the event returned
*
* Return:
*  void
*
*************************************************************************************************************************/
void CustomEventHandler(uint32 Event, void *EventParameter)
{
    switch(Event)
	{
        /**********************************************************
        *                       General Events
        ***********************************************************/
		case CYBLE_EVT_STACK_ON: //This event is received when the BLE component is started			
            StartAdvertisement = TRUE; //Set the advertisement flag
		break;
            
        case CYBLE_EVT_STACK_BUSY_STATUS: //This event is generated when the internal stack buffer is full and no more data can be accepted or the stack has buffer available and can accept data
            BLEStackStatus = *(uint8*)EventParameter; //Extract the BLE stack status
        break;
        
        /**********************************************************
        *                       GAP Events
        ***********************************************************/		
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP: //This event is received when the device starts or stops advertising
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
            {
                StartAdvertisement = TRUE; //Set the advertisement flag
            }
        break;
			
		case CYBLE_EVT_GAP_DEVICE_DISCONNECTED: //This event is received when the device is disconnected
			StartAdvertisement = TRUE; //Set the advertisement flag
        break;

        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND: //This event is received when the device is connected over GATT level
			ConnectionHandle = *(CYBLE_CONN_HANDLE_T *)EventParameter; //Update the attribute handle on GATT connection
			
			DeviceConnected = TRUE; //Set device connection status flag
            ChangePowerPinDriveMode = FALSE; //Clear change power pin drive mode flag
        break;
			
        case CYBLE_EVT_GATT_DISCONNECT_IND: //This event is received when device is disconnected
			DeviceConnected = FALSE; //Clear device connection status flag
            TemperatureNotificationEnabled = FALSE; //Clear the temperature notification flag
            PressureNotificationEnabled =  FALSE; //Clear the pressure notification flag
            AltitudeNotificationEnabled = FALSE; //Clear the altitude notification flag
            UpdateTemperatureNotificationAttribute = TRUE; //Clear the update temperature notification flag
            UpdatePressureNotificationAttribute = TRUE; //Clear the update pressure notification flag
            UpdateAltitudeNotificationAttribute = TRUE; //Clear the update altitude notification flag
            /* Clear temperature notification data */
            TemperatureNotificationData[0x03] = TemperatureNotificationData[0x02] = TemperatureNotificationData[0x01] = TemperatureNotificationData[0x00] = ZERO;
            /* Clear pressure notification data */
            PressureNotificationData[0x03] = PressureNotificationData[0x02] = PressureNotificationData[0x01] = PressureNotificationData[0x00] = ZERO;
            /* Clear altitude notification data */
            AltitudeNotificationData[0x03] = AltitudeNotificationData[0x02] = AltitudeNotificationData[0x01] = AltitudeNotificationData[0x00] = ZERO;
            LastTemperatureData = ZERO; //Clear last stored temperature notification data
            LastPressureData = ZERO; //Clear last stored pressure notification data
            LastAltitudeData = ZERO; //Clear last stored altitude notification data
            ChangePowerPinDriveMode = TRUE; //Set change power pin drive mode flag
            ConnectionParametersUpdateRequired = TRUE; //Set the Connection Parameters Update flag
            UpdateNotificationCCCDAttribute(); //Update the CCCD writing by the Central device
		break;
            
        case CYBLE_EVT_GATTS_WRITE_REQ: //When this event is triggered, the peripheral has received a write command on the custom characteristic
			/* Extract the write value from the event parameter */
            WriteRequestedParameter = (CYBLE_GATTS_WRITE_REQ_PARAM_T *)EventParameter;
			
			/* Check if command is for notification attribute and update the flag for sending notifications */
            if(WriteRequestedParameter->handleValPair.attrHandle == CYBLE_BMP180_TEMPERATURE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
            {
                /* Set temperature notification flag so that application can start sending notifications */
				TemperatureNotificationEnabled = WriteRequestedParameter->handleValPair.value.val[CYBLE_BMP180_TEMPERATURE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX];
                
                UpdateTemperatureNotificationAttribute = TRUE; //Set flag to allow CCCD to be updated for next read operation
            }
            
            /* Check if command is for notification attribute and update the flag for sending notifications */
            if(WriteRequestedParameter->handleValPair.attrHandle == CYBLE_BMP180_PRESSURE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
            {
                /* Set pressure notification flag so that application can start sending notifications */
				PressureNotificationEnabled = WriteRequestedParameter->handleValPair.value.val[CYBLE_BMP180_PRESSURE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX];
                
                UpdatePressureNotificationAttribute = TRUE; //Set flag to allow CCCD to be updated for next read operation
            }
            
            /* Check if command is for notification attribute and update the flag for sending notifications */
            if(WriteRequestedParameter->handleValPair.attrHandle == CYBLE_BMP180_ALTITUDE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
            {
                /* Set altitude notification flag so that application can start sending notifications */
				AltitudeNotificationEnabled = WriteRequestedParameter->handleValPair.value.val[CYBLE_BMP180_ALTITUDE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX];
                
                UpdateAltitudeNotificationAttribute = TRUE; //Set flag to allow CCCD to be updated for next read operation
            }
            
            /* Send response to the write command received */
			CyBle_GattsWriteRsp(ConnectionHandle);
        break;
            
        /**********************************************************
        *                       L2CAP Events
        ***********************************************************/
            
        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP: //This event is generated when the L2CAP connection parameter update response received
            ConnectionParametersUpdateRequired = FALSE; //Clear the Connection Parameters Update flag
        break;
			
        default:
        break;
	}
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: SendTemperatureOverNotification
**************************************************************************************************************************
* Summary: This function sends the temperature data as BLE notification.
* It updates the notification handle with temperature data and triggers the BLE component to send notification.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void SendTemperatureOverNotification(void)
{
	/* If stack is not busy, then send the notification */
    if(BLEStackStatus == CYBLE_STACK_STATE_FREE)
    {
        /* Update notification handle with temperature data */
    	TemperatureNotificationHandle.attrHandle = CYBLE_BMP180_TEMPERATURE_CHAR_HANDLE;
    	TemperatureNotificationHandle.value.val = TemperatureNotificationData;
    	TemperatureNotificationHandle.value.len = sizeof(TemperatureNotificationData);
    	
    	/* Send the updated handle as part of attribute for notifications */
    	CyBle_GattsNotification(ConnectionHandle, &TemperatureNotificationHandle);
    }
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: SendPressureOverNotification
**************************************************************************************************************************
* Summary: This function sends the pressure data as BLE notification.
* It updates the notification handle with pressure data and triggers the BLE component to send notification.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void SendPressureOverNotification(void)
{
	/* If stack is not busy, then send the notification */
    if(BLEStackStatus == CYBLE_STACK_STATE_FREE)
    {
        /* Update notification handle with pressure data */
    	PressureNotificationHandle.attrHandle = CYBLE_BMP180_PRESSURE_CHAR_HANDLE;
    	PressureNotificationHandle.value.val = PressureNotificationData;
    	PressureNotificationHandle.value.len = sizeof(PressureNotificationData);
    	
    	/* Send the updated handle as part of attribute for notifications */
    	CyBle_GattsNotification(ConnectionHandle, &PressureNotificationHandle);
    }
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: SendAltitudeOverNotification
**************************************************************************************************************************
* Summary: This function sends the altitude data as BLE notification.
* It updates the notification handle with altitude data and triggers the BLE component to send notification.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void SendAltitudeOverNotification(void)
{
	/* If stack is not busy, then send the notification */
    if(BLEStackStatus == CYBLE_STACK_STATE_FREE)
    {
        /* Update notification handle with altitude data */
    	AltitudeNotificationHandle.attrHandle = CYBLE_BMP180_ALTITUDE_CHAR_HANDLE;
    	AltitudeNotificationHandle.value.val = AltitudeNotificationData;
    	AltitudeNotificationHandle.value.len = sizeof(AltitudeNotificationData);
    	
    	/* Send the updated handle as part of attribute for notifications */
    	CyBle_GattsNotification(ConnectionHandle, &AltitudeNotificationHandle);
    }
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: UpdateNotificationCCCDAttribute
**************************************************************************************************************************
* Summary: This function updates the notification handle status and reports 
* it to BLE component database so that it can be read by Central device.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void UpdateNotificationCCCDAttribute(void)
{
	/* Update notification attribute only when there is change in temperature CCCD */
    if(UpdateTemperatureNotificationAttribute)
    {
        UpdateTemperatureNotificationAttribute = FALSE; //Clear the flag
        
        /* Write the current temperature notification status to the local variable */
        TemperatureNotificationCCCDValue[0x00] = TemperatureNotificationEnabled;
        TemperatureNotificationCCCDValue[0x01] = ZERO;
        
        /* Update CCCD handle with notification status data */
    	TemperatureNotificationCCCDHandle.attrHandle = CYBLE_BMP180_TEMPERATURE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
    	TemperatureNotificationCCCDHandle.value.val = TemperatureNotificationCCCDValue;
    	TemperatureNotificationCCCDHandle.value.len = sizeof(TemperatureNotificationCCCDValue);
    	
    	/* Send the updated handle as part of attribute for notifications */
    	CyBle_GattsWriteAttributeValue(&TemperatureNotificationCCCDHandle, 0x00, &ConnectionHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);
    }
    
    /* Update notification attribute only when there is change in pressure CCCD */
    if(UpdatePressureNotificationAttribute)
    {
        UpdatePressureNotificationAttribute = FALSE; //Clear the flag
        
        /* Write the current pressure notification status to the local variable */
        PressureNotificationCCCDValue[0x00] = PressureNotificationEnabled;
        PressureNotificationCCCDValue[0x01] = ZERO;
        
        /* Update CCCD handle with notification status data */
    	PressureNotificationCCCDHandle.attrHandle = CYBLE_BMP180_PRESSURE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
    	PressureNotificationCCCDHandle.value.val = PressureNotificationCCCDValue;
    	PressureNotificationCCCDHandle.value.len = sizeof(PressureNotificationCCCDValue);
    	
    	/* Send the updated handle as part of attribute for notifications */
    	CyBle_GattsWriteAttributeValue(&PressureNotificationCCCDHandle, 0x00, &ConnectionHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);
    }
    
    /* Update notification attribute only when there is change in altitude CCCD */
    if(UpdateAltitudeNotificationAttribute)
    {
        UpdateAltitudeNotificationAttribute = FALSE; //Clear the flag
        
        /* Write the current altitude notification status to the local variable */
        AltitudeNotificationCCCDValue[0x00] = AltitudeNotificationEnabled;
        AltitudeNotificationCCCDValue[0x01] = ZERO;
        
        /* Update CCCD handle with notification status data */
    	AltitudeNotificationCCCDHandle.attrHandle = CYBLE_BMP180_ALTITUDE_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
    	AltitudeNotificationCCCDHandle.value.val = AltitudeNotificationCCCDValue;
    	AltitudeNotificationCCCDHandle.value.len = sizeof(AltitudeNotificationCCCDValue);
    	
    	/* Send the updated handle as part of attribute for notifications */
    	CyBle_GattsWriteAttributeValue(&AltitudeNotificationCCCDHandle, 0x00, &ConnectionHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);
    }
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: UpdateConnectionParameters
**************************************************************************************************************************
* Summary: This function sends the Connection Parameters Update Request to the Central device
* immediately after connected and modifies the connection interval for low power operation.
* The Central device can accept or reject this request based on the Central device configuration.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void UpdateConnectionParameters(void)
{
    if(DeviceConnected && ConnectionParametersUpdateRequired)
    {
        ConnectionParametersUpdateRequired = FALSE; //Clear the Connection Parameters Update flag
        /* Send Connection Parameter Update request with desired parameter values */
        CyBle_L2capLeConnectionParamUpdateRequest(ConnectionHandle.bdHandle, &ConnectionParametersHandle);
    }
}
/***********************************************************************************************************************/

/* [] END OF FILE */
