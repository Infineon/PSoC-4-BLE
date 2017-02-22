/*******************************************************************************
* File Name: common.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>


/***************************************
*        API Constants
***************************************/

/* LED states */
#define LED_ON                              (0u)
#define LED_OFF                             (1u)
#define SERVICE_UUID                        0x03
#define SERVICE_SOLICITATION                0X14
#define TRUE                                  1u
#define FALSE                                 0u

#define DISCONNECTED                        (0u)
#define ADVERTISING                         (1u)
#define CONNECTED                           (2u)
#define CONNECTED_SERVER_DISCOVERED         (3u)

/* Delay value to produce blinking LED */
#define BLINK_DELAY                         (250u)

/* Constants for buttonState */
#define BUTTON_IS_PRESSED                   (1u)
#define BUTTON_IS_NOT_PRESSED               (0u)


/***************************************
*        Function Prototypes
***************************************/

void HandleLeds(void);
uint8 ButtonPressed(void);
void MakeWordFromBytePtr(uint8 bytePtr[], uint16 *wordPtr);
void StackEventHandler(uint32 event,void * eventParam);
void DisEventHandler(uint32,void* eventParam);
void TpsServiceAppEventHandler(uint32 event, void *eventParam);
int ConvertTxPowerlevelToInt8(CYBLE_BLESS_PWR_LVL_T pwrLevel);
void DecreaseTxPowerLevelValue(CYBLE_BLESS_PWR_LVL_T * pwrLevel);

CY_ISR_PROTO(ButtonPressInt);


/***************************************
*        External data references
***************************************/
extern uint8                state;
extern uint16               advBlinkDelayCount;
extern uint8                advLedState;
extern uint8                buttonState;
extern uint16               supportedCategories;


/* [] END OF FILE */
