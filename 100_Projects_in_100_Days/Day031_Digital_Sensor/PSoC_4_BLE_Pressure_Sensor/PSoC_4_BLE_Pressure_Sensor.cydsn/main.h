/******************************************************************************
* Project Name		: PSoC_4_BLE_Pressure_Sensor
* File Name			: main.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit
*******************************************************************************/

/***********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
************************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
************************************************************************************/

#if !defined(MAIN_H)
#define MAIN_H

#include <common.h>
#include <BLEApplication.h>
#include <I2CMaster.h>

/*************************Macro Definitions**********************************/
#define LED_DELAY_COUNT 0x32 //Counter value for LED Delay
#define NOTIFICATION_DELAY_COUNT 0x20 //Counter value for Notification Delay
/*****************************************************************************/

/*************************Function Prototypes*******************************/
void InitializeSystem(void);
void ReadSensorDataAndNotify(void);
void HandleStatusLED(void);
void EnterExitLowPowerMode(void);
CY_ISR_PROTO(ButtonISR);
/*****************************************************************************/
#endif
/* [] END OF FILE */
