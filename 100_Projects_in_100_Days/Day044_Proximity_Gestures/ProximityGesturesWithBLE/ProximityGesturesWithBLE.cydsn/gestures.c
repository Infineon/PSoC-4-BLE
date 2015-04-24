/***************************************************************************//**
* @file    gestures.c
* @version 1.0
* @authors SLAN
*
* @par Description
*    This file contains the implementation of proximity gestures in x and y directions
*
* @par Notes
*    -
*
* @par Hardware Dependency
*    1. CY8CKIT-042 PSoC4 Pioneer Kit
*    2. CY8CKIT-024 CapSense Proximity Shield
*
* @par References
*    1. 001-92239: AN92239 "Proximity sensing with CapSense" 
*
* @par Code Tested With
*    1. PSoC Creator 3.0 SP1 (3.0.0.3023)
*    2. GCC 4.7.3
*	 3. MDK 4.54.0.0
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
*   Included Headers
*******************************************************************************/
#include "gestures.h"
#include <project.h>

/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/
/*Decides the no of loops times each LED should be ON when LEDs are driven after gesture is completed*/
#define LED_ON_TIME_LOOPS			(5u)
/*Total count of LEDs in each axis*/
#define LED_COUNT					(3u)
/*Macros for LED names*/
#define	FIRST_LED					(0u)
#define	SECOND_LED					(1u)
#define	THIRD_LED					(2u)
/*Macro used to clear the LED loop time counter*/
#define	RESET_COUNTER				(0u)
/* To optimimize power consumption further, make this macro 1, so that LEDs are not driven */
#define DRIVE_LEDS                  (1u)

/*******************************************************************************
*   Module Variable and Constant Declarations with Applicable Initializations
*******************************************************************************/
/*Gesture structure instance for detecting X-Axis gestures*/
gesture XAxis;
/*Contains LED drive direction*/
LED_DRIVE LEDDriveSequence = TURN_ALL_LEDS_OFF;
LED_DRIVE previousLEDDriveSequence = TURN_ALL_LEDS_OFF;
/*LEC counter to track LED drive time after gesture complete*/
uint8 LEDCounter = RESET_COUNTER;


