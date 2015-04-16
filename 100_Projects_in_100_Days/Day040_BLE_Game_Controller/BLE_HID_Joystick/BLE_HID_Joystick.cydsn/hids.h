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

#define JOYSTICK_TIMEOUT            (10u)     /* Сounts in hundreds of ms */

/* Keyboard scan codes for notification defined in section 
*  10 Keyboard/Keypad Page of HID Usage Tables spec ver 1.12 
*/
#define SIM_KEY_MIN                 (4u)        /* Minimum simulated key 'a' */
#define SIM_KEY_MAX                 (40u)       /* Maximum simulated key '0' */
#define JOYSTICK_JITTER_SIZE        (1u)

#define JOYSTICK_DATA_SIZE          (3u)


/***************************************
*       Function Prototypes
***************************************/
void HidsCallBack(uint32 event, void *eventParam);
void HidsInit(void);
void SimulateJoystick(void);


/***************************************
* External data references
***************************************/
extern uint16 Joystick_Simulation;
extern uint8 protocol;  
extern uint8 suspend;


/* [] END OF FILE */
