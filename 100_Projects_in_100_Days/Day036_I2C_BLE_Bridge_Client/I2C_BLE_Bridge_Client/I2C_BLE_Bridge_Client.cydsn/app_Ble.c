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

#include "main.h"

uint16 I2CReadDataCharHandle    = 0;                /* Handle for the I2CRead characteristic */

uint16 I2CWriteDataCharHandle	= 0;                /* Handle for the I2CWrite characteristic */

uint16 I2CReadDataDescHandle    = 0;                /* Handle for the I2CRead_Data characteristic descriptor */

uint16 I2CReadServiceHandle     = 0;                /* Handle for the I2CRead service */

uint16 I2CReadServiceEndHandle  = 0;                /* End handle for the I2CRead service */

uint16 I2CWriteServiceHandle    = 0;                /* Handle for the I2CWrite service */

uint16 I2CWriteServiceEndHandle = 0;                /* End handle for the I2CWrite service */

uint16 mtuSize                  = CYBLE_GATT_MTU;   /* MTU size to be used by Client and Server after MTU exchange */

uint8 connectionInit = 1;     /* flag to check if its the first iteration after connection */

const uint8 enableNotificationParam[2] = {0x01, 0x00};

volatile static bool peerDeviceFound         = false;

volatile static bool notificationEnabled     = false;

static INFO_EXCHANGE_STATE_T    infoExchangeState   = INFO_EXCHANGE_START;

CYBLE_GATT_ATTR_HANDLE_RANGE_T  attrHandleRange;

CYBLE_GATTC_FIND_INFO_REQ_T     charDescHandleRange;

/* structure to be passed for the enable notification request */
CYBLE_GATTC_WRITE_REQ_T     enableNotificationReqParam   = {
                                                                {(uint8*)enableNotificationParam, 2, 2},
                                                                0
                                                            };

/* I2C Read service UUID*/
const uint8 I2CReadUUID[16] = 	{ 
									0x00u, 0x00u, 0xFBu, 0x34u, 0x9Bu, 0x5Fu, 0x80u, 0x00u, \
									0x00u, 0x80u, 0x00u, 0x10u, 0x01u, 0x00u, 0x0Au, 0x00u  \
								};
/* I2C_Read_data charcteristic UUID */
const uint8 I2CReadDataUUID[16]={ 
									0x00u, 0xFBu, 0x34u, 0x9Bu, 0x5Fu, 0x80u, 0x00u, 0x00u, \
									0x80u, 0x00u, 0x10u, 0x00u, 0x02u, 0x00u, 0x0Au, 0x00u 	\
								};
/* I2C_Write service UUID */
const uint8 I2CWriteUUID[16] = 	{ 
									0x00u, 0xFBu, 0x34u, 0x9Bu, 0x5Fu, 0x80u, 0x00u, 0x00u,	\
									0x80u, 0x00u, 0x10u, 0x00u, 0x03u, 0x00u, 0x0Au, 0x00u 	\
								};
/* I2C_Write_data charcteristic UUID */
const uint8 I2CWriteDataUUID[16] ={ 
									0x00u, 0xFBu, 0x34u, 0x9Bu, 0x5Fu, 0x80u, 0x00u, 0x00u, \
									0x80u, 0x00u, 0x10u, 0x00u, 0x04u, 0x00u, 0x0Au, 0x00u 	\
								};

/* structure to be passed for discovering service by UUID */
const CYBLE_GATT_VALUE_T    I2CReadServiceUuidInfo = { 
                                                        (uint8 *) I2CReadUUID, \
                                                        CYBLE_GATT_128_BIT_UUID_SIZE,\
                                                        CYBLE_GATT_128_BIT_UUID_SIZE \
                                                      };

/* structure to be passed for discovering service by UUID */
const CYBLE_GATT_VALUE_T    I2CWriteServiceUuidInfo = { 
                                                        (uint8 *) I2CWriteUUID, \
                                                        CYBLE_GATT_128_BIT_UUID_SIZE,\
                                                        CYBLE_GATT_128_BIT_UUID_SIZE \
                                                      };

static CYBLE_GAP_BD_ADDR_T         peer_addr;

