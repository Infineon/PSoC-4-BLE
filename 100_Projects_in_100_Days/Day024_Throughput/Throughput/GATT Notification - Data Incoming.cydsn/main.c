/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This is the source code for the project which demonstrates BLE data transfer
*  via GATT characteristic notification.
*  This project receives data from the Server device and calculates throughput.
*
* Hardware Dependency:
*  CY8CKIT-042-BLE Bluetooth Low Energy (BLE) Pioneer Kit
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
#include <stdbool.h>
#include "common.h"


/*******************************************************************************
* Macros
*******************************************************************************/
#define MAX_MTU_SIZE                (512)
#define DEFAULT_MTU_SIZE            (23)


/*******************************************************************************
* Structures
*******************************************************************************/
typedef struct
{
    uint8 count;
    CYBLE_GAP_BD_ADDR_T peripheralDetail[10];
} PERIPHERAL_LIST;


/*******************************************************************************
* Variables
*******************************************************************************/
uint32 totalByteCounter = 0;
bool charNotificationEnabled = false;
uint8 userStoppedScan = 0;
PERIPHERAL_LIST peripherals;

uint8 cccdNotifEnabledValue[] = {0x01, 0x00};

CYBLE_GATT_VALUE_T cccdNotifFlagSetStruct =
{
    cccdNotifEnabledValue,
    2,
    0
};


/*******************************************************************************
* Function Definitions
*******************************************************************************/


/*******************************************************************************
* Function Name: StartScan()
********************************************************************************
* Summary:
* Starts a scan on the Central device.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function starts BLE scan and resets scan related variables.
*
* Side Effects:
* None
*
*******************************************************************************/
static void StartScan(void)
{
    /* Re-initialize all variables for scanning and then start a fresh scan */
    peripherals.count = 0;
    
    CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
    UART_UartPutString("\n\rList of devices: ");
}


/*******************************************************************************
* Function Name: StackEventHandler()
********************************************************************************
* Summary:
* Event handler function for the BLE events processing.
*
* Parameters:
* uint32 eventCode: The event to be processed
* void * eventParam: Pointer to hold the additional information associated 
*                    with an event
*
* Return:
* None
*
* Theory:
* The function handles the events for BLE scan, connection, disconnection etc.
* The function initiates a GATT MTU exchange request upon connection, and it 
* also enables notifications on the Server's characteristic to get data.
* The service discovery procedure is avoided and it is assumed that the Server's 
* GATT database is known because the discovery procedure for custom services is 
* not part of the BLE component yet, and will come up shortly in the next 
* version of the component.
*
* Side Effects:
* None
*
*******************************************************************************/
void StackEventHandler(uint32 eventCode, void * eventParam)
{
    CYBLE_GAPC_ADV_REPORT_T * advReport;
    CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T * handleValueNotification;
    CYBLE_GATTC_WRITE_REQ_T writeRequestData =
    {
        cccdNotifFlagSetStruct,
        0x000F
    };
    uint8 counter;
    
    switch(eventCode)
    {
        case CYBLE_EVT_STACK_ON:
            /* Stack initialized; ready for scan */
            UART_UartPutString("Scanning. \n\r");
            UART_UartPutString("Press 'C' followed by the device number when you want to connect to that device. \n\r");
            UART_UartPutString("Press 'D' to disconnect from a connected peer device. \n\r");
            UART_UartPutString("Press 'S' to refresh the scan list. \n\r");
            StartScan();
            break;

            
        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
            /* A new device listed in the scan report */
            advReport = (CYBLE_GAPC_ADV_REPORT_T *)eventParam;
            
            /* Process only for Advertisement packets, not on scan response packets */
            if(advReport->eventType != CYBLE_GAPC_SCAN_RSP)
            {
                /* Store information for a maximum of 10 devices */
                if(peripherals.count < 10)
                {
                    peripherals.count++;
                    
                    UART_UartPutString("\n\r");
                    UART_UartPutChar(HexToDecimal(peripherals.count - 1, 1));
                    UART_UartPutChar(HexToDecimal(peripherals.count - 1, 0));
                    UART_UartPutString(". Address: ");
                    
                    for(counter = 6; counter > 0; counter--)
                    {
                        UART_UartPutChar(HexToAscii(advReport->peerBdAddr[counter - 1], 1));
                        UART_UartPutChar(HexToAscii(advReport->peerBdAddr[counter - 1], 0));
                        UART_UartPutChar(' ');
                    }
                    
                    /* Update our cache with the information of device */
                    memcpy(peripherals.peripheralDetail[peripherals.count - 1].bdAddr, advReport->peerBdAddr, 6);
                    peripherals.peripheralDetail[peripherals.count - 1].type = advReport->peerAddrType;
                    
                }
                else
                {
                    /* Buffer exceeded; ignore data */
                }
            }
            break;
        
        
        case CYBLE_EVT_GAPC_SCAN_START_STOP:
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
            {
                if(userStoppedScan == true)
                {
                    /* Scan stopped manually; do not restart scan */
                    userStoppedScan = false;
                }
                else
                {
                    /* Scanning timed out; Restart scan */
                    UART_UartPutString("\n\n\rRestarting scan. ");
                    StartScan();
                }
            }
            break;

            
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            UART_UartPutString("\n\rConnected. ");
            /* Once the devices are connected, the Client will not do a service
             * discovery and will assume that the handles of the Server are 
             * known. This is because discovery of custom service is not a part
             * of the BLE component today and will be in the next upgrade to BLE
             * component. 
             */
            
            /* Initiate an MTU exchange request */
            CyBle_GattcExchangeMtuReq(cyBle_connHandle, CYBLE_GATT_MTU);
            break;
            
            
        case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
            /* Enable notifications on the characteristic to get data from the 
             * Server.
             */
            CyBle_GattcWriteCharacteristicDescriptors(cyBle_connHandle, &writeRequestData);
            break;

            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            /* Device disconnected; restart scan */
            charNotificationEnabled = false;
            totalByteCounter = 0;
            UART_UartPutString("\n\n\rDisconnected. ");
            UART_UartPutString("Scanning again.");
            StartScan();
            break;
            
            
        case CYBLE_EVT_GATTC_WRITE_RSP:
            /* Write response for the CCCD write; this means that the
             * notifications are now enabled.
             * Start timing measurements to calculate throughput.
             */
            charNotificationEnabled = true;
            UART_UartPutString("\n\rCalculating throughput. Please wait... ");
            Timer_Enable();
            break;
            
        
        case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
            handleValueNotification = (CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *)eventParam;
            if(handleValueNotification->handleValPair.attrHandle == 0x000E)
            {
                totalByteCounter += handleValueNotification->handleValPair.value.len;
            }
            break;
            
            
        default:
            break;
    }
}


