/******************************************************************************
* Project Name		: PSoC_4_BLE_RGB_Power_LED_Control
* File Name			: main.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : ROIT
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
*	Contains all macros and function declaration used in the main.c file 
********************************************************************************/
#if !defined(MAIN_H)
#define MAIN_H

#include <project.h>
#include <BLEApplications.h>
#include <HandleLowPower.h>

/***************************Macro Declarations*******************************/
/* Respective indexes of color coordiantes in the 4-byte data received
* over RGB LED control characteristic */
#define RED_INDEX						0
#define GREEN_INDEX						1
#define BLUE_INDEX						2
#define INTENSITY_INDEX					3
	
/* PrISM Density value for Max intensity. Note that the LED on the BLE Pioneer 
* kit is active Low, so the correct value to drive to maximum intensity is 0.
* This is taken care by the UpdateRGBled() function */
#define RGB_LED_MAX_VAL					255

/* PrISM density value for LED OFF. Note that LED on BLE Pioneer kit is
* active low */
#define RGB_LED_OFF						255
	
/* PrISM density value for LED ON. Note that LED on BLE Pioneer kit is
* active low */
#define RGB_LED_ON						0

/* Firmware thresholds for the two extremes of LED intensity */
#define LED_FULL_INTENSITY				0xF0
#define LED_NO_COLOR_THRESHOLD			0x04

/* General Macros */
#define TRUE							1
#define FALSE							0
#define ZERO							0
/****************************************************************************/
	
/**************************Function Declarations*****************************/
CY_ISR_PROTO(MyISR);
void InitializeSystem(void);
void UpdateRGBled(void);
/****************************************************************************/
#endif
/* [] END OF FILE */