/*******************************************************************************
* Function Name: HandleBleProcessing
********************************************************************************
* Summary:
*        Function which updates the LED status , scans and connect to peripheral
*        based on the BLE component state machine
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void HandleBleProcessing(void)
{            
    switch (cyBle_state)
    {
        case CYBLE_STATE_SCANNING:
        {
#ifdef 	LED_INDICATION	
	
			SCAN_LED_ON();
			
#endif /* LED_INDICATION */	

            if(peerDeviceFound)
            {
                CyBle_GapcStopScan();
				
#ifdef 	LED_INDICATION		
	
                ALL_LED_OFF();
				
#endif /* LED_INDICATION */

            }
            break;
        }
        
        case CYBLE_STATE_CONNECTED:
        { 
			if(connectionInit)
			{
				connectionInit = 0;
				
#ifdef 	LED_INDICATION	
	
			CONNECT_LED_ON();
			
#endif /* LED_INDICATION */	     

#ifdef	 ENABLE_I2C_ONLY_WHEN_CONNECTED	
				/* Start I2C Slave operation */
				I2C_Start();
				
				/* Initialize I2C write buffer */
				I2C_I2CSlaveInitWriteBuf((uint8 *) wrBuf, I2C_WRITE_BUFFER_SIZE);
				
				/* Initialize I2C read buffer */
				I2C_I2CSlaveInitReadBuf((uint8 *) rdBuf, I2C_READ_BUFFER_SIZE);
#endif	
			}
            /* if Client does not has all the information about attribute handles 
             * call procedure for getting it */
            if((I2C_WRITE_DATA_ATTR_HANDLE_FOUND != infoExchangeState))
            {				
                attrHandleInit();
            }
            
            /* enable notifications if not enabled already */
            else if(false == notificationEnabled)
            {
                enableNotifications();
            }
#ifdef 	ENABLE_I2C_ONLY_WHEN_CONNECTED            
            /* if client has all required info and stack is free, handle I2C traffic */
            else if(CyBle_GattGetBusStatus() != CYBLE_STACK_STATE_BUSY)
            {
                HandleI2CTraffic();
            }
#endif            

            break;
        }
                
        case CYBLE_STATE_DISCONNECTED:
        {
            if(peerDeviceFound)
            {
                apiResult = CyBle_GapcConnectDevice(&peer_addr);
                
			    if(CYBLE_ERROR_OK == apiResult)
			    {
				    peerDeviceFound = false;
			    }
            }
            else
            {
#ifdef 	LED_INDICATION	
	
               	DISCON_LED_ON();
				
				CyDelay(3000);
				
#endif /* LED_INDICATION */		

			    CyBle_GapcStartScan(CYBLE_SCANNING_FAST); 
            }
            break;
        }
        
		case CYBLE_STATE_CONNECTING:
        case CYBLE_STATE_INITIALIZING:
        case CYBLE_STATE_STOPPED:
        default:
            break;       
    }
}

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
    CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T    *readResponse;
    
	CYBLE_GAPC_ADV_REPORT_T		            *advReport;
   
	CYBLE_GATTC_FIND_BY_TYPE_RSP_PARAM_T    *findResponse;
    
	CYBLE_GATTC_FIND_INFO_RSP_PARAM_T       *findInfoResponse;	
    
    switch (event)
    {
        case CYBLE_EVT_STACK_ON:
			
			CyBle_GapcStartScan(CYBLE_SCANNING_FAST); 
            
			break;
        
        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
            
			advReport = (CYBLE_GAPC_ADV_REPORT_T *) eventParam;
            
            if((advReport->eventType == CYBLE_GAPC_SCAN_RSP) && (advReport->data[1] == 0xff)
                    && (advReport->data[2] == 0x31)  && (advReport->data[3] == 0x01)
                    && (advReport->data[4] == 'I') && (advReport->data[5] == '2')
					&& (advReport->data[6] == 'C') )
            {
                peerDeviceFound = true;
                
                memcpy(peer_addr.bdAddr, advReport->peerBdAddr, sizeof(peer_addr.bdAddr));
                
				peer_addr.type = advReport->peerAddrType;
            }
		    break;    
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            /* RESET all flags */
            peerDeviceFound         = false;
            
			notificationEnabled     = false;
            
			infoExchangeState       = INFO_EXCHANGE_START;
			
			connectionInit = 1;
			
#ifdef 	ENABLE_I2C_ONLY_WHEN_CONNECTED
			I2C_Stop();
#endif	
            break;
        
        case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
            
            readResponse = (CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T *) eventParam;
            
            if(0 == memcmp((uint8 *)&(readResponse->attrData.attrValue[5]), (uint8 *)I2CReadDataUUID, 16))
            {
                I2CReadDataCharHandle = readResponse->attrData.attrValue[3];
                
				I2CReadDataCharHandle |= (readResponse->attrData.attrValue[4] << 8);
                
                infoExchangeState = I2C_READ_DATA_ATTR_HANDLE_FOUND;
            }
            else if(0 == memcmp((uint8 *)&(readResponse->attrData.attrValue[5]), (uint8 *)I2CWriteDataUUID, 16))
            {
                I2CWriteDataCharHandle = readResponse->attrData.attrValue[3];
                
				I2CWriteDataCharHandle |= (readResponse->attrData.attrValue[4] << 8);
                
                infoExchangeState = I2C_WRITE_DATA_ATTR_HANDLE_FOUND;
               
            }
            break;
			
		case CYBLE_EVT_GATTC_FIND_INFO_RSP:
            
            findInfoResponse = (CYBLE_GATTC_FIND_INFO_RSP_PARAM_T *) eventParam;
            
            if((0x29 == findInfoResponse->handleValueList.list[3]) && \
                                (0x02 == findInfoResponse->handleValueList.list[2]))
            {
                I2CReadDataDescHandle = findInfoResponse->handleValueList.list[0];
                
				I2CReadDataDescHandle |= findInfoResponse->handleValueList.list[1] << 8;
            
                infoExchangeState = I2C_READ_DATA_CCCD_HANDLE_FOUND;
            }
           
            break;	
            
        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
           
			HandleI2CNotifications((CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *)eventParam);
			
			break;
			
		case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
            
            findResponse            = (CYBLE_GATTC_FIND_BY_TYPE_RSP_PARAM_T *) eventParam;
            
			if(infoExchangeState == INFO_EXCHANGE_START)
			{
	            I2CReadServiceHandle    = findResponse->range->startHandle;
	            
				I2CReadServiceEndHandle = findResponse->range->endHandle;
	            
	            infoExchangeState = I2C_READ_SERVICE_HANDLE_FOUND;
			}
            
			else if(infoExchangeState == I2C_READ_DATA_CCCD_HANDLE_FOUND)
			{
	            I2CWriteServiceHandle    = findResponse->range->startHandle;
	            
				I2CWriteServiceEndHandle = findResponse->range->endHandle;
	            
	            infoExchangeState = I2C_WRITE_SERVICE_HANDLE_FOUND;
			}
            break;	
		
		case CYBLE_EVT_GATTC_WRITE_RSP:
            
            notificationEnabled = true;
            
            break;	

        default:
            break;
    }
}


