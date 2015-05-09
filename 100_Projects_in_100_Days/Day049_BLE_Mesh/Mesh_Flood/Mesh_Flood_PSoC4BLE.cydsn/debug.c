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
	switch(ble_state)
	{
		case CYBLE_STATE_STOPPED:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: CYBLE_STATE_STOPPED ");
		#endif
		break;
		
		case CYBLE_STATE_INITIALIZING:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: CYBLE_STATE_INITIALIZING ");
		#endif
		break;
		
		case CYBLE_STATE_CONNECTED:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: CYBLE_STATE_CONNECTED ");
		#endif
		break;
		
		case CYBLE_STATE_ADVERTISING:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: CYBLE_STATE_ADVERTISING ");
		#endif
		break;
		
		case CYBLE_STATE_SCANNING:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: CYBLE_STATE_SCANNING ");
		#endif
		break;
		
		case CYBLE_STATE_CONNECTING:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: CYBLE_STATE_CONNECTING ");
		#endif
		break;
		
		case CYBLE_STATE_DISCONNECTED:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: CYBLE_STATE_DISCONNECTED ");
		#endif
		break;
		
		default:
		#ifdef DEBUG_ENABLED
			UART_UartPutString(" |BLE State: UNKNOWN STATE ");
		#endif
		break;
	}
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
	#ifdef DEBUG_ENABLED
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
	#ifdef DEBUG_ENABLED
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

/* [] END OF FILE */
