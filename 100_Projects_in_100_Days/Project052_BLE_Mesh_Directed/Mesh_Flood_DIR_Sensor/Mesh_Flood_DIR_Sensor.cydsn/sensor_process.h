/*******************************************************************************
* File Name: sensor_process.h
*
* Version: 1.0
*
* Description:
*  This file contains the headers and constants  for sensor related functions.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(SENSOR_PROCESS_H)
#define SENSOR_PROCESS_H
#include "project.h"

/*****************************************************
*                  Enums and macros
*****************************************************/ 
#define BUTTON_PRESSED						(0u)
#define BUTTON_NOT_PRESSED					(1u)

#define COLOR_STATE_RED						0x01
#define COLOR_STATE_GREEN					0x02
#define COLOR_STATE_BLUE					0x03
#define COLOR_STATE_YELLOW					0x04
#define COLOR_STATE_CYAN					0x05
#define COLOR_STATE_PURPLE					0x06
#define COLOR_STATE_WHITE					0x07
#define COLOR_STATE_HALF_INT				0x08
#define COLOR_STATE_OFF						0x09
/*****************************************************
*                  Function Declarations
*****************************************************/
void CheckSensorStatus(void);
void SetNextColor(void);
void Button_ISR(void);

#endif
/* [] END OF FILE */
