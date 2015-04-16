/******************************************************************************
* Project Name		: HueControl
* File Name			: BLEApplications.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : PRIA
*
********************************************************************************
* Copyright (2014-15), Cypress Semiconductor Corporation. All Rights Reserved.
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
*	Contains  macros and function declaration used in the BLEApplication.c file 
********************************************************************************/
#if !defined(BLEAPPLICATIONS_H)
#define BLEAPPLICATIONS_H
#include <project.h>	
/****************************************************************************/

/**************************Function Declarations*****************************/
void CustomEventHandler(uint32 event, void * eventParam);
void UpdateConnectionParam(void);
void HandleStatusLED(void);
/****************************************************************************/
	
/***************************Macro Declarations*******************************/
											
/* Connection Update Parameter values to modify connection interval. These values
* are sent as part of CyBle_L2capLeConnectionParamUpdateRequest() which requests
* Client to update the existing Connection Interval to new value. Increasing 
* connection interval will reduce data rate but will also reduce power consumption.
* These numbers will influence power consumption */

/* Minimum connection interval = CONN_PARAM_UPDATE_MIN_CONN_INTERVAL * 1.25 ms*/
#define CONN_PARAM_UPDATE_MIN_CONN_INTERVAL	100        	
/* Maximum connection interval = CONN_PARAM_UPDATE_MAX_CONN_INTERVAL * 1.25 ms */
#define CONN_PARAM_UPDATE_MAX_CONN_INTERVAL	110        	
/* Slave latency = Number of connection events */
#define CONN_PARAM_UPDATE_SLAVE_LATENCY		0          
/* Supervision timeout = CONN_PARAM_UPDATE_SUPRV_TIMEOUT * 10*/
#define CONN_PARAM_UPDATE_SUPRV_TIMEOUT		200     

/* RGB Flood Light control commands sent over BLE */
typedef enum
{
    BLE_LED_INTENSITY_INCREASE  = 0,
    BLE_LED_INTENSITY_DECREASE  = 1,
    BLE_LED_LIGHT_OFF           = 2,
    BLE_LED_LIGHT_ON            = 3,
    BLE_LED_COLOR_RED           = 4,
    BLE_LED_COLOR_GREEN         = 5,
    BLE_LED_COLOR_BLUE          = 6,
    BLE_LED_COLOR_WHITE         = 7,
    BLE_LED_COLOR_RED1          = 8,
    BLE_LED_COLOR_GREEN1        = 9,
    BLE_LED_COLOR_BLUE1         = 10,
    BLE_LED_FLASH               = 11,
    BLE_LED_COLOR_RED2          = 12,
    BLE_LED_COLOR_GREEN2        = 13,
    BLE_LED_COLOR_BLUE2         = 14,
    BLE_LED_STROBE              = 15,
    BLE_LED_COLOR_RED3          = 16, 
    BLE_LED_COLOR_GREEN3        = 17,
    BLE_LED_COLOR_BLUE3         = 18,
    BLE_LED_FADE                = 19,
    BLE_LED_COLOR_YELLOW        = 20,
    BLE_LED_COLOR_GREEN4        = 21,
    BLE_LED_COLOR_PINK          = 22,
    BLE_LED_SMOOTH              = 23
}BLE_RGB_CONTROL_COMMANDS;
											
/* Number of loops indicating LED Blink rate values for different stages of 
* BLE connection */
#define	LED_ADV_BLINK_PERIOD_ON			(10000)
#define LED_ADV_BLINK_PERIOD_OFF		(15000)

/* Byte size*/
#define ONE_BYTE                        (8u)
/****************************************************************************/
#endif
/* [] END OF FILE */
