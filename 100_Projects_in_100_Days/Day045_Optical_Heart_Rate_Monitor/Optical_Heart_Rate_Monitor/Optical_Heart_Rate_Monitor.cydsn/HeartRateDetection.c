/*****************************************************************************
* File Name: HeartRateDetection.c
*
* Version: 1.0
*
* Description:
* This file implements the heart rate detection algorithm.
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
#include "WatchdogTimer.h"
#include "RefBuf.h"
#include "TIA.h"
#include "DiffAmp.h"
#include "ADC.h"
#include "LedInfrared1.h"
#include "LedInfrared2.h"
#include "MyIsr.h"

#if (FIR_FILTER_EN)
    #include "filter.h"
#endif  /* #if (FIR_FILTER_EN) */

#if (UART_DEBUG_EN)
    #include "UART.h"
#endif  /* #if (UART_DEBUG_EN) */


/*****************************************************************************
* Signal processing macros
*****************************************************************************/
#define SIGNAL_THRESHOLD                 (5000)
#define SIGNAL_HYSTERESIS                (SIGNAL_THRESHOLD - 500)
#define NOISE_THRESHOLD                  (1500)
#define NEGATIVE_NOISE_THRESHOLD         (1500)
#define LOW_BASELINE_RESET               (25)
#define AUTO_RESET_COUNT                 (150)
#define SENSOR_DEBOUNCE_COUNT            (10)

#define HEART_RATE_LIMIT_LOW             (40)
#define HEART_RATE_LIMIT_HIGH            (200)

#define SAMPLES_TAKEN_FOR_HRS            (40)


/*****************************************************************************
* Global variables
*****************************************************************************/
uint8 heartRateFiltered = 0;

static bool validBeat = false;
static bool debounceEnable = true;
static uint8 heartRateUnfiltered = 0;
static uint8 debounceCounter = 0;
static uint8 autoResetCounter = 0;
static uint8 lowBaselineResetCounter = 0;
static uint16 baseline = 0;
static uint16 signalCount = 0;


/*****************************************************************************
* Function Definitions
*****************************************************************************/

/*****************************************************************************
* Static Functions
*****************************************************************************/

/*****************************************************************************
* Function Name: Print16BitValue()
******************************************************************************
* Summary:
* Splits a 16-bit value into two bytes and sends them via UART TX.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Splits the 16-bit value in two and sends via TX8, MSB first.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
#if (UART_DEBUG_EN)
static void Print16BitValue(uint16 value)
{
	uint8 splitValues[2];
	
	splitValues[0] = (uint8) (((value) & 0xFF00) >> 8);
	splitValues[1] = (uint8) (value); 
	UART_PutChar(splitValues[0]);	
	UART_PutChar(splitValues[1]);
}
#endif  /* #if (UART_DEBUG_EN) */ 


