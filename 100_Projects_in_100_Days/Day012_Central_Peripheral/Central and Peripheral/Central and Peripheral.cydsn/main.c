/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates simultaneous usage of the BLE GAP Peripheral and 
*  Broadcaster roles. The device would connect to a peer device, while also 
*  broadcasting (non-connectable advertising) at the same time.
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
#include <stdio.h>
#include <common.h>


/***************************************
*        Global Variables
***************************************/
uint8 DeviceN=0; 
uint8 Stop=0;
uint8 ConnectCommand=0;
uint8 NotificationsEnabled=0;
uint8 SendDevicesNum=0;
uint8 DevicesNearBy=0;
uint8 BlinkDelayCount;
uint8 flag=0;
uint8 ConnectedAs;
uint8 state=DISCONNECTED;
CYBLE_CONN_HANDLE_T  connHandle;
CYBLE_GAP_BD_ADDR_T  peerAddr[CYBLE_MAX_ADV_DEVICES];


/***************************************
*        Function Prototypes
***************************************/
void StackEventHandler(uint32 event,void* eventParam);
void ExecuteClientCommand(uint8 );

/***************************************
*Connection Interval parameters
*Min and Max values will be multiplied by 1.25
*Timeout value will be multiplied by 10
***************************************/
#define CONN_PARAM_UPDATE_MIN_CONN_INTERVAL 800		      
#define CONN_PARAM_UPDATE_MAX_CONN_INTERVAL 800	       
#define CONN_PARAM_UPDATE_SLAVE_LATENCY 0	    
#define CONN_PARAM_UPDATE_SUPRV_TIMEOUT 2000


static CYBLE_GAP_CONN_UPDATE_PARAM_T ConnectionParam =
{
    CONN_PARAM_UPDATE_MIN_CONN_INTERVAL,  		      
    CONN_PARAM_UPDATE_MAX_CONN_INTERVAL,		       
    CONN_PARAM_UPDATE_SLAVE_LATENCY,			    
    CONN_PARAM_UPDATE_SUPRV_TIMEOUT 			         	
};

/*******************************************************************************
* Function Name: ExecuteClientCommand
********************************************************************************
*
* Summary:
*  This is used to start scanning based on the value written to the characteristic by client
*
* Parameters:
* uint8 ClientCommand:     value written  to characteristic by client
*  
*
* Return:
*  None
*
*******************************************************************************/

