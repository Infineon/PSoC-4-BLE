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
#define LED_ON                              (1u)
#define LED_OFF                             (0u)

#define DISCONNECTED                        (0u)
#define ADVERTISING                         (1u)
#define CONNECTED                           (2u)
#define CYBLE_SCAN_ANY_CONN_ANY              (0u)

/* Delay value to produce blinking LED */
#define BLINK_DELAY                         (250u)

/* Constants for buttonState */
#define   BUTTON_IS_PRESSED                   (1u)
#define   BUTTON_IS_NOT_PRESSED               (0u)
#define   SCANNING_ADVERITISING             (5u)
#define   CONNECTED_SCANNING                (6u)
#define   SCANNING                          (7u)
#define   CYBLE_MAX_ADV_DEVICES             (8u)
#define   MASTER                            (9u)
#define   SLAVE                             (10u)
#define   START_SCANNING                    (1u)
#define   TRUE                              1u
#define   FALSE                             0u

/***************************************
*        Function Prototypes
***************************************/

void HandleLeds(void);
void MakeWordFromBytePtr(uint8 bytePtr[], uint16 *wordPtr);
void StackEventHandler(uint32 event, void * eventParam);
CYBLE_GAP_BONDED_DEV_ADDR_LIST_T  bondedDeviceList;

void DisplayBondedlist();
extern uint8 addPeripheral;
extern char HexToAscii(uint8 value, uint8 nibble);
/* [] END OF FILE */
