/******************************************************************************
* Project Name		: BLE_Central_Observer
* File Name			: BLE_Central_Observer.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 CP1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Owner				: MADY
********************************************************************************/
/******************************************************************************
*In this file the functions that handle the BLE Stack events and other operations
*like Scanning, Connection, Disconnection, Displaying of advertisement packets
*are present. 
******************************************************************************/

#include "BLE_Central_Observer.h"
#include "main.h"

uint8 devIndex;
uint8 Periph_Selected;
 
/* 'connHandle' is a varibale of type 'CYBLE_CONN_HANDLE_T' (defined in 
* BLE_StackGatt.h) and is used to store the connection handle parameters after
* connecting with the peripheral device. */
CYBLE_CONN_HANDLE_T			connHandle;

	/* 'apiResult' is a varibale of type 'CYBLE_API_RESULT_T' (defined in 
* BLE_StackTypes.h) and is used to store the return value from BLE APIs. */
	
CYBLE_API_RESULT_T 		    apiResult;
	
/* 'connectPeriphDevice' is a varibale of type 'CYBLE_GAP_BD_ADDR_T' (defined in 
* BLE_StackGap.h) and is used to store address of the connected device. */
CYBLE_GAP_BD_ADDR_T     connectPeriphDevice[10];

/*******************************************************************************
* Function Name: Get_Adv_Scan_Packets
********************************************************************************
*
* Summary:
* This function gets information from Advertisement and Scan Response packets
*
* Parameters:  
* scanReport - Advertisement report received by the Central
*
* Return: 
*  None
*
*******************************************************************************/

void Get_Adv_Scan_Packets(CYBLE_GAPC_ADV_REPORT_T* scanReport)
{
 uint8 RepIndex; //Index for Bytes in Scan Response Packet
    {
        if (devIndex < 9)
        {
             IsDetected = 1;
            // Receiving Advertisement Packet
            if ((scanReport->eventType == CYBLE_GAPC_CONN_UNDIRECTED_ADV)||
                (scanReport->eventType == CYBLE_GAPC_CONN_DIRECTED_ADV)||
                (scanReport->eventType == 	CYBLE_GAPC_SCAN_UNDIRECTED_ADV)||
                (scanReport->eventType == 	CYBLE_GAPC_NON_CONN_UNDIRECTED_ADV))
            {
                memcpy(connectPeriphDevice[devIndex].bdAddr, scanReport->peerBdAddr,
                sizeof(connectPeriphDevice[devIndex].bdAddr));
                
                printf ("\n\n");  
                printf ("Found Device No: %d\r\n",devIndex);
        		
        	    printf("RSSI: %d \r\n",scanReport->rssi);
                printf("peerBdAddr: %x%x%x%x%x%x \r\n",
                scanReport->peerBdAddr[5u], 
                scanReport->peerBdAddr[4u], 
                scanReport->peerBdAddr[3u], 
                scanReport->peerBdAddr[2u], 
                scanReport->peerBdAddr[1u], 
                scanReport->peerBdAddr[0u]);

                printf("Peer device adveritsing data Length: %d \r\n", scanReport->dataLen);
                devIndex++;
            }
            //receiving Scan response Packet
            else if (scanReport->eventType == CYBLE_GAPC_SCAN_RSP)
            {
              printf ("Scan Response Data:  \r\n");
              if(scanReport->dataLen!=0)
              {
                for(RepIndex = 0u; RepIndex < scanReport->dataLen; RepIndex++)
                {
                  printf("%x", scanReport->data[RepIndex]);
                }
                printf("\r\n");
                printf ("\n");
              }
            }
        }
        else
        {
            printf ("Device Counts Exceeds 10 \r\n");
        }
    }
		
}

/*******************************************************************************
* Function Name: StackEventHandler
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
void StackEventHandler(uint32 event, void *eventParam)
{
	CYBLE_GAPC_ADV_REPORT_T advReport;

	switch(event)
	{
		case CYBLE_EVT_STACK_ON:
            printf("BLE ON:Started to Scan\r\n");
           
	        if(CYBLE_ERROR_OK == CyBle_GapcStartScan(CYBLE_SCANNING_FAST))
	        {
              GREEN_LED_ON(); /*scanning*/
	        }
			break;
		
        case CYBLE_EVT_TIMEOUT:
         if( CYBLE_GAP_SCAN_TO ==*(uint16*) eventParam)
            {
                // Start Scanning again when timeout occurs
                CyBle_GapcStartScan(CYBLE_SCANNING_SLOW);
                GREEN_LED_ON();
             }
			
		case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
                /*Get the information from advertisement and scan response packets*/
             Get_Adv_Scan_Packets((CYBLE_GAPC_ADV_REPORT_T *)eventParam);
             memcpy(&advReport, eventParam, sizeof(advReport)); 

		break;
			
 		case CYBLE_EVT_GATT_CONNECT_IND:
			/* When the peripheral device is connected, store the connection handle.*/
            IsConnected = 1;
            connHandle = *(CYBLE_CONN_HANDLE_T *)eventParam;
			break;
		
		case CYBLE_EVT_GAP_DEVICE_CONNECTED:
             devIndex = 0;
	         printf("Peripheral connected. Press 'D' for disconnection \r\n");
             BLUE_LED_ON();
             if(CYBLE_ERROR_OK != CyBle_GapcStartScan(CYBLE_SCANNING_FAST))
		      {
               printf ("Start Scanning Failed \r\n");
		      }
              else
              {
                printf ("Scanning\r\n");
              }
        break;
			
		case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            RED_LED_ON();
            IsConnected = 0;
            printf("Peripheral Disconnected \r\n");
            if (!(IsSelected))
            {
              if(CYBLE_ERROR_OK != CyBle_GapcStartScan(CYBLE_SCANNING_FAST))
    	       {
                 printf ("Start Scanning Failed");
               }
               else
               {
                 GREEN_LED_ON();
                 printf ("Scanning\r\n");
               }
            }
           break;

		default:
			break;
	}
}
/*******************************************************************************
* Function Name: Handle_ble_Central_Observer_State
********************************************************************************
* Summary:
* This function is used to handle connectio requests
*
* Parameters:
* None
*
* Return:
* void
*
*******************************************************************************/

void Handle_ble_Central_Observer_State(void)
{
    if (CYBLE_STACK_STATE_BUSY == CyBle_GattGetBusStatus())
    {
        /*If stack is busy wait for ready signal*/
        return;
    }
	
	if (!IsConnected)    
    {
      if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
	   {
         if(IsSelected)
		  {
			apiResult = CyBle_GapcConnectDevice(&connectPeriphDevice[Periph_Selected]);
			if(CYBLE_ERROR_OK != apiResult )
			{
			   printf ("Connection Request to peripheral failed \r\n");
			}
            else
            {
               printf ("Connection Request Sent to Peripheral \r\n");
            }
            IsSelected = 0;
		  }
		}
	}
	
}

/* [] END OF FILE */
