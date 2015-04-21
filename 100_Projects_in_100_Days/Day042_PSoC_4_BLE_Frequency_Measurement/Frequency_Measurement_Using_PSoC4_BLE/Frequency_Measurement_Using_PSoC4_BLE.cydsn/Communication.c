/***************************************************************************//**
* @file    Communication.c
* @version 1.0
* @author  Chethan (dche@cypress.com)
*
* @par Description
*   This file contains the source code to the APIs for transmitting project's 
*   debug data via UART.
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 BLE Pioneer Kit
*
* @par References
*    1. 001-32359: AN2283 "PSoC 1 Measuring Frequency"
*
* @par Code Tested With
*    1. PSoC Creator  3.1 SP2 (3.1.0.2177)
*    2. GCC 4.8.4
*
**//****************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
********************************************************************************
* All rights reserved. 
* This software, including source code, documentation and related 
* materials (“Software”), is owned by Cypress Semiconductor 
* Corporation (“Cypress”) and is protected by and subject to worldwide 
* patent protection (United States and foreign), United States copyright 
* laws and international treaty provisions. Therefore, you may use this 
* Software only as provided in the license agreement accompanying the 
* software package from which you obtained this Software (“EULA”). 
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive, 
* non-transferable license to copy, modify and compile the Software source code
* solely for your use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.

* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use 
* of the Software or any product or circuit described in the Software. Cypress
* does not authorize its products for use in any products where a malfunction
* or failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such  
* system or application assumes all risk of such use and in doing so agrees to  
* indemnify Cypress against all liability. 
*******************************************************************************/

/*******************************************************************************
*   Included Headers
*******************************************************************************/
#include "main.h"

#if(UART_DEBUG_ENABLE)
/*******************************************************************************
* Function Name: UART_SendDebugData
********************************************************************************
*
* Summary:
*  Function sends 32-bit count value to PC via UART communication
*
* Parameters:
*  CountValue: 32-bit count value to be sent to PC
*
* Return:
*  None
*
*******************************************************************************/
void UART_SendDebugData(uint32 CountValue)
{
	/* Send upper 16-bits of count */
	UART_PutHexInt(HI16(CountValue));
	
	/* Send lower 16-bits of count */
	UART_PutHexInt(LO16(CountValue));
}
	
/*******************************************************************************
* Function Name: UART_PutHexByte
********************************************************************************
*
* Summary:
*  Sends a byte of data in Hex representation (two characters, uppercase for A-F) 
*  via the Tx pin.
*
* Parameters:
*  TxHexByte: The byte to be converted to ASCII characters and sent via the Tx pin.
*
* Return:
*  None
*
*******************************************************************************/
void UART_PutHexByte(uint8 txHexByte) 
{
    static char8 const CYCODE SW_Tx_UART_hex[] = "0123456789ABCDEF";
    UART_UartPutChar((uint8) SW_Tx_UART_hex[txHexByte >> SW_Tx_UART_BYTE_UPPER_NIBBLE_SHIFT]);
    UART_UartPutChar((uint8) SW_Tx_UART_hex[txHexByte & SW_Tx_UART_BYTE_LOWER_NIBBLE_MASK]);
}
	
/*******************************************************************************
* Function Name: UART_PutHexInt
********************************************************************************
*
* Summary:
*  Sends a 16-bit unsigned integer in Hex representation (four characters,
*  uppercase for A-F) via the Tx pin.
*
* Parameters:
*  TxHexInt: The uint16 to be converted to ASCII characters and sent via
*            the Tx pin.
*
* Return:
*  None
*
*******************************************************************************/
void UART_PutHexInt(uint16 txHexInt) 
{
    UART_PutHexByte((uint8)(txHexInt >> SW_Tx_UART_U16_UPPER_BYTE_SHIFT));
    UART_PutHexByte((uint8)(txHexInt & SW_Tx_UART_U16_LOWER_BYTE_MASK));
}
	
/*******************************************************************************
* Function Name: UART_PutCRLF
********************************************************************************
*
* Summary:
*  Sends a carriage return (0x0D) and a line feed (0x0A) via the Tx pin.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void UART_PutCRLF(void) 
{ 
   UART_UartPutChar(0x0Du);
   UART_UartPutChar(0x0Au);
}
#endif 