/*******************************************************************************
* Function Name: MyTimerIsr()
********************************************************************************
* Summary:
* Interrupt service routine for the timer block.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The ISR is fired when the timer reaches terminal count. In the ISR, the total
* number of bytes received until now is cached locally (to avoid any potential 
* race conditions) and then the throughput is calculated from it.
* Since the ISR fires 10 seconds after the timer is started, the total number
* of bytes is divided by 10. The bytes are converted to kilobytes by dividing 
* by 1024. It is then converted to kilo bits by multiplying the result by 8.
*
* Side Effects:
* None
*
*******************************************************************************/
void MyTimerIsr(void)
{
    uint32 cachedTotalByteCounter = totalByteCounter;
    uint32 throughput;

    /* Total bytes received in 10 seconds converted to Kilo bits per second */
    throughput = (cachedTotalByteCounter >> 7) / 10;

    UART_UartPutString("\n\rThroughput is: ");
    UART_UartPutChar(HexToDecimal(throughput, 2));
    UART_UartPutChar(HexToDecimal(throughput, 1));
    UART_UartPutChar(HexToDecimal(throughput, 0));
    UART_UartPutString(" kbps.");
    
    Timer_ClearInterrupt(Timer_INTR_MASK_TC);
}


/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
* The top-level application function for the project.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function initializes BLE, UART and Timer components, and then processes
* BLE events regularly.
* The function also processes UART Rx data to connect to a device, disconnect
* from a device, and refresh scan results.
*
* Side Effects:
* None
*
*******************************************************************************/
int main()
{
    uint8 command;
    
    /* Enable global interrupts for BLE */
    CyGlobalIntEnable; 
    
    /* Start and configure components for the project */
    CyBle_Start(StackEventHandler);
    UART_Start();
    Timer_Init();
    TimerInterrupt_StartEx(MyTimerIsr);

    /* Clear screen and put a welcome message */
    UART_UartPutChar(12);
    UART_UartPutString("========= BLE GATT Throughput Measurement - Client side =========\n\n\r");

    /* Infinite loop */
    for(;;)
    {
        /* Mandatory to process BLE events generated by stack */
        CyBle_ProcessEvents();
        
        /* Commands for connecting, disconnecting and restarting scan on the 
         * Central side.
         */
        command = UART_UartGetChar();
        if(command != 0u)
        {
            switch(CyBle_GetState())
            {
                case CYBLE_STATE_SCANNING:
                    if((command == 'c') || (command == 'C'))
                    {
                        /* User wants to connect to a device */
                        UART_UartPutString("\n\rConnect to device: ");
                        while((command = UART_UartGetChar()) == 0);
                        
                        /* accept the digits that are in between the range '0' and '9' */
                        if((command >= '0') && (command <= '9'))
                        {
                            UART_UartPutChar(command); 
                            command = command - '0';
                            
                            if(command < peripherals.count)
                            {
                                /* Valid device; stop scanning and connect */
                                CyBle_GapcStopScan();
                                userStoppedScan = true;
                                
                                while(userStoppedScan == true)
                                {
                                    CyBle_ProcessEvents();
                                }
                                
                                CyBle_GapcConnectDevice(&peripherals.peripheralDetail[command]);
                            }
                            else
                            {
                                UART_UartPutString(". Invalid number entered. ");
                            }
                        }
                        else  /* If entered digit is not in between the range '0' and '9' */
                        {
                            UART_UartPutString("Invalid character entered. ");
                        }
                    }
                    else if((command == 's') || (command == 'S'))
                    {
                        /* User wants to refresh the scan list */
                        UART_UartPutString("\n\n\rScan list refreshed.");
                        
                        CyBle_GapcStopScan();
                        userStoppedScan = true;
                        
                        while(userStoppedScan == true)
                        {
                            CyBle_ProcessEvents();
                        }
                        
                        StartScan();
                    }
                    break;
                    
                    
                case CYBLE_STATE_CONNECTED:
                    if((command == 'd') || (command == 'D'))
                    {
                        /* User wants to disconnect from the peer device */
                        CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                    }
                    break;

                    
                default:
                    break;
            }
        }
    }
}

/* [] END OF FILE */
