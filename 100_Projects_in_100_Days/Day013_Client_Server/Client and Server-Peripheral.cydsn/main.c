/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates simultaneous usage of the BLE GATT Client  and 
*  Server roles. 
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
#include  "common.h"
#include <stdio.h>

/***************************************
*        Global Variables
***************************************/
uint8 discoverServer=0;
uint8 disSupport=0;


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

void StackEventHandler(uint32 event,void * eventParam)
{
    /*Local avriables*/
    uint8 i;
    CYBLE_GAP_BD_ADDR_T localAddr;
        
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
        
                printf("BLE Stack ON: \r\n");
                /*Get local device address*/
                CyBle_GetDeviceAddress(&localAddr);

                for(i = CYBLE_GAP_BD_ADDR_SIZE; i > 0u; i--)
                {
                    printf("%2.2x", localAddr.bdAddr[i-1]);
                }
                
                printf("\r\n");
                 
                /*Start to advertise*/
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            
            break;                
       
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
                
            if(CyBle_GetState()==CYBLE_STATE_DISCONNECTED)
            {
                printf("Advertising stopped\r\n");
            }
            else if(CyBle_GetState()==CYBLE_STATE_ADVERTISING)
            {
                printf("Started to Advertise\r\n");
            }
            break;
            
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:             
                printf("Device Connected\r\n)");                                               
            break;
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
                disSupport=0;
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
                
         case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
                printf("\r\n");
                printf("Discovery complete.\r\n");
                printf("Discovered services: \r\n");
                
                for(i = 0u; i < CYBLE_SRVI_COUNT; i++)
                {          /*Check for DIS service*/
                    if(cyBle_serverInfo[i].uuid == CYBLE_UUID_DEVICE_INFO_SERVICE)
                    {
                        if(cyBle_serverInfo[i].range.startHandle < cyBle_serverInfo[i].range.endHandle)
                        {
                            printf("Peer device supports Device Information Service \r\n");
                            disSupport=1;
                        }
                        else
                        {
                            printf("Peer device doesn't supports Device Information Service \r\n");
                    
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
* Function Name: Handleleds
********************************************************************************
*
* Summary:
*  This is used to TURN ON and TURN OFF the LEDs based on the "state".
*
* Parameters:
*  None

* Return:
*  None
*
*******************************************************************************/
void HandleLeds()
{
    /*if Disconnected TURN ON RED LED*/
    if(CyBle_GetState()==CYBLE_STATE_DISCONNECTED)
    {
        Advertising_Led_Write(LED_OFF);
        Disconnect_Led_Write(LED_ON);
        Connect_Led_Write(LED_OFF);
    }
    /*if Advertising TURN ON BLUE LED*/
    else if(CyBle_GetState()==CYBLE_STATE_ADVERTISING)
    {
        Advertising_Led_Write(LED_ON);
        Disconnect_Led_Write(LED_OFF);
        Connect_Led_Write(LED_OFF);
    }
    /*If Connected TURN ON GREEN LED*/
    else if(CyBle_GetState()==CYBLE_STATE_CONNECTED)
    {
        Advertising_Led_Write(LED_OFF);
        Disconnect_Led_Write(LED_OFF);
        Connect_Led_Write(LED_ON);
    }
}



/*******************************************************************************
* Function Name: SW_Interrupt
********************************************************************************
*
* Summary:
*   Handles the mechanical button press.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(DIS_Interrupt)
{
    discoverServer=1;
    SW2_ClearInterrupt();/*Clear the interrupt*/
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
*******************************************************************************/
int main()
{
    /* Local variables */
    char8 command;
    int8 intTxPowerLevel; 
    CYBLE_API_RESULT_T apiResult;
    CYBLE_BLESS_PWR_IN_DB_T txPower;
   
    
    CyGlobalIntEnable; 
    
    isr_DIS_StartEx(DIS_Interrupt);
    
    UART_Start();
    
    CyBle_Start(StackEventHandler); /*Start BLE*/
      
     /* Register the event handler for DIS specific events */
    CyBle_DisRegisterAttrCallback(DisEventHandler);
    
    /* Register the event handler for TPS specific events */
    CyBle_TpsRegisterAttrCallback(TpsServiceAppEventHandler);
    
    
    for(;;)
    {
        CyBle_ProcessEvents();
        HandleLeds();
        
        /*If SW2 is pressed then start to discover the services of server*/
        if(discoverServer==TRUE)
        {
          CyBle_GattcStartDiscovery(cyBle_connHandle);
          discoverServer=FALSE;
        }
        
        command = UART_UartGetChar();
        
        if( command != 0u)
        {/*Client related functions*/
            
            if(disSupport)
            {
                switch(command)
                {
                    case '1':   /*To read the Manufacturer Name */                
                            printf("Manufacturer Name:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_MANUFACTURER_NAME);            
                        break;
                            
                    case '2':   /*To read the Model number*/
                            printf("Model Number:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_MODEL_NUMBER);
                        break;
                        
                    case '3':   /*To read the serial number*/
                            printf("Serial Number:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_SERIAL_NUMBER);
                        break;
                            
                    case '4':  /*To read hardware Revision*/              
                            printf("Hardware Rev:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_HARDWARE_REV);
                        break;
                            
                    case '5':  /*To read Firmware Revision*/
                            printf("Firmware Rev:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_FIRMWARE_REV);
                        break;
                            
                    case '6':  /*To read Software Revision*/
                            printf("Software Rev:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_SOFTWARE_REV);
                        break;
                            
                    case '7':  /*To read System ID*/
                            printf("System ID:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_SYSTEM_ID);
                        break;
                            
                    case '8':  /*To read IEEE 11073 -20601 certifications details*/
                            printf("IEEE 11073-20601:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_REG_CERT_DATA);
                        break; 
                            
                    case '9':  /*To read PNP ID*/
                            printf("PNP ID:");
                            CyBle_DiscGetCharacteristicValue(cyBle_connHandle,CYBLE_DIS_PNP_ID);
                        break;
                            
                    /* Decrease Tx power level of BLE radio if button is pressed */
                    case'd':
                    
                            /* Specify connection channel for reading Tx power level */
                            txPower.bleSsChId = CYBLE_LL_CONN_CH_TYPE;

                            /* Get current Tx power level */
                            CyBle_GetTxPowerLevel(&txPower);

                            /* Decrease the Tx power level by one scale */
                            DecreaseTxPowerLevelValue(&txPower.blePwrLevelInDbm);

                            /* Set the new Tx power level */
                            apiResult = CyBle_SetTxPowerLevel(&txPower);

                            if(CYBLE_ERROR_OK == apiResult)
                            {
                                /* Convert power level to numeric int8 value */
                                intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);
                                (void) CyBle_TpssSetCharacteristicValue(CYBLE_TPS_TX_POWER_LEVEL,
                                 CYBLE_TPS_TX_POWER_LEVEL_SIZE,&intTxPowerLevel);

                                /* Display new Tx Power Level value */
                                printf("Tx power level is set to %d dBm\r\n", intTxPowerLevel);
                            }   
    
                    break;
                        
                    case 'n': /*Send notification to the client about the TX power level*/
                            /* Specify connection channel for reading Tx power level */
                            txPower.bleSsChId = CYBLE_LL_CONN_CH_TYPE;
                            /* Get current Tx power level */
                            CyBle_GetTxPowerLevel(&txPower);
                            intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);
                            apiResult = CyBle_TpssSendNotification(cyBle_connHandle,CYBLE_TPS_TX_POWER_LEVEL,CYBLE_TPS_TX_POWER_LEVEL_SIZE,&intTxPowerLevel);                
                            if(apiResult == CYBLE_ERROR_NTF_DISABLED)
                                printf("Notifications are not yet enabled\n\r");
                    break;
                }
            }
            else
            {
                switch(command)
                {
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        printf("Client hasn't discovered the services of server or server doesn't support DIS service\r\n");
                    break;
                        
                    case'd':
                    
                            /* Specify connection channel for reading Tx power level */
                            txPower.bleSsChId = CYBLE_LL_CONN_CH_TYPE;

                            /* Get current Tx power level */
                            CyBle_GetTxPowerLevel(&txPower);

                            /* Decrease the Tx power level by one scale */
                            DecreaseTxPowerLevelValue(&txPower.blePwrLevelInDbm);

                            /* Set the new Tx power level */
                            apiResult = CyBle_SetTxPowerLevel(&txPower);

                            if(CYBLE_ERROR_OK == apiResult)
                            {
                                /* Convert power level to numeric int8 value */
                                intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);
                                (void) CyBle_TpssSetCharacteristicValue(CYBLE_TPS_TX_POWER_LEVEL,
                                 CYBLE_TPS_TX_POWER_LEVEL_SIZE,&intTxPowerLevel);

                                /* Display new Tx Power Level value */
                                printf("Tx power level is set to %d dBm\r\n", intTxPowerLevel);
                            }   
    
                    break;
                        
                    case 'n': /*Send notification to the client about the TX power level*/
                            /* Specify connection channel for reading Tx power level */
                            txPower.bleSsChId = CYBLE_LL_CONN_CH_TYPE;
                            /* Get current Tx power level */
                            CyBle_GetTxPowerLevel(&txPower);
                            intTxPowerLevel = ConvertTxPowerlevelToInt8(txPower.blePwrLevelInDbm);                            
                            apiResult = CyBle_TpssSendNotification(cyBle_connHandle,CYBLE_TPS_TX_POWER_LEVEL,CYBLE_TPS_TX_POWER_LEVEL_SIZE,&intTxPowerLevel);                
                            if(apiResult == CYBLE_ERROR_NTF_DISABLED)
                                printf("Notifications are not yet enabled\n\r");
                    break;
                }        
            }                
        }                            
    }
}

/* [] END OF FILE */

