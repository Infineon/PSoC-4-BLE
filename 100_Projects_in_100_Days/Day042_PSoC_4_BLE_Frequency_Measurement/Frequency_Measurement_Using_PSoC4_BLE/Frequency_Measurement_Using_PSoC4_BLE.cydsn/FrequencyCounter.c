/***************************************************************************//**
* @file    FrequencyCounter.c
* @version 1.0
* @author  Chethan (dche@cypress.com)
*
* @par Description
*   This file contains the source code to the APIs for computing the input signal 
*   frequency.
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 BLE Pioneer Kit
*
* @par References
*    1. 001-32359: AN2283 "PSoC 1 Measuring Frequency"
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

/*******************************************************************************
*   Included Headers
*******************************************************************************/
#include "main.h"

/* Variable to store the captured count values of counter */
uint16 Input_Signal_Counter1_Count, Input_Signal_Counter2_Count;
uint16 Ref_Clock_Counter1_Count, Ref_Clock_Counter2_Count;

/* Variable to store the 32-bit count value */
uint32 Input_Signal_Count, Ref_Clock_Count;

/* Variable to store the integer part and decimal part of the computed frequency */
uint32 Freq_Integer;
uint16 Freq_Decimal;

/* Variable to indicate PWM_2s ISR is executed */
extern uint8 Calculate_Frequency;

/* Variable to indicate valid capture events dtection */
uint8 Input_Sig_Ctr_Capture, Ref_Clk_Ctr_Capture;

/* Array to store the computed frequency and count value in ASCII format */
uint8 Input_Frequency[FREQ_DATA_LEN];
uint8 InputCounter_ASCII[FREQ_DATA_LEN];
uint8 RefCounter_ASCII[FREQ_DATA_LEN];

/* Interrupt Service Routine for Input_Sig_Counter, Ref_Clock_Counter and PWM_2s */
CY_ISR_PROTO(PWM_2s_InterruptHandler);
CY_ISR_PROTO(Input_Sig_Ctr_InterruptHandler);
CY_ISR_PROTO(Ref_Clk_Ctr_InterruptHandler);

/*******************************************************************************
* Function Name: Initialize_Freq_Meas_System
********************************************************************************
*
* Summary:
*  Function starts component related to frequency measurement 
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Initialize_Freq_Meas_System(void)
{
	/* Start the PWM and counter ISRs */
	PWM_2s_ISR_StartEx(PWM_2s_InterruptHandler);
	Input_Sig_Ctr_ISR_StartEx(Input_Sig_Ctr_InterruptHandler);
	Ref_Clk_Ctr_ISR_StartEx(Ref_Clk_Ctr_InterruptHandler);
	
	/* Start PWM component to generate pulse width of 1s */
	PWM_2s_Start();
	
	/* Start the input signal counter1 and counter2 */
	Input_Signal_Counter1_Start();
	Input_Signal_Counter2_Start();
	
	/* Start the reference clock counter1 and counter2 */
	Ref_Clock_Counter1_Start();
	Ref_Clock_Counter2_Start();
	
	#if (OPAMP_ENABLE)
		/* Start the unity gain buffer */
		Opamp_1_Start();

		/* Start the opamp to convert input arbitrary signal to square wave */
		Comparator_Start();
	#endif
}

/*******************************************************************************
* Function Name: Compute_Frequency
********************************************************************************
*
* Summary:
*  Function computes the input signal frequency using the latched count value 
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Compute_Frequency(void)
{
	/* Variable to store the ratio of Input counter count and Ref counter count */
	float32 CountRatio = 0;
	
	/* Variable to store the input signal frequency */
	float32 Frequency_Temp = 0; 
	
	/* Compute the 32-bit count value */
	Input_Signal_Count = ((int32)(Input_Signal_Counter2_Count << 16)) + Input_Signal_Counter1_Count +1;
	Ref_Clock_Count = ((int32)(Ref_Clock_Counter2_Count << 16)) + Ref_Clock_Counter1_Count + 1;
	
	/* Check if divide by zero condition occurs */
	if(Ref_Clock_Count!=0)
	{
		/* Compute the ratio of input_sig_counter_count and the ref_clock_counter count */
		CountRatio = (float) ((float)Input_Signal_Count/ (float)Ref_Clock_Count);
	}
	else
		CountRatio=0;
	
	/* Calculate the input signal frequency */
	Frequency_Temp = (CountRatio * CLOCK_FREQENCY);
	
	/* Get the integer value of the calculated frequency */
	Freq_Integer = (int32)Frequency_Temp;
	
	/* Reset the input_frequency array before storing the frequency value */
	Reset_Array(Input_Frequency, DATA_END);
	
	/* If input frequency is less than 1KHz, format the data to display in units of Hz */
	if(Frequency_Temp < ONE_KHZ)
	{
		/* Extract only the decimal part of the computed frequency value */
		Frequency_Temp -= Freq_Integer;
		
		/* Convert the decimal value into integer value for ASCII conversion */
		Freq_Decimal = (int32)(Frequency_Temp * THOUSAND);
		
		/* Format the computed frequency value to display in units of Hz */
		FormatFrequencyData(FREQ_HZ);
	}
	/* If input frequency is less than 1MHz, format the data to display in units of KHz */
	else if(Frequency_Temp < ONE_MHZ)
	{
		FormatFrequencyData(FREQ_KHZ);
	}
	/* If input frequency is greater than or equal to 1MHz, format the data to 
	*  display in units of MHz */
	else
	{
		FormatFrequencyData(FREQ_MHZ);
	}
}

