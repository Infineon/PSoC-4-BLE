/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates BLE component as Time multiplexed Central. 
*  The device would connect to a peer device, which maintains the connection 
*  for a particular amount of time.
*
*  Hardware Dependency:
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
#include "stdio.h"

/*MACROS*/
#define RTC_SOURCE_COUNTER                          (0u)
#define RTC_COUNTER_ENABLE                          (1u)
#define RTC_COUNT_PERIOD                            ((uint32)32767)
#define RTC_INTERRUPT_SOURCE                        CY_SYS_WDT_COUNTER0_INT

/*Function Defintion*/
uint8 AddressCompare(CYBLE_GAP_BD_ADDR_T ,uint8* );


/***************************************
*        Global Variables
***************************************/
CYBLE_GAP_BD_ADDR_T  peerAddr[CYBLE_MAX_ADV_DEVICES];
CYBLE_GAP_BD_ADDR_T  deviceAddr;
CYBLE_GAPC_ADV_REPORT_T advReport;
uint8 connectCommand=FALSE;
uint8 deviceN=0;
uint8 devicesNearBy=0;
uint8 addPeripheral=TRUE;
uint8 timeOut=FALSE;
uint8 state=DISCONNECTED;
uint8 nextDevice=0;
uint8 newDevice=TRUE;
uint8 deviceToRemove=0;
uint8 inWhitelist=FALSE;
uint8 removebondedDevice=FALSE;

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

void StackEventHandler(uint32 event,void* eventparam)
{
    /*Declaring local variables*/   
    uint8 i,j;
    
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
                printf("\r\n=========BLE STACK ON==========\r\n");                                
                printf("\r\nPress 'c' followed by the device number to connect to a device\r\n");
                printf("Press 'r' followed by the device number to remove device from bonded list\r\n");
                printf("Press 'a' to add new peripheral.\r\n");
                CyBle_GapGetBondedDevicesList(&bondedDeviceList);
                
                /*Set scanning timeout as infinite*/
                cyBle_discoveryInfo.scanTo=0;
                
                /*Enable whitelist if number of bonded devices is not zero*/
                if(bondedDeviceList.count!=0)
                {
                    cyBle_discoveryInfo.scanFilterPolicy=CYBLE_GAPC_ADV_ACCEPT_WHITELIST_PKT;
                    addPeripheral=FALSE;
                    DisplayBondedlist();
                    printf("\r\nWhitelist is enabled\r\n");
                    printf("Multiplexing process is started\r\n");
                }
                else
                {
                    printf("\r\nWhitelist is not enabled. Central is Scanning for peripherals\r\n\r\n");
                }                
                
                /*Start Discovery*/
                CyBle_GapcStartDiscovery(&cyBle_discoveryInfo);
            break;
            
        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:            
                    advReport= *(CYBLE_GAPC_ADV_REPORT_T *)eventparam;                         
                    newDevice=TRUE;
                    
                    /*If received advertisement packet is from "nextDevice" in the bonded 
                     *devices list and not adding new peripheral then connect to this device*/
                    if(AddressCompare(bondedDeviceList.bdAddrList[nextDevice],advReport.peerBdAddr) &&  addPeripheral==FALSE)
                    {
                        connectCommand=TRUE;                        
                        CyBle_GapcStopScan();                      
                    }
                    /*if adding new peripheral*/
                    else if((devicesNearBy <= CYBLE_MAX_ADV_DEVICES) && addPeripheral==TRUE)
                    {  
                        /*Compare the address of the peer device with the addresses already in the list*/
                        for(j = 0u; j < devicesNearBy; j++)
                        {
                            if(AddressCompare(peerAddr[j],advReport.peerBdAddr))
                            {
                                newDevice = FALSE;
                            }                            
                        }
                        /*if new device then add to the list*/
                        if(newDevice==TRUE)
                        {
                            /* Increase the number of peer devices when 
                             * received aadvertising packet from different peer devices.*/
                            ++devicesNearBy;
                            
                            printf("Device %d-->:", devicesNearBy);
                            if(bondedDeviceList.count!=0)
                            {
                                /*Check if the received advertising packet is from bonded device list or not*/
                                for(i=0;i<bondedDeviceList.count;i++)
                                {
                                   if(AddressCompare(bondedDeviceList.bdAddrList[i],advReport.peerBdAddr)==TRUE)
                                    {
                                        inWhitelist=TRUE;
                                        break;
                                    }
                                }
                            }
                            /*Display the address of the received advertising packet*/
                            for(i = 0u; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
                            {
                                peerAddr[devicesNearBy].bdAddr[i] = advReport.peerBdAddr[i];
                                UART_UartPutChar(HexToAscii(advReport.peerBdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 1));
                                UART_UartPutChar(HexToAscii(advReport.peerBdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 0));
                                UART_UartPutChar(' ');                                                               
                            }                    
                            /*Mention whether the address of peer device  is part of whitelist or not*/
                            if(inWhitelist)
                            {
                                printf(" [ Part of Whitelist ]");
                                inWhitelist=FALSE;
                            }
                            else
                            {
                                printf(" [ Not Part of Whitelist ]");
                            }                                
                            printf("\r\n");
                        }
                    }                                                                                 
            break;
       
        case CYBLE_EVT_GAPC_SCAN_START_STOP:
               
                if(state==SCANNING)   /*If device was scanning means scanning is stopped*/
                {
                    state=DISCONNECTED;
                    /*if scanning is stopped to connect to peer device*/
                    if(connectCommand==TRUE)
                    {    
                        /*If connecting to New peripheral*/
                        if(addPeripheral==TRUE)
                        {
                            CyBle_GapcConnectDevice(&peerAddr[deviceN]); 
                        }
                        else /*Connecting to the device in the bonded list*/
                        {
                            CyBle_GapcConnectDevice(&bondedDeviceList.bdAddrList[nextDevice]);
                        }
                        connectCommand=FALSE;                    
                    }
                    /*if scanning is stopped to add new peripheral then start scanning again*/
                    else if(connectCommand==FALSE && removebondedDevice==FALSE)
                    {                        
                        CyBle_GapcStartDiscovery(&cyBle_discoveryInfo);  
                    }
                }    
                
                /*if device was in disconnected then then device is scanning*/
                else if(state==DISCONNECTED)
                {                    
                    state=SCANNING;
                }
            break;
                
        case CYBLE_EVT_GAP_DEVICE_CONNECTED: 
                Connection_LED_Write(LED_ON);
                state=CONNECTED;
                deviceN=0;
                devicesNearBy=0;                                
                connectCommand=FALSE;   
                CyBle_GapAuthReq(cyBle_connHandle.bdHandle,&cyBle_authInfo);
            break;
                
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
                Connection_LED_Write(LED_OFF); 
                state=DISCONNECTED;                
                CyBle_GapGetBondedDevicesList(&bondedDeviceList);
                
                if(bondedDeviceList.count!=0 && addPeripheral==FALSE)
                {   /*Enable whitelist*/
                    cyBle_discoveryInfo.scanFilterPolicy=CYBLE_GAPC_ADV_ACCEPT_WHITELIST_PKT;
                }                
                
                if(removebondedDevice==FALSE)
                {  /*To continue Multiplexing process*/
                    CyBle_GapcStartDiscovery(&cyBle_discoveryInfo);                
                }
            break;
                
        default:
            break;
    }
}


