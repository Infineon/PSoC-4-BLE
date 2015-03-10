/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This is the source code for the project which demonstrates BLE Privacy on
*  the Peripheral side. The Peripheral advertises with a private address.
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
#include <common.h>


/*******************************************************************************
* Enumerations
*******************************************************************************/
typedef enum
{
    ADVERTISEMENT_PUBLIC,
    ADVERTISEMENT_PRIVATE
} ADVERTISEMENT_STATE;


typedef enum
{
    AUTHENTICATION_NOT_CONNECTED,
    AUTHENTICATION_WAITING_FOR_PASSKEY,
    AUTHENTICATION_PASSKEY_ENTERED,
    AUTHENTICATION_COMPLETE_BONDING_REQD,
    AUTHENTICATION_BONDING_COMPLETE,
    AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT,
    AUTHENTICATION_BONDING_REMOVE_GO_AHEAD
} AUTHENTICATION_STATE;


/*******************************************************************************
* Global variables
*******************************************************************************/
ADVERTISEMENT_STATE advState = ADVERTISEMENT_PUBLIC;
AUTHENTICATION_STATE authState = AUTHENTICATION_NOT_CONNECTED;
CYBLE_GAP_BONDED_DEV_ADDR_LIST_T bondedDeviceList;

CYBLE_GAP_BD_ADDR_T privateAddress;
uint8 connectionIrk[CYBLE_GAP_SMP_IRK_SIZE] = {0};


/*******************************************************************************
* Function definitions
*******************************************************************************/


/*******************************************************************************
* Function Name: AdvertisePublicly()
********************************************************************************
* Summary:
* Starts an advertisement with a public address.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function starts BLE advertisements with a public address.
*
* Side Effects:
* None
*
*******************************************************************************/
static void AdvertisePublicly(void)
{
    /* Set the original public address as the device address */
    CyBle_SetDeviceAddress(&cyBle_deviceAddress);
    
    /* Revert if any changes made to advertisement settings */
    cyBle_discoveryModeInfo.advData = &cyBle_discoveryData;
    cyBle_discoveryParam.ownAddrType = CYBLE_GAP_ADDR_TYPE_PUBLIC;
    CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
}


