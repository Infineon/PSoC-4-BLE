/******************************************************************************
* Project Name		: PSoC4_BLE_RTC
* File Name			: Configuration.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: kris@cypress.com
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************
******************************************************************************
*                           Description
*******************************************************************************
*  Firmware feature enable/disable constants
*
*******************************************************************************/

#if !defined(CONFIGURATION_H)
#define CONFIGURATION_H
    
/***************************************
*    Project configuration flags
***************************************/ 
#define BLE_GATT_CLIENT_ENABLE                      (1u) /* Enables the GATT client to discover and fetch time from the 
                                                          * peer device's GATT current timer server */
    
#define RTC_ENABLE                                  (1u) /* Enable RTC operation in firmware */
    
#define DISCONNECT_BLE_AFTER_TIME_SYNC              (1u) /* After the RTC is synced with BLE time server's time,
                                                          * disconnect the link to reduce system power consumption */
    
#define RESTART_ADV_ON_DISCONNECTION                (0u) /* After the BLE interface is disconnected, restart the 
                                                          * advertisement */
    
#define CONSOLE_LOG_ENABLED                         (1u) /* Enable UART console logging */
    
#if CONSOLE_LOG_ENABLED    
#define DISPLAY_ON_BUTTON_PRESS                     (1u) /* When UART logging is enabled, on the press of user button
                                                          * display the RTC time on the UART interface */
#endif    
  
/***************************************
*    constants
***************************************/
#define TRUE 1
#define FALSE 0
    
#endif /* End of #if !defined(CONFIGURATION_H) */
