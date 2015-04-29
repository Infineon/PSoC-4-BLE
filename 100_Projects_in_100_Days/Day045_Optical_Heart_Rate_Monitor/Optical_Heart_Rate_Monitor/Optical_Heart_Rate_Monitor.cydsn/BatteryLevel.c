/*****************************************************************************
* File Name: BatteryLevel.c
*
* Version: 1.0
*
* Description:
* This file handles the battery measurement functionality.
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
#include "MyIsr.h"
#include "ADC.h"
#include "VrefInputPin.h"


/*****************************************************************************
* Macros
*****************************************************************************/
#define VREF_VOLTAGE_MV                 (1024)
#define ADC_FULL_SCALE_OUT              (2047)


/*****************************************************************************
* Global variables
*****************************************************************************/
uint8 batteryLevel = 0;


/*****************************************************************************
* Function Definitions
*****************************************************************************/

/*****************************************************************************
* Function Name: BatteryLevel_Measure()
******************************************************************************
* Summary:
* Measures the current battery level.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function checks if the battery measurement enable flag is set in the 
* ADC ISR, and then measures the current battery level.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void BatteryLevel_Measure(void)
{
    uint16 adcCountsVref;
    static uint32 vddaVoltageMv;
    
    /* Disconnect the VREF pin from the chip and lose its existing voltage */
    CY_SET_REG32(CYREG_SAR_CTRL, CY_GET_REG32(CYREG_SAR_CTRL) & ~(0x01 << 7));
    VrefInputPin_SetDriveMode(VrefInputPin_DM_STRONG);
    VrefInputPin_Write(0);
    CyDelayUs(10);
    
    /* Switch SAR reference to 1.024V to charge external cap */
    VrefInputPin_SetDriveMode(VrefInputPin_DM_ALG_HIZ);
    CY_SET_REG32(CYREG_SAR_CTRL, (CY_GET_REG32(CYREG_SAR_CTRL) & ~(0x000000F0Lu)) | (0x00000040Lu) | (0x01Lu << 7));
    CyDelayUs(100);
    
    /* Switch the reference back to VDDA/2 for measuring the REF voltage */
    CY_SET_REG32(CYREG_SAR_CTRL, CY_GET_REG32(CYREG_SAR_CTRL) & ~(0x01 << 7));
    CY_SET_REG32(CYREG_SAR_CTRL, (CY_GET_REG32(CYREG_SAR_CTRL) & ~(0x00000070Lu)) | (0x00000060Lu));
    
    /* Enable channel 1 of the ADC, disable channel 0 */
    ADC_SetChanMask(0x02);
    
    /* Clear ADC interrupt triggered flag and start a new conversion */
    canMeasureBattery = false;
    ADC_StartConvert();
    while(true != canMeasureBattery);

    /* Since our ADC reference is VDDA/2, we get full scale (11-bit) at VDDA/2.
     * We can calculate VDDA by the formula:
     * VDDA = (VREF * (Full scale ADC out) * 2) / (ADC out for VREF)
     */
    adcCountsVref = ADC_GetResult16(1);
    if(adcCountsVref != 0)
    {
        vddaVoltageMv = ((uint32)VREF_VOLTAGE_MV * ADC_FULL_SCALE_OUT * 2) / (uint32)adcCountsVref;
    }
    
    /* Battery level is implemented as a linear plot from 2.0V to 3.0V 
     * Battery % level = (0.1 x VDDA in mV) - 200
     */
    batteryLevel = ((uint32)(vddaVoltageMv / 10)) - 200;
    if((batteryLevel > 100) && (batteryLevel < 230))
    {
        batteryLevel = 100;
    }
    else if(batteryLevel >= 230)
    {
        batteryLevel = 0;
    }
    
    
    /* Enable channel 0 again, disable channel 1 */
    ADC_SetChanMask(0x01);
    
    /* Enable bypass cap for the VDDA/2 reference */
    CY_SET_REG32(CYREG_SAR_CTRL, CY_GET_REG32(CYREG_SAR_CTRL) | (0x01 << 7));
}

/* [] END OF FILE */