/*******************************************************************************
* Function Name: GestureDetection
****************************************************************************//**
* @par Summary
*    This function detects forward and backward gestures using
*	 two proximity sensors
*
* @return
*    None
*
* @param[in]
*	 Proximity sensor information, gesture structure instance reference 
*
* @param[out] 
*	 None    
*
* @pre
*    None
*
* @post
*    None
*
* @par Theory of Operation
*    Logic to say left to right gesture is completed is given below
*	 1) Both the proximity sensors are OFF
*	 2) Left proximity sensor turned ON
*	 3) Both Left and Right proximity sensors ON
* 	 4) Only right proximity sensor ON
*	 5) Both the proximity sensors are OFF
* 	 To determine invalid gestures following additional checks are implemented
*    1) Configurable debounce for each state
*
* @par Notes
*    None
*
**//***************************************************************************/
void GestureDetection(SENSOR_NAMES Sensor1, SENSOR_NAMES Sensor2, gesture* axisPtr)
{    
	ZONE_NAMES currentZone = INVALID_ZONE;
	/*This function has different cases based on the proximity sensor status
	*	and each state follow similar steps which are given below*/
	/*Steps for any case
	*	1) Reset debounce counters on first entry to this state
	*	2) Update the current zone 
	*	3) Based on current and previous zone decide if it is forward or backward gesture
	*	4) Apply debounce before taking final decision on gesture tracking
	*	5) Update the previous zone */

	/* Sensor1 ON, Sensor2 OFF , Zone 1 */	
    if(CapSense_CheckIsWidgetActive(Sensor1) && (CapSense_CheckIsWidgetActive(Sensor2)==0))
    {		
		currentZone = ZONE_ONE;
		/*Check forward and backward gesture in this zone*/
		GestureForwardDetection(axisPtr,currentZone);
		GestureBackwardDetection(axisPtr,currentZone);	
    }
	/* Sensor1 OFF, Sensor2 ON , Zone 3 */
    else if((CapSense_CheckIsWidgetActive(Sensor1) == 0) && CapSense_CheckIsWidgetActive(Sensor2))
    {
		currentZone = ZONE_THREE;
		/*Check forward and backward gesture in this zone*/
		GestureForwardDetection(axisPtr,currentZone);
		GestureBackwardDetection(axisPtr,currentZone);	
    }
    /* Sensor1 ON, Sensor2 ON , Zone 2 */
    else if(CapSense_CheckIsWidgetActive(Sensor1) && CapSense_CheckIsWidgetActive(Sensor2))
    {
		currentZone = ZONE_TWO;
		/*Check forward and backward gesture in this zone*/
		GestureForwardDetection(axisPtr,currentZone);
		GestureBackwardDetection(axisPtr,currentZone);				
    }     
    else 
    {
		/*Both proximity sensors are OFF*/
		if (axisPtr->forwardGestureTracking)
		{
			if (axisPtr->previousZone == ZONE_THREE)
			{
				/*If previous zone is ZONE_THREE (Only sensor2 is ON) then forward gesture is complete*/
				axisPtr->forwardGestureComplete = true;
				/*Decide the LED direction based on X-Axis or Y-Axis*/
				if (Sensor1 == LEFT_SENSOR)
				{
					/*Based on sensors being used to detect X-Axis or Y-Axis is determined*/
					LEDDriveSequence = LEFT_TO_RIGHT;
				}
				else
				{
					LEDDriveSequence = BOTTOM_TO_TOP;
				}
				/*Reset the LED on time counter to start the time*/
				LEDCounter = RESET_COUNTER;
			}
		}			

		if (axisPtr->backwardGestureTracking)
		{
			if (axisPtr->previousZone == ZONE_ONE)
			{
				/*If previous zone is ZONE_ONE (Only sensor1 is ON) then backward gesture is complete*/
				axisPtr->backwardGestureComplete = true;
				/*Decide the LED direction based on X-Axis or Y-Axis*/
				if (Sensor1 == LEFT_SENSOR)
				{
					/*Based on sensors being used to detect X-Axis or Y-Axis is determined*/
					LEDDriveSequence = RIGHT_TO_LEFT;
				}
				else
				{
					LEDDriveSequence = TOP_TO_BOTTOM;
				}
				LEDCounter = RESET_COUNTER;
			}			
		}			
		/*Reset the gesture to start tracking the next gestures*/
		GestureVariableInit(axisPtr);
	}
	/*Drive LEDs*/
	DriveLEDs(currentZone);
}


/*******************************************************************************
* Function Name: GestureForwardDetection
****************************************************************************//**
* @par Summary
*    This function is written to prevent code duplication for forward gesture tracking.
*	 If the difference between the current zone and previous zone is >1 then the
*	 gesture tracking is turned OFF, Debounce for each state is also applied
*
* @return
*    None
*
* @param[in]
*	 Gesture structure instance reference 
*
* @param[out] 
*	 None    
*
* @pre
*    None
*
* @post
*    None
*
* @par Theory of Operation
*	 None
*
* @par Notes
*    None
*
**//***************************************************************************/