/*******************************************************************************
* Function Name: FormatFrequencyData
********************************************************************************
*
* Summary:
*  Function converts the computed frequency in hexadecimal to ASCII format 
*  and stores in an array along with the unit of frequency 
*
* Parameters:
*  FormatType: Value indicates the range of input frequency for formatting the 
*			   input signal frequency
*
* Return:
*  None
*
*******************************************************************************/
void FormatFrequencyData(uint8 FormatType)
{
	/* Variable to count the number of loops */
	uint8 loopNo = 0;
	
	/* If input frequency in in Hz, format the data to display in units of Hz */
	if((FormatType == FREQ_HZ) ||(FormatType == ZERO_HZ ))
	{
		/* If frequency is less than 1KHZ, store " Hz" in the first 3 bytes of 
		*  Input_Frequency array */
		/* Store 'z" in the first byte of the array */
		Input_Frequency[0] = ASCII_z;
		
		/* Store "H" in the second byte of the arrray */
		Input_Frequency[1] = ASCII_H;
		
		/* Store space character in the third byte */
		Input_Frequency[2] = ASCII_SPACE;
		
		if(FormatType == ZERO_HZ )
		{
			Freq_Decimal = 0;
			Freq_Integer = 0;
		}
		
		/* Store the decimal digits from fourth byte in the array in ASCII format */
		for(loopNo = DATA_START; loopNo < DECIMAL_END; loopNo++)
		{
			/* Get the value of one's place of the integer */
			Input_Frequency[loopNo] = Freq_Decimal % 10;

			/* Convert the integer to ASCII */
			Input_Frequency[loopNo] += INTEGER_TO_ASCII;
			
			/* Shift the Ten's place to one's place */
			Freq_Decimal /= 10;
		}
		/* Store the decimal point in the array */
		Input_Frequency[loopNo++] = ASCII_DECIMAL_POINT;
		
		/* Enter the integer digits in the array in ASCII format */
		while((Freq_Integer > 0) && (loopNo < DATA_END))
		{
			/* Get the value of one's place of the integer */
			Input_Frequency[loopNo] = Freq_Integer % 10;
			
			/* Convert the integer to ASCII */
			Input_Frequency[loopNo] += INTEGER_TO_ASCII;
			
			/* Shift the Ten's place to one's place */			
			Freq_Integer /= 10;
			
			loopNo++;
		}
	}
	else if(FormatType == FREQ_KHZ)
	{
		/* If frequency is less than 1MHZ, store "KHz" in the first 3 bytes of Frequency_Format array */
		/* Store 'z" in the first byte of the array */
		Input_Frequency[0] = ASCII_z;
		
		/* Store "H" in the second byte of the array */		
		Input_Frequency[1] = ASCII_H;

		/* Store "K" in the third byte of the array */	
		Input_Frequency[2] = ASCII_K;
		
		/* Enter the frequency in ASCII format from fourth byte */
		loopNo = DATA_START;
		
		/* Convert the frequency value from hexadecimal to decimal */
		while((Freq_Integer > 0) && (loopNo < DATA_END))
		{
			if(loopNo != DECIMAL_END)
			{
				/* Get the value of one's place of the integer */
				Input_Frequency[loopNo] = Freq_Integer % 10;
				
				/* Convert the integer to ASCII */
				Input_Frequency[loopNo] += INTEGER_TO_ASCII;

				/* Shift the Ten's place to one's place */					
				Freq_Integer /= 10;
			}
			/* After decimal digits are entered, store the decimal point in 
			*  the 7th byte of Input_Frequency[] array*/
			else
				Input_Frequency[loopNo] = ASCII_DECIMAL_POINT;
				
			loopNo++;
		}
	}
	else
	{
		/* If frequency is above or equal to 1 MHZ, store "MHz" in the first 3 bytes of Frequency_Format array */
		
		/* Store 'z" in the first byte of the array */
		Input_Frequency[0] = ASCII_z;
		
		/* Store "H" in the second byte of the array */	
		Input_Frequency[1] = ASCII_H;

		/* Store "M" in the third byte of the array */		
		Input_Frequency[2] = ASCII_M;

		/* For frequencies greater than or equal to 1 MHZ, decimal digits will represent the "KHz" frequency */
		Freq_Integer = Freq_Integer/1000;
		
		/* Enter the frequency in ASCII format from fourth byte */
		loopNo = DATA_START;
		
		while((Freq_Integer > 0) && (loopNo < DATA_END))
		{
			if(loopNo != DECIMAL_END)
			{
				/* Get the value of one's place of the integer */
				Input_Frequency[loopNo] = Freq_Integer % 10;
				
				/* Convert the integer to ASCII */
				Input_Frequency[loopNo] += INTEGER_TO_ASCII;

				/* Shift the Ten's place to one's place */					
				Freq_Integer /= 10;
			}
			/* If decimal digits are entered, store the decimal point in 7th byte in the array*/
			else
				Input_Frequency[loopNo] = ASCII_DECIMAL_POINT;
				
			loopNo++;
		}
	}
}

