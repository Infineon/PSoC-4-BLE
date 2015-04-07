/******************************************************************************
* Project Name		: PSoC4_BLE_RTC
* File Name			: Init.c
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
* This file contains system initialization routine
*
*******************************************************************************/

#include <BLE Connection.h>
#include <Button.h>
#include <Configuration.h>
#include <Project.h>
#include <RTC.h>
#include <Stdio.h>

/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
*
* Summary:
*  Systm initialization function.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
void InitializeSystem(void)
{
    CyGlobalIntEnable;      /* Enable Global Interrupts*/
    
    CySysClkIloStop();      /* WCO is up and running, ILO is no longer required */
    
    BLE_Engine_Start();     /* Kick start the BLE GATT server and client interface */
    
    RTC_Interrupt_StartEx(WDT_Handler); /* Initialize RTC interrupt handler */

#if CONSOLE_LOG_ENABLED
    Debug_Console_Start();  /* Consolle log interface */
    printf("=====BLE RTC Example=====\r\n\n");
#endif /* End of #if CONSOLE_LOG_ENABLED */

#if DISPLAY_ON_BUTTON_PRESS
    Button_Start();
#endif /* End of #if DISPLAY_ON_BUTTON_PRESS */

} 

/* [] END OF FILE */