/*******************************************************************************
* Function Name: attrHandleInit
********************************************************************************
*
* Summary:
*  This function gathhers all the information like attribute handles and MTU size
*  from the server.
*
* Parameters:
*  None.
*
* Return:
*   None.
*
*******************************************************************************/
void attrHandleInit()
{
    switch(infoExchangeState)
    {
        case INFO_EXCHANGE_START:        
            
			CyBle_GattcDiscoverPrimaryServiceByUuid(cyBle_connHandle, I2CReadServiceUuidInfo);
            
			break;
        
        case I2C_READ_SERVICE_HANDLE_FOUND:
            
			attrHandleRange.startHandle    = I2CReadServiceHandle;
            
			attrHandleRange.endHandle      = I2CReadServiceEndHandle;
            
			CyBle_GattcDiscoverAllCharacteristics(cyBle_connHandle, attrHandleRange);
            
			break;
        
        case I2C_READ_DATA_ATTR_HANDLE_FOUND:
            
			charDescHandleRange.startHandle = I2CReadDataCharHandle + 1;
            
			charDescHandleRange.endHandle   = I2CReadServiceEndHandle;

            CyBle_GattcDiscoverAllCharacteristicDescriptors(cyBle_connHandle, &charDescHandleRange);
            
			break;
			
        case I2C_READ_DATA_CCCD_HANDLE_FOUND:
			
			CyBle_GattcDiscoverPrimaryServiceByUuid(cyBle_connHandle, I2CWriteServiceUuidInfo);
			
			break;
			
		case I2C_WRITE_SERVICE_HANDLE_FOUND:
            
			attrHandleRange.startHandle    = I2CWriteServiceHandle;
            
			attrHandleRange.endHandle      = I2CWriteServiceEndHandle;

            CyBle_GattcDiscoverAllCharacteristics(cyBle_connHandle, attrHandleRange);
            
			break; 
            
        default:
            break;    
    }
    
    CyBle_ProcessEvents();
}

/*******************************************************************************
* Function Name: enableNotifications
********************************************************************************
*
* Summary:
*  This function enables notfications from servers. 
*
* Parameters:
*  None.
*
* Return:
*   None.
*
*******************************************************************************/
void enableNotifications()
{     
    enableNotificationReqParam.attrHandle = I2CReadDataDescHandle;   
    
	CyBle_GattcWriteCharacteristicDescriptors(cyBle_connHandle, (CYBLE_GATTC_WRITE_REQ_T *)(&enableNotificationReqParam));
}

/* [] END OF FILE */

