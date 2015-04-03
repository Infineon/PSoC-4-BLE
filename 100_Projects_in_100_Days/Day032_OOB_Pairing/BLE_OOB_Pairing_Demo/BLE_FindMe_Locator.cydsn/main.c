/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This code demonstrates Out-of-Band (OOB) pairing using PSoC4 BLE kit. This
*  code is based on the example project 'BLE_FindMe'. In this project, PSoC4 is
*  configured as a Find Me Locator with GAP role as central.
*
*******************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/*******************************************************************************
*   Included Headers
*******************************************************************************/
#include "project.h"
#include "stdio.h"


/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/

/* Firmware starts processing the data only if at least 2 bytes are recieved */ 
#define MIN_UART_BYTES          (2)

/* Command to initiate a connection with a peer or target device */
#define CMD_CONNECT             ('C')

/* Total length = 7 bytes address x 2 + 16 bytes key x 2
 * Data is sent as hex characters. i.e. 2 hex chars per byte. 
 * The 7th byte of address indicates the address type such as
 * public or private. 
 */
#define CONNECT_DATA_LENGTH     (46)
#define SECURITY_KEY_LENGTH     (16)
#define LED_ON                  (0u)
#define LED_OFF                 (1u)


/*******************************************************************************
*   Module Variable and Constant Declarations with Applicable Initializations
*******************************************************************************/
static CYBLE_API_RESULT_T apiResult;
static uint8 securityKey[16] = {0};
static uint8 tagData[64];


/*******************************************************************************
*   Function Prototypes
*******************************************************************************/
void ProcessUartData(void);
uint8 HexCharToByte(uint8 c);
void HexStringToByteArray(uint8* hexString, uint32 length);


/****************************************************************************** 
Function Name: AppCallBack
*******************************************************************************

Summary:
 Handles the events from the BLE stack

Parameters:
 eventCode:    the event code
 *eventParam:  the event parameters

Return:
 None.

******************************************************************************/
void AppCallBack(uint32 event, void* eventParam)
{
	int i;
	
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            Disconnect_LED_Write(LED_ON);
            break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
			
            /* Indicate to stack that the OOB data is available. OOB data needs
             * to be set before initiating the auth (pair) request. 
             */
			if(CyBle_GapSetOobData(cyBle_connHandle.bdHandle, CYBLE_GAP_OOB_ENABLE, securityKey, NULL, NULL)  != CYBLE_ERROR_OK)
			{
				printf("PSoC: Error in Setting OOB Data \r\n");
			}
			else
			{
				printf("PSoC: OOB Data is set with key:");
				for(i = 0; i < 16; i++)
				{
					printf("%2.2x", securityKey[i]);
				}
				printf("\r\n");
			}

			/* Send authorization request. */
            apiResult = CyBle_GapAuthReq(cyBle_connHandle.bdHandle, &cyBle_authInfo);
            
            if(CYBLE_ERROR_OK != apiResult)
        	{
        		printf("PSoC: CyBle_GapAuthReq API Error: %x \r\n", apiResult);
        	}
        	else
        	{
        		printf("PSoC: Pairing is initiated \r\n");
                Disconnect_LED_Write(LED_OFF);
        	}
            break;
            
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            printf("PSoC: ***** Pairing is Successful! ***** \r\n");
            break;
			
		case CYBLE_EVT_GAP_AUTH_FAILED:
        printf("PSoC: EVT_GAP_AUTH_FAILED, reason: ");
        switch(*(CYBLE_GAP_AUTH_FAILED_REASON_T *)eventParam)
        {
            case CYBLE_GAP_AUTH_ERROR_CONFIRM_VALUE_NOT_MATCH:
                printf("CONFIRM_VALUE_NOT_MATCH\r\n");
                break;
                
            case CYBLE_GAP_AUTH_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
                printf("INSUFFICIENT_ENCRYPTION_KEY_SIZE\r\n");
                break;
            
            case CYBLE_GAP_AUTH_ERROR_UNSPECIFIED_REASON:
                printf("UNSPECIFIED_REASON\r\n");
                break;
                
            case CYBLE_GAP_AUTH_ERROR_AUTHENTICATION_TIMEOUT:
                printf("AUTHENTICATION_TIMEOUT\r\n");
                break;
                
            default:
                printf("0x%x  \r\n", *(CYBLE_GAP_AUTH_FAILED_REASON_T *)eventParam);
                break;
        }
        break;

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
            /* Accepted = 0x0000, Rejected  = 0x0001 */
            CyBle_L2capLeConnectionParamUpdateResponse(cyBle_connHandle.bdHandle, 0u); 
            break;

        default:
            break;
    }
}

