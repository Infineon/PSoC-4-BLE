/*******************************************************************************
* File Name: main.h
*
* Version: 1.0
*
* Description:
*  This is header file containing function declarations for all application related
* functions as well as Macros.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(MAIN_H)
#define MAIN_H
#include <project.h>
#include <debug.h>
#include <ble_process.h>
#include <WDT.h>

/*****************************************************
*             Pre-processor Directives
*****************************************************/ 
//#define DEBUG_ENABLED
#define ENABLE_ADV_DATA_COUNTER
//#define ENABLE_CENTRAL_DISCOVERY

/*****************************************************
*                  Variables
*****************************************************/ 
volatile uint16 current_watchdog_counter;

/*****************************************************
*                  Enums and macros
*****************************************************/ 
#define TRUE								1
#define FALSE								0
#define BLE_PERIPHERAL						1
#define BLE_CENTRAL							2
	
#define CENTRAL_STATE_SPAN					45
#define PERIP_RGB_HOLD_TIME					10

#define WatchDog_CurrentCount()				(current_watchdog_counter)

/*****************************************************
*                  Function Declarations
*****************************************************/
void InitializeSystem(void);
void UpdateRGBled(uint8*, uint8);

#endif

/* [] END OF FILE */
