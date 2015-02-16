/****************************************************************************
* File Name: Temperature.c
*
* Version: 1.0
*
* Description:
* This file implements functions related to temperature measurement.
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
#include "Temperature.h"

/*****************************************************************************
* Static Functions
*****************************************************************************/

/*****************************************************************************
* Function Name: MeasureSensorVoltage()
******************************************************************************
* Summary:
* Measures the voltage connected at ADC input. 
*
* Parameters:
* None
*
* Return:
* uint16 - Measured voltage
*
* Theory:
* This functions sequences the AMux to next channel and connects reference 
* signal or thermistor or offset signal at ADC input. It then triggers the ADC
* and measures the signal.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
static uint16 MeasureSensorVoltage ()
{
    /* Connect next channel available at AMux input to Amux output */
    /* Note: If no channels are connected, channel 0 gets connected by this 
    *  fucntion */
    AMuxSeq_Next();
    
    /* Start sample conversion */
    ADC_StartConvert();
    
    /* Wait till end of two conversions and drop one sample for signal to settle 
    *  down, it's not required if reference is continuously available.  
    *  To reduce current consumption, CPU can be put to sleep while ADC conversion
    *  is in process. */
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
    ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
    
    /* Stop ADC coversion */ 
    ADC_StopConvert();
    
    /* Return 16 bit measured value */
    return (ADC_GetResult16(0));
}

/*****************************************************************************
* Public Functions
*****************************************************************************/

/*****************************************************************************
* Function Name: ProcessTemperature()
******************************************************************************
* Summary:
* Measures the current temperature. 
*
* Parameters:
* None
*
* Return:
* uint32 - Measured temperature
*
* Theory:
* This function measures the reference voltage, thermistor voltage and 
* offset voltage. 

* Channel 0 - Reference voltage
* Channel 1 - Thermistor voltage
* Channel 2 - Offset voltage
* 
* This function removes offset via correlated double sampling and then
* measures the temperature.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
int32 ProcessTemperature (void)
{
    /* Local variables used for temperature measurement */
    static int32 temperature = 0;
    #if (SIMULATE_TEMPERATURE_SENSOR)
        
    int32 thermistorResistance = 0;
    int16 referenceVoltage = 0;
    int16 thermistorVoltage = 0;
    int16 offsetVoltage = 0;
    
    /* Check if measurement interval has expired */
    if(SystemFlag & MEASUREMENT_INTERVAL)
    {
        /* Disconnect all the pins from Amux input */
        AMuxSeq_DisconnectAll();
        
        /* Measure reference voltage - Channel 0 */
        referenceVoltage = MeasureSensorVoltage();
        /* Measure thermistor voltage - Channel 0 */
        thermistorVoltage = MeasureSensorVoltage();
        /* Measure offset voltage - Channel 0 */
        offsetVoltage = MeasureSensorVoltage();
        
        /* Measure thermistor resistance and remove offset by using correlated 
        *  double sampling. */
        thermistorResistance = Thermistor_GetResistance((referenceVoltage - offsetVoltage), (thermistorVoltage - offsetVoltage));
        
        /* Compute temperature and remove decimal places */
        temperature = Thermistor_GetTemperature(thermistorResistance);
        temperature = temperature / 100;
    }
    #else
    /* Check if measurement interval has expired */
    if (SystemFlag & MEASUREMENT_INTERVAL)
    {
        temperature++;
        if(temperature > 100)
        {
            temperature = 0;
        }
    }
    #endif
    
    /* Return measured temperature */
    return (temperature);
}

/* [] END OF FILE */
