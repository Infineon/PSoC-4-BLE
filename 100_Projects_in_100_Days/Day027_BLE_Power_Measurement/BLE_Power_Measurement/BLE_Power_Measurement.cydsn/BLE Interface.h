/******************************************************************************
* Project Name		: BLE_Power_Measurement
* File Name			: BLE Interface.h
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

/******************************************************************************
* Description - Header file for BLE interface specific routines and macros
*******************************************************************************/

#if !defined(BLE_INTERFACE_H)
#define BLE_INTERFACE_H
    
#include <project.h>    
    
/*****************************************************
*                  Enums and macros
*****************************************************/    
#define CRYSTAL_STARTUP_TIME                        (15u) /* Startup time = 15 * 61us */    
    
#define MINIMUM_CONNECTION_INTERVAL                 (790u) /* 790 * 1.25 = 987.5ms */
#define MAXIMUM_CONNECTION_INTERVAL                 (800u) /* 800 * 1.25 = 1000ms */
#define SLAVE_LATENCY                               (0u)   /* No slave latency */
#define SUPERVISION_TIMEOUT                         (500u) /* Supervision timeout of 5 seconds */    
    
#define INITIAL_HEART_RATE_VALUE                    (60u) /* Initial value of notification to be used */    
    
/*****************************************************
*                  Function declarations
*****************************************************/    
CYBLE_API_RESULT_T BLE_Engine_Start(void);
void BLE_Run(void);
#define BLE_ManagePower()                           CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP)
    
#endif /* End of #if !defined(BLE_INTERFACE_H) */

/* [] END OF FILE */
