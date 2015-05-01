/*******************************************************************************
* File Name: LED.h
*
* Description:
*  This is the header file for the LED ON and OFF functionality on the CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#ifndef _LED_H_
#define _LED_H_
	
#include "project.h"

	
#define RED_LED_ON()	{ RED_LED_Write(0); GREEN_LED_Write(1); BLUE_LED_Write(1);}	
#define GREEN_LED_ON()	{ RED_LED_Write(1); GREEN_LED_Write(0); BLUE_LED_Write(1);}	
#define BLUE_LED_ON()	{ RED_LED_Write(1); GREEN_LED_Write(1); BLUE_LED_Write(0);}	
#define ALL_LED_OFF()   { RED_LED_Write(1); GREEN_LED_Write(1); BLUE_LED_Write(1);}	
	
#endif	


/* [] END OF FILE */
