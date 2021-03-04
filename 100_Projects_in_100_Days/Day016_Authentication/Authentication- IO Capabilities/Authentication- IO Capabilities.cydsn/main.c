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
#include <math.h>
#include "common.h"


/***************************************
*        Global Variables
***************************************/

char8 command;
uint8 CharWrite=0;

CYBLE_API_RESULT_T apiResult;
CYBLE_CONN_HANDLE_T connHandle;
CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;

/***************************************
*        Function Declaration
***************************************/
void Ble_Event_handler(uint32 event,void* eventParam);


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
    
    CYBLE_GAP_AUTH_INFO_T authInfo;
    CYBLE_GAP_IOCAP_T cyble_IO;
    CYBLE_GAP_SMP_KEY_DIST_T KEY;
    CYBLE_GATT_ERR_CODE_T gattErr;
    uint8 j;   
    
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:  /*BLE Stack ON*/
        
                printf("Bluetooth Stack ON:\r\n");
                
                /*Update the IO capabilities before Starting to advertise*/
                
                printf("Set the I/O capabilities:\r\n");
                while((command= UART_UartGetChar()) == 0);
                
                               
                cyble_IO=(uint8)(command - '0');
                
                if((command >= '0') && (command <= '4'))
                {
                    /*Update the I/O capability as per the user selection*/
                    apiResult= CyBle_GapSetIoCap(cyble_IO);
                                
                    if(apiResult==CYBLE_ERROR_OK)
                    {
                        printf("Updated the I/O capabilities:");
                        if((uint8)(command-'0')==0)
                        {
                            printf("DISPLAY_ONLY\r\n");
                        }
                        else if((uint8)(command-'0')==1)
                        {
                            printf("DISPLAY_YESNO\r\n");
                        }
                        else if((uint8)(command-'0')==2)
                        {
                            printf("KEYBOARD_ONLY\r\n");
                        }
                        else if((uint8)(command-'0')==3)
                        {
                            printf("NOINPUT_NOOUTPUT\r\n");
                        }
                        else if((uint8)(command-'0')==4)
                        {
                            printf("KEYBOARD_DISPLAY\r\n");
                        }
                    }
                
                }
                else
                printf("INVALID_PARAMETER\r\n");
                   
                /*  I/O capabilities updated.Now Start Advertise   */
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
                
        case CYBLE_EVT_TIMEOUT:
                printf("Time OUT\r\n");
            break;
                        
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP: 
       
                if(CyBle_GetState()==CYBLE_STATE_ADVERTISING)
                {
                    printf("Started to advertise\r\n");                    
                }
                else if(CyBle_GetState()==CYBLE_STATE_DISCONNECTED)
                {
                    printf("Advertising Stopped");
                }
            break;
        
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            
                printf("Device Connected\r\n");
                                
                /*Req Client to Start Authentication process*/
                CyBle_GapAuthReq(connHandle.bdHandle, &cyBle_authInfo);
                        
            break;
        
        case CYBLE_EVT_GATT_CONNECT_IND:
                connHandle = *(CYBLE_CONN_HANDLE_T *)eventParam;
            break;
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
               /*Device disconnected*/
                printf("Device Disonnected\r\n");
                connHandle.bdHandle=0;
                CharWrite=0;
        
            break;
            
        case CYBLE_EVT_GAP_AUTH_REQ:
                
                /*Authentication Request recieved from Master */
         
                printf("\r\n");
                printf("CYBLE_EVT_GAP_AUTH_REQ from Master:\r\n");
                                 
                authInfo=*((CYBLE_GAP_AUTH_INFO_T *)eventParam);
                
                /*Get the details about the Authentication Request*/
                if((authInfo.security)==0x00)
                {
                    printf("No Security\r\n");
                }
                else if((authInfo.security)==0x01)
                {
                    printf("Security: Unauthenticated and Encryption\r\n");
                }
                else if ((authInfo.security)==0x02)
                {
                    printf("security: Authentication and Encryption\r\n");
                }
                else if((authInfo.security)==0x03)
                {
                    printf("Unauthenticated and data signing\r\n");
                }
                else if ((authInfo.security)==0x04)
                {
                    printf("Authentication and data signing\r\n");
                }

                if((*(CYBLE_GAP_AUTH_INFO_T *)eventParam).bonding==CYBLE_GAP_BONDING_NONE)
                {
                    printf("Bonding: No \r\n");
                }
                else
                {
                    printf("Bonding:yes\r\n");
                }
                

                printf("eKeySize: 0x%x\r\n", (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).ekeySize);
                printf("err: 0x%x\r\n",(*(CYBLE_GAP_AUTH_INFO_T *)eventParam).authErr);
                
            break;
        
        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
                /*Passkey Entry Request*/
                printf("EVT_PASSKEY_ENTRY_REQUEST press 'p' to enter passkey \r\n");
            break;

        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
                /*Passkey Display Request*/
                printf("CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST. Passkey is: %ld.\r\n",*(uint32*)eventParam);
                printf("Please enter the passkey on peer device.\r\n");
            
            break;
        
        case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
                /*Key Information exchange completed */
               
                
                printf("\r\n");
                printf("KEYINFO_EXCHANGE_COMPLETED\r\n");
                KEY= *(CYBLE_GAP_SMP_KEY_DIST_T *)eventParam;
                
                /*Print Long term Key */
                printf("Long term Key: ");
                for(j=0;j<CYBLE_GAP_SMP_LTK_SIZE;j++)
                {
                   printf("%2.2x",KEY.ltkInfo[CYBLE_GAP_SMP_LTK_SIZE-j-1]);
                }
                
                
                /*Print Encrypted Diversifier*/
                printf("\r\nEncrypted Diversifier: ");
                
                for(j=0;j<2;j++)
                {
                    printf("%2.2x",KEY.midInfo[1-j]);
                }
                
                /*Print Random Number*/
                printf("\r\nRandom Number:");
                for(j=2;j<CYBLE_GAP_SMP_MID_INFO_SIZE;j++)
                {
                    printf("%2.2x",KEY.midInfo[CYBLE_GAP_SMP_MID_INFO_SIZE-j+1]);
                }

                /*Print Indentifier Resolving Key*/
                printf("\r\nIndentifier Resolving Key: ");
                for(j=0;j<CYBLE_GAP_SMP_IRK_SIZE;j++)
                {
                    printf("%2.2x",KEY.irkInfo[CYBLE_GAP_SMP_IRK_SIZE-j-1]);
                }
                
                /*Print Address of peer device*/
                printf("\r\nPublic device/Static Random address: ");
                for(j=1;j<CYBLE_GAP_SMP_IDADDR_DATA_SIZE;j++)
                {
                    printf("%2.2x",KEY.idAddrInfo[CYBLE_GAP_SMP_IDADDR_DATA_SIZE-j]);
                }
               
                /*Print Address type of peer Device*/
                printf("\r\nAddress Type:");
                if(KEY.idAddrInfo[0]==0x00)
                {
                    printf("PUBLIC\r\n");
                }
                else
                {
                    printf("RANDOM\r\n");
                }
                
                /*Print Connection Signature Resolving Key*/
                printf("\r\nConnection Signature Resolving Key: ");
                for(j=0;j<CYBLE_GAP_SMP_CSRK_SIZE;j++)
                {
                    printf("%2.2x",KEY.csrkInfo[CYBLE_GAP_SMP_CSRK_SIZE-j-1]);
                }
                printf("\r\n");
            break;

        
        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
                
                /*Encyrption Change event*/
                printf("\r\n");
                
                /*Get the current status of Encryption*/
                if(*(uint8 *)eventParam==0x00)
                {
                    printf("Encrytpion OFF\r\n");
                }
                else if(*(uint8 *)eventParam==0x01)
                {
                    printf("Encryption ON\r\n");
                }
                else
                {
                    printf("ERROR\r\n");
                }
            break;
        
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
                
                /*Authentication Completed*/
                
                authInfo = *(CYBLE_GAP_AUTH_INFO_T *)eventParam;
                printf("AUTHENTICATION_COMPLETED:\r\n");
            
                /* Authentication status of connection after pairing process*/
                if((authInfo.security)==0x00)
                {
                    printf("No Security\r\n");
                 }   
                else if((authInfo.security)==0x01)
                {
                    printf("Security: Unauthenticated and Encryption\r\n");
                }
                else if ((authInfo.security)==0x02)
                {
                    printf("security: Authentication and Encryption\r\n");
                }
                else if((authInfo.security)==0x03)
                {
                    printf("Unauthenticated and data signing\r\n");
                }
                else if ((authInfo.security)==0x04)
                {
                    printf("Authentication and data signing\r\n");
                }
                
                /*Used Bonding or not*/
                if((*(CYBLE_GAP_AUTH_INFO_T *)eventParam).bonding==CYBLE_GAP_BONDING_NONE)
                {
                    printf("Bonding: No \r\n");
                }
                else
                {
                    printf("Bonding:yes\r\n");
                }

                printf("eKeySize: 0x%x\r\n", (*(CYBLE_GAP_AUTH_INFO_T *)eventParam).ekeySize);
                printf("err: 0x%x\r\n",(*(CYBLE_GAP_AUTH_INFO_T *)eventParam).authErr);
                
            
            break;

        case CYBLE_EVT_GAP_AUTH_FAILED:
                
                /*Authentication Failed.Display the error code */
            printf("CYBLE_EVT_GAP_AUTH_FAILED: %d \r\n", *(uint8 *) eventParam);
            
            break;
            
        case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
                    
                /*Received Writewithout Response command from the Client*/
                wrReqParam =(CYBLE_GATTS_WRITE_REQ_PARAM_T*) eventParam; 
                
                gattErr = CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair, 0u, 
                        &wrReqParam->connHandle, CYBLE_GATT_DB_PEER_INITIATED);             
               
               if(wrReqParam->handleValPair.attrHandle==CYBLE_CUSTOM_SERVICE_CUSTOM_CHARACTERISTIC_CHAR_HANDLE
                && gattErr == CYBLE_GATT_ERR_NONE)
                {
                    printf("Characteristic value updated by Client  :\r\n");
                    
                    /*Set the variable "CharWrite"to indicate that client write to the Custom Characteristic*/
                    CharWrite=1;    
                    printf("%2.2x\r\n",*wrReqParam->handleValPair.value.val);
                }
            
              
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
    /*Declare local variables*/
    
   uint8 i;
   uint32 passkey=0;
   uint32 pow;
   uint8 value;
    
   CyGlobalIntEnable;  /* To enable global interrupts. */   

   
   UART_Start();
    

   CyBle_Start(StackEventHandler);

    for(;;)
    {
        /*process BLE events*/
        CyBle_ProcessEvents();
        
        /*get the character from UART terminal*/
        command = UART_UartGetChar();
        		
        if(command != 0u)
        {
            switch(command)
            {
              case 'p':                   /* Enter passkey  */
                printf("Enter 6 digit passkey:\n"); 
                
                /*Set the passkey to zero before receiving New passkey */
                passkey = 0u;
                pow = 100000ul;
                
                /*Get 6 digit number from UART terminal*/
                for(i = 0u; i < CYBLE_GAP_USER_PASSKEY_SIZE; i++)
                {
                    while((command = UART_UartGetChar()) == 0)
                    {
                        CyBle_ProcessEvents();
                    }
                    
                    
                    /* accept the digits that are in between the range '0' and '9'  */
                    if((command >= '0') && (command <= '9'))  
                    {
                        passkey += (uint32)(command - '0') * pow;
                        pow /= 10u;
                        UART_UartPutChar(command); 
                    }
                    else  /* If entered digit is not in between the rnage '0' and '9'*/
                    {
                        printf(" Wrong digit \r\n");
                        break;
                    }
                }
                printf("\r\n");
 
                /*Send Pass key Response to to create an Authenticated Link*/
                apiResult = CyBle_GapAuthPassKeyReply(connHandle.bdHandle,passkey,1);
               
                
                /*Check the result of CyBle_GapAuthPassKeyReply*/
                if(apiResult== CYBLE_ERROR_INVALID_PARAMETER)
                {
                    printf("CYBLE_ERROR_INVALID_PARAMETER \r\n");               
                }
                else if(apiResult==CYBLE_ERROR_MEMORY_ALLOCATION_FAILED)
                {
                    printf("CYBLE_ERROR_MEMORY_ALLOCATION_FAILED\r\n");
                }
                else if(apiResult==CYBLE_ERROR_NO_DEVICE_ENTITY)
                {
                    printf("CYBLE_ERROR_NO_DEVICE_ENTITY\r\n");
                }
                else if(apiResult==CYBLE_ERROR_OK)
                {
                    printf("Sent passKey succesfully \r\n");
                }
                    
              break;
            }
        }
        
        
        /*  If the Authenticated client has written value to the custom characteristic
          * then increment the value by 1 and write it back to custom characteristic*/
        
        
         /*Check whether state is connected or not before writing to the custom characterisitc*/
        if(CharWrite && CyBle_GetState()==CYBLE_STATE_CONNECTED)  
       {
            CharWrite=0;
            value=*wrReqParam->handleValPair.value.val;
        
            /*increase the value by 1*/
            value++;   
        
            wrReqParam->handleValPair.value.val=&value;
            printf("Update Characteristic value to: %2.2x\r\n",*wrReqParam->handleValPair.value.val);
            
            /*Write new value to custom Characteristic*/
            CyBle_GattsWriteAttributeValue(&wrReqParam->handleValPair,0,&wrReqParam->connHandle,CYBLE_GATT_DB_LOCALLY_INITIATED);

                
           
       }
    }
}

/* [] END OF FILE */
