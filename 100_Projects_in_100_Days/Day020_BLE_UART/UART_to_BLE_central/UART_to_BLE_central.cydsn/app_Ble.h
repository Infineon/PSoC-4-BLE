/*******************************************************************************
* File Name: app_Ble.h
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

#if !defined(APP_BLE_H)
    
    #define APP_BLE_H
    
    #include <project.h>
    #include "app_UART.h"
    #include "stdbool.h"
    
    /***************************************
    *   Enumerated Types and Structs
    ***************************************/
    typedef enum 
    {
        INFO_EXCHANGE_START             = 0x00,
        BLE_UART_SERVICE_HANDLE_FOUND   = 0x01,
        TX_ATTR_HANDLE_FOUND            = 0x02,
        RX_ATTR_HANDLE_FOUND            = 0x04,
        TX_CCCD_HANDLE_FOUND            = 0x08,
        MTU_XCHNG_COMPLETE              = 0x10,
        
        SERVICE_AND_CHAR_HANDLES_FOUND  = BLE_UART_SERVICE_HANDLE_FOUND | \
                                                TX_ATTR_HANDLE_FOUND | RX_ATTR_HANDLE_FOUND,
        
        ALL_HANDLES_FOUND               = BLE_UART_SERVICE_HANDLE_FOUND | \
                                                TX_ATTR_HANDLE_FOUND | RX_ATTR_HANDLE_FOUND | \
                                                TX_CCCD_HANDLE_FOUND,
        
        INFO_EXCHANGE_COMPLETE          = BLE_UART_SERVICE_HANDLE_FOUND | \
                                                TX_ATTR_HANDLE_FOUND | RX_ATTR_HANDLE_FOUND | \
                                                TX_CCCD_HANDLE_FOUND | MTU_XCHNG_COMPLETE
        
    } INFO_EXCHANGE_STATE_T;
    
    /***************************************
    *       Function Prototypes
    ***************************************/
    void HandleBleProcessing(void);
    void attrHandleInit(void);
    void enableNotifications(void);
    
#endif

/* [] END OF FILE */