/*******************************************************************************
* Function Name: WDT_Start
********************************************************************************
*
* Summary:
* This is used to set the WDT register to use Counter 0.
* Interrupt will occur for every 1sec
*
* Parameters:
*  None

* Return:
*  None
*
*******************************************************************************/
void WDT_Start(void)
{
    /* configures WDT to trigger an interrupt every second for RTC operation */
    
    CySysWdtUnlock(); /* Unlock the WDT registers for modification */
    
    CySysWdtWriteMode(RTC_SOURCE_COUNTER, CY_SYS_WDT_MODE_INT);
    
    CySysWdtWriteClearOnMatch(RTC_SOURCE_COUNTER, RTC_COUNTER_ENABLE);
     
    /*Set time period as 1 second*/
    CySysWdtWriteMatch(RTC_SOURCE_COUNTER, RTC_COUNT_PERIOD);
    
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    
    CySysWdtLock();
}



/*******************************************************************************
* Function Name: CY_ISR(WDT_Handler)
********************************************************************************
*
* Summary:
*  This is interrupt service routine WDT counter.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
CY_ISR(WDT_Handler)
{
   if(CySysWdtGetInterruptSource() & RTC_INTERRUPT_SOURCE)
    {
        CySysWdtClearInterrupt(RTC_INTERRUPT_SOURCE);        
        timeOut=TRUE;
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
    char8 command; 
    uint8 i;
    
    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
    
    UART_Start();
    WDT_Start(); 
    UART_UartPutChar(12);     
    CyBle_Start(StackEventHandler);
    
    RTC_Interrupt_StartEx(WDT_Handler);
    
    for(;;)
    {
           CyBle_ProcessEvents();
     
            /*Store bonding information*/
            if((cyBle_pendingFlashWrite != 0u) &&
               ((UART_SpiUartGetTxBufferSize() + UART_GET_TX_FIFO_SR_VALID) == 0u))
            {                                
                 if(CYBLE_ERROR_OK== CyBle_StoreBondingData(1u))                                          
                {
                    if(addPeripheral==TRUE && state==CONNECTED)
                    {
                        DisplayBondedlist();
                    }
                    /*enable whitelist*/                                      
                    CyBle_GapGetBondedDevicesList(&bondedDeviceList);                                      
                }                               
            }
            
            /*If WDT time out happens*/
            if(timeOut==TRUE)
            {                
                timeOut=FALSE;                                
                
                /*If number of bonded devices not zero*/
                if(bondedDeviceList.count!=0)
                {
                    ++nextDevice;
                    /*nextDevice value should be less than number of bonded devices-1*/
                    if(nextDevice>bondedDeviceList.count-1)
                    {
                        nextDevice=0;
                    }
                    
                    if(bondedDeviceList.count>1 && addPeripheral==FALSE && removebondedDevice==FALSE)/*Connected to alteast one device*/
                    {   
                        /*Disconnect the connection*/
                        if(state==CONNECTED)
                        {
                            CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                        }
                        else if(state==SCANNING)  /*If Central is scanning when timeout happen means device is missing*/
                        {         
                            printf("Device missing: ");
                            
                            if(nextDevice==0)
                            {  /*Dispaly address of missing bonded device*/
                                for(i = 0u; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
                                {                                    
                                    UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[bondedDeviceList.count-1].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 1));
                                    UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[bondedDeviceList.count-1].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 0));
                                    UART_UartPutChar(' ');                                                               
                                } 
                            }
                            else if(nextDevice < (bondedDeviceList.count))
                            {
                              for(i = 0u; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
                                {                                    
                                    UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[nextDevice-1].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 1));
                                    UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[nextDevice-1].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 0));
                                    UART_UartPutChar(' ');                                                               
                                }   
                            }
                           printf("\r\n");
                           /*Stop Discovery*/
                           CyBle_GapcStopDiscovery();                                                                                                
                        } 
                    }
                    else if(bondedDeviceList.count==1 && state==SCANNING && addPeripheral==FALSE)
                    {
                        printf("Device missing: ");
                        for(i = 0u; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
                        {   /*Display address of missing bonded device*/                 
                            UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[nextDevice].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 1));
                            UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[nextDevice].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 0));
                            UART_UartPutChar(' ');                                                               
                        }
                        printf("\r\n");
                    }
                }
            }
            
            /*To process the commands from UART*/
            command = UART_UartGetChar();
		    if(command != 0u)
            {
                switch(command)
                {   
                    case 'a':
                    case 'A':    /*To add new peripheral*/
                            
                            /*Disable the whitelist*/
                            cyBle_discoveryInfo.scanFilterPolicy=CYBLE_GAPC_ADV_ACCEPT_ALL_PKT;                            
                            if(bondedDeviceList.count!=0 && addPeripheral==FALSE)
                            {
                                addPeripheral=TRUE;
                                removebondedDevice=FALSE;
                                printf("\r\nScanning with whitelist Disabled:\r\n");
                                if(state==SCANNING)
                                {   /*If state is SCANNING then stop scanning*/
                                    CyBle_GapcStopScan();  
                                }
                                else if(state==CONNECTED)
                                {   /*If state is CONNECTED then disconnect the connection*/
                                    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                                }
                            }
                            else if(addPeripheral==TRUE && removebondedDevice==FALSE)
                            {
                                printf("Already in progress of adding peripheral\r\n");
                            }
                        break;
                                        
                    case 'r':        
                    case 'R':  /*To remove Device from Bonded list*/                                                        
                            
                            /*Display the bonded device list*/
                            if(bondedDeviceList.count!=0)
                            {   
                                removebondedDevice=TRUE;
                                if(state==SCANNING)
                                {  /*If state is SCANNING then stop scanning*/ 
                                    CyBle_GapcStopScan();
                                }
                                if(state==CONNECTED)
                                {   /*If state is CONNECTED then disconnect the connection*/
                                    CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                                }
                                printf("\r\nMultiplexing Process is Stopped\r\n");
                                
                                
                                /*wait to stop the mutliplexing process*/
                                while(state==CONNECTED ||state==SCANNING)
                                {
                                    CyBle_ProcessEvents();
                                }
                            
                                
                                DisplayBondedlist();
                                printf("\r\nSelect Device to remove from bonded list: ");
                                do
                                {
                                    command = UART_UartGetChar(); 
                                }
                                while(command == 0u);
                                
                                deviceToRemove = (uint8)(command - '0');
                                printf("%c \r\n", command); /* print number */
                                
                                /*If selected device number is valid then remove the bonded device*/
                                if((deviceToRemove >= 1) && (deviceToRemove <=bondedDeviceList.count ))
                                {                                                                        
                                    if(CYBLE_ERROR_OK== CyBle_GapRemoveDeviceFromWhiteList(&bondedDeviceList.bdAddrList[deviceToRemove-1]))
                                    {
                                        printf("Removed from bonded list\r\n");                                                                                
                                        DisplayBondedlist();
                                    }
                                    else
                                    {
                                        printf("Failed to remove from bonded list\r\n");
                                    }
                                }
                                else
                                {
                                     printf("\r\nWrong Digit \r\n");
                                }     
                                
                                /*Start Mutliplexing process again after removing bonded devices informations*/
                                removebondedDevice=FALSE;                            
                                if(bondedDeviceList.count==0)     /*No bonded devices*/
                                {   
                                    /*Disable Whitelist*/
                                    cyBle_discoveryInfo.scanFilterPolicy=CYBLE_GAPC_ADV_ACCEPT_ALL_PKT;
                                    addPeripheral=TRUE;
                                    printf("\r\nStarted to scan with whitelist disabled\r\n");
                                }
                                else
                                {
                                    printf("\r\nMutliplexing process Started\r\n");
                                }
                            
                                /*Start Discovery*/
                                CyBle_GapcStartDiscovery(&cyBle_discoveryInfo);
                            }
                            else
                            {
                                printf("\r\nNo devices in the bonded list\r\n");
                            }                                                                                                             
                        break;
                            
                    case 'c':
                    case 'C':    /* Select peer device  */                                                        
                                
                            if(devicesNearBy)      /*check if there is any peer device before selecting the device*/
                            {                                                                       
                                 connectCommand = TRUE;  
                                 printf("\r\nSelect Device to connect:"); fflush(stdout);
                                
                                /*Get device number from UART*/
                                do
                                {
                                    command = UART_UartGetChar(); 
                                }
                                while(command == 0u);
                                
                                /*Disaply the Device number selected by user*/
                                if((command >= '1') && (command <= '8'))
                                {
                                    deviceN = (uint8)(command - '0');
                                    printf("%c \r\n", command); /* print number */
                                }
                                else
                                {
                                     printf(" Wrong digit \r\n");
                                } 
                                
                                if(state==SCANNING)
                                {                    
                                    CyBle_GapcStopScan();   /* Stop scanning if it is scanning 
                                                         * before sending connection request */                                                    
                                }
                            }
                           
                            else if(state==SCANNING)           /* if no near by devices*/
                            {
                               printf("no devices near by\r\n");
                            }
                            printf("\r\n");
                        break;
                   }
            }            
       
    }
}

