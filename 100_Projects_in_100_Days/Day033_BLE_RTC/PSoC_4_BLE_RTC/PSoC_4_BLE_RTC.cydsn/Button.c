/******************************************************************************
* Project Name		: PSoC4_BLE_RTC
* File Name			: Button.c
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
* This file contains the user button handling functions and isrs.
*
*******************************************************************************/

#include <Configuration.h>
#include <project.h>

#if DISPLAY_ON_BUTTON_PRESS
/***************************************
*    Function declarations
***************************************/ 
CY_ISR_PROTO(Button_InterruptHandler);

/***************************************
*    Global variables
***************************************/
static uint8 buttonStatus = 0;

/*******************************************************************************
* Function Name: Button_Start
********************************************************************************
*
* Summary:
*  Starts the user button interrupt
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
void Button_Start(void)
{
    Button_interrupt_StartEx(Button_InterruptHandler);
}

/*******************************************************************************
* Function Name: Button_InterruptHandler
********************************************************************************
*
* Summary:
*  ISR for button press GPIO interrupt
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
CY_ISR(Button_InterruptHandler)
{
    buttonStatus = 1;
    
    Button_interrupt_ClearPending();
    
    User_Button_ClearInterrupt();
}

/*******************************************************************************
* Function Name: Button_IsPressed
********************************************************************************
*
* Summary:
*  Let the application know the state of the user button. It's a clear on read
*  API
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
uint8 Button_IsPressed(void)
{
    uint8 status;
    uint8 apiResult = 0;
    
    status = CyEnterCriticalSection();
    
    apiResult = buttonStatus;
    
    buttonStatus = 0;
    
    CyExitCriticalSection(status);
    
    return apiResult;
}

#endif /* End of #if DISPLAY_ON_BUTTON_PRESS */

/* [] END OF FILE */
