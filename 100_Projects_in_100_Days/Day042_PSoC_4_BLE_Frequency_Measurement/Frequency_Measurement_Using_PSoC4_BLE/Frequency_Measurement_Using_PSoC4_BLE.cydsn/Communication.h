/***************************************************************************//**
* @file    Communication.h
* @version 1.0
* @author  Chethan (dche@cypress.com)
*
* @par Description
*    This file contains the constants & function prototypes for UART communication 
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 BLE Pioneer Kit
*
* @par References
*    1. 001-32359: AN2283 "PSoC1: Frequency Measurement"
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
*   Macros and #define Constants
*******************************************************************************/
#ifndef __COMMUNICATION_H
#define __COMMUNICATION_H

/*******************************************************************************
*   Included Headers and Macros
*******************************************************************************/
#include <project.h>
	
/* Macro to enable or disable sending debug data to PC via UART */
#define UART_DEBUG_ENABLE 		(1)	

#if(UART_DEBUG_ENABLE)
	/* Macros used for UART communication */
	#define SW_Tx_UART_BYTE_UPPER_NIBBLE_SHIFT        (0x04u)
	#define SW_Tx_UART_U16_UPPER_BYTE_SHIFT           (0x08u)
	#define SW_Tx_UART_BYTE_LOWER_NIBBLE_MASK         (0x0Fu)
	#define SW_Tx_UART_U16_LOWER_BYTE_MASK            (0xFFu)
	
/*******************************************************************************
*   UART communication function prototypes
*******************************************************************************/	
	/* API sends the counter latched data and calculated frequency to PC via UART */
	void UART_SendDebugData(uint32 CountValue);

	/* API sends a byte of data in hex format to PC via UART */
	void UART_PutHexByte(uint8 txHexByte);

	/* API sends a 16-bit integer in hex representation to PC via UART */
	void UART_PutHexInt(uint16 txHexInt);

	/* API sends carriage return (0x0D) and a line feed (0x0A) to PC via UART */
	void UART_PutCRLF(void); 

#endif /* end of #if(UART_DEBUG_ENABLE)*/
#endif /* End of #ifndef __COMMUNICATIONS_H */
/* END OF FILE */