/*****************************************************************************
* Function Name: HeartRate_ProcessOutput()
******************************************************************************
* Summary:
* Calculates the baseline for the signal waveform provided. Also calculates
* the signal delta from the baseline.
*
* Parameters:
* uint16 sample - the waveform sample for which baseline is to be calculated
*
* Return:
* None
*
* Theory:
* The baseline implementation is similar to CapSense component and is done 
* in the following way:
* 1. Baseline tracks the sample waveform in the range of noise threshold to
*    negative noise threshold. 
* 2. Above the noise threshold, baseline does not change. 
* 3. Above the signal threshold, baseline auto-reset counter starts and on 
*    overflow, the baseline is reset to the latest sample. 
* 4. Below the negative noise threshold, low baseline reset counter starts
*    and on overflow, the baseline is reset to the latest sample.
*
* The function also calculates the difference between the baseline and sample, 
* and concludes whether the current sample represents a heart-beat or not.
* Signal debounce is also implemented.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
static void HeartRate_ProcessOutput(uint16 sample)
{
    uint16 difference = 0;
    static bool firstTime = true;
    
    signalCount = 0;
    
    if(firstTime)
    {
        baseline = sample;
        firstTime = false;
    }
    else
    {
        if(sample >= baseline)
        {
            lowBaselineResetCounter = 0;
            difference = sample - baseline;

            if(difference > NOISE_THRESHOLD)
            {
                signalCount = difference;
                
                if(signalCount >= SIGNAL_THRESHOLD)
                {
                    if(debounceEnable)
                    {
                        debounceCounter++;
                        
                        if(debounceCounter >= SENSOR_DEBOUNCE_COUNT)
                        {
                            validBeat = true;
                            debounceEnable = false;
                        }
                    }
                    
                    autoResetCounter++;
                    
                    if(autoResetCounter >= AUTO_RESET_COUNT)
                    {
                        baseline = sample;
                        signalCount = 0;
                        debounceCounter = 0;
                        autoResetCounter = 0;
                        validBeat = false;
                        debounceEnable = true;
                    }
                }
                else if((signalCount >= SIGNAL_HYSTERESIS) && (true == validBeat))
                {
                    autoResetCounter++;
                    
                    if(autoResetCounter >= AUTO_RESET_COUNT)
                    {
                        baseline = sample;
                        signalCount = 0;
                        debounceCounter = 0;
                        autoResetCounter = 0;
                        validBeat = false;
                        debounceEnable = true;
                    }
                }
                else
                {
                    autoResetCounter = 0;
                    debounceCounter = 0;
                    validBeat = false;
                    debounceEnable = true;
                }
            }
            else
            {
                /* Track ADC counts */
                baseline = baseline + (difference >> 3);
                autoResetCounter = 0;
                debounceCounter = 0;
                validBeat = false;
                debounceEnable = true;
            }
        }
        else
        {
            autoResetCounter = 0;
            debounceCounter = 0;
            validBeat = false;
            debounceEnable = true;
            difference = baseline - sample;
            
            if(difference > NEGATIVE_NOISE_THRESHOLD)
            {
                /* Low baseline reset count */
                lowBaselineResetCounter++;
                
                if(LOW_BASELINE_RESET <= lowBaselineResetCounter)
                {
                    baseline = sample;
                    lowBaselineResetCounter = 0;
                }
            }
            else
            {
                /* Track ADC counts */
                baseline = baseline - (difference >> 3);
                lowBaselineResetCounter = 0;
            }
        }
        
    }
}


/*****************************************************************************
* Public Functions
*****************************************************************************/

/*****************************************************************************
* Function Name: HeartRate_Measure()
******************************************************************************
* Summary:
* Implements the measurement algorithm for heart rate.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Polls the ADC completion flag and if set, performs further actions.
* Identifies a valid heart beat by first passing the ADC output through a 
* FIR Filter. Then calls HeartRate_ProcessOutput() to identify signal.
* On a valid heart beat, a Q-R transition (rising edge) is identified by 
* caching the previous output. On every rising edge, the corresponding 
* watchdog timestamp is measured, and time taken for certain number of 
* heart beats is extrapolated to 60 seconds timeframe to get a bps value.
* 
* For measuring the time taken for these beats, a measurement window is 
* created (starting with 1 sample) and later increased to a large number.
* This gives both speed to first measurement, and accuracy.
*
* Further processing (median filter etc) is done to get a clean output.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void HeartRate_Measure(void)
{
    uint8 loopCounter = 0;
    uint16 adcOut = 0;
    uint16 firstFilteredOut = 0;
    uint32 xBeatTime = 0;
    uint32 sumOutput = 0;

    static bool beatStatus = false;
    static uint8 hrPreviousSample0 = 0;
    static uint8 hrPreviousSample1 = 0;
    static uint8 beatCount = 1;
    static uint16 secondFilteredOut = 0;
    static uint32 beatTime[SAMPLES_TAKEN_FOR_HRS];
    static uint32 lastBeatTimestamp;

    if(canMeasureHeartRate)
    {
        canMeasureHeartRate = false;
        adcOut = ADC_GetResult16(0);
        
#if (FIR_FILTER_EN)
        /* Apply FIR filter on the raw ADC output */
        firstFilteredOut = FirFilter(adcOut);
#else
        firstFilteredOut = adcOut;