/*******************************************************************************
* Function Name: Convert_HextoDec
********************************************************************************
*
* Summary:
*  Function converts 32bit hexadecimal value to integer and stores in an array in 
*  ASCII format
*
* Parameters:
*  SourceValue: The input value which should be represented in integer format
*  DestinationAddress: Pointer to the array in which the ASCII values of the integer 
*  value should be stored.
*
* Return:
*  None
*
*******************************************************************************/
void Convert_HextoDec(uint32 SourceValue, uint8* DestinationAddress)
{
	while(SourceValue > 0)
	{
			/* Get the value of one's place of the integer */
			*DestinationAddress = SourceValue % 10;
			
			/* Convert the integer to ASCII */
			*DestinationAddress += INTEGER_TO_ASCII;

			/* Shift the Ten's place to one's place */					
			SourceValue /= 10;
		
			DestinationAddress++;
	}
}

/*******************************************************************************
* Function Name: Reset_Array
********************************************************************************
*
* Summary:
*  Function resets all the bytes in the array to 0x20 (hex value for space) 
*
* Parameters:
*  DestinationAddress: Pointer to the array whose value needs to be reset to 0x20
*  noOfBytes: Number of bytes that needs to be reset in the array.
*
* Return:
*  None
*
*******************************************************************************/
void Reset_Array(uint8 *DestinationAddress, uint8 noOfBytes)
{
	uint8 loopNo = 0;
 	
	/* Reset all the bytes in the array to hexadecimal value of 0x20 */
	for(loopNo = 0; loopNo < noOfBytes; loopNo++)
	{
		*DestinationAddress = ASCII_SPACE;
		DestinationAddress++;
	}
}

/*******************************************************************************
* Function Name: PWM_2s_InterruptHandler
********************************************************************************
*
* Summary:
*   The custom Interrupt Service Routine for PWM_2s ISR.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(PWM_2s_InterruptHandler)
{
	static uint8 FirstTime = 0;
	
    /* Clear ISR for next interrupt */
	PWM_2s_ReadStatusRegister();
	
	/* If this is the first ISR after device reset, do not set the Calculate_Frequency variable */
	if(!FirstTime)
	{
		FirstTime = 1;
	}
	else
	{
		/* Set the 2s interval flag to indicate CPU to compute the frequency */
		Calculate_Frequency = 1;
	}
}

/*******************************************************************************
* Function Name: Input_Sig_Ctr_InterruptHandler
********************************************************************************
*
* Summary:
*   The custom Interrupt Service Routine for Input_Sig_Ctr ISR.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(Input_Sig_Ctr_InterruptHandler)
{
    /* Clear ISR for next interrupt */
	Input_Signal_Counter1_ClearInterrupt(Input_Signal_Counter1_INTR_MASK_CC_MATCH);
	
	/* Store the captured count value */
	Input_Signal_Counter1_Count = Input_Signal_Counter1_ReadCapture();
	Input_Signal_Counter2_Count = Input_Signal_Counter2_ReadCapture();
	
	/* Indicate that a valid capture event is detected */
	Input_Sig_Ctr_Capture = 1;
}

/*******************************************************************************
* Function Name: Ref_Clk_Ctr_InterruptHandler
********************************************************************************
*
* Summary:
*   The custom Interrupt Service Routine for Ref_Clk_Ctr ISR.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(Ref_Clk_Ctr_InterruptHandler)
{
    /* Clear ISR for next interrupt */
	Ref_Clock_Counter1_ClearInterrupt(Ref_Clock_Counter1_INTR_MASK_CC_MATCH);

	/* Store the captured count value */
	Ref_Clock_Counter1_Count = Ref_Clock_Counter1_ReadCapture();
	Ref_Clock_Counter2_Count = Ref_Clock_Counter2_ReadCapture();

	/* Indicate that a valid capture event is detected */
	Ref_Clk_Ctr_Capture = 1;
}