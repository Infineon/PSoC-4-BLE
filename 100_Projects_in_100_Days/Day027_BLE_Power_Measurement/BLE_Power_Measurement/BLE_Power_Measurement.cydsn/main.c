/******************************************************************************
* Project Name		: BLE_Power_Measurement
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
* This is a template project that you can use to achieve best possible system
* power in your BLE system. The project demonstrates how and when to configure
* the PSoC 4 BLE device in sleep and deepsleep modes during and after the BLE
* RF event. It also provides hooks for other peripherals in your application 
* to be plugged-in to and achieve best overall system power.
*
* Multiple debugging options are provided for you to observe the health of the 
* system when you modify the template to fit your application needs.
*
******************************************************************************
*                         HARDWARE CONFIGURATION
*******************************************************************************
* Look at the TopDesign.cysch file for a description of hardware and test setup
* for power measurement.
******************************************************************************/

/******************************************************************************
*                           Header Files
*******************************************************************************/
#include <Application.h>
#include <BLE Interface.h>
#include <Configuration.h>
#include <Init.h>
#include <LowPower.h>
#include <project.h>

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        System entry point. This calls the BLE and other peripheral Component
* APIs for achieving the desired system behaviour
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main()
{
    /* This function will initialize all the Components and system variables */
	InitializeSystem();

    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
        /***********************************************************************
        *  Process all the pending BLE events in the stack
        ***********************************************************************/
        BLE_Run();
         
        /***********************************************************************
        *  Put BLE sub system in DeepSleep mode when it is idle
        ***********************************************************************/ 
        BLE_ManagePower();
    
        /***********************************************************************
        *  Run your application specific code here
        ***********************************************************************/  
        if(Application_GetPowerState() == ACTIVE)
        {
            Application_Run();
        }
        
        /***********************************************************************
        *  Process application and system power modes 
        ***********************************************************************/
        Application_ManagePower();   
        
        System_ManagePower();
        
    }	/* End of while(1) */
    
} /* End of main */

/* [] END OF FILE */
