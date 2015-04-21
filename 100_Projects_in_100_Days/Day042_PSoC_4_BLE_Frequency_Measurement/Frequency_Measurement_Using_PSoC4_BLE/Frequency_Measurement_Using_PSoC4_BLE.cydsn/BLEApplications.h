/***************************************************************************//**
* @file    BLEApplications.h
* @version 1.0
* @author  Chethan (dche@cypress.com)
*
* @par Description
*   This file contains the constants and function prototypes for BLE communication
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 BLE Pioneer Kit
*
* @par References
*    1. 001-32359: AN2283 "PSoC 1 Measuring Frequency"
*
* @par Code Tested With
*    1. PSoC Creator  3.1 SP2 (3.1.0.2177)
*    2. GCC 4.8.4
*
**//****************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
********************************************************************************
* All rights reserved. 
* This software, including source code, documentation and related 
* materials (“Software”), is owned by Cypress Semiconductor 
* Corporation (“Cypress”) and is protected by and subject to worldwide 
* patent protection (United States and foreign), United States copyright 
* laws and international treaty provisions. Therefore, you may use this 
* Software only as provided in the license agreement accompanying the 
* software package from which you obtained this Software (“EULA”). 
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive, 
* non-transferable license to copy, modify and compile the Software source code
* solely for your use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.

* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use 
* of the Software or any product or circuit described in the Software. Cypress
* does not authorize its products for use in any products where a malfunction
* or failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such  
* system or application assumes all risk of such use and in doing so agrees to  
* indemnify Cypress against all liability. 
*******************************************************************************/

/********************************************************************************
* Contains all macros and function declaration used in the BLEApplications.c 
* file.
********************************************************************************/
#ifndef BLEAPPLICATIONS_H
#define BLEAPPLICATIONS_H
	
#include <project.h>

/*************************Pre-processor directives****************************/
/* 'ENABLE_LOW_POWER_MODE' pre-processor directive enables the low power mode 
* handling in the firmware, ensuring low power consumption during project usage.
* To disable, comment the following #define. 
* If disabled, prevent usage of the project with coin cell */
#define ENABLE_LOW_POWER_MODE
	
/****************************************************************************/

/***************************Macro Definitions*******************************/

#define ZERO 								(0)

/* Data length of Frequency value sent over notification */
#define FREQUENCY_NOTIFICATION_DATA_LEN		(10)

/* Bit mask for notification bit in CCCD (Client Characteristic 
* Configuration Descriptor) written by Client device. */
#define CCCD_NTF_BIT_MASK					(0x01)

/* Client Characteristic Configuration descriptor data length. This is defined
* as per BLE spec. */
#define CCC_DATA_LEN						(2)

/* Connection Update Parameter values to modify connection interval. These values
* are sent as part of CyBle_L2capLeConnectionParamUpdateRequest() which requests
* Client to update the existing Connection Interval to new value. Increasing 
* connection interval will reduce data rate but will also reduce power consumption.
* These numbers will influence power consumption */

/* Minimum connection interval = CONN_PARAM_UPDATE_MIN_CONN_INTERVAL * 1.25 ms*/
#define CONN_PARAM_UPDATE_MIN_CONN_INTERVAL	(1600)       
 	
/* Maximum connection interval = CONN_PARAM_UPDATE_MAX_CONN_INTERVAL * 1.25 ms */
#define CONN_PARAM_UPDATE_MAX_CONN_INTERVAL	(1600)        
	
/* Slave latency = Number of connection events */
#define CONN_PARAM_UPDATE_SLAVE_LATENCY		(0)      
    
/* Supervision timeout = CONN_PARAM_UPDATE_SUPRV_TIMEOUT * 10*/
#define CONN_PARAM_UPDATE_SUPRV_TIMEOUT		(600)      

/* LED Blink rate values for different stages of BLE connection */
#ifdef ENABLE_LOW_POWER_MODE
	#define	LED_ADV_BLINK_PERIOD_ON				(5)
	#define LED_ADV_BLINK_PERIOD_OFF			(20)
#else
	#define	LED_ADV_BLINK_PERIOD_ON				(10000)
	#define LED_ADV_BLINK_PERIOD_OFF			(15000)
#endif

/* Macros for LED ON and OFF values */
#define LED_ON							0x00
#define LED_OFF							0x01
/****************************************************************************/

/**************************Function Declarations*****************************/
void InitializeBLESystem(void);
void CustomEventHandler(uint32 event, void * eventParam);
void SendDataOverFreqCounterNotification(uint8* Count_Value);
void UpdateNotificationCCCD(void);
void UpdateConnectionParam(void); 
void HandleStatusLED(void);
/****************************************************************************/
#endif
/* [] END OF FILE */
