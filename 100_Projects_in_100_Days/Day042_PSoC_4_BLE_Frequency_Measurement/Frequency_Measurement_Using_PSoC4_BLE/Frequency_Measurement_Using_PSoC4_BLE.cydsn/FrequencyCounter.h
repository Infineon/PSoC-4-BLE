/***************************************************************************//**
* @file    FrequencyCounter.h
* @version 1.0
* @author  Chethan (dche@cypress.com)
*
* @par Description
*    This file contains the constants & function prototypes for frequency measurement
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 BLE Pioneer Kit
*
* @par References
*    1. 001-32359: AN2283 "PSoC1: Frequency Measurement"
*
* @par Code Tested With
*    1. PSoC Creator  3.1 SP2 (3.1.0.2177)
*    2. GCC 4.8.4
*
**//****************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
********************************************************************************
* All rights reserved. 
* This software, including source code, documentation and related 
* materials (“Software”), is owned by Cypress Semiconductor 
* Corporation (“Cypress”) and is protected by and subject to worldwide 
* patent protection (United States and foreign), United States copyright 
* laws and international treaty provisions. Therefore, you may use this 
* Software only as provided in the license agreement accompanying the 
* software package from which you obtained this Software (“EULA”). 
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive, 
* non-transferable license to copy, modify and compile the Software source code
* solely for your use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.

* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use 
* of the Software or any product or circuit described in the Software. Cypress
* does not authorize its products for use in any products where a malfunction
* or failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such  
* system or application assumes all risk of such use and in doing so agrees to  
* indemnify Cypress against all liability. 
*******************************************************************************/

#ifndef __FREQUENCYCOUNTER_H
#define __FREQUENCYCOUNTER_H
/*******************************************************************************
*   Included Headers and Macros
*******************************************************************************/
#include <project.h>

/* Frequency value is stored from 3 byte of the array */
#define DATA_START 		(3)

/* Array size for storing the frequency value and range */
#define DATA_END		(10)

/* Seventh byte of the array contains the decimal point */
#define DECIMAL_END		(6)

/* Macro to specify the frequency range for formatting the frequency data */
#define FREQ_MHZ 		(1)
#define FREQ_KHZ 		(2)
#define FREQ_HZ 		(3)
#define ZERO_HZ         (4)

/* ASCII Constants */	
#define ASCII_z					(0x7A)
#define ASCII_H					(0x48)
#define ASCII_K					(0x4B)
#define ASCII_M					(0x4D)
#define ASCII_SPACE				(0x20)
#define ASCII_DECIMAL_POINT		(0x2E)
#define INTEGER_TO_ASCII		(0x30)
	
#define THOUSAND        (1000)	
#define ONE_KHZ			(1000)
#define ONE_MHZ			(1000000)

/* Macro to specify the reference clock used for Ref_Counter in the project */
#define CLOCK_FREQENCY 	(6000000)

/*******************************************************************************
*  Function declarations
*******************************************************************************/

/* Function formats the frequency data depending on the frequency range */
void FormatFrequencyData(uint8 FormatType);

/* Function initializes all the components related to frequency measurement */
void Initialize_Freq_Meas_System(void);

/* Function computes the frequency value using the latched counter values */
void Compute_Frequency(void);

/* Function converts the hexadecimal count value into decimal count value (ASCII) 
*  for displaying on hyperterminal */
void Convert_HextoDec(uint32 SourceValue, uint8 * DestinationAddress);

/* Function resets all the bytes (to space character) in the frequency array 
   before storing the frequency value in ASCII format */
void Reset_Array(uint8 *DestinationAddress, uint8 noOfBytes);

#endif /* end of #ifndef __FREQUENCYCOUNTER_H */
/* END OF FILE */
