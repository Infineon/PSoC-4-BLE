/******************************************************************************
* Project Name		: BLE_Dongle_Peripheral_IAS
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CYBL10162-56LQXI
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CySmart USB Dongle (part of CY8CKIT-042-BLE Bluetooth Low 
*                     Energy Pioneer Kit) 
* Owner				: ROIT
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
/******************************************************************************
*                           THEORY OF OPERATION
*******************************************************************************
* This project works on CySmart USB Dongle and acts as a Find Me Reporter. 
* After the Central devices (PSoC_4_BLE_Central_IAS project on BLE Pioneer Kit)
* are connected, the Push Button on the BLE Pioneer Kit will rotate the Alert 
* level between No, Mid and High Alert on CySmart USB Dongle, which is demonstrated 
* by the different method of blinking LED. 
* Refer to BLE Pioneer Kit user guide for details.
*******************************************************************************
* Hardware connection required for testing -
* Status LED- P3[3] (Hard-wired on the CySmart USB Dongle)
******************************************************************************/
#include <main.h>

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  System entry and execution point for application
*
* Parameters:  
*  None
*
* Return: 
*  int
*
*******************************************************************************/
int main()
{
	/* Start the components */
	InitializeSystems();
    
    for(;;)
    {
        /* CyBle_ProcessEvents() allows BLE stack to process pending events */
        CyBle_ProcessEvents();
    }
}

/*******************************************************************************
* Function Name: InitializeSystems
********************************************************************************
*
* Summary:
*  Starts all components and initializes as required.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
void InitializeSystems(void)
{
	/* Enable Global Interrupt */
	CyGlobalIntEnable;
	
	/* Start PWM. This PWM controls the LED status */
	LED_PWM_Start();
	
    /* Start CYBLE component and register generic event handler */
    CyBle_Start(GenericAppEventHandler);
	
    /* register the event handler for IAS specific events */
    CyBle_IasRegisterAttrCallback(iasEventHandler);
	
	/* Set drive mode of Alert LED pin to High-Z to shutdown LED */
	Alert_LED_SetDriveMode(Alert_LED_DM_ALG_HIZ);
}

/* [] END OF FILE */