void ExecuteClientCommand(uint8 ClientCommand)
{    
   
    switch(ClientCommand)
    {
     
        case START_SCANNING:
                /*Start scanning*/
                CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
                printf("Received command from Client:START SCANNING\r\n");
                printf("\r\n");
            break;
                
        default:
            break;
                
    }
}

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
    
    /*Declaring local variables*/
    uint8 i,ClientCommand;
    uint32 value;
    CYBLE_GAPC_ADV_REPORT_T advReport;
    CYBLE_API_RESULT_T apiResult;
    CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;
    
    
    switch(event)
    {
        /*STACK ON*/
        case CYBLE_EVT_STACK_ON:
     
                 printf("BLE STACK ON:\r\n");
                 printf("\r\n");
                
            break;
                
            /* SCAN progress result*/
        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
            
                /*copy the advertising packet recieved to advReport*/
                advReport=*(CYBLE_GAPC_ADV_REPORT_T *)eventParam;
               
                printf("CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:\r\n");
               
               /*==EVENT TYPE===== */
                printf("eventType:");
                
                if( advReport.eventType==CYBLE_GAPC_CONN_UNDIRECTED_ADV)
                {
                    printf("Connectable undirected advertising\r\n");
                }
                else if(advReport.eventType==CYBLE_GAPC_CONN_DIRECTED_ADV)
                {
                    printf("Connectable directed advertising\r\n");
                }
                else if(advReport.eventType==CYBLE_GAPC_SCAN_UNDIRECTED_ADV)
                {
                    printf("Scannable undirected advertising\r\n");
                }
                else if(advReport.eventType==CYBLE_GAPC_NON_CONN_UNDIRECTED_ADV)
                {
                    printf("Non connectable undirected advertising\r\n");
                }
                else if(advReport.eventType==CYBLE_GAPC_SCAN_RSP)
                {
                    printf("SCAN_RSP\r\n");
                }
                
                printf("\r\n");
                
                /*==PEER address type===*/
                printf("    peerAddrType: ");
                
                  if(advReport.peerAddrType==CYBLE_GAP_ADDR_TYPE_PUBLIC)
                {
                    printf("PUBLIC\r\n");
                }
                else if(advReport.peerAddrType==CYBLE_GAP_ADDR_TYPE_RANDOM)
                {
                    printf("RANDOM \r\n");
                }
               
                
                printf("    peerBdAddr: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",
                    advReport.peerBdAddr[5u], advReport.peerBdAddr[4u],
                    advReport.peerBdAddr[3u], advReport.peerBdAddr[2u],
                    advReport.peerBdAddr[1u], advReport.peerBdAddr[0u]);
             
                
                /* Copy the address of the devices to peerAddr[] array if the 
                 * received advertising packet is not a duplicate one*/
                
                if((DevicesNearBy <= CYBLE_MAX_ADV_DEVICES) && (advReport.eventType!=CYBLE_GAPC_SCAN_RSP))
                {
                    for(i = 0u; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
                    {
                        peerAddr[DevicesNearBy].bdAddr[i] = advReport.peerBdAddr[i];
                    }                    
                    
                    /* Increase the number of peer devices when 
                     * received aadvertising packet from different peer devices.*/
                    ++DevicesNearBy;
                    
                }
                printf("\r\n"); 
            
            break;
           
            /*Adveritising started or stopped*/
        case  CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            
                if(state==ADVERTISING)
                {
                    state=DISCONNECTED;/*Update "state variable"*/
                    printf("Advertising Stopped\r\n");
                    if(Stop)
                    {
                        CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
                        Stop=0;
                    }
                } 
                else if(state==DISCONNECTED)
                {
                    state=ADVERTISING;   /*Update "state" variable*/
                    printf("Started to Advertise\r\n");
                }
                printf("\r\n");
            
            break;
            
            /*scanning started or stopped*/
         case  CYBLE_EVT_GAPC_SCAN_START_STOP:
                 
                if(state==SCANNING)
                {
                    state=DISCONNECTED;/*Update "state" variable*/
                    printf("Stopped Scanning\r\n");
                
                    if(Stop)
                    {   /*Start Advertising*/
                        CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
                        Stop=0;
                    }
                }
                else if(state==DISCONNECTED)
                {
                    DevicesNearBy=0;
                    state=SCANNING; /*Update "state" variable*/
                    printf("Started to Scan\r\n");
                }
                else if( state==CONNECTED_SCANNING)
                {
                    state=CONNECTED;         /*Scanning stopped while maintaining connection.Update "state" variable*/
                    printf("Stopped scanning\r\n");
                    SendDevicesNum=1;
                }
                else if(state==CONNECTED)
                {
                    DevicesNearBy=0;
                    state=CONNECTED_SCANNING;/* Scanning started while maintaining connection.
                                              * Update "state" variable*/
                    printf("Started To scan\r\n");                    
                }
            
                
                /* If scanning is stopped by user to connect to peer device then 
                 * connect to the peer device*/
                 if(1u == ConnectCommand)            
                {
                    
                    /* Connect to selected device */
                    apiResult = CyBle_GapcConnectDevice(&peerAddr[DeviceN]);
                    
                    if(apiResult != CYBLE_ERROR_OK)
                    {
                        printf("ConnectDevice API Error: %x \r\n", apiResult);
                    }
                    else
                    {    /*Display the address of the device to which you are trying to connect*/
                        printf("Connected to device with address - %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",
                                peerAddr[0u].bdAddr[5u], peerAddr[0u].bdAddr[4u], 
                                peerAddr[0u].bdAddr[3u], peerAddr[0u].bdAddr[2u],
                                peerAddr[0u].bdAddr[1u], peerAddr[0u].bdAddr[0u]);
                        flag=1;
                       
                    }
                    ConnectCommand = 0u;
                }
    
            
            break;
            
        case CYBLE_EVT_GATT_CONNECT_IND:
                
                /*copy the value of connection handle to "connHanlde"  */
                connHandle = *(CYBLE_CONN_HANDLE_T *)eventParam;
        break;
            
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:/*Device connected*/
           
                state=CONNECTED;/*Update "state" variable*/
                printf("Device connected\r\n");
                
                /* If local device sends the connection request 
                 * then set the variable "ConnnectsAs=MASTER"*/
                if (flag)
                {
                    ConnectedAs=MASTER;
                    printf("Connected as Master\r\n");
                }
                else 
                {
                    /*If peer device sends connection request */
                    ConnectedAs=SLAVE;
                    printf("Connected as Slave\r\n");
                }
                printf("\r\n");
                
            break;
         
            /* Response for connection request from master of link  */
        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
            
                value=*(uint32 *) eventParam;
                
                if(value==CYBLE_L2CAP_CONN_PARAM_ACCEPTED)
                {
                    printf("new connection parameters are accepted by master\r\n");
                }
                else if(value==CYBLE_L2CAP_CONN_PARAM_REJECTED)
                {
                    printf("new connection parameters are rejected by master\r\n");
                }
            break;
                
            /*Request to update connection parameters from slave    */
         case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
                
                CyBle_L2capLeConnectionParamUpdateResponse(connHandle.bdHandle,CYBLE_L2CAP_CONN_PARAM_ACCEPTED); 
            
            break;
            
         case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:/*Device disconnected*/
                printf("Device disconnected\r\n");
                state=DISCONNECTED; /*Update "state" variable*/
                /*set variables to zero*/
                NotificationsEnabled=0;
                connHandle.bdHandle = 0u;
                ConnectedAs=DISCONNECTED;
                flag=0;
                       
            break;
           
            /*Write Request  from Client*/
        case CYBLE_EVT_GATTS_WRITE_REQ:
                    
                wrReqParam=(CYBLE_GATTS_WRITE_REQ_PARAM_T*) eventParam;
                                      
                /*write request is for custom characteristic*/
                
                if(wrReqParam->handleValPair.attrHandle == CYBLE_CUSTOM_SERVICE_CUSTOM_CHARACTERISTIC_CHAR_HANDLE) 
                {
                    ClientCommand= wrReqParam->handleValPair.value.val[0];
                    ExecuteClientCommand(ClientCommand);
                }
                
                /*Write request is for CCCD*/
                else if(wrReqParam->handleValPair.attrHandle==CYBLE_CUSTOM_SERVICE_CUSTOM_CHARACTERISTIC_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE)
                {
                    if (wrReqParam->handleValPair.value.val[0]==1)
                    {
                        printf("Notifications enabled\r\n");
                        NotificationsEnabled=1;
                    }
                    else if(wrReqParam->handleValPair.value.val[0]==0)
                    {
                        NotificationsEnabled=0;
                        printf("Notifications Disabled\r\n"); 
                    }
                }
                
                CyBle_GattsWriteRsp(wrReqParam->connHandle);/* response to write characterisitc from client*/
               
            break;
                
        case CYBLE_EVT_GATTC_WRITE_RSP:     /*Write Response sent for write command*/
                
                printf("write response is sent");
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
void Handleleds()
{
    if(state==DISCONNECTED)      /*If disconnected then TURN OFF all LEDs*/
    {  
        Connection_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        Scanning_LED_Write(LED_OFF);  
    }
    else if(state==CONNECTED)  /*If connected then TURN ON BLUE LED*/
    {
        Connection_LED_Write(LED_ON);
        Advertising_LED_Write(LED_OFF);
        Scanning_LED_Write(LED_OFF);
    }
    else if(state==SCANNING)    /*If scanning TURN ON RED LED*/
    {
        Connection_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);
        Scanning_LED_Write(LED_ON);
    }
    else if (state==ADVERTISING)  /*If advertising TURN ON GREEN LED*/
    { 
        Connection_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_ON);
        Scanning_LED_Write(LED_OFF);
    }
    else if(state==CONNECTED_SCANNING)   /*If scanning while maitaining connection then TOGGLE BLUE and RED LEDs*/
    {
        if((BLINK_DELAY/2)>BlinkDelayCount)
        {
            CyDelay(5);           
            Scanning_LED_Write(LED_ON);
            Connection_LED_Write(LED_OFF);
        }
        else if((BLINK_DELAY/2)<BlinkDelayCount)
        {
             CyDelay(5);
            
            Scanning_LED_Write(LED_OFF);
            Connection_LED_Write(LED_ON);
            
                if(BlinkDelayCount==BLINK_DELAY)
                {
                    BlinkDelayCount=0;
                }
        }
         BlinkDelayCount++;   
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
*******************************************************************************/
int main()
{
    
    /*Declaring variables*/
    char8 command;
    uint8 j;
    CYBLE_API_RESULT_T apiResult;
    CYBLE_GATTS_HANDLE_VALUE_NTF_T CentralPeripheral;
    BlinkDelayCount=0;
    
    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
    
    /*TURN OFF the LEDS*/
    Advertising_LED_Write(LED_OFF);
    Scanning_LED_Write(LED_OFF);
    Connection_LED_Write(LED_OFF);
    
    /*Start UART */
    UART_Start();
    
    
    CyBle_Start(StackEventHandler); /*Start BLE*/
   
    
    while(1)
    {
        /* CyBle_ProcessEvents() allows BLE stack to process pending events */
        CyBle_ProcessEvents();
        Handleleds();
        
        command = UART_UartGetChar();
		if(command != 0u)
        {
            switch(command)
            {
            
            case 'c':                 /* Initiate connection. The connection will be requested after 
                                       * CYBLE_EVT_GAPC_SCAN_START_STOP_ENDED is received*/
                                     
                
                    if(DevicesNearBy)     /*send the connection request if there is atleast one peer device*/
                    {
                        ConnectCommand = 1u;
                    }
                    
                    if(state==SCANNING)
                    {
                    
                        CyBle_GapcStopScan();                           /* Stop scanning if it is scanning 
                                                                         * before sending connection request */
                    
                        printf("Stop Scanning ... \r\n");
                    }
                    else if(CyBle_GetState() == CYBLE_STATE_CONNECTING)
                    {
                        printf("Trying to connect to the previous device.\r\n");
                    }
                    else if(state==DISCONNECTED && DevicesNearBy)
                    {
                        flag=1;
                        CyBle_GapcConnectDevice(&peerAddr[DeviceN]);
                    }
                    else 
                    {
                        printf("NO near by devices to connect\r\n");
                    }
                               
                break;
            
             case 'd':                   /* Disconnect */
             
                    apiResult = CyBle_GapDisconnect(connHandle.bdHandle); 
                    
                    if(apiResult != CYBLE_ERROR_OK)
                    {
                         printf("Disconnect Device API Error: %x \r\n", apiResult);
                    }
                    else
                    {
                         printf("Disconnect Device API Success \r\n");
                    }
                
                 break;
                    
            case 's':                   /* Start Scanning */
                                                            
                    /*if Device is in Disconnect State then start scanning*/
                    if(state==DISCONNECTED)
                    {
                        apiResult = CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
            			
                        if(apiResult != CYBLE_ERROR_OK)
                        {
                             printf("CyBle_GapcStartScan API Error: %xd \r\n", apiResult);
                            
                        }
              			else
                        {
            			 	printf("CyBle_GapcStartScan API Success \r\n");
                        }
                    }
                    else if(state==ADVERTISING)
                    { /*Stop advertising before starting to scan*/
                        Stop=1;
                        CyBle_GappStopAdvertisement();
                    }
                
    			
                break;
                    
            case 'z':                   /* Select peer device  */
                    
                    if(DevicesNearBy)      /*check if there is any peer device before selecting the device*/
                    {
                           
                         for(j = 0u; j < DevicesNearBy ; j++)
                         {
                                printf("Device %d: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n", j,
                                        peerAddr[j].bdAddr[5u],peerAddr[j].bdAddr[4u],
                                        peerAddr[j].bdAddr[3u],peerAddr[j].bdAddr[2u],
                                        peerAddr[j].bdAddr[1u],peerAddr[j].bdAddr[0u]);
                         }
                            
                          printf("Select Device:"); fflush(stdout);
                                                 
                        do
                        {
                            command = UART_UartGetChar(); 
                        }
                        while(command == 0u);
                        
                        if((command >= '0') && (command <= '9'))
                        {
                            DeviceN = (uint8)(command - '0');
                            printf("%c \r\n", command); /* print number */
                        }
                        else
                        {
                             printf(" Wrong digit \r\n");
                        }
                                              
                        printf("send 'c' to connect to the selected device\r\n");
                    }
                   
                    else           /* if no near by devices*/
                    {
                       printf("no devices near by\r\n");
                    }
                    printf("\r\n");
                break;
                 
            case 'a':               
                    
                    /* Starts advertisement if the device is in disconnected state*/
                    if(state==DISCONNECTED)  /*if the device is in disconnected state*/
                    {
                        if(CYBLE_ERROR_OK == CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST))
                        {
                            printf("Device entered General Discovery mode.\r\n"); 
                        }
                        else
                        {
                            printf("Device failed to enter General Discovery mode.\r\n"); 
                        }
                    }
                    else if(state==SCANNING )   /*If device is scanning*/
                    {/*Stop scanning before advertising*/
                       Stop=1;
    				   CyBle_GapcStopScan();
                    }
                     
                    printf("\r\n");
                break; 
             
            case 'u':   /*update connection interval parameter*/
             
                    if(state==CONNECTED) /* Check if already connection is established or not */
                    {
                         
                        if(ConnectedAs==SLAVE)            /* if it connected as slave */
                        {
                            /*send update connection request*/
                            apiResult = CyBle_L2capLeConnectionParamUpdateRequest(connHandle.bdHandle,&ConnectionParam);
                            
                            if(apiResult == CYBLE_ERROR_OK)
                            {
                                printf("Update connection parameters request sent successfully\r\n");
                            }
                            else if(apiResult==CYBLE_ERROR_INVALID_PARAMETER)
                            {
                                printf(" Update connection parameters request sending failed,EEROR:%x\r\n",apiResult);
                            }
                            
                        }
                        else if(ConnectedAs==MASTER)          /* if it is connected as master*/
                        {
                            
                            apiResult=CyBle_GapcConnectionParamUpdateRequest(connHandle.bdHandle,&ConnectionParam);
                            
                            if(apiResult == CYBLE_ERROR_OK)
                            {
                                printf("Update connection parameters command sent successfully\r\n");
                            }
                            else
                            {
                                printf(" Update connection parameters command is failed:%x\r\n",apiResult);
                            }
                                                    
                        }
                    }
                    else   /* if not connected to peer device*/
                    printf("no connection with peer device\r\n");
                    
                    printf("\r\n");
            
                break;        
            
            }
        }
        
       /* send notification to client about the number of devices near by  */
       if(NotificationsEnabled && SendDevicesNum)
       {
            CentralPeripheral.attrHandle=CYBLE_CUSTOM_SERVICE_CUSTOM_CHARACTERISTIC_CHAR_HANDLE;
            CentralPeripheral.value.val=&DevicesNearBy;
            CentralPeripheral.value.len=1;
            printf("DeviceNearBy: %d\r\n",DevicesNearBy);
            
            apiResult= CyBle_GattsNotification(connHandle,& CentralPeripheral);/*sending notifications*/
            
            if(apiResult==CYBLE_ERROR_OK)
            {
                 printf("Sent notification to Client\r\n");
            }
            else
            {
                 printf("failed to send notification to Client\r\n");
            }
             /*clear the variable to zero */
            SendDevicesNum=0;
            printf("\r\n");
       }
        
    }
}

/* [] END OF FILE */
