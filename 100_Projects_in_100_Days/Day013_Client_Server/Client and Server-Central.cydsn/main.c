/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates simultaneous usage of the BLE GATT Client and 
* Server roles. 
*
* Hardware Dependency:
*  CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
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
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*******************************************************************************/

/*******************************************************************************
* Included headers
*******************************************************************************/
#include <project.h>
#include "common.h"
#include <stdio.h>


/***************************************
*        Global Variables
***************************************/
uint8 deviceDetected=0;
CYBLE_GAP_BD_ADDR_T  peerDeviceAddr;


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

void StackEventHandler(uint32 event,void* eventParam)
{
    
   /*Declaring Local variables*/
    uint8 i;
    CYBLE_GAPC_ADV_REPORT_T scanReport;
    CYBLE_API_RESULT_T apiResult;              
   
    switch(event)
    {
        
        case CYBLE_EVT_STACK_ON:
        
                printf("BLE Stack ON:\r\n");
                /*Start to Scan after Stack ON*/
                CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
            break;                   
            
        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
                scanReport=  *(CYBLE_GAPC_ADV_REPORT_T*)eventParam;
                 
                /*Check for the Manufacturer specific data in scan response*/
                if(scanReport.data[1] == MANUFACTURER_SPECIFIC_DATA && scanReport.data[2] == COMPANY_LSB && scanReport.data[3] == COMPANY_MSB 
                && scanReport.data[4] == MAN_SPEC_DATA_LSB && scanReport.data[5] == MAN_SPEC_DATA_MSB)
                {
                    printf("peer address:\r\n");
                    
                    for(i=0;i<CYBLE_GAP_BD_ADDR_SIZE;i++)
                    {
                        peerDeviceAddr.bdAddr[i]=scanReport.peerBdAddr[i];
                        printf("%2.2x",scanReport.peerBdAddr[CYBLE_GAP_BD_ADDR_SIZE-1-i]);
                    }
                    
                    printf("\r\n");
                    deviceDetected=TRUE;
                    
                    /*Start to scan after disconnection*/
                    CyBle_GapcStopScan();
                }
                 
            break;
                
        case CYBLE_EVT_GAPC_SCAN_START_STOP:
                
                if(CyBle_GetState()==CYBLE_STATE_SCANNING)
                {
                    printf("Started to Scan\r\n");
                }
                else if(CyBle_GetState()==CYBLE_STATE_DISCONNECTED)
                {    
                    printf("Stop scanning:\r\n");
                    if(deviceDetected==TRUE)
                    {
                        apiResult=CyBle_GapcConnectDevice(&peerDeviceAddr);
                        if(apiResult==CYBLE_ERROR_OK)
                        {
                            printf("success\r\n");
                        }
                        else 
                        {
                            printf("connection failed:%x\r\n",apiResult);
                        }
                        deviceDetected=FALSE;   
                    }
                }
            break;
        
         case CYBLE_EVT_GAP_DEVICE_CONNECTED:
                /*Start to dicovery the servioes of the serve after connection*/
                CyBle_GattcStartDiscovery(cyBle_connHandle);  
            break;
                
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:           
                /*Start to scan after disconnection*/
                CyBle_GapcStartScan(CYBLE_SCANNING_FAST); 
            break;
                
        case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
                printf("\r\n");
                printf("Discovery complete.\r\n");
                
                for(i = 0u; i < CYBLE_SRVI_COUNT; i++)
                {          
                    /*Checking for the Tx power service*/
                    if(cyBle_serverInfo[i].uuid == CYBLE_UUID_TX_POWER_SERVICE)
                    {
                        if(cyBle_serverInfo[i].range.startHandle < cyBle_serverInfo[i].range.endHandle)
                        {
                            printf("Peer device supports Tx power Service \r\n");      
                        }
                        else
                        {
                            printf("Peer device doesn't supports Tx power Service \r\n");
                        }
                    }

                }                
                printf("\r\n");
            break;                   
                
        default:               
             break;
                
    }
}

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Theory:
* This function initializes the BLE component and then procesthe BLE events routinely
*******************************************************************************/

int main()
{
    /* Local variables*/
    uint8 value;
    char8 command;
    const char8 serialNumber[] = {'1','2','3','4','5','6'};       
    
    CyGlobalIntEnable;  
    
    UART_Start();
    
    CyBle_Start(StackEventHandler);/*Start BLE*/
    
    /* Set Serial Number string not initialized in GUI */
    CyBle_DissSetCharacteristicValue(CYBLE_DIS_SERIAL_NUMBER, sizeof(serialNumber), (uint8 *)serialNumber);
        
    
    /* Register the event handler for TPS specific events */
    CyBle_TpsRegisterAttrCallback(TpsServiceAppEventHandler);
   
    for(;;)
    {
        CyBle_ProcessEvents();
        
        command=UART_UartGetChar();
        if(command!=0u)
        {
            switch(command)
            {
                case 'r':    /*Read Tx power level*/           
                        CyBle_TpscGetCharacteristicValue(cyBle_connHandle,CYBLE_TPS_TX_POWER_LEVEL);              
                    break;      
                        
                case 'e':   /*Enable the notification*/
                        value=1;
                        CyBle_TpscSetCharacteristicDescriptor(cyBle_connHandle,CYBLE_TPS_TX_POWER_LEVEL,CYBLE_TPS_CCCD,
                            CYBLE_TPS_TX_POWER_LEVEL_SIZE*2,&value);            
                    break;
                        
                case 'd':   /*Disable the notification*/
                        value=0;
                        CyBle_TpscSetCharacteristicDescriptor(cyBle_connHandle,CYBLE_TPS_TX_POWER_LEVEL,CYBLE_TPS_CCCD,
                            CYBLE_TPS_TX_POWER_LEVEL_SIZE*2,&value);
                    break;
            }
        }
    }
}

/* [] END OF FILE */
