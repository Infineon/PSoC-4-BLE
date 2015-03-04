/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef _LED_H_
#define _LED_H_
	
#include "project.h"

	
#define RED_LED_ON()	{ RED_LED_Write(0); GREEN_LED_Write(1); BLUE_LED_Write(1);}	
#define GREEN_LED_ON()	{ RED_LED_Write(1); GREEN_LED_Write(0); BLUE_LED_Write(1);}	
#define BLUE_LED_ON()	{ RED_LED_Write(1); GREEN_LED_Write(1); BLUE_LED_Write(0);}	
#define ALL_LED_OFF()   { RED_LED_Write(1); GREEN_LED_Write(1); BLUE_LED_Write(1);}	
	
#endif	


/* [] END OF FILE */
