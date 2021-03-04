/******************************************************************************
* Project Name		: PSoC_4_BLE_CapSense_Proximity
* File Name			: BLEApplications.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: ROIT
*
********************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation. All Rights Reserved.
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
* Contains all macros and function declaration used in the BLEApplications.c 
* file.
********************************************************************************/
#if !defined(BLEAPPLICATIONS_H)
#define BLEAPPLICATIONS_H
#include <project.h>

/*************************Pre-processor directives****************************/
/* 'ENABLE_LOW_POWER_MODE' pre-processor directive enables the low power mode 
* handling in the firmware, ensuring low power consumption during project usage.
* To disable, comment the following #define. 
* If disabled, prevent usage of the project with coin cell */
#define ENABLE_LOW_POWER_MODE
	
/* 'CAPSENSE_ENABLED' pre-processor directive enables CapSense functionality 
* in the firmware. To disable Capsense, comment the following #define as well 
* as disable the CapSense component from the TopDesign */
#define CAPSENSE_ENABLED
/****************************************************************************/
	
/**************************Function Declarations*****************************/
void CustomEventHandler(uint32 event, void * eventParam);
void SendDataOverCapSenseNotification(uint8 proximityValue);
void UpdateNotificationCCCD(void);
void HandleStatusLED(void);
/****************************************************************************/

/***************************Macro Definitions*******************************/
/* Data length of CapSense Proximity data sent over notification */
#define CAPSENSE_NOTIFICATION_DATA_LEN		1

/* Bit mask for notification bit in CCCD (Client Characteristic 
* Configuration Descriptor) written by Client device. */
#define CCCD_NTF_BIT_MASK					0x01

/* Client Characteristic Configuration descriptor data length. This is defined
* as per BLE spec. */
#define CCC_DATA_LEN						2

/* LED Blink rate values for different stages of BLE connection */
#ifdef ENABLE_LOW_POWER_MODE
#define	LED_ADV_BLINK_PERIOD_ON			8000    
#define LED_ADV_BLINK_PERIOD_OFF		8000
#else
#define	LED_ADV_BLINK_PERIOD_ON			10000
#define LED_ADV_BLINK_PERIOD_OFF		15000
#endif

/* Macros for LED ON and OFF values */
#define LED_ON							0x00
#define LED_OFF							0x01
/****************************************************************************/

#endif
/* [] END OF FILE */
