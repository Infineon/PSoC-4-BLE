/*******************************************************************************
* File Name: app_UART.h
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

#if !defined (APP_UART_H)
    
    #define APP_UART_H
    
    #include <project.h>
    #include "main.h"    
    #include "stdbool.h"  
    
    /***************************************
    *       Constants
    ***************************************/    
    #define UART_IDLE_TIMEOUT           1000
    #define UART_RX_INTR_MASK     0x00000004
    
    /***************************************
    *       External data references
    ***************************************/
    extern uint16 mtuSize;
    extern uint16 txCharHandle;
    extern uint16 rxCharHandle; 
     
    /***************************************
    *       Function Prototypes
    ***************************************/
    void HandleUartTxTraffic(void);
    void HandleUartRxTraffic(CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *);
    void DisableUartRxInt(void);
    void EnableUartRxInt(void);
    
#endif
/* [] END OF FILE */
