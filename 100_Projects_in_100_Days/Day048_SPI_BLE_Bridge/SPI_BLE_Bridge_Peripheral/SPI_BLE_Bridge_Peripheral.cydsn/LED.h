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

	
#define DISCON_LED_ON()		{ DISCON_LED_Write(0); CONNECT_LED_Write(1); ADV_LED_Write(1);}	
#define CONNECT_LED_ON()	{ DISCON_LED_Write(1); CONNECT_LED_Write(0); ADV_LED_Write(1);}	
#define ADV_LED_ON()		{ DISCON_LED_Write(1); CONNECT_LED_Write(1); ADV_LED_Write(0);}	
#define ALL_LED_OFF()   	{ DISCON_LED_Write(1); CONNECT_LED_Write(1); ADV_LED_Write(1);}	
	
#endif	/* _LED_H_ */


/* [] END OF FILE */
