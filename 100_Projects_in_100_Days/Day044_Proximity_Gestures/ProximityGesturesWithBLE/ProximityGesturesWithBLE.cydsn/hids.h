/*******************************************************************************
* File Name: hids.h
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
*          Constants
***************************************/
#define KEYBOARD_DATA_SIZE          (8u)

/***************************************
*       Function Prototypes
***************************************/
void HidsCallBack(uint32 event, void *eventParam);
void HidsInit(void);
void SimulateKeyboard(void);
uint8 SendKeyForDetectedGesture(void);

/***************************************
* External data references
***************************************/
extern uint16 keyboardSimulation; 
extern uint8 suspend;

/* [] END OF FILE */
