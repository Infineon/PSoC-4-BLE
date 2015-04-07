/******************************************************************************
* Project Name		: PSoC4_BLE_RTC
* File Name			: main.c
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
*                           THEORY OF OPERATION
*******************************************************************************
* This project demonstrates PSoC 4 BLE acting as a BLE time client that syncs 
* time from an iOS device (iOS support time server out of the box) and runs an 
* RTC in PSoC 4 BLE.
*
*******************************************************************************
*                                 NOTES
*******************************************************************************
* This project will best work with iOS devices as iOS supports Current Time 
* Service (CTS) out of the box. On othee BLE Central devices (Android, Cypress's
* CySmart Host Emulation tool etc.), the RTC will start from time 0 after it 
* discovers that CTS client is not supported by the peer device.
*
* Device "Debug Select" option in project's .cydwr file's "system" tab is set to 
* GPIO to reduce power consumption of the device. If you like to debug this 
* example project, change the "Debug Select" option to SWD.
*
* System heap memory size is set to 1K in .cydwr -> system tab. This is required
* for using printf function in this example. Set the heap size to 0 (to reduce 
* SRAM consumption of the project) if you have disabled the console log
* 
******************************************************************************/

#include <BLE Connection.h>
#include <Init.h>
#include <LowPower.h>
#include <project.h>
#include <RTC.h>

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function.
*
* Parameters:  
*  None
*
* Return: 
*  Never
*
*******************************************************************************/
int main(void)
{
    InitializeSystem();
	
    while(1)
    {
        /* Handles any pending BLE events and allows data communication over BLE. Must be called in system main loop */
        BLE_Run(); 
        
        /* Configure the system in lowest possible mode between BLE and RTC events */
        System_ManagePower();
        
        /* Update current RTC value on the UART console */
        RTC_UI_Update();
    }
}

/* [] END OF FILE */
