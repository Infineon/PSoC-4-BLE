/******************************************************************************
* Project Name		: HueControl
* File Name			: NecTransmitter.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner             : PRIA
*
********************************************************************************
* Copyright (2015-16), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/

/********************************************************************************
*	Contains all macros and function declaration used in the NecTransmitter.c  
********************************************************************************/
#if !defined(NECTRANSMITTER_H)
#define NECTRANSMITTER_H

#include <project.h>

/***************************Macro Declarations*******************************/
#define MAX_COMPARE_NEC_PULSE_TIMER             (65534u)

/* Macro definition for NEC pulse timer counts for different NEC timings. These 
 * constants are based on following NEC timings
 *   
 *    	      (time in us)	count for 6MHz clock
 *  leader	    9000	        54000 (= time_in_us * freq_in_MHz)
 *  spacer	    4500	        27000
 *  0 low	    560	            3360
 *  0 hih	    560	            3360
 *  1 low	    560	            3360
 *  1 high	    1690	        10140
 */
#define LEADER_COUNTS                   (54000u) 
#define SPACER_COUNTS                   (27000u)
#define BIT_0_1_LOW_COUNTS              (3360u)
#define BIT_0_HIGH_COUNTS               (3360u)
#define BIT_1_HIGH_COUNTS               (10140u)

/* Minimum (LSB) and maximum (MSB) bit numbers for uint16 */    
#define LSB                             (0u)
#define MSB                             (15u)

/* Macros for setting status of NecOutPin */    
#define PIN_LOW                         (0x00)
#define PIN_HIGH                        (0x01)

/* NEC address of RGB Flood Light */      
#define LED_FLOOD_LIGHT_ADDRESS         (0xEF00u)
    
/* NEC commands supported by RGB Flood Light */      
#define NEC_LED_INTENSITY_INCREASE      (0xFF00u)
#define NEC_LED_INTENSITY_DECREASE      (0xFE01u)
#define NEC_LED_LIGHT_OFF               (0xFD02u)
#define NEC_LED_LIGHT_ON                (0xFC03u)
#define NEC_LED_COLOR_RED               (0xFB04u)
#define NEC_LED_COLOR_GREEN             (0xFA05u)
#define NEC_LED_COLOR_BLUE              (0xF906u)
#define NEC_LED_COLOR_WHITE             (0xF807u)
#define NEC_LED_COLOR_RED1              (0xF708u)
#define NEC_LED_COLOR_GREEN1            (0xF609u)
#define NEC_LED_COLOR_BLUE1             (0xF50Au)
#define NEC_LED_FLASH                   (0xF40Bu)
#define NEC_LED_COLOR_RED2              (0xF30Cu)
#define NEC_LED_COLOR_GREEN2            (0xF20Du)
#define NEC_LED_COLOR_BLUE2             (0xF10Eu)
#define NEC_LED_STROBE                  (0xF00Fu)
#define NEC_LED_COLOR_RED3              (0xEF10u)
#define NEC_LED_COLOR_GREEN3            (0xEE11u)
#define NEC_LED_COLOR_BLUE3             (0xED12u)
#define NEC_LED_FADE                    (0xEC13u)
#define NEC_LED_COLOR_YELLOW            (0xEB14u)
#define NEC_LED_COLOR_GREEN4            (0xEA15u)
#define NEC_LED_COLOR_PINK              (0xE916u)
#define NEC_LED_SMOOTH                  (0xE817u)    
/****************************************************************************/    

/**************************Data Type Definitions*****************************/    
/* States in NEC data transmission */
typedef enum
{
    LEADER = 0,
    SPACER = 1 ,
    BIT_0_1_LOW = 2,
    BIT_0_HIGH = 3,
    BIT_1_HIGH = 4,
    IDLE = 5
}NEC_STATES;

/* Bit values to be transmitted through NEC */
typedef enum
{
    BIT_LOW = 0,
    BIT_HIGH = 1,
    NO_BIT = 2
}NEXT_BIT;

/* Different type of supported data in one NEC data packet */
typedef enum
{
    NECDATA_TYPE_ADDRESS = 0,
    NECDATA_TYPE_COMMAND = 1
}NECDATA_TYPE;

/* NEC status data type */
typedef enum
{
    NEC_TX_COMPLETE = 0,
    NEC_TX_BUSY = 1
}NECDATA_TX_STATUS;
/****************************************************************************/

/**************************Function Declarations*****************************/
extern void SendNecCode(uint16 address, uint16 command);
extern NEXT_BIT FindNextBit(void);
extern NECDATA_TX_STATUS ReturnNecStatus(void);
CY_ISR_PROTO(NecTimerCustomIsr);
/****************************************************************************/

#endif /* #include NECTRANSMITTER_H */
/* [] END OF FILE */
