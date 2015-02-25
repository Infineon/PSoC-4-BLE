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
#ifndef _BLE_UART_CLIENT_H_
#define _BLE_UART_CLIENT_H_
	
#include "project.h"	
#include "stdio.h"
#include "debug.h"	
#include "main.h"	
	
	
void StackEventHandler(uint32 event, void *eventparam);	

void Handle_ble_Central_Observer_State(void);
	
#endif


/* [] END OF FILE */