/*******************************************************************************
* Function Name: DisplayBondedlist()
********************************************************************************
* Summary:
* Display address of the devices in the bonded list
*
* Parameters:
* none
*
* Return:
* none
*
*******************************************************************************/
void DisplayBondedlist()
{
    uint8 j,i;    
    CyBle_GapGetBondedDevicesList(&bondedDeviceList);
    
    if(bondedDeviceList.count!=0)
    {  /*If there is at least one bonded device*/
        if(addPeripheral==TRUE)
        {
            addPeripheral=FALSE;           
        }        
        printf("\r\nCurrent List of Bonded Devices:\r\n");
           
        for(j=bondedDeviceList.count;j>0;j--)
        {   /*Display bonded device list*/         
            printf("Device %d -->",bondedDeviceList.count-j+1);
            for(i = 0u; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
            {
                UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[bondedDeviceList.count-j].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 1));
                UART_UartPutChar(HexToAscii(bondedDeviceList.bdAddrList[bondedDeviceList.count-j].bdAddr[CYBLE_GAP_BD_ADDR_SIZE-i - 1], 0));
                UART_UartPutChar(' ');                   
            } 
            printf("\r\n");
        }
    }
    else   /*No Bonded devices in the list*/
    {  
        printf("No Device in the bonded list\r\n\r\n");        
    }
    
}

/*******************************************************************************
* Function Name: AddressCompare
********************************************************************************
*
* Summary:
*  This is used to TURN ON and TURN OFF the LEDs based on the "state".
*
* Parameters:
*  CYBLE_GAP_BD_ADDR_T address1:  Address 1 among the two addresses to be compared
*  uint8*              address2:  Address 2 among the two addresses to be compared

* Return:
*  uint8:  TRUE   (if addresses are same)
*          FALSE   (if addresses are not same)
*
*******************************************************************************/
uint8 AddressCompare(CYBLE_GAP_BD_ADDR_T address1,uint8* address2)
{
    if(address1.bdAddr[0]==address2[0]&& address1.bdAddr[1]==address2[1]
    && address1.bdAddr[2]==address2[2]&& address1.bdAddr[3]==address2[3]
    && address1.bdAddr[4]==address2[4]&& address1.bdAddr[5]==address2[5])
    {
        return TRUE;    /*Addresses are same*/
    }
    else
    {
        return FALSE;  /*Addresses are different*/
    }
}
/* [] END OF FILE */
