/*******************************************************************************
* File Name: WDT.h
*
* Version: 1.0
*
* Description:
*  This file contains the headers and constants for WDT API.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(WDT_H)
#define WDT_H
#include "project.h"

/*****************************************************
*                  Enums and macros
*****************************************************/ 
#define WATCHDOG_COUNT_VAL					3276
#define WATCHDOG_REG_UPDATE_WAIT_TIME		90

/*****************************************************
*                  Function Declarations
*****************************************************/
void InitializeWatchdog(uint32 match_val);
void WDT_INT_Handler(void);
#endif
/* [] END OF FILE */