#endif  /* #if (FIR_FILTER_EN) */
        
        /*
         * IIR filter on top of the FIR filtered output.
         * 6/7 of Previous + 1/7 of new sample.
         */
        sumOutput = ((uint32)secondFilteredOut * 6) + (uint32)firstFilteredOut;
        secondFilteredOut = (uint16)(sumOutput / 7);
        
        /* Calculate baseline and identify a beat */
        HeartRate_ProcessOutput(secondFilteredOut);

#if (UART_DEBUG_EN)
        /* Header */
        UART_PutChar(0x0D);
        UART_PutChar(0x0A);
        
        Print16BitValue(secondFilteredOut);
        Print16BitValue(baseline);
//        UART_PutChar(validBeat);
        UART_PutChar(CY_GET_REG32(CYREG_CTBM1_OA0_OFFSET_TRIM));

        /* Tail */
        UART_PutChar(0x0F);
#endif  /* #ifdef (UART_DEBUG_EN) */
        
        /* Detect a valid heart beat */
        if(validBeat)
        {
            lastBeatTimestamp = WatchdogTimer_GetTimestamp();
            
            if(false == beatStatus)
            {
                /*
                 * Rising edge on ADC detected - this indicates a new beat. 
                 * Start timing measurements 
                 */
                beatTime[beatCount - 1] = lastBeatTimestamp;
                xBeatTime = beatTime[beatCount - 1] - beatTime[0];
                if(xBeatTime)
                {
                    heartRateUnfiltered = (uint8)((uint32)60000 * (beatCount - 1) / xBeatTime);
                }
                else
                {
                    heartRateUnfiltered = 0;
                }
                
                if(beatCount < SAMPLES_TAKEN_FOR_HRS)
                {
                    /* Create a sampling window */
                    beatCount++;
                }
                else
                {
                    /* Roll the window after the complete window is populated */
                    for(loopCounter = 1; loopCounter < SAMPLES_TAKEN_FOR_HRS; loopCounter++)
                    {
                        beatTime[loopCounter - 1] = beatTime[loopCounter];
                    }
                }
                
                /* Apply 3rd order median filter on the heart rate value obtained */
                {
                    uint8 temp;
                    
                    if(hrPreviousSample0 > hrPreviousSample1)
                    {
                        temp = hrPreviousSample1;
                        hrPreviousSample1 = hrPreviousSample0;
                        hrPreviousSample0 = temp;
                    }
                    
                    if(hrPreviousSample1 > heartRateUnfiltered)
                    {
                        hrPreviousSample1 = heartRateUnfiltered;
                    }
                    
                    heartRateFiltered = ((hrPreviousSample0 > hrPreviousSample1) ? hrPreviousSample0 : hrPreviousSample1);
                    
                    hrPreviousSample0 = hrPreviousSample1;
                    hrPreviousSample1 = heartRateUnfiltered;
                }
                
                /* Reset the final number if it is out of bounds */
                if((heartRateFiltered < HEART_RATE_LIMIT_LOW) || (heartRateFiltered > HEART_RATE_LIMIT_HIGH))
                {
                    heartRateFiltered = 0;
                }
            }
            
            /* 
             * Indicate an ongoing beat so that timing is not 
             * measured again for this beat. 
             */
            beatStatus = true;
        }
        else
        {
            /* Prepare for next rising edge to detect next beat */
            beatStatus = false;

            /* Reset heart rate measurement 3 seconds after detection stops */
            if((WatchdogTimer_GetTimestamp() - lastBeatTimestamp) > 3000)
            {
                heartRateUnfiltered = 0;
                heartRateFiltered = 0;
                hrPreviousSample0 = 0;
                hrPreviousSample1 = 0;
                beatCount = 1;
            }
        }
    }
}


/*****************************************************************************
* Function Name: HeartRate_Start()
******************************************************************************
* Summary:
* Starts the heart rate measurement block.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function initializes all the hardware required for the measurement.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
void HeartRate_Start(void)
{
    /* Start OpAmps */
    RefBuf_Start();
    TIA_Start();
    DiffAmp_Start();
    
    /* Turn on LEDs */
    LedInfrared1_Write(false);
    LedInfrared2_Write(false);
}


/* [] END OF FILE */
