/*******************************************************************************
* File Name: debug.h
*
* Version: 1.0
*
* Description:
*  This is header file for debug.c.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(DEBUG_H)
#define DEBUG_H
#include <main.h>
	
void SendBLEStatetoUART(CYBLE_STATE_T );
void PrintNum(uint8);
void PrintHex(uint8);

#endif
/* [] END OF FILE */
