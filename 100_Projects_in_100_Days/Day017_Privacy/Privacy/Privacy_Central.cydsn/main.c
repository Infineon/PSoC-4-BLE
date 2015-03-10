/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This is the source code for the project which demonstrates BLE Privacy on
*  the Central side. The Central resolves a private address of the Peripheral.
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
#include <common.h>


/*******************************************************************************
* Enumerations
*******************************************************************************/
typedef enum
{
    AUTHENTICATION_NOT_CONNECTED,
    AUTHENTICATION_COMPLETE_BONDING_REQD,
    AUTHENTICATION_BONDING_COMPLETE,
    AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT,
    AUTHENTICATION_BONDING_REMOVE_GO_AHEAD
} AUTHENTICATION_STATE;


/*******************************************************************************
* Structures
*******************************************************************************/
typedef struct
{
    uint8 count;
    CYBLE_GAP_BD_ADDR_T peripheralDetail[10];
} PERIPHERAL_LIST;


/*******************************************************************************
* Global variables
*******************************************************************************/
AUTHENTICATION_STATE authState = AUTHENTICATION_NOT_CONNECTED;
PERIPHERAL_LIST peripherals;
CYBLE_GAP_BONDED_DEV_ADDR_LIST_T bondedDeviceList;
uint8 bondedDeviceBdHandle;
uint8 connectionIrk[CYBLE_GAP_SMP_IRK_SIZE] = {0};
uint8 userStoppedScan = 0;
bool irkPresentFlag = false;
bool resolveAddressFlag = false;


