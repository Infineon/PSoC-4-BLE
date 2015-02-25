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
#ifndef _MAIN_H_
#define _MAIN_H_
#include "project.h"
#include "LED.h"	
    
extern uint8 Periph_Selected;
extern uint8 devIndex;
	
extern uint32  UartRxDataSim;
extern uint8 IsSelected;
extern uint8 IsDetected;
extern uint8 IsConnected;
extern CYBLE_CONN_HANDLE_T connHandle;
    
#endif	

/* [] END OF FILE */
