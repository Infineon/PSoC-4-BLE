/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "app_Ble.h"


/*******************************************************************************
* Function Name: AppCallBack
********************************************************************************
* Summary:
*        Call back event function to handle varios events from BLE stack
*
* Parameters:
*  event:		event returned
*  eventParam:	link to value of the events returned
*
* Return:
*  void
*
*******************************************************************************/

void AppCallBack(uint32 event, void *eventParam)
{
	CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;
    
   	switch (event)
   	{
	    case CYBLE_EVT_STACK_ON:
			/* start advertising */
	        apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
			
			if(apiResult == CYBLE_ERROR_OK)
			{
				
#ifdef 	LED_INDICATION	
				ADV_LED_ON();
#endif	/* LED_INDICATION */ 

			}
	        break;

	    case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			
			sendNotifications = 0;
			
#ifdef	 ENABLE_SPI_ONLY_WHEN_CONNECTED	
			/* Stop SPI Slave operation */
			SPI_Stop();
			
#endif			
			
#ifdef 	LED_INDICATION
		    /* Indicate disconnect event to user */
			DISCON_LED_ON();
			
			CyDelay(3000); 
#endif	/* LED_INDICATION */ 

			/* start advertising */
	        apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
			
			if(apiResult == CYBLE_ERROR_OK)
			{
#ifdef 	LED_INDICATION	
				ADV_LED_ON();
#endif	/* LED_INDICATION */ 
			}	
	        break;    
     
	    case CYBLE_EVT_GATT_CONNECT_IND:
			
#ifdef 	LED_INDICATION	
			CONNECT_LED_ON();
#endif	/* LED_INDICATION */ 	
	        
#ifdef	 ENABLE_SPI_ONLY_WHEN_CONNECTED	
			/* Start SPI Slave operation */
			SPI_Start();
			
#endif	
			break;
        
	    /* Client may do Write Value or Write Value without Response. Handle both */   
	    case CYBLE_EVT_GATTS_WRITE_REQ:
	    case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
	        wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
			
	        /* Handling Notification Enable */
			if(wrReqParam->handleValPair.attrHandle == CYBLE_SPI_READ_SPI_READ_DATA_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
	        {
				CYBLE_GATT_HANDLE_VALUE_PAIR_T    SPINotificationCCDHandle;
				uint8 SPICCDValue[2];
				
	            /* Extract CCCD Notification enable flag */
	            sendNotifications = wrReqParam->handleValPair.value.val[0];
				
				/* Write the present SPI notification status to the local variable */
				SPICCDValue[0] = sendNotifications;
				
				SPICCDValue[1] = 0x00;
				
				/* Update CCCD handle with notification status data*/
				SPINotificationCCDHandle.attrHandle = CYBLE_SPI_READ_SPI_READ_DATA_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE;
				
				SPINotificationCCDHandle.value.val = SPICCDValue;
				
				SPINotificationCCDHandle.value.len = 2;
				
				/* Report data to BLE component for sending data when read by Central device */
				CyBle_GattsWriteAttributeValue(&SPINotificationCCDHandle, 0, &cyBle_connHandle, CYBLE_GATT_DB_LOCALLY_INITIATED);			
	        }
				
	        /* Handling Write data from Client */
	        else if(wrReqParam->handleValPair.attrHandle == CYBLE_SPI_WRITE_SPI_WRITE_DATA_CHAR_HANDLE)
	        {
				while(SPI_SpiIsBusBusy())
				{
					
				}
				SPI_SpiUartClearTxBuffer();
				SPI_SpiUartPutArray(wrReqParam->handleValPair.value.val,wrReqParam->handleValPair.value.len);
					
	        }
	            
	        if (event == CYBLE_EVT_GATTS_WRITE_REQ)
			{
	            CyBle_GattsWriteRsp(cyBle_connHandle);
			}
	        break;
     
		default:
			break;
    }
}


/* [] END OF FILE */
