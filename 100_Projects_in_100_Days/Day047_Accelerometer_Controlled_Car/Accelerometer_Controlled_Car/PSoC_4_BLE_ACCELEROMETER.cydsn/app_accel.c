/*****************************************************************************************
* File Name: app_UART.c
*
* Description:
*  Common BLE application code for client devices.
*
*****************************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*****************************************************************************************/

#include "app_accel.h"
#include "app_Ble.h"


/*****************************************************************************************
* Function Name: SendCommand
******************************************************************************************
*
* Summary:
*  This function send the command over BLE notification. 
*
* Parameters:
*  None
*
* Return:
*   None.
*
*****************************************************************************************/

void SendCommand()
{
   
    CYBLE_GATTS_HANDLE_VALUE_NTF_T      uartTxDataNtf;
    
    if(Scan_Accelerometer())
	{
		if(connStatus== CONNECTED)
		{
		        
			   	uartTxDataNtf.value.val  = &BLE_Command;
		        uartTxDataNtf.value.len  = 1;
		        uartTxDataNtf.attrHandle = CYBLE_SERVER_ACCEL_SERVER_ACCEL_TX_DATA_CHAR_HANDLE;
		        CyBle_GattsNotification(cyBle_connHandle, &uartTxDataNtf);
		        CyBle_ProcessEvents();
		  
		        
		}
	}
	else/* ADC under process. Put PSoC into Sleep*/
	{
    CySysPmSleep();		
	}
}

/*****************************************************************************************
* Function Name: SendCommand
******************************************************************************************
*
* Summary:
*  This function reads the x,y,z  analog value and based on the reading the gesture is 
*  idetifiend. A command is set based on the idetified gesture. If ADC has not completed
*  reading the values, then this will return 0.
*
* Parameters:
*  None
*
* Return:
*   uint8 status. Success or failure.
*
*****************************************************************************************/
uint8 Scan_Accelerometer()
{  
	
	if(Sensor_ADC_IsEndConversion(Sensor_ADC_RETURN_STATUS))
	{
		ADC_x_current = Sensor_ADC_CountsTo_mVolts(0, Sensor_ADC_GetResult16(0));
		ADC_y_current = Sensor_ADC_CountsTo_mVolts(1, Sensor_ADC_GetResult16(1));
		ADC_z_current = Sensor_ADC_CountsTo_mVolts(2, Sensor_ADC_GetResult16(2));
		/*Normal gesture idetified*/
		if((ADC_y_current>=(Y_DEFAULT-40) && ADC_y_current<= (Y_DEFAULT+40))&&(ADC_z_current>=(Z_DEFAULT-40) && ADC_z_current<=(Z_DEFAULT+40))&& (ADC_x_current>=(X_DEFAULT-40) && ADC_x_current<=(X_DEFAULT+40))&& state!=NORMAL)
		{
			
			state= NORMAL;
			BLE_Command='p';
			return 1;
			
		}
		/*Backward gesture idetified*/
		if((ADC_y_current<=1100)&& (ADC_z_current>=1150)&& state!=BACKWARD)
		{
				
			state= BACKWARD;
			BLE_Command='b';
			return 1;
		}
		/*Forward gesture idetified*/
		if((ADC_y_current>=1350)&& (ADC_z_current>=1150)&& state!=FORWARD)
		{
		    state= FORWARD;
			BLE_Command='s';
			return 1;
		}
		/*Right gesture idetified*/
		if((ADC_x_current<=1050)&& (ADC_z_current>=1150)&& state!=RIGHT)
		{
			
			state= RIGHT;
			BLE_Command='R';
			return 1;
		}
		/*Left gesture idetified*/
		if((ADC_x_current>=1350)&& (ADC_z_current>=1150)&& state!=LEFT)
		{
			state= LEFT;
            BLE_Command='L';
			return 1;
		}
		return 0;/*No change of state */
	
    }
    else
		return 0;/*ADC is busy. Return 0 */
}





/* [] END OF FILE */