/****************************************************************************** 
Function Name: main
*******************************************************************************

Summary:
 Main routine of this firmware. Initializes the components once and hanles the
 BLE events and UART data in a loop forever.

Parameters:
 None.

Return:
 None.

******************************************************************************/
int main()
{
    /* This start-up delay is for the Android app to open when the locator
     * device is plugged into the phone's USB port so that the first printf
     * message will not be missed. 
     */    
	CyDelay(3000);
	
    CyGlobalIntEnable;
    
    UART_DEB_Start();               /* Start communication component */
	
    printf("PSoC: BLE Find Me Locator is Connected\r\n");
    
    Disconnect_LED_Write(LED_OFF);
    apiResult = CyBle_Start(AppCallBack);
    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("PSoC: CyBle_Start API Error: 0x%x \r\n", apiResult);
    }
    
    while(1)
    {        
        /*******************************************************************
        *  Processes all pending BLE events in the stack
        *******************************************************************/        
        CyBle_ProcessEvents();
		
		/* Process the received UART data */
        ProcessUartData();
    }
}


/****************************************************************************** 
Function Name: ProcessUartData
*******************************************************************************

Summary:
 Parses the commands received through the UART and initiates actions based on
 the command. For example, this function initiates connection to a peer device
 upon receiving the connect command.

Parameters:
 None.

Return:
 None.

******************************************************************************/
void ProcessUartData(void)
{
	uint32 data;
	uint32 length;
	uint32 i;
	CYBLE_GAP_BD_ADDR_T peerAddr; 
	
	if(UART_DEB_SpiUartGetRxBufferSize() < MIN_UART_BYTES) {return;}
	
	data = UART_DEB_UartGetChar(); 
	if(data == 0){return;}
	if(data == CMD_CONNECT)
	{
		length = UART_DEB_UartGetChar(); /* length */		
		if(length == 0){return;}
		
		i = 0;
		do
		{
			data = UART_DEB_UartGetChar(); 
			if(data != 0x00)
			{
				tagData[i++] = data;
			}
		}while(data != 0x0D);
		
		if(length != CONNECT_DATA_LENGTH)
		{
			printf("PSoC: Length is not equal to %d \r\n", CONNECT_DATA_LENGTH);
			return;
		}
        else
		{
			printf("PSoC: CONNECT command is Received. \r\n");
			
			tagData[length] = (uint8)'\r';
			tagData[length + 1] = (uint8)'\n';
			tagData[length + 2] = 0;
			
			printf("%s", tagData);
		}
		
		/* Convert Hex string into byte array */
		HexStringToByteArray(tagData, length);
		
		for(i = 0; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
		{
			peerAddr.bdAddr[i] = tagData[i]; 
		}
		
		peerAddr.type = tagData[i];
		
		for(i = 0; i < SECURITY_KEY_LENGTH; i++)
		{
			securityKey[i] = tagData[CYBLE_GAP_BD_ADDR_SIZE + 1 + i];
		}
		
		printf("PSoC: Connection initiated to device address: ");
        
		for(i = 0; i < CYBLE_GAP_BD_ADDR_SIZE; i++)
		{
    		printf("%2.2x", peerAddr.bdAddr[CYBLE_GAP_BD_ADDR_SIZE - i - 1]);
		}
		printf("\r\n");
		
		/* Initiate Connection */
		apiResult = CyBle_GapcConnectDevice(&peerAddr);
        if(apiResult != CYBLE_ERROR_OK)
		{
			printf("PSoC: Error in Connection Inititation \r\n");
		}
	}
}


/****************************************************************************** 
Function Name: HexStringToByteArray
*******************************************************************************

Summary:
 Converts a hex string to byte array. Each 2 hex characters will be converted to
 one equivalent byte. The resulting byte array is stored in the same hex char
 array starting from index 0 so the original content is lost.

Parameters:
 hexString: Pointer to the array of hex characters
 length:    Length of the array

Return:
 None.

******************************************************************************/
void HexStringToByteArray(uint8* hexString, uint32 length) 
{
	uint32 i;
	uint8 hexToByte;
	
	for (i = 0; i < length; i += 2) 
	{
		hexToByte = ((HexCharToByte(hexString[i]) << 4) | HexCharToByte(hexString[i + 1]));
		hexString[i / 2] = hexToByte;
	}
}


/****************************************************************************** 
Function Name: HexCharToByte
*******************************************************************************

Summary:
 Converts one hex character to the equivalent byte value.  

Parameters:
 c: The hex character

Return:
 uint8: Byte value equivalent to the hex char.

******************************************************************************/
uint8 HexCharToByte(uint8 c) {
    if (c >= '0' && c <= '9')
        return (c - '0');
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
		
		return 0;
}


/* [] END OF FILE */
