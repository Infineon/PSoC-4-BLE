/***************************************************************************//**
* @file    gestures.h
* @version 1.0
* @authors SLAN
*
* @par Description
*    This file contains the public interface of gestures
*
* @par Notes
*	None
*
* @par Hardware Dependency
*    1. CY8CKIT-042 PSoC4 Pioneer Kit
*    2. CY8CKIT-024 CapSense Proximity Shield
*
* @par References
*    1. 001-92239: AN92239 "Proximity sensing with CapSense"
*
* @par Code Tested With
*    1. PSoC Creator  3.0 SP1 (3.0.0.3023)
*    2. GCC 4.7.3
*	 3. MDK
*
**//****************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
********************************************************************************
* All rights reserved. 
* This software, including source code, documentation and related 
* materials (“Software”), is owned by Cypress Semiconductor 
* Corporation (“Cypress”) and is protected by and subject to worldwide 
* patent protection (United States and foreign), United States copyright 
* laws and international treaty provisions. Therefore, you may use this 
* Software only as provided in the license agreement accompanying the 
* software package from which you obtained this Software (“EULA”). 
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive, 
* non-transferable license to copy, modify and compile the Software source code
* solely for your use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.

* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use 
* of the Software or any product or circuit described in the Software. Cypress
* does not authorize its products for use in any products where a malfunction
* or failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such  
* system or application assumes all risk of such use and in doing so agrees to  
* indemnify Cypress against all liability. 
*******************************************************************************/

/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/
#ifndef __GESTURES_H    /* Guard to prevent multiple inclusions */
#define __GESTURES_H

/*******************************************************************************
*   Included Headers
*******************************************************************************/
#include "CapSense.h"
#include <cytypes.h>
#include <stdbool.h>	

/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/
/*Marcos used for turning LED ON and OFF*/
#define TURN_LED_OFF				(1u)
#define TURN_LED_ON					(0u)

/*Condition for state change in state machine should be valid for DEBOUNCE
*	number of loops (each loop takes ~23ms). Increase debounce under high
*	high noisy condition. Greater values of debounce will reduce gesture 
*	detection speed*/
#define GESTURE_STATE_DEBOUNCE		(1u)

/*Decides when LEDs should be drive with respect to gesture detection*/
#define LED_DRIVE_DURING_GESTURE    (0u)
#define	LED_DRIVE_AFTER_GESTURE		(1u)
#define LED_DRIVE_SEQUENCE          (LED_DRIVE_AFTER_GESTURE)

/*Decides in which gesture should be detected*/
#define XAXIS                       (1u)
#define GESTURE_AXIS				(XAXIS)

/*******************************************************************************
*   Data Type Definitions
*******************************************************************************/
/*Enum used for different states in state machine*/
typedef enum
{
	ZONE_ONE		= 0x01u,
	ZONE_TWO		= 0x02u,
	ZONE_THREE		= 0x03u,
	INVALID_ZONE	= 0xFFu
}ZONE_NAMES;

/*Enum used for sesnor numbering for Kit-24*/
typedef enum
{
	LEFT_SENSOR 	= 0x00u,
	RIGHT_SENSOR    = 0x01u,
	BOTTOM_SENSOR	= 0x02u,
	TOP_SENSOR	 	= 0x03u
} SENSOR_NAMES;

/*Enum used to turn ON and OFF LEDs*/
typedef enum
{
	LED_ON_GESTURES			=0x00,
	LED_OFF_GESTURES	    =0x01
}LED_STATES;

/*******************************************************************************
*   Data Type Definitions
*******************************************************************************/
/*Enum indicates in which direction LEDs need to be driven*/
typedef enum
{
	LEFT_TO_RIGHT 		= 0x00u,
	RIGHT_TO_LEFT    	= 0x01u,
	BOTTOM_TO_TOP		= 0x02u,
	TOP_TO_BOTTOM 		= 0x03u,
	TURN_ALL_LEDS_OFF	= 0xFFu
} LED_DRIVE;

/*Structure for gesture tracking*/
typedef struct {
	/*Contains previous state data in state machine*/
	ZONE_NAMES previousZone ;
	/*contains the current state data in state machine*/
	ZONE_NAMES currentZone;
	/*Used for tracking forward gesture*/
	bool  forwardGestureTracking;
	/*Indicates whether forward gesture is completed or not*/
	bool  forwardGestureComplete;
	/*Used for tracking backward gesture*/
	bool  backwardGestureTracking;
	/*Indicates whether backward gesture is completed or not*/
	bool  backwardGestureComplete;
	/*Debounce for states in forward direction*/
	uint8 forwardDebounceCounter;
	/*Debounce for states in backward direction*/
	uint8 backwardDebounceCounter;
}gesture;

	
/*******************************************************************************
*   Extern Variable and Constant Declarations
*******************************************************************************/
/*Gesture structure instance for X-Axis gesture detection*/
extern gesture XAxis;

extern LED_DRIVE LEDDriveSequence;

extern LED_DRIVE previousLEDDriveSequence;

/*******************************************************************************
*   Function Declarations
*******************************************************************************/

/*API detects gestures and has the state machine implemented*/
void GestureDetection(SENSOR_NAMES, SENSOR_NAMES, gesture*);
/*API contains the common code in all the states for forward gesture*/
void GestureForwardDetection(gesture*, ZONE_NAMES);
/*API contains the common code in all the states for backward gesture*/
void GestureBackwardDetection(gesture*, ZONE_NAMES);
/*Initialize the gesture structure with default values*/
void GestureVariableInit(gesture*);
/*Drives LEDs based on the gestures and macro LED_DRIVE_AFTER_GESTURE*/
void DriveLEDs(ZONE_NAMES);

#endif /* #ifndef __CAPSENSEFILTERS_H */