void GestureForwardDetection(gesture* axisPtr, ZONE_NAMES currentZone)
{
	bool StateCondition = false;
	/*First entry into this state reset the state debounce counters*/
	if(axisPtr->currentZone != currentZone)
	{
		axisPtr->forwardDebounceCounter = GESTURE_STATE_DEBOUNCE;
		axisPtr->backwardDebounceCounter = GESTURE_STATE_DEBOUNCE;
	}
	/*Update current zone, previous and current zone will be used to track 
	  whether gesture is forward or backward*/
	axisPtr->currentZone = currentZone;	
	
	if (axisPtr->forwardGestureTracking)
	{
		if (axisPtr->previousZone != axisPtr->currentZone)
		{
			if (axisPtr->currentZone - axisPtr->previousZone != 1)
			{
				axisPtr->forwardGestureTracking = false;
			}
		}
	}
	/*Track forward gesture */
	/* Condition = Previously no signal on both prox sensor, currently only signal on sensor1 for first zone 
	For other Zones current - previous should be equal to one to continue tracking gestures*/
	if(currentZone == ZONE_ONE)
	{
		if(INVALID_ZONE == axisPtr->previousZone)
		{
			StateCondition = true;
		}
	}
	else
	{
		StateCondition = axisPtr->forwardGestureTracking;
	}
	
	if(StateCondition)
	{
		/*Apply state debounce*/
		if(axisPtr->forwardDebounceCounter >0)
		{
			axisPtr->forwardDebounceCounter--;
		}
	}
	else
	{
		/*Reset debounce counter for invalid condition*/
		axisPtr->forwardDebounceCounter = GESTURE_STATE_DEBOUNCE;
	}
	
	/*Condition valid for forward gesture for debounce time , record gesture tracking*/
	if(axisPtr->forwardDebounceCounter == 0)
	{
		axisPtr->previousZone = currentZone;
		if(currentZone == ZONE_ONE)
		{
			/*Valid zone signal conditions for debounce time start tracking the forward gesture*/
			axisPtr->forwardGestureTracking = true;
		}
	}
}

/*******************************************************************************
* Function Name: GestureBackwardDetection
****************************************************************************//**
* @par Summary
*    This function is written to prevent code duplication for backward gesture tracking.
*	 If the difference between the previous zone and current zone is >1 then the
*	 gesture tracking is turned OFF
*
* @return
*    None
*
* @param[in]
*	 Gesture structure instance reference and current 
*
* @param[out] 
*	 None    
*
* @pre
*    None
*
* @post
*    None
*
* @par Theory of Operation
*	 None
*
* @par Notes
*    None
*
**//***************************************************************************/
void GestureBackwardDetection(gesture* axisPtr, ZONE_NAMES currentZone)
{
	bool StateCondition = false;
	
	if (axisPtr->backwardGestureTracking)
	{
		if (axisPtr->previousZone != axisPtr->currentZone)
		{
			if (axisPtr->previousZone - axisPtr->currentZone != 1)
			{
				axisPtr->backwardGestureTracking = false;
			}
		}
	}
	
	/*Track backward gesture */
	/* Condition = Previously no signal on both prox sensor, currently only signal on sensor2 for first zone 
	For other Zones previous - cureent should be equal to one to continue tracking gestures*/
	if(currentZone == ZONE_THREE)
	{
		if(INVALID_ZONE == axisPtr->previousZone)
		{
			StateCondition = true;
		}
	}
	else
	{
		StateCondition = axisPtr->backwardGestureTracking;
	}
	
	/*Debounce check for backward gesture in this state*/
	if(StateCondition)
	{
		if(axisPtr->backwardDebounceCounter >0)
		{
			axisPtr->backwardDebounceCounter--;
		}
	}
	else
	{
		axisPtr->backwardDebounceCounter = GESTURE_STATE_DEBOUNCE;
	}
	/*Valid backward gesture detected for debounce times*/
	if(axisPtr->backwardDebounceCounter == 0)
	{
		axisPtr->previousZone = currentZone;
		if(currentZone == ZONE_THREE)
		{
			axisPtr->backwardGestureTracking = true;
		}
	}
}

/*******************************************************************************
* Function Name: GestureVariableInit
****************************************************************************//**
* @par Summary
*    All the gesture tracking variables are initialized to default values
*
* @return
*    None
*
* @param[in]
*	 Gesture structure instance reference 
*
* @param[out] 
*	 None    
*
* @pre
*    None
*
* @post
*    None
*
* @par Theory of Operation
*	 None
*
* @par Notes
*    None
*
**//***************************************************************************/

