/*******************************************************************************
* File Name: debug.c
*
* Version: 1.0
*
* Description:
* This file contains the definiton for debug functions, using UART as communication
* medium. These function sends UART data to KitProg/PSoC 5LP, which enumerats as
* COM port on connected PC.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <debug.h>

/*******************************************************************************
* Function Name: SendBLEStatetoUART
********************************************************************************
* Summary:
*        Sends the string to UART corresponding to the present BLE state
*
* Parameters:
*  ble_state: current state of the BLE.
*
* Return:
*  void
*
*******************************************************************************/
void SendBLEStatetoUART(CYBLE_STATE_T ble_state)
{
	#if (DEBUG_ENABLED == 1)
	/* Depending on current BLE state sent, place the string on UART */
	switch(ble_state)
	{
		case CYBLE_STATE_STOPPED:
			UART_UartPutString(" |BLE State: CYBLE_STATE_STOPPED ");
		break;
		
		case CYBLE_STATE_INITIALIZING:
			UART_UartPutString(" |BLE State: CYBLE_STATE_INITIALIZING ");
		break;
		
		case CYBLE_STATE_CONNECTED:
			UART_UartPutString(" |BLE State: CYBLE_STATE_CONNECTED ");
		break;
		
		case CYBLE_STATE_ADVERTISING:
			UART_UartPutString(" |BLE State: CYBLE_STATE_ADVERTISING ");
		break;
		
		case CYBLE_STATE_SCANNING:
			UART_UartPutString(" |BLE State: CYBLE_STATE_SCANNING ");
		break;
		
		case CYBLE_STATE_CONNECTING:
			UART_UartPutString(" |BLE State: CYBLE_STATE_CONNECTING ");
		break;
		
		case CYBLE_STATE_DISCONNECTED:
			UART_UartPutString(" |BLE State: CYBLE_STATE_DISCONNECTED ");
		break;
		
		default:
			UART_UartPutString(" |BLE State: UNKNOWN STATE ");
		break;
	}
	
	UART_UartPutCRLF(' ');
	#else
		ble_state = ble_state;
	#endif
}

/*******************************************************************************
* Function Name: PrintNum
********************************************************************************
* Summary:
*        Converts decimal number to characters in ASCII that can be printed on 
* terminal.
*
* Parameters:
*  num: number to be converted to string.
*
* Return:
*  void
*
*******************************************************************************/
void PrintNum(uint8 num)
{
	#if (DEBUG_ENABLED == 1)
	uint8 temp[3];
	
	temp[0] = num%10;
	num = num/10;
	temp[1] = num%10;
	num = num/10;
	temp[2] = num%10;
	
	if(temp[2] == 0)
	{
		if(temp[1] == 0)
		{
			UART_UartPutChar('0' + temp[0]);
		}
		else
		{
			UART_UartPutChar('0' + temp[1]);
			UART_UartPutChar('0' + temp[0]);	
		}
	}
	else
	{
		UART_UartPutChar('0' + temp[2]);
		UART_UartPutChar('0' + temp[1]);
		UART_UartPutChar('0' + temp[0]);
	}
	#else
		num = num;
	#endif
}

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
	#if (DEBUG_ENABLED == 1)
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
	#else
		num = num;
	#endif
}

/*******************************************************************************
* Function Name: HexToAscii
********************************************************************************
* Summary:
*        Converts either the higher or lower nibble of a hex byte to its
* corresponding ASCII.
*
* Parameters:
*  value - hex value to be converted to ASCII
*  nibble - 0 = lower nibble, 1 = higher nibble
*
* Return:
*  char - hex value for the value/nibble specified in the parameters
*
*******************************************************************************/
char HexToAscii(uint8 value, uint8 nibble)
{
    #if (DEBUG_ENABLED == 1)
	if(nibble == 1)
    {
        /* bit-shift the result to the right by four bits */
        value = value & 0xF0;
        value = value >> 4;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else if (nibble == 0)
    {
        /* extract the lower nibble */
        value = value & 0x0F;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else
    {
        value = ' ';  /* return space for invalid inputs */
    }
    
    return value;
	#else
		value = value;
		nibble = nibble;
		return(1);
	#endif
}
/* [] END OF FILE */
