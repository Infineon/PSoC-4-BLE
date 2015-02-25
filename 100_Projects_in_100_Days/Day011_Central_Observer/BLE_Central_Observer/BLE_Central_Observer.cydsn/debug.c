/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "debug.h"


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
            UART_DEB_UartPutChar(ch);
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
        UART_DEB_UartPutChar(*buffer++);
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


void print_event(uint32 event)
{
	switch(event)
	{
		case CYBLE_EVT_HOST_INVALID :
			printf("CYBLE_EVT_HOST_INVALID \r\n");
			break;
		
		case CYBLE_EVT_STACK_ON :
			printf("CYBLE_EVT_STACK_ON \r\n");
			break;	
		case CYBLE_EVT_TIMEOUT :
			printf("CYBLE_EVT_TIMEOUT \r\n");
			break;	
		case CYBLE_EVT_HARDWARE_ERROR :
			printf("CYBLE_EVT_HARDWARE_ERROR \r\n");
			break;				
		case CYBLE_EVT_HCI_STATUS :
			printf("CYBLE_EVT_HCI_STATUS \r\n");
			break;				
		case CYBLE_EVT_STACK_BUSY_STATUS :
			printf("CYBLE_EVT_STACK_BUSY_STATUS \r\n");
			break;				
		case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT :
			printf("CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT \r\n");
			break;				
		case CYBLE_EVT_GAP_AUTH_REQ :
			printf("CYBLE_EVT_GAP_AUTH_REQ \r\n");
			break;				
		case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST :
			printf("CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST \r\n");
			break;				
		case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST :
			printf("CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST \r\n");
			break;				
		case CYBLE_EVT_GAP_AUTH_COMPLETE :
			printf("CYBLE_EVT_GAP_AUTH_COMPLETE \r\n");
			break;				
		case CYBLE_EVT_GAP_AUTH_FAILED :
			printf("CYBLE_EVT_GAP_AUTH_FAILED \r\n");
			break;				
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP :
			printf("CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP \r\n");
			break;				
		case CYBLE_EVT_GAP_DEVICE_CONNECTED :
			printf("CYBLE_EVT_GAP_DEVICE_CONNECTED \r\n");
			break;				
		case CYBLE_EVT_GAP_DEVICE_DISCONNECTED :
			printf("CYBLE_EVT_GAP_DEVICE_DISCONNECTED \r\n");
			break;				
		case CYBLE_EVT_GAP_ENCRYPT_CHANGE :
			printf("CYBLE_EVT_GAP_ENCRYPT_CHANGE \r\n");
			break;				
		case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE :
			printf("CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE \r\n");
			break;				
		case CYBLE_EVT_GAPC_SCAN_START_STOP :
			printf("CYBLE_EVT_GAPC_SCAN_START_STOP \r\n");
			break;				
		case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT :
			printf("CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT \r\n");
			break;				
		case CYBLE_EVT_GATTC_ERROR_RSP :
			printf("CYBLE_EVT_GATTC_ERROR_RSP \r\n");
			break;				
		case CYBLE_EVT_GATT_CONNECT_IND :
			printf("CYBLE_EVT_GATT_CONNECT_IND \r\n");
			break;				
		case CYBLE_EVT_GATT_DISCONNECT_IND :
			printf("CYBLE_EVT_GATT_DISCONNECT_IND \r\n");
			break;				
		case CYBLE_EVT_GATTS_XCNHG_MTU_REQ :
			printf("CYBLE_EVT_GATTS_XCNHG_MTU_REQ \r\n");
			break;				
		case CYBLE_EVT_GATTC_XCHNG_MTU_RSP :
			printf("CYBLE_EVT_GATTC_XCHNG_MTU_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP :
			printf("CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP :
			printf("CYBLE_EVT_GATTC_READ_BY_TYPE_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_FIND_INFO_RSP :
			printf("CYBLE_EVT_GATTC_FIND_INFO_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP :
			printf("CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_READ_RSP :
			printf("CYBLE_EVT_GATTC_READ_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_READ_BLOB_RSP :
			printf("CYBLE_EVT_GATTC_READ_BLOB_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_READ_MULTI_RSP :
			printf("CYBLE_EVT_GATTC_READ_MULTI_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTS_WRITE_REQ :
			printf("CYBLE_EVT_GATTS_WRITE_REQ \r\n");
			break;				
		case CYBLE_EVT_GATTC_WRITE_RSP :
			printf("CYBLE_EVT_GATTC_WRITE_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTS_WRITE_CMD_REQ :
			printf("CYBLE_EVT_GATTS_WRITE_CMD_REQ \r\n");
			break;				
		case CYBLE_EVT_GATTS_PREP_WRITE_REQ :
			printf("CYBLE_EVT_GATTS_PREP_WRITE_REQ \r\n");
			break;				
		case CYBLE_EVT_GATTS_EXEC_WRITE_REQ :
			printf("CYBLE_EVT_GATTS_EXEC_WRITE_REQ \r\n");
			break;				
		case CYBLE_EVT_GATTC_EXEC_WRITE_RSP :
			printf("CYBLE_EVT_GATTC_EXEC_WRITE_RSP \r\n");
			break;				
		case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF :
			printf("CYBLE_EVT_GATTC_HANDLE_VALUE_NTF \r\n");
			break;				
		case CYBLE_EVT_GATTC_HANDLE_VALUE_IND :
			printf("CYBLE_EVT_GATTC_HANDLE_VALUE_IND \r\n");
			break;				
		case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF :
			printf("CYBLE_EVT_GATTS_HANDLE_VALUE_CNF \r\n");
			break;				
		case CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ :
			printf("CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ :
			printf("CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP :
			printf("CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP \r\n");
			break;				
		case CYBLE_EVT_L2CAP_COMMAND_REJ :
			printf("CYBLE_EVT_L2CAP_COMMAND_REJ \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_CONN_IND :
			printf("CYBLE_EVT_L2CAP_CBFC_CONN_IND \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_CONN_CNF :
			printf("CYBLE_EVT_L2CAP_CBFC_CONN_CNF \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND :
			printf("CYBLE_EVT_L2CAP_CBFC_DISCONN_IND \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF :
			printf("CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_DATA_READ :
			printf("CYBLE_EVT_L2CAP_CBFC_DATA_READ \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND :
			printf("CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND :
			printf("CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND \r\n");
			break;				
		case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND :
			printf("CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND \r\n");
			break;				
		case CYBLE_EVT_PENDING_FLASH_WRITE :
			printf("CYBLE_EVT_PENDING_FLASH_WRITE \r\n");
			break;				
		case CYBLE_EVT_MAX :
			printf("CYBLE_EVT_MAX \r\n");
			break;				
			
		default:
		   break;
	}
}

/* [] END OF FILE */