void GestureVariableInit(gesture* axisPtr)
{
	/*Set the current and previous zone to invalid*/
	axisPtr->previousZone = INVALID_ZONE;
	axisPtr->currentZone = INVALID_ZONE;
	/*Turn OFF forward tracking and tracking complete variables*/
	axisPtr->forwardGestureTracking = false;
	axisPtr->forwardGestureComplete = false;
	/*Turn OFF backward tracking and tracking complete variables*/
	axisPtr->backwardGestureTracking = false;
    axisPtr->backwardGestureComplete = false;
	/*Reset X-Axis and Y-Axis debounce counters*/
	axisPtr->forwardDebounceCounter = GESTURE_STATE_DEBOUNCE;
	axisPtr->backwardDebounceCounter = GESTURE_STATE_DEBOUNCE;
}

/*******************************************************************************
* Function Name: DriveLEDs
****************************************************************************//**
* @par Summary
*    Based on the gesture detected LEDs are driven for visual feedback
*
* @return
*    None
*
* @param[in]
*	 current zone where the hand is present. 
*
* @param[out] 
*	 None    
*
* @pre
*    None
*
* @post
*    None
*
* @par Theory of Operation
*	 None
*
* @par Notes
*    None
*
**//***************************************************************************/

void DriveLEDs(ZONE_NAMES currentZone)
{
    #if(DRIVE_LEDS)
	/*Turn off all the LEDs and based on conditions turn ON required before driving LEDs*/
	LED_STATES LED1 = LED_OFF_GESTURES;
	LED_STATES LED2 = LED_OFF_GESTURES;
	LED_STATES LED3 = LED_OFF_GESTURES;
	LED_STATES LED4 = LED_OFF_GESTURES;
	LED_STATES LED5 = LED_OFF_GESTURES;
    #endif 

	#if(LED_DRIVE_SEQUENCE)
	{
        #if(DRIVE_LEDS)
		/*Three LEDs are used as visual feedback in X direction and three in Y direction
		LEDs are driven in the same direction as gesture
		LED1, LED2 and LED3 are placed from left to right
		LED5, LED2 and LED4 are placed from bottom to top
		Each LED is driven for 'N' loop times as configured in the macro*/
		
		switch(LEDDriveSequence)
		{
			case LEFT_TO_RIGHT:
			{
				/*Left to Right gesture is detected turn ON LEDs one after the other in the same order */
				if ((LEDCounter / LED_ON_TIME_LOOPS) == FIRST_LED)
				{
					LED1 = LED_ON_GESTURES;
				}
				else if ((LEDCounter / LED_ON_TIME_LOOPS) == SECOND_LED)
				{
					LED2 = LED_ON_GESTURES;
				}
				else if ((LEDCounter / LED_ON_TIME_LOOPS) == THIRD_LED)
				{
					LED3 = LED_ON_GESTURES;
				}
				break;
			}
			case RIGHT_TO_LEFT:
			{
				/*Right to Left gesture is detected turn ON LEDs one after the other in the same order */
				if ((LEDCounter / LED_ON_TIME_LOOPS) == FIRST_LED)
				{
					LED3 = LED_ON_GESTURES;
				}
				else if ((LEDCounter / LED_ON_TIME_LOOPS) == SECOND_LED)
				{
					LED2 = LED_ON_GESTURES;
				}
				else if ((LEDCounter / LED_ON_TIME_LOOPS) == THIRD_LED)
				{
					LED1 = LED_ON_GESTURES;
				}
				break;
			}
			case BOTTOM_TO_TOP:
			{
				/*Bottom to Top gesture is detected turn ON LEDs one after the other in the same order */
				if ((LEDCounter / LED_ON_TIME_LOOPS) == FIRST_LED)
				{
					LED5 = LED_ON_GESTURES;
				}
				else if ((LEDCounter / LED_ON_TIME_LOOPS) == SECOND_LED)
				{
					LED2 = LED_ON_GESTURES;
				}
				else if ((LEDCounter / LED_ON_TIME_LOOPS) == THIRD_LED)
				{
					LED4 = LED_ON_GESTURES;
				}
				break;
			}
			case TOP_TO_BOTTOM:
			{
				/*Top to Bottom gesture is detected turn ON LEDs one after the other in the same order */
				if ((LEDCounter / LED_ON_TIME_LOOPS) == FIRST_LED)
				{
					LED4 = LED_ON_GESTURES;
				}
				if ((LEDCounter / LED_ON_TIME_LOOPS) == SECOND_LED)
				{
					LED2 = LED_ON_GESTURES;
				}
				if ((LEDCounter / LED_ON_TIME_LOOPS) == THIRD_LED)
				{
					LED5 = LED_ON_GESTURES;
				}
				break;
			}
			default:
			{
				LED1 = LED_OFF_GESTURES;
		 		LED2 = LED_OFF_GESTURES;
				LED3 = LED_OFF_GESTURES;
				LED4 = LED_OFF_GESTURES;
				LED5 = LED_OFF_GESTURES;
			}
		}
    #endif 
		/*LED counter to track loop timings for LED drive*/
		if (LEDDriveSequence != TURN_ALL_LEDS_OFF)
		{
			LEDCounter++;
			if(LEDCounter >= LED_ON_TIME_LOOPS * LED_COUNT)
			{
				/*LED drive based on gesture is completed , change the LED sequence
				to turn OFF all the LEDs*/
				LEDDriveSequence = TURN_ALL_LEDS_OFF;
			}
		}
	}
	#else
    #if(DRIVE_LEDS)
	{
		/*Three positions are detected using two proximity sensors
		*	LED1, LED2 and LED3 are the LEDs on the X-axis from left to right
		*	LED5, LED2 and LED4 are the LEDs on the Y-axis from bottom to top
		*	When X-axis gestures are being detected LEDs drive conditions are given below
		*	Position1 - left sensor ON and right sensor OFF - LED1 turned ON
		*	Position2 - left sensor ON and right sensor ON - LED2 turned ON
		*	Position3 - left sensor OFF and right sensor ON - LED3 turned ON
		*	When Y-axis gestures are being detected LEDs drive conditions are given below
		*	Position1 - bottom sensor ON and top sensor OFF - LED5 turned ON
		*	Position2 - bottom sensor ON and top sensor ON - LED2 turned ON
		*	Position3 - bottom sensor OFF and top sensor ON - LED4 turned ON*/
		switch(currentZone)
		{
			case ZONE_ONE:
			{
				#if(GESTURE_AXIS)
				{
					LED1 = LED_ON;
				}
				#else
				{
					LED5 = LED_ON;
				}
				#endif
				break;
			}
			case ZONE_TWO:
			{
				LED2 = LED_ON;
				break;
			}
			case ZONE_THREE:
			{
				#if(GESTURE_AXIS)
				{
					LED3 = LED_ON;
				}
				
				#else
				{
					LED4 = LED_ON;
				}
				#endif
				break;
			}
			default:
			{
				LED1 = LED_OFF;
		 		LED2 = LED_OFF;
				LED3 = LED_OFF;
				LED4 = LED_OFF;
				LED5 = LED_OFF;
			}
		}
	}
    #endif 
	#endif
    
    #if(DRIVE_LEDS)
	/*Drive all the LEDs*/
	LED1_Write(LED1);
	LED2_Write(LED2);
	LED3_Write(LED3);
	LED4_Write(LED4);
	LED5_Write(LED5);
    #endif 
}


/**** END OF FILE ****/
