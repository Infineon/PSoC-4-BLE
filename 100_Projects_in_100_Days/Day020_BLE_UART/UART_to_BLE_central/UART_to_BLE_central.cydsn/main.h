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

#if !defined(MAIN_H)

    #define MAIN_H
    
    #include <project.h>
    #include "app_Ble.h"
    #include "app_LED.h"
    #include "stdbool.h"    

    /***************************************
    *   Conditional compilation parameters
    ***************************************/      
//    #define     FLOW_CONTROL
    #define     PRINT_MESSAGE_LOG
//    #define     LOW_POWER_MODE
    
    /***************************************
    *       Function Prototypes
    ***************************************/
    void AppCallBack(uint32 , void *);  
    
#endif

/* [] END OF FILE */
