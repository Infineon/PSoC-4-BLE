/******************************************************************************
* Project Name		: BLE_Power_Measurement
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
*******************************************************************************/

/********************************************************************************
* Description - Contains all the configuration flags that you could use to 
* enable/disable firmware features.
********************************************************************************/

#if !defined(CONFIGURATION_H)
#define CONFIGURATION_H
    
/*****************************************************
*                  Configuration flags
*****************************************************/
#define ENABLE_900_XTAL_STARTUP     1  /* Required to configure slave clock accuracy to 50ppm. Don't disable this */
#define CON_PARAM_UPDATE            1  /* When enabled, updated BLE connection interval to 1 second */
#define SEND_NOTIFICATIONS          0  /* Enables firmware to send HRM notifications when CCCD is enabled */
#define NOTIF_INTERVAL_FOUR_SEC     0  /* Sends HRM notification once in 4 connection intervals. If connection interval 
                                        * is 1 sec, then the firmware sends notifications every 4 seconds */
#define DEBUG_ENABLE                1  /* Enables GPIO toggling on different power modes. Poor man's power profiler */
#define TX_RX_GPIO_ENABLE           1  /* When enabled, depicts the state of radio Tx and Rx on port 3_2 and port 3_3  
                                        * respectively */
#define DEEPSLEEP_ONLY              0  /* Enable this option to measure the power consumed by PSoC 4 BLE device when in 
                                        * DeepSleep mode */
#define SLEEP_ONLY                  0  /* Enable this option to measure the power consumed by PSoC 4 BLE device when in 
                                        * Sleep mode at operating frequency set by SLEEP_OPERATING_FREQUENCY (see below)*/ 
    
#if SLEEP_ONLY
#define SLEEP_OPERATING_FREQUENCY   16 /* IMO frequency in MHz for which PSoC 4 BLE device sleep current is to be measured */     
#endif    

#if CON_PARAM_UPDATE
    #define NOTIFICATION_OFFSET     10 /* When to start sending notification after CCCD is enabled */
#else
    #define NOTIFICATION_OFFSET     1
#endif /* End of #if CON_PARAM_UPDATE */     
    
#endif /* End of #if !defined(CONFIGURATION_H) */

/* [] END OF FILE */
