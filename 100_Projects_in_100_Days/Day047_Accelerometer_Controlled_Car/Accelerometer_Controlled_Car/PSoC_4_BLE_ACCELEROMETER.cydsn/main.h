/*******************************************************************************
* File Name: main.h
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef MAIN_H

    #define MAIN_H
    
    #include <project.h>
    #include "stdbool.h"
    #include "app_Ble.h"
    
    
     
    /***************************************
    *       Function Prototypes
    ***************************************/
    void AppCallBack(uint32 , void *);  
	void MangeSystemPower();
	
	/***************************************
    *       Variables
    ***************************************/
	uint8 connStatus;
	CYBLE_LP_MODE_T lpMode;
 	CYBLE_BLESS_STATE_T blessState;
 	uint8 interruptStatus;
	
	/***************************************
    *       Constants
    ***************************************/
    #define ACTIVE 1
	#define WAKEUP_SLEEP 2
	#define WAKEUP_DEEPSLEEP 3
	#define SLEEP 4
	#define DEEPSLEEP 5
	#define CONNECTED 1
	
	
#endif

/* [] END OF FILE */
