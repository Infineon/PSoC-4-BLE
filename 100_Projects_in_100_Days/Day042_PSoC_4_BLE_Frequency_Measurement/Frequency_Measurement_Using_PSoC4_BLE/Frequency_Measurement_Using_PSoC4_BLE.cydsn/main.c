/***************************************************************************//**
* @file    main.c
* @version 1.0
* @authors DCHE
*
* @par Description
*   This file contains the main source code for Frequency Measurement using
*	PSoC 4 BLE.
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
*   Included Headers and Macros
*******************************************************************************/
#include "main.h"
/*******************************************************************************
*   Variable and Function Declarations
*******************************************************************************/

/*'deviceConnected' flag is used by application to know whether a Central device  
* has been connected. This is updated in BLE event callback */
extern uint8 deviceConnected;

/*'startNotification' flag is set when the central device writes to CCC (Client  
* Characteristic Configuration) of the Custom characteristic to 
* enable notifications */
extern uint8 startNotification;	

/* 'restartAdvertisement' flag is used to restart advertisement */
extern uint8 restartAdvertisement;

/* Array to store the computed frequency in ASCII format */
extern uint8 Input_Frequency[FREQ_DATA_LEN];

/* Array to store the input counter count in ASCII format */
extern uint8 InputCounter_ASCII[FREQ_DATA_LEN];

/* Array to store the ref clock counter count in ASCII format */
extern uint8 RefCounter_ASCII[FREQ_DATA_LEN];

/* Variable to indicate CPU to compute frequency of the input signal */
uint8 Calculate_Frequency;

/* Variable that holds the input signal count and reference clock count */
extern uint32 Input_Signal_Count, Ref_Clock_Count;

/* Variable to indicate that a valid capture is detected for input signal 
   counter and reference clock counter */
extern uint8 Input_Sig_Ctr_Capture, Ref_Clk_Ctr_Capture;

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*   This is the main entry point for this application. This function initializes all the 
*	components used in the project. It computes the frequency whenever a capture event is 
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
int main()
{
	#if(UART_DEBUG_ENABLE)
		/* Variable to store the loop number */
		uint8 loopNo = 0;
	#endif
	/* Enable global interrupt mask */
	CyGlobalIntEnable;	
	
	/* Disable ILO as it is not used */
	CySysClkIloStop();
	
	/* Initialize components related to BLE communication */
	InitializeBLESystem();
	
	/* Initialize components related to frequency counting */
	Initialize_Freq_Meas_System();
	
	/* Start UART component if UART debug is enabled */
	#if(UART_DEBUG_ENABLE)
		/* Start UART component and send welcome string to hyper terminal on PC */
		UART_Start();
		UART_UartPutString("Welcome to Frequency Measurement Using PSoC 4 BLE\n");
		UART_PutCRLF();
	#endif
	
    while(1)
    {
		/* Compute frequency once in every PWM interval(2s) */
		if(Calculate_Frequency == TRUE)
		{
			/* Check if valid capture event is detected */
			if((Input_Sig_Ctr_Capture == 1) && (Ref_Clk_Ctr_Capture == 1))
			{
				/* Compute frequency using the latched count value, computed frequency 
				will be stored in ASCII format in a global array */
				Compute_Frequency();
				
				#if(UART_DEBUG_ENABLE)
					/* Print input signal counter value in hexadecimal */
					UART_UartPutString("Input Signal Counter Value: ");
					UART_SendDebugData(Input_Signal_Count);
					UART_UartPutString("      ");
					
					/* Print input signal counter value in ASCII format */	
					/* Reset the array before storing the ASCII character */
					Reset_Array(InputCounter_ASCII, DATA_END);
					
					Convert_HextoDec(Input_Signal_Count, InputCounter_ASCII);
					for(loopNo = 0; loopNo < DATA_END; loopNo++)
					{
						UART_UartPutChar(InputCounter_ASCII[DATA_END - loopNo -1]);
					}
					UART_PutCRLF();	

					/* Print reference clock counter value */
					UART_UartPutString("Reference Clock Counter Value: ");
					UART_SendDebugData(Ref_Clock_Count);
					UART_UartPutString("      ");
					
					/* Print input signal counter value in ASCII format */	
					/* Reset the array before storing the ASCII character */				
					Reset_Array(RefCounter_ASCII, DATA_END);
					Convert_HextoDec(Ref_Clock_Count, RefCounter_ASCII);
					for(loopNo = 0; loopNo < DATA_END; loopNo++)
					{
						UART_UartPutChar(RefCounter_ASCII[DATA_END - loopNo -1]);
					}
					UART_PutCRLF();
					
					/* Print Input Signal Frequency in decimal format */
					UART_UartPutString("Input Frequency: ");
					for(loopNo = 0; loopNo < DATA_END; loopNo++)
					{
						UART_UartPutChar(Input_Frequency[DATA_END - loopNo -1]);
					}
					UART_PutCRLF();			
				#endif
				/* Reset the capture flag after computing the frequency */
				Input_Sig_Ctr_Capture = 0;
				Ref_Clk_Ctr_Capture = 0;
			} 
			/* If valid capture event is not registered, set the value of frequency to 
			   zero */
			else
			{
				/* Reset the input_frequency array before storing the frequency value */
				Reset_Array(Input_Frequency, DATA_END);

				/* If no capture event is detected in the 1s interval, set the frequency to zero */
				FormatFrequencyData(ZERO_HZ);
				
				#if(UART_DEBUG_ENABLE)
					/* Print Input Signal Frequency in decimal format */
					UART_UartPutString("Input Frequency: ");
					for(loopNo = 0; loopNo < DATA_END; loopNo++)
					{
						UART_UartPutChar(Input_Frequency[DATA_END - loopNo -1]);
					}
					UART_PutCRLF();	
				#endif
			}
			/* Reset the 2s interval flag for computing the frequency in the next interval */
			Calculate_Frequency = 0;
			/* Send frequency value only if BLE device is connected */
			if(TRUE == deviceConnected) 
			{
				/* Send frequency value when notifications are enabled */
				if((startNotification & CCCD_NTF_BIT_MASK))
				{

					/* Send the frequency value to BLE central device by notifications */
					SendDataOverFreqCounterNotification(Input_Frequency);
				}
			}
		}

		
		/* Function to handle LED status depending on BLE state */
		HandleStatusLED();
		
		/* Handle CCCD value update only if BLE device is connected */
		if(TRUE == deviceConnected) 
		{
	
			/* When the Client Characteristic Configuration descriptor (CCCD) is written
			* by Central device for enabling/disabling notifications, then the same
			* descriptor value has to be explicitly updated in application so that
			* it reflects the correct value when the descriptor is read */
			UpdateNotificationCCCD();	
		}		
		if(restartAdvertisement)
		{
			/* Reset 'restartAdvertisement' flag*/
			restartAdvertisement = FALSE;
			
			/* Start Advertisement and enter Discoverable mode*/
			CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);	
		}
		/*Process Event callback to handle BLE events. The events generated and 
		* used for this application are inside the 'CustomEventHandler' routine*/
		CyBle_ProcessEvents();
		
		/* Put CPU to sleep */
		CySysPmSleep();
    }
}
/* [] END OF FILE */
