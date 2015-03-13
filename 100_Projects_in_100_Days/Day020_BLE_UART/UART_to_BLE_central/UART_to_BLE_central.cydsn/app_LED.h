/*******************************************************************************
* File Name: app_LED.h
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

#if !defined(APP_LED_H)

    #define APP_LED_H
    
    #include "main.h"
    #include <project.h>    

    /***************************************
    *       Constants
    ***************************************/  
    #define LED_OFF             0x01
    #define LED_ON              0x00
    
    #ifdef LOW_POWER_MODE
        #define SCAN_LED_TIMEOUT    3
        #define CONN_LED_TIMEOUT    30
    #else
        #define SCAN_LED_TIMEOUT    30000
        #define CONN_LED_TIMEOUT    50000
    #endif
    
    /***************************************
    *       Function Prototypes
    ***************************************/
    void HandleLeds();  
    
#endif

/* [] END OF FILE */
