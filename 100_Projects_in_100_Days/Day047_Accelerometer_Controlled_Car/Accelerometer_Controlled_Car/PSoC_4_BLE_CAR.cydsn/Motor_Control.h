/*******************************************************************************
* File Name: Motor_Control.h
*
* Description:
*  This is the header file for handling the Motor Control
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

#include <project.h>

/* Defines for controlling forward and backward direction of Motor */

#define FRONT 			2u
#define REAR 			1u

/* Client Receives the following commands for controlling Motor Direction */

#define FORWARD 		115u
#define REVERSE 		98u
#define LEFT 			76u
#define RIGHT 			82u
#define STOP 			112u

/* Function to Update the Motor Direction based on the command received */

void UpdateMotor(uint8 Motor_Command);

/* [] END OF FILE */
