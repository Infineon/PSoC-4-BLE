/*******************************************************************************
* File Name: debug.c
*
* Version 1.0
*
* Description:
*  Common BLE application code for printout debug messages.
*
*******************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"


#if defined(__ARMCC_VERSION)

/* For MDK/RVDS compiler revise fputc function for printf functionality */
struct __FILE
{
    int handle;
};

enum
{
    STDIN_HANDLE,
    STDOUT_HANDLE,
    STDERR_HANDLE
};

FILE __stdin = {STDIN_HANDLE};
FILE __stdout = {STDOUT_HANDLE};
FILE __stderr = {STDERR_HANDLE};

int fputc(int ch, FILE *file)
{
    int ret = EOF;

    switch( file->handle )
    {
        case STDOUT_HANDLE:
            UART_UartPutChar(ch);
            ret = ch ;
            break ;

        case STDERR_HANDLE:
            ret = ch ;
            break ;

        default:
            file = file;
            break ;
    }
    return ret ;
}

#elif defined (__ICCARM__)      /* IAR */

/* For IAR compiler revise __write() function for printf functionality */
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    size_t nChars = 0;

    if (buffer == 0)
    {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return (0);
    }

    for (/* Empty */; size != 0; --size)
    {
        UART_UartPutChar(*buffer++);
        ++nChars;
    }

    return (nChars);
}

#else  /* (__GNUC__)  GCC */

/* For GCC compiler revise _write() function for printf functionality */
int _write(int file, char *ptr, int len)
{
    int i;
    file = file;
    for (i = 0; i < len; i++)
    {
        UART_UartPutChar(*ptr++);
    }
    return len;
}


#endif  /* (__ARMCC_VERSION) */

/*******************************************************************************
* Function Name: PrintHex
********************************************************************************
* Summary:
*        Converts HEX number to characters in ASCII that can be printed on 
* terminal.
*
* Parameters:
*  num: HEX to be converted to string.
*
* Return:
*  void
*
*******************************************************************************/
void PrintHex(uint8 num)
{
	uint8 temp[2];
	
	temp[0] = num%16;
	num = num/16;
	temp[1] = num%16;

	UART_UartPutString("0x");
	if(temp[1] < 10)
	{
		UART_UartPutChar('0' + temp[1]);
	}
	else
	{
		UART_UartPutChar('A' + (temp[1] - 10));
	}

	if(temp[0] < 10)
	{
		UART_UartPutChar('0' + temp[0]);
	}
	else
	{
		UART_UartPutChar('A' + (temp[0] - 10));
	}	
}

void DebugOut(uint32 event, void* eventParam)
{
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
            printf("EVT_STACK_ON \r\n");
            break;

        case CYBLE_EVT_STACK_BUSY_STATUS:
            printf("EVT_STACK_BUSY_STATUS \r\n");
            break;

        case CYBLE_EVT_TIMEOUT: /* 0x01 -> GAP limited discoverable mode timeout. */
                                /* 0x02 -> GAP pairing process timeout. */
                                /* 0x03 -> GATT response timeout. */
            printf("EVT_TIMEOUT: %d \r\n", *(uint8 *)eventParam);
            break;

        case CYBLE_EVT_HARDWARE_ERROR:    /* This event indicates that some internal HW error has occurred. */
            printf("EVT_HARDWARE_ERROR \r\n");
            break;

        case CYBLE_EVT_HCI_STATUS:
            printf("EVT_HCI_STATUS \r\n");
            break;

            
        /**********************************************************
        *                       GAP Events
        ***********************************************************/

        case CYBLE_EVT_GAP_AUTH_REQ:
            printf("EVT_GAP_AUTH_REQ \r\n");
            break;

        case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
            printf("EVT_GAP_PASSKEY_ENTRY_REQUEST \r\n");
            break;

        case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
            printf("EVT_GAP_PASSKEY_DISPLAY_REQUEST: %d \r\n", (int) (*(CYBLE_GAP_PASSKEY_DISP_INFO_T *)eventParam).passkey);
            break;

        case CYBLE_EVT_GAP_AUTH_FAILED:
            printf("EVT_GAP_AUTH_FAILED, reason: ");
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

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            printf("EVT_GAP_DEVICE_CONNECTED: %x \r\n", cyBle_connHandle.bdHandle);
            break;

        case CYBLE_EVT_GAPC_CONNECTION_UPDATE_COMPLETE:
            printf("EVT_GAPC_CONNECTION_UPDATE_COMPLETE \r\n");
            break;

        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            printf("EVT_GAP_DEVICE_DISCONNECTED, reason: %x \r\n", *(uint8*)eventParam);
            break;

        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            printf("EVT_GAP_AUTH_COMPLETE: security:%x, bonding:%x, ekeySize:%x, authErr %x \r\n",
                        ((CYBLE_GAP_AUTH_INFO_T *)eventParam)->security,
                        ((CYBLE_GAP_AUTH_INFO_T *)eventParam)->bonding, 
                        ((CYBLE_GAP_AUTH_INFO_T *)eventParam)->ekeySize, 
                        ((CYBLE_GAP_AUTH_INFO_T *)eventParam)->authErr);
            break;

        case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
            printf("EVT_GAP_ENCRYPT_CHANGE: %d \r\n", *(uint8 *)eventParam);
            break;


        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATTC_ERROR_RSP:
            printf("EVT_GATTC_ERROR_RSP: opcode: ");
            switch(((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->opCode)
            {
                case CYBLE_GATT_FIND_INFO_REQ:
                    printf("FIND_INFO_REQ");
                    break;

                case CYBLE_GATT_READ_BY_TYPE_REQ:
                    printf("READ_BY_TYPE_REQ");
                    break;

                case CYBLE_GATT_READ_BY_GROUP_REQ:
                    printf("READ_BY_GROUP_REQ");
                    break;

                default:
                    printf("%x", ((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->opCode);
                    break;
            }
            printf(",  handle: %x,  ", ((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->attrHandle);
            printf("errorcode: ");
            switch(((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->errorCode)
            {
                case CYBLE_GATT_ERR_ATTRIBUTE_NOT_FOUND:
                    printf("ATTRIBUTE_NOT_FOUND");
                    break;

                default:
                    printf("%x", ((CYBLE_GATTC_ERR_RSP_PARAM_T *)eventParam)->errorCode);
                    break;
            }
            printf("\r\n");
            break;

        case CYBLE_EVT_GATT_CONNECT_IND:
            printf("EVT_GATT_CONNECT_IND: attId %x, bdHandle %x \r\n", 
                ((CYBLE_CONN_HANDLE_T *)eventParam)->attId, ((CYBLE_CONN_HANDLE_T *)eventParam)->bdHandle);
            break;

        case CYBLE_EVT_GATT_DISCONNECT_IND:
            printf("EVT_GATT_DISCONNECT_IND \r\n");
            break;


        /**********************************************************
        *                       L2CAP Events
        ***********************************************************/

        case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
            printf("EVT_L2CAP_CONN_PARAM_UPDATE_REQ \r\n");
            break;

            
        /**********************************************************
        *                       Debug Events
        ***********************************************************/

        case CYBLE_DEBUG_EVT_BLESS_INT:
            break;


        default:
        #if(CYBLE_GATT_ROLE_SERVER)
            ServerDebugOut(event, eventParam);
        #endif /* (CYBLE_GATT_ROLE_SERVER) */
        #if(CYBLE_GATT_ROLE_CLIENT)
            ClientDebugOut(event, eventParam);
        #endif /* (CYBLE_GATT_ROLE_SERVER) */
            break;
    }
}


/* [] END OF FILE */
