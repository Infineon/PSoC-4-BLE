/*****************************************************************************
* File Name: MyIsr.c
*
* Version: 1.0
*
* Description:
* This file contains the custom ISR for the ADC.
*
* Note:
* 
* Owner:
* UDYG
*
* Related Document:
* PSoC 4 BLE Heart Rate Monitor Solution Demo IROS: 001-92353
*
* Hardware Dependency:
* PSoC 4 BLE HRM Solution Demo Board
*
* Code Tested With:
* 1. PSoC Creator 3.1
* 2. ARM-GCC Compiler
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
#include "CompileTimeOptions.h"
#include <cytypes.h>
#include <stdbool.h>
#include "ADC.h"


/*****************************************************************************
* Global variables
*****************************************************************************/
volatile bool canMeasureBattery = false;
volatile bool canMeasureHeartRate = false;


/*****************************************************************************
* Function Definitions
*****************************************************************************/


/*****************************************************************************
* Function Name: MyAdcIsr()
******************************************************************************
* Summary:
* ISR for the SAR-ADC.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* 1. Sets the heart rate measurement go-ahead flag. 
* 2. Sets the battery measurement go-ahead flag.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
#if (HRM_EN || BATTERY_EN) 
CY_ISR(MyAdcIsr)
{
#if (BATTERY_EN)
    canMeasureBattery = true;
#endif  /* #if (BATTERY_EN) */

#if (HRM_EN)
    canMeasureHeartRate = true;
#endif  /* #if (HRM_EN) */

    /* Clear pending interrupt */
    ADC_SAR_INTR_REG |= 0x01;
}
#endif  /* #if (HRM_EN || BATTERY_EN) */


/* [] END OF FILE */
