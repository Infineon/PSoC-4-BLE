/*******************************************************************************
* File Name: app_UART.h
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef APP_UART_H
    
    
    
    #include <project.h>  
    #include "stdbool.h"    
	
	#define SCAN_Complete   1
	#define IDLE 			0
	
	
    #define ACCELEROMETER_X_MUX_CHANNEL_NUMBER 0
	#define ACCELEROMETER_Y_MUX_CHANNEL_NUMBER 1
	#define ACCELEROMETER_Z_MUX_CHANNEL_NUMBER 2
	
	#define X_DEFAULT 1220+10
	#define Y_DEFAULT 1210+10
	#define Z_DEFAULT 1040+10
   
    
    /***************************************
    *       Function Prototypes
    ***************************************/
    void SendCommand();
	uint8 Scan_Accelerometer();
	
	/***************************************
    *       Variables
    ***************************************/
    uint8 scan_status;
	uint8 x,y,z;
	int16 ADC_x_current, ADC_y_current, ADC_z_current,ADC_x_previous, ADC_y_previous, ADC_z_previous;
	uint8 BLE_Command;
	uint8 state;
	
	/***************************************
    *       Constants
    ***************************************/
	/* States */
	
	#define NORMAL 0
	#define LEFT 1
	#define RIGHT 2
	#define FORWARD 3
	#define BACKWARD 4
	
    
#endif

/* [] END OF FILE */