/*******************************************************************************
* Function Name: AdvertisePrivately()
********************************************************************************
* Summary:
* Starts an advertisement with a random resolvable-private address.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function removes the Device name from the advertisement packet which was
* configured in the BLE component wizard, and starts advertisement with a 
* random resolvable-private address.
*
* Side Effects:
* None
*
*******************************************************************************/
static void AdvertisePrivately(void)
{
    /* Create a new data structure for Advertisement data 
     * without the device name.
     */
    CYBLE_GAPP_DISC_DATA_T privateAdvData = 
    {
        {
            0x02,
            0x01,
            0x06
        },
        3
    };
    
    uint8 counter;
    
    /* Generate a new resolvable private address */
    if(CYBLE_ERROR_OK == CyBle_GapGenerateDeviceAddress(&privateAddress, CYBLE_GAP_RANDOM_PRIV_RESOLVABLE_ADDR, connectionIrk))
    {
        /* Set the new device address */
        CyBle_SetDeviceAddress(&privateAddress);
        
        /* Advertise but without the device name */
        cyBle_discoveryModeInfo.advData = &privateAdvData;
        cyBle_discoveryParam.ownAddrType = CYBLE_GAP_ADDR_TYPE_RANDOM;
        CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        
        UART_UartPutString("Advertising with new private address: ");
        
        for(counter = 6; counter > 0; counter--)
        {
            UART_UartPutChar(HexToAscii(privateAddress.bdAddr[counter - 1], 1));
            UART_UartPutChar(HexToAscii(privateAddress.bdAddr[counter - 1], 0));
            UART_UartPutChar(' ');
        }
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
* The function initiates a pairing request upon connection. 
*
* Side Effects:
* None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void * eventParam)
{
    CYBLE_GAP_SMP_KEY_DIST_T * keys;
    uint8 counter;
    
    /* Common events for advertisement with public or private address */
    switch(event)
    {
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            /* Initiate pairing process */
            UART_UartPutString("\n\rConnected. ");
            CyBle_GapAuthReq(cyBle_connHandle.bdHandle, &cyBle_authInfo);
            break;
        
        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            UART_UartPutString("Enter the passkey shown in the peer device: ");
            authState = AUTHENTICATION_WAITING_FOR_PASSKEY;
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
            /* Copy the IRK key to a buffer; to be used for creating a 
             * private address.
             */
            keys = (CYBLE_GAP_SMP_KEY_DIST_T *) eventParam;
            memcpy(connectionIrk, keys->irkInfo, CYBLE_GAP_SMP_IRK_SIZE);
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
             * removed later.
             */
            authState = AUTHENTICATION_BONDING_COMPLETE;
            break;
            
        default:
            break;
    }        
    
    
    switch(advState)
    {
        /* If the bonding is not done and we are trying to connect to any 
         * device for the first time.
         */
        case ADVERTISEMENT_PUBLIC:
            switch(event)
            {
                case CYBLE_EVT_STACK_ON:
                    /* Stack initialized; start advertisement */
                    UART_UartPutString("Advertising with public address: ");
                    for(counter = 6; counter > 0; counter--)
                    {
                        UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 1));
                        UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 0));
                        UART_UartPutChar(' ');
                    }
                    AdvertisePublicly();
                    break;
                
                case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
                    /* If advertisement timed out and we were not connected 
                     * yet, restart advertisement.
                     */
                    if(authState != AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
                    {
                        if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
                        {
                            AdvertisePublicly();
                        }
                    }
                    else
                    {
                        UART_UartPutString("Advertisement stopped. ");
                        authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
                    }
                    break;

                case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
                    /* Disconnected but bonding is not complete; restart 
                     * advertisement.
                     */
                    UART_UartPutString("\n\n\rDisconnected. ");
                    if(authState != AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
                    {
                        UART_UartPutString("Advertising with public address: ");
                        for(counter = 6; counter > 0; counter--)
                        {
                            UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 1));
                            UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 0));
                            UART_UartPutChar(' ');
                        }
                        AdvertisePublicly();
                    }
                    else
                    {
                        authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
                    }
                    break;
                
                default:
                    break;
            }
            break;
            
            
        /* If the bonding has already happened and we are advertising with 
         * a private address.
         */
        case ADVERTISEMENT_PRIVATE:
            switch(event)
            {
                case CYBLE_EVT_STACK_ON:
                    /* Stack initialized; start advertisement */
                    AdvertisePrivately();
                    break;
                    
                case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
                    /* If advertisement timed out and we were not connected 
                     * yet, restart advertisement.
                     */
                    if(authState != AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
                    {
                        if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
                        {
                            UART_UartPutString("\n\n\r60 seconds elapsed. ");
                            AdvertisePrivately();
                        }
                    }
                    else
                    {
                        UART_UartPutString("Advertisement stopped. ");
                        authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
                    }
                    break;
                
                case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
                    /* Disconnected and bonding was previously complete; start 
                     * advertisement with private resolvable address but only 
                     * if the bonding is not to be removed. If the bonding is 
                     * to be removed then advertisement with public address
                     * will be started after the bonding is removed.
                     */
                    UART_UartPutString("\n\n\rDisconnected. ");
                    if(authState != AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT)
                    {
                        AdvertisePrivately();
                    }
                    else
                    {
                        authState = AUTHENTICATION_BONDING_REMOVE_GO_AHEAD;
                    }
                    break;
                
                default:
                    break;
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
* Also decides whether advertisement with public or private address is to be 
* done. Plus it maintains the bonding aspect - whether to add a device to bond
* list or to remove it.
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
    uint32 passkey = 0;
    uint32 pow;
    char8 command;
    uint8 counter;
    CYBLE_GAP_BD_ADDR_T bondedDeviceAddress;
    CYBLE_GAP_BD_ADDR_T clearAllDevices = {{0,0,0,0,0,0},0};
    uint8 bondedDeviceBdHandle;
    uint8 bondedDeviceKeysFlag;
    
    /* Enable global interrupts */
    CyGlobalIntEnable; 
    
    /* Start BLE and UART components for the project */
    CyBle_Start(StackEventHandler);
    UART_Start();
    
    /* Clear screen and put a welcome message */
    UART_UartPutChar(12);
    UART_UartPutString("========= BLE Privacy Demo - Peripheral =========\n\n\r");

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
        
        /* Start advertisement with private address */
        advState = ADVERTISEMENT_PRIVATE;
        authState = AUTHENTICATION_BONDING_COMPLETE;
    }
    else
    {
        /* Start advertisement with public address */
        advState = ADVERTISEMENT_PUBLIC;
        authState = AUTHENTICATION_NOT_CONNECTED;
    }

    
    /* Infinite loop */
    while(1)
    {
        /* Process pending BLE events */
        CyBle_ProcessEvents();
        
        switch(authState)
        {
            case AUTHENTICATION_WAITING_FOR_PASSKEY:
                /*Set the passkey to zero before receiving New passkey */
                passkey = 0u;
                pow = 100000;
                
                /*Get 6 digit number from UART terminal*/
                for(counter = 0; counter < CYBLE_GAP_USER_PASSKEY_SIZE; counter++)
                {
                    while((command = UART_UartGetChar()) == 0);
                    
                    /* accept the digits that are in between the range '0' and '9' */
                    if((command >= '0') && (command <= '9'))  
                    {
                        passkey += (uint32)(command - '0') * pow;
                        pow /= 10u;
                        UART_UartPutChar(command); 
                    }
                    else  /* If entered digit is not in between the range '0' and '9' */
                    {
                        UART_UartPutChar("\n\rWrong passkey entered. Disconnecting. ");
                        CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
                        break;
                    }
                }

                if(CYBLE_GAP_USER_PASSKEY_SIZE == counter)
                {
                    /*Send Pass key Response to to create an Authenticated Link*/
                    CyBle_GapAuthPassKeyReply(cyBle_connHandle.bdHandle, passkey, CYBLE_GAP_ACCEPT_PASSKEY_REQ);
                    authState = AUTHENTICATION_PASSKEY_ENTERED;
                }
                else
                {
                    authState = AUTHENTICATION_NOT_CONNECTED;
                }
                
                break;
        
                
            case AUTHENTICATION_COMPLETE_BONDING_REQD:
                /* Store bonding data of the current connection */
                while(CYBLE_ERROR_OK != CyBle_StoreBondingData(1));
                
                authState = AUTHENTICATION_BONDING_COMPLETE;
                UART_UartPutString("\n\rBonding complete. Press 'R' at any time to remove this bond. ");
                
                /* Ready for private advertisement the next time */
                advState = ADVERTISEMENT_PRIVATE;
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
                        else if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
                        {
                            /* Stop advertisement */
                            authState = AUTHENTICATION_BONDING_REMOVE_WAITING_EVENT;
                            CyBle_GappStopAdvertisement();
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
                UART_UartPutString("Cleared the list of bonded devices. \n\n\r");
                
                /* Clear the IRK used for random private address creation */
                for(counter = 0; counter < CYBLE_GAP_SMP_IRK_SIZE; counter++)
                {
                    connectionIrk[counter] = 0;
                }
                
                /* Start advertising with a public address */
                advState = ADVERTISEMENT_PUBLIC;
                UART_UartPutString("Advertising with public address: ");
                for(counter = 6; counter > 0; counter--)
                {
                    UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 1));
                    UART_UartPutChar(HexToAscii(cyBle_deviceAddress.bdAddr[counter - 1], 0));
                    UART_UartPutChar(' ');
                }
                AdvertisePublicly();
                    
                authState = AUTHENTICATION_NOT_CONNECTED;
                break;
                
                
            default:
                break;
        }
    }
}


/* [] END OF FILE */
