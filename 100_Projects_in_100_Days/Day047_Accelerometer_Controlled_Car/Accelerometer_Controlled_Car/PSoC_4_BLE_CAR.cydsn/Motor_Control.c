/*******************************************************************************
* File Name: Motor_Control.c
*
* Description:
*  This is the source file for handling the Motor Control
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

#include "Motor_Control.h"

/*******************************************************************************
* Function Name:   UpdateMotor
********************************************************************************
*
* Summary:
*  This function controls the Motor Direction based on the command received.
*
* Parameters:
*  Motor_Command: Command for controlling Motor Direction .
*
* Return:
*  None.
*
*******************************************************************************/

void UpdateMotor(uint8 Motor_Command)
{
	switch(Motor_Command)
	{
		/* These commands are pre-defined. The accelerometer server
		sends the same commands based on the direction of acceleration.
		These commands are then used to control the motor */
		
		case FORWARD: 
					Control_Reg_Motor_1_Write(FRONT);
					PWM_Motor_1_Start();
					Control_Reg_Motor_2_Write(FRONT);
					PWM_Motor_2_Start();
					Clock_PWM_Start();
					break;
					
		case REVERSE:
					Clock_PWM_Stop();
					Control_Reg_Motor_1_Write(REAR);
					PWM_Motor_1_Start();
					Control_Reg_Motor_2_Write(REAR);
					PWM_Motor_2_Start();
					Clock_PWM_Start();
					break;
		
		case LEFT:
					Clock_PWM_Stop();
					Control_Reg_Motor_1_Write(REAR);
					PWM_Motor_1_Start();
					Control_Reg_Motor_2_Write(FRONT);
					PWM_Motor_2_Start();
					Clock_PWM_Start();
					break;
					
		case RIGHT:
					Clock_PWM_Stop();
					Control_Reg_Motor_1_Write(FRONT);
					PWM_Motor_1_Start();
					Control_Reg_Motor_2_Write(REAR);
					PWM_Motor_2_Start();
					Clock_PWM_Start();
					break;
					
		case STOP:
					PWM_Motor_1_Stop();
					PWM_Motor_2_Stop();
					Clock_PWM_Stop();
					break;
					
		
		default:
					PWM_Motor_1_Stop();
					PWM_Motor_2_Stop();
					Clock_PWM_Stop();
		
	}
}
/* [] END OF FILE */
