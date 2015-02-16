/****************************************************************************
* File Name: CommonFunctions.c
*
* Version: 1.0
*
* Description:
* This file implements the common functions used by the application.
*
* Note:
* 
* Owner:
* PMAD
*
******************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
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
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/


/*****************************************************************************
* Included headers
*****************************************************************************/
#include "CommonFunctions.h"
#include "BLE_HTSS.h"

/*****************************************************************************
* Macros and constants
*****************************************************************************/
#define             DRIVE_HIGH              (1u)
#define             DRIVE_LOW               (0u)

/* System Flag used to indicate system status */
uint32 SystemFlag = 0;

/*****************************************************************************
* Function Definitions
*****************************************************************************/

/*****************************************************************************
* Public Functions
*****************************************************************************/

/*****************************************************************************
* Function Name: InitializeSystem()
******************************************************************************
* Summary:
* Initializes the complete system resources. 
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Initializes the system resources - enables BLE, starts ADC, clear SW 
* interrupt and drives the reference signals for temperature measurement. 
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void InitializeSystem (void)
{
    /* Clear wakeup pin interrupt */
    SW_ClearInterrupt();
    
    /* Generate reference signals for temperature measurement */
    V_HIGH_Write(DRIVE_HIGH);
    V_LOW_Write(DRIVE_LOW);
    
    /* Enable BLE and related resouces */
    EnableBLE();
    
    /* Enable ADC for temperature measurement */
    ADC_Start();
}

/*****************************************************************************
* Function Name: PrepareForDeepSleep()
******************************************************************************
* Summary:
* Prepare the system for deep sleep
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Disables the block to prepare the system for deep sleep 
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void PrepareForDeepSleep (void)
{
    /* Disable reference signal to ensure no leakage currents */
    V_HIGH_Write(DRIVE_LOW);
    V_LOW_Write(DRIVE_LOW);
    
    /* If ADC is enabled then disable ADC */
    /* Note: Calling ADC_Sleep twice will disable ADC. ADC_Start function 
    *  should be called instead of ADC_Wakeup function to re-enable the ADC 
    */
    if (ADC_SAR_CTRL_REG  & ADC_ENABLE)
    {
        ADC_Sleep();
    }
}

/*****************************************************************************
* Function Name: WakeupFromDeepSleep()
******************************************************************************
* Summary:
* Wakes up the system from deep sleep
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Enables the reference signal and start ADC for temperature measurement.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void WakeupFromDeepSleep (void)
{
    /* Generate reference signals for temperature measurement */
    V_HIGH_Write(DRIVE_HIGH);
    V_LOW_Write(DRIVE_LOW);
    
    /* Enable ADC for temperature measurement */
    ADC_Wakeup();
}

/* [] END OF FILE */
