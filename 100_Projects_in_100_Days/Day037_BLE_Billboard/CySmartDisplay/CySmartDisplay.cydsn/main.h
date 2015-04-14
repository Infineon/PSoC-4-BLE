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
#ifndef _MAIN_H_
#define _MAIN_H_

#include <project.h>
#include <stdio.h>

#define TRUE            1
#define FALSE           0
#define HIGH            1
#define LOW             0
#define SPI_DELAY       150
#define MODULE_COUNT    6

uint8   deviceConnected;   // TRUE when Device is connected to Server
extern  char buffer[180];
    
/*******************************************************************************
* RGB LED Defines
*******************************************************************************/
#define RED_LED_ON()	{ RED_LED_Write(0); GREEN_LED_Write(1); BLUE_LED_Write(1);}	
#define GREEN_LED_ON()	{ RED_LED_Write(1); GREEN_LED_Write(0); BLUE_LED_Write(1);}	
#define BLUE_LED_ON()	{ RED_LED_Write(1); GREEN_LED_Write(1); BLUE_LED_Write(0);}	
#define ALL_LED_OFF()   { RED_LED_Write(1); GREEN_LED_Write(1); BLUE_LED_Write(1);}	

/*******************************************************************************
* Function declaration
*******************************************************************************/
void StackEventHandler(uint32 event, void *eventParam);
void StandardDisplayInit(void);
void DisplayMessage(char *message, uint8 length);
void DisplayBrightness(uint8 level);
void DispaySpeed(uint8 speed);

#endif
/* [] END OF FILE */