/*******************************************************************************
* Function definitions
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
    userStoppedScan = 0;
    peripherals.count = 0;
    
    CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
    UART_UartPutString("\n\rList of devices: ");
}


/*******************************************************************************
* Function Name: ResolvePrivateAddress()
********************************************************************************
* Summary:
* Tries to resolve a private address with a given IRK.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* If an IRK (Identity Resolving Key) is present, the function tries to resolve
* a private address with this key. If the address is resolved, it then updates
* the bond information with this.
*
* Side Effects:
* None
*
*******************************************************************************/
static void ResolvePrivateAddress(void)
{
    uint8 myIrk[16];
    uint32 apiReturn;
    
    memcpy(myIrk, connectionIrk, CYBLE_GAP_SMP_IRK_SIZE);
    
    /* Random address; see if it can be resolved */
    if(irkPresentFlag == true)
    {
        apiReturn = CyBle_GapcResolveDevice(peripherals.peripheralDetail[peripherals.count - 1].bdAddr, myIrk);
        if(apiReturn == CYBLE_ERROR_OK)
        {
            UART_UartPutString(" Resolved. ");
            /* Update bonding information for the device */
            if(CYBLE_ERROR_OK == CyBle_GapcSetRemoteAddr(bondedDeviceBdHandle, peripherals.peripheralDetail[peripherals.count - 1]))
            {
                /* Store to flash; don't stop scanning for this */
                while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
                UART_UartPutString("Updated bonding info. ");
            }
        }
        else
        {
            UART_UartPutString(" Not Resolved. IRK doesn't match. Return code: 0x");
            UART_UartPutChar(HexToAscii(apiReturn, 1));
            UART_UartPutChar(HexToAscii(apiReturn, 0));
        }
    }
    else
    {
        UART_UartPutString(" Cannot Resolve. No IRK stored. ");
    }
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
* The function is responsible for handling the events generated by the stack.
* The function scans for devices and resolves a private address.
*
* Side Effects:
* None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void * eventParam)
{
    CYBLE_GAP_SMP_KEY_DIST_T * keys;
    CYBLE_GAPC_ADV_REPORT_T * advReport;
    uint8 counter;
    
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
            /* Stack initialized; start scanning */
            UART_UartPutString("Scanning. \n\r");
            UART_UartPutString("Press 'C' followed by the device number when you want to connect to that device. \n\r");
            UART_UartPutString("Press 'D' to disconnect from a connected peer device. \n\r");
            UART_UartPutString("Press 'S' to refresh the scan list. \n\r");
            UART_UartPutString("Press 'R' to clear the bond list. \n\r");
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
                    if(advReport->peerAddrType == CYBLE_GAP_ADDR_TYPE_RANDOM)
                    {
                        UART_UartPutString(". Random Address: ");
                        resolveAddressFlag = true;
                    }
                    else
                    {
                        UART_UartPutString(". Public Address: ");
                    }
                    
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
            if(authState != AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
            {
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
            }
            else
            {
                /* User stopped scan for bonding purposes */
                UART_UartPutString("Scan stopped. ");
                authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
            }
            break;
            
            
        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            UART_UartPutString("Enter this passkey in your peer device: ");
            UART_UartPutChar(HexToDecimal(*(uint32 *)eventParam, 5));
            UART_UartPutChar(HexToDecimal(*(uint32 *)eventParam, 4));
            UART_UartPutChar(HexToDecimal(*(uint32 *)eventParam, 3));
            UART_UartPutChar(HexToDecimal(*(uint32 *)eventParam, 2));
            UART_UartPutChar(HexToDecimal(*(uint32 *)eventParam, 1));
            UART_UartPutChar(HexToDecimal(*(uint32 *)eventParam, 0));
            break;
            
            
        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
            /* Copy the IRK key to a buffer; to be used for resolving a 
             * private address.
             */
            keys = (CYBLE_GAP_SMP_KEY_DIST_T *) eventParam;
            memcpy(connectionIrk, keys->irkInfo, CYBLE_GAP_SMP_IRK_SIZE);
            irkPresentFlag = true;
            break;
            
            
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            /* Authentication complete; initiate bonding */
            UART_UartPutString("\n\rAuthentication complete.");
            authState = AUTHENTICATION_COMPLETE_BONDING_REQD;
            break;
            
            
        case CYBLE_EVT_GAP_AUTH_FAILED:
            /* Authentication failed */
            UART_UartPutString("\n\rAuthentication failed. Disconnecting. ");
            CyBle_GapDisconnect(cyBle_connHandle.bdHandle);

            /* Update authentication state so that bond information could be 
             * removed later 
            */
            authState = AUTHENTICATION_BONDING_COMPLETE;
            break;
            
            
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            UART_UartPutString("\n\rConnected. ");
            
            /* Cache the BD handle to resolve private addresses later */
            bondedDeviceBdHandle = cyBle_connHandle.bdHandle;
            break;
            
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            UART_UartPutString("\n\n\rDisconnected. ");
            if(authState != AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
            {
                /* Disconnected; restart scan. */
                UART_UartPutString("Scanning again.");
                StartScan();
            }
            else
            {
                authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
            }
            break;
            
            
        default:
            break;
    }
}


/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
* The top level application function.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Initializes the BLE and UART components and then process BLE events regularly.
* Checks whether an IRK is stored in the bond information of this device and 
* loads it (if available) for private address resolution.
* It implements bonding and privacy (resolvable private address resolution) 
* feature.
* It also handles UART commands for connecting to a device, disconnecting from
* a device, refreshing scan, and clearing the bonded devices' list.
*
* For more information on privacy feature, refer to Bluetooth 4.1 Specification, 
* Volume 3, Part C, Section 10.7.
*
* Side Effects:
* None
*
*******************************************************************************/
int main()
{
    uint8 command;
    uint8 counter;
    CYBLE_GAP_BD_ADDR_T clearAllDevices = {{0,0,0,0,0,0},0};
    CYBLE_GAP_BD_ADDR_T bondedDeviceAddress;
    uint8 bondedDeviceKeysFlag;
    
    /* Enable global interrupts */
    CyGlobalIntEnable; 
    
    /* Start BLE and UART components for the project */
    CyBle_Start(StackEventHandler);
    UART_Start();
    
    /* Clear screen and put a welcome message */
    UART_UartPutChar(12);
    UART_UartPutString("========= BLE Privacy Demo - Central =========\n\n\r");
    
    
    /* Find out whether the device has bonded information stored already or not */
    CyBle_GapGetBondedDevicesList(&bondedDeviceList);
    if(bondedDeviceList.count != 0)
    {
        CYBLE_GAP_SMP_KEY_DIST_T securityKeys;
        
        /* Take the last bonded device */
        memcpy(bondedDeviceAddress.bdAddr, bondedDeviceList.bdAddrList[0].bdAddr, 6);
        bondedDeviceAddress.type = bondedDeviceList.bdAddrList[0].type;
        
        /* Get the BD handle of that bonded device */
        CyBle_GapGetPeerBdHandle(&bondedDeviceBdHandle, &bondedDeviceAddress);
        
        /* Get the IRK for that bonded device */
        bondedDeviceKeysFlag |= 0x01;   /* Identity Resolution Key */ 
        CyBle_GapGetPeerDevSecurityKeyInfo(bondedDeviceBdHandle, &bondedDeviceKeysFlag, &securityKeys);
        memcpy(connectionIrk, securityKeys.irkInfo, CYBLE_GAP_SMP_IRK_SIZE);
        
        /* Ready to resolve private address */
        irkPresentFlag = true;
        authState = AUTHENTICATION_BONDING_COMPLETE;
    }
    else
    {
        /* Cannot resolve private address */
        irkPresentFlag = false;
        authState = AUTHENTICATION_NOT_CONNECTED;
    }
    
    
    /* Infinite loop */
    while(1)
    {
        /* Process pending BLE events */
        CyBle_ProcessEvents();
        
        
        /* Resolve a private address */
        if(resolveAddressFlag == true)
        {
            ResolvePrivateAddress();
            resolveAddressFlag = false;
        }
        
        
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
        
        
        /* Handle the state machine for authentication and bonding */
        switch(authState)
        {
            case AUTHENTICATION_COMPLETE_BONDING_REQD:
                /* Store bonding data of the current connection */
                while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
                
                authState = AUTHENTICATION_BONDING_COMPLETE;
                UART_UartPutString("\n\rBonding complete. ");
                
                break;
                
            
            case AUTHENTICATION_BONDING_COMPLETE:
                /* See if the user pressed 'R' button to remove the bond. */
                command = UART_UartGetChar();
                
                if(command != 0u)
                {
                    if((command == 'r') || (command == 'R'))
                    {
                        /* User wants the bond to be removed */
                        UART_UartPutString("\n\rRemove the bond. ");
                        if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
                        {
                            /* Disconnect */
                            authState = AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT;
                            CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                        }
                        else if(CyBle_GetState() == CYBLE_STATE_SCANNING)
                        {
                            /* Stop scan */
                            authState = AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT;
                            CyBle_GapcStopScan();
                        }
                        else
                        {
                            authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
                        }
                    }
                }
                break;
                
                
            case AUTHENTICATION_BONDING_REMOVE_GO_AHEAD:
                /* Remove all bonded devices in the list */
                CyBle_GapRemoveDeviceFromWhiteList(&clearAllDevices);
                
                while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
                UART_UartPutString("Cleared the list of bonded devices. \n\r");
                
                /* Clear the IRK used for random private address resolution */
                for(counter = 0; counter < CYBLE_GAP_SMP_IRK_SIZE; counter++)
                {
                    connectionIrk[counter] = 0;
                }
                irkPresentFlag = false;
                
                /* Start scanning again */
                UART_UartPutString("Scanning again.");
                StartScan();
                
                authState = AUTHENTICATION_NOT_CONNECTED;
                break;
                
                
            default:
                break;
        }
    }
}


/* [] END OF FILE */
