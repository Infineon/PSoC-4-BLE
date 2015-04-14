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
#include <main.h>

CYBLE_CONN_HANDLE_T connHandle;
CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;

void UpdateCharacteristics(void);

/*******************************************************************************
* Function Name: StackEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the CYBLE Component.
*
* Parameters:
*  uint8 event:       Event from the CYBLE component.
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of event structure is described in the component
*                     datasheet.
*
* Return:
*  None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void *eventParam)
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    switch(event)
    {
    /**********************************************************
    *                       General Events
    ***********************************************************/
        case CYBLE_EVT_STACK_ON: /* Generated when BLE component is started */
            printf ("CySmartDisplay Server ON\r\n");
            
            /* Starting advertisement */
            if(!(CYBLE_ERROR_OK == CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST)))
            {
                printf ("Starting Advertisement Failed; Status = 0x%x \r\n", apiResult);
            }
            else
            {
                printf ("Starting to advertise\r\n");
            }
            break;

            
        case CYBLE_EVT_TIMEOUT: /* Generated when the connection timesout */
            printf("Connection Time Out\r\n");
            break;    

            
    /**********************************************************
    *                       GAP Events
    ***********************************************************/
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP: // Generated when advertisement is Started or Stopped
            printf("\r\n");
            if (CyBle_GetState() != CYBLE_STATE_ADVERTISING)
                {
                    /*SLOW Advertising Timeout. Stack disables Advertising.*/
                    RED_LED_ON();
                    printf("Advertisement is disabled\r\n");
                    
                    /*Continue SLOW Advertising by enabing it in SLOW mode*/
                    apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_SLOW);
                    if (apiResult != CYBLE_ERROR_OK)
                    {
                        printf ("Restarting Slow Advertisement Failed; Status = 0x%x",apiResult);
                    }
                    else
                    {
                        printf ("Restarting slow advertisement after total timeout\r\n");
                    }
                }
                else
                {
                    GREEN_LED_ON();
                    printf("Advertisement is enabled \r\n");
                    /* Device is now in Advertising state */
                }
            break;

                
        case CYBLE_EVT_GAP_DEVICE_CONNECTED: // Generated when connection is established
            BLUE_LED_ON();
            printf("Device Connected\r\n");
            
            CyBle_GattcExchangeMtuReq(cyBle_connHandle, CYBLE_GATT_MTU);
            
            /*Copy the default settings to the GATT Attribute*/
            UpdateCharacteristics();
            break;

                   
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED: // Generated when connection is disconnected
            RED_LED_ON ();
            printf("\r\n");
            printf("Device Disconnected\r\n");
            connHandle.bdHandle = 0;
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            if (apiResult != CYBLE_ERROR_OK)
            {
                printf ("Restarting Advertisement Failed; Status = 0x%x",apiResult);
            }
            else
            {
                printf ("Restarting advertisement after device disconnect\r\n");
            }
            break;
            
    /**********************************************************
    *                       GATT Events
    ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND: // Generated at GATT connection
            connHandle = *(CYBLE_CONN_HANDLE_T *)eventParam;
            deviceConnected = TRUE;
            break;

        
        case CYBLE_EVT_GATT_DISCONNECT_IND: // Generated at GATT disconnection
            deviceConnected = FALSE;
            break;
            
            
        case CYBLE_EVT_GATTS_WRITE_REQ: // Generated when the connected Central device sends a write request
			printf("Command from Client\r\n");
            /* Extract the Write data sent by Client */
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
			
			/*If the attribute handle of the characteristic written to is equal to that of 
			* the characteristic, then extract the data */
			if(CYBLE_MESSAGE_MESSAGE_CONTROL_CHAR_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
				/*Start displaying the new message*/
                DisplayMessage((char *)wrReqParam->handleValPair.value.val, (uint8)wrReqParam->handleValPair.value.len);
            }
            
            if(CYBLE_MESSAGE_BRIGHTNESS_CONTROL_CHAR_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
				/*Change the brightness*/
                DisplayBrightness(wrReqParam->handleValPair.value.val[0]);
            }
            
            if(CYBLE_MESSAGE_SPEED_CONTROL_CHAR_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
				/*Change the speed*/
                DispaySpeed(wrReqParam->handleValPair.value.val[0]);
            }
            
            UpdateCharacteristics();
			
			/* Send the response to the write request received. */
			CyBle_GattsWriteRsp(connHandle);
			
			break;
            
            
    /**********************************************************
    *                       Other Events
    ***********************************************************/
        default:
            break;
    }
}

void UpdateCharacteristics(void)
{
    /* 'message' stores MESSAGE data parameters */
    CYBLE_GATT_HANDLE_VALUE_PAIR_T		message;
    
    message.attrHandle = CYBLE_MESSAGE_MESSAGE_CONTROL_CHAR_HANDLE;
    message.value.val = (uint8 *)buffer;
    message.value.len = strlen(buffer);
    message.value.actualLen = strlen(buffer);
    
    CyBle_GattsWriteAttributeValue(&message,FALSE,&connHandle,CYBLE_GATT_DB_LOCALLY_INITIATED);
}

/* [] END OF FILE */
