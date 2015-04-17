/******************************************************************************
* Project Name		: PSoC_4_BLE_LowPowerModes
* File Name			: BLEApplications.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : ANKU
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
#include <cytypes.h>
    
/*************************Pre-processor directives****************************/
/* 'ENABLE_LOW_POWER_MODE' pre-processor directive enables the low power mode 
* handling in the firmware, ensuring low power consumption during project usage.
* To disable, comment the following #define. 
* If disabled, prevent usage of the project with coin cell */
#define ENABLE_LOW_POWER_MODE
    
/* 'ENABLE_LED_NOTIFICATION' pre-processor directive enables the LED 
* handling in the firmware, ensuring that different LED indications 
* are provided in different power modes during project usage.
* To disable, comment the following #define. 
* If disabled, prevent usage of the project with coin cell */    
#define ENABLE_LED_NOTIFICATION
/****************************************************************************/  
    
/**************************Function Declarations*****************************/
void CustomEventHandler(uint32 event, void * eventParam);
void SendDataOverLPMselNotification(uint8 *LPMselData, uint8 len);
void UpdateNotificationCCCD(void);
void UpdateConnectionParam(void);
void HandleStatusLED(void);
void WDT_INT_Handler(void);
void InitializeWatchdog(void);
/****************************************************************************/
	
/***************************Macro Declarations*******************************/
/* Read and write length of LPM LED data */
#define LPM_CHAR_DATA_LEN					1
											
/* Client Characteristic Configuration descriptor data length. This is defined
* as per BLE spec. */
#define CCCD_DATA_LEN						2

/* Bit mask for notification bit in CCCD (Client Characteristic 
* Configuration Descriptor) written by Client device. */
#define CCCD_NTF_BIT_MASK					0x01

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
	
/* The 16 bit Counter 0 in Watchdog is used for counting time to keep
* the Status LED on for pre-detremined time. The count value is set for 
* 1 second and 3 such interrupts are used to select 3 seconds of ON time */
#define WATCHDOG_ONE_SEC_COUNT_VAL			32768

/* Time (in seconds)for which the LED is to be kept ON after connection */
#define LED_CONN_ON_TIME					2

/* Watchdog Interrupt Vector number in PSoC 4 BLE. See PSoC 4 BLE TRM for
* details */ 
#define WATCHDOG_INT_VEC_NUM				8
											
/* Desired Watchdog Interrupt priority */ 
#define WATCHDOG_INT_VEC_PRIORITY			3

/* After Resetting or Writing the Counter 0 register with match value, it takes
* ~3 LFCLK cycles to take effect. Provide a delay of 100 us after above actions
* for changes to take effect */
#define WATCHDOG_REG_UPDATE_WAIT_TIME		100
											
/* Number of loops indicating LED Blink rate values for different stages of 
* BLE connection */
#ifdef ENABLE_LOW_POWER_MODE
#define	LED_ADV_BLINK_PERIOD_ON			4
#define LED_ADV_BLINK_PERIOD_OFF		20
#else
#define	LED_ADV_BLINK_PERIOD_ON			10000
#define LED_ADV_BLINK_PERIOD_OFF		15000
#endif

/* Period for which the LPM LED should be kept ON from the time last 
* data was sent. The time is this period plus 1 second. */
#define LED_OFF_TIME_PERIOD				2
/****************************************************************************/

/**************************Extern Variables***********************************/
extern uint8 LPMselData; 
/*****************************************************************************/  

#endif
/* [] END OF FILE */
