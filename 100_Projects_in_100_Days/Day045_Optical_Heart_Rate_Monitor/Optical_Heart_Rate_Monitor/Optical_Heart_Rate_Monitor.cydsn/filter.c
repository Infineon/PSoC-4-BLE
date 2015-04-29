/*****************************************************************************
* File Name: filter.c
*
* Version: 1.0
*
* Description:
* This file defines the function to implement the FIR filter.
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
#include <cytypes.h>
#include "filter.h"


/*****************************************************************************
* Macros and constants
*****************************************************************************/
#define FIR_ORDER               (FIR_TAPS - 1)
#define COEFFICIENT_SHIFT       (15)

/* 
 * Original coefficients shifted left by COEFFICIENT_SHIFT places so that we
 * calculations can be done in fixed-point logic.
 * Symmetrical coefficients so only taps/2 in number required to be stored.
 */
const uint32 coefficient[FIR_TAPS >> 1] = 
{
    82,     240,     664,     1319,     
    2199,   3234,    4054,    4591
};

/*****************************************************************************
* Function Name: FirFilter()
******************************************************************************
* Summary:
* Implements a FIR filter in the firmware. 
*
* Parameters:
* value: The input signal to be filtered.
*
* Return:
* Filtered output.
*
* Theory:
* This function performs the following in order:
*   1. Cache a shift register as per the number of taps required. On every
*      call, shift the register by 1 position and accomodate the new signal.
*   2. Perform the signal convolution with the coefficients designed. Since
*      the number of taps is even and the taps are symmetric, the function
*      optimizes the convolution as visible in the code.
*   3. Right-shifts the final sum to account for the fixed point calculation.
*   4. Returns the obtained output.
*
* Side Effects:
* None
*
* Note:
* 1. Since the coefficients are 15-bit wide, the signal must be limited to a
*    width of 16-bits to avoid overflow.
* 2. The function assumes that the number of taps is even.
* 3. The function assumes that the coefficients are symmetrical.
*
*****************************************************************************/
uint16 FirFilter(uint16 input)
{
    static uint32 previousInput[FIR_TAPS];
    uint32 product = 0;
    uint32 sum = 0;
    uint32 loopCounter;
    
    /* Shift previous data by 1 each */
    for(loopCounter = FIR_ORDER; loopCounter != 0; loopCounter--)
    {
        previousInput[loopCounter] = previousInput[loopCounter - 1];
    }
    previousInput[loopCounter] = (uint32)input;

    /* Optimized convolution for even number of taps
     * since coefficients are symmetrical */
    for(loopCounter = (FIR_TAPS >> 1); loopCounter != 0;)
    {
        loopCounter--;
        product = (previousInput[loopCounter] + previousInput[FIR_ORDER - loopCounter]) * coefficient[loopCounter];
        sum += product;
    }
        
    /* Compensate for the shift in coefficients beforehand */  
    sum >>= COEFFICIENT_SHIFT;       
    return (uint16)sum;
}


/* [] END OF FILE */
