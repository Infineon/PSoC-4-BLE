/******************************************************************************
* Project Name		: HueControl
* File Name			: NecTransmitter.c
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
#include "NecTransmitter.h"

/************************** Constant definitions *****************************/
#define ONE   (0x01u)
/****************************************************************************/

/**************************Variable Declarations*****************************/
/* necAddress stores NEC address of RGB Flood light */
volatile static uint16 necAddress;

/* necAddress stores command to be sent to RGB Flood light to control color or 
 * intensity etc.
 */
volatile static uint16 necCommand;

/* necBusyFlag indicates the transmission status of previous address and 
 * command.
 */
volatile NECDATA_TX_STATUS necBusyFlag = NEC_TX_COMPLETE;
/****************************************************************************/

/******************************************************************************
* Function Name: SendNecCode
*******************************************************************************
* Summary:
*        This function updates necAddress and necCommand global variables and
*        starts the NecPulseTimer to initiate transmission of NEC data.
*        It is a non-blocking function, the NEC data transmission is perfomeds
*        in the NecPulseTimer isr.
*
* Parameters:
*  address:	nec address of RGB Flood light.
*  command:	nec command for particular operation - color or brightness or 
*           status control of flood light.
*
* Return:
*  void
*
*******************************************************************************/
void SendNecCode (uint16 address, uint16 command)
{
    /* Update the necBusyFlag to indicate busy status */
    necBusyFlag = NEC_TX_BUSY;
    
    /* Update global variables used by NecPulseTimer isr */
    necAddress = address;
    necCommand = command;    
    
    /* Write compare value for initiating a leader pulse*/
    NecPulseTimer_WriteCompare(MAX_COMPARE_NEC_PULSE_TIMER);  
    
    /* Start NecPulseTimer and associated isr */
    NecTimerIsr_StartEx(NecTimerCustomIsr);
    NecPulseTimer_Start();     
}

/*******************************************************************************
* Function Name: ReturnNecStatus
********************************************************************************
* Summary:
*        This function returns the status of previous NEC data transmission.
*
* Parameters:
*  void
*
* Return:
*  necBusyFlag: Indicates the status of previous NEC data transmission
*
*******************************************************************************/
NECDATA_TX_STATUS ReturnNecStatus(void)
{
    /* Return the value of necBusyFlag */
    return(necBusyFlag);
}

/*******************************************************************************
* Function Name: FindNextBit
********************************************************************************
* Summary:
*        This function returns the next bit to be transmitted. It is called
*        by the NecTimerIsr to find value of successive bits to be transmitted.
*
* Parameters:
*  void
*
* Return:
*  nextBit: Indicates the next bit to be transmitted.
*
*******************************************************************************/
NEXT_BIT FindNextBit(void)
{
    /* bitNo increments every time this function is called. It starts from 
     * LSB to MSB for address data, and then resets to LSB, and counts LSB to MSB  
     * for command data.
     */
    static uint8 bitNo = LSB;
    
    /* nextBit indicates the nextBit to be transmitted. If both address and 
     * command are already sent, it indicates NO_BIT
     */
    NEXT_BIT nextBit = NO_BIT;
    
    /* necDataType is used to track whether address or command is being 
     * transmitted when this function is called.
     */
    static NECDATA_TYPE necDataType = NECDATA_TYPE_ADDRESS;
    
    /* data is a temporary variable, it contains either necAddress or necCommand
     * depending on necDataType
     */
    uint16 data;
    
    /* Find which data is to be transmitted. 
     * necDataType is initialized to NECDATA_TYPE_ADDRESS. Once the bitNo 
     * increments to a value greater than MSB, the next data should be
     * NECDATA_TYPE_COMMAND and bitNo should start incrementing from LSB. 
     * If previous data was already NECDATA_TYPE_COMMAND 
     * and bitNo increments to value greater than MSB, data transmission is
     * complete and thus the function should return NO_BIT.
     */
    if (bitNo > MSB)
    {
        bitNo = LSB;
        if(necDataType == NECDATA_TYPE_ADDRESS)
        {
            necDataType = NECDATA_TYPE_COMMAND;
        }
        else
        {
            necDataType = NECDATA_TYPE_ADDRESS;
            nextBit = NO_BIT;
            return nextBit;
        } 
    }
    
    /* Update temporary variable data based on necDataType. */
    if(necDataType == NECDATA_TYPE_ADDRESS)
    {
        data =  necAddress;
    }
    else
    {
        data = necCommand;   
    }
    
    /* Find value of next bit of data to be transmitted. 
     * If next bit is one, send BIT_HIGH, else send BIT_LOW
     */
    if((ONE << bitNo) & data)
    {
        nextBit = BIT_HIGH;
    }
    else
    {
        nextBit = BIT_LOW;
    }
    
    /* Increment bitNo for next call to this function. */
    bitNo++;
    
    /* return the nextBit to be transmitted. */
    return nextBit;
}

/*******************************************************************************
* Function Name: NecTimerCustomIsr
********************************************************************************
* Summary:
*        ISR routine for NecPulseTimer. This routine maintains the state of 
*        NEC transmissions, and sends the data to necOut line accordingly.
*        Note that the data sent to necOut line is in same format as that what 
*        an IR receiver drives on receiving NEC data over IR link.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
CY_ISR(NecTimerCustomIsr)
{
    static NEC_STATES necState = LEADER;
    NEC_STATES nextNecState = IDLE;
    uint16 currentCount = 0u;
    NEXT_BIT nextBit;
    
    /* Read the current counter value to time the next state */
    currentCount = NecPulseTimer_ReadCounter();
    
    switch (necState)
    {
        case LEADER:
        {
            /* Make the pin state low for leader pulse */
            NecOutPin_Write(PIN_LOW);
            
            /* Set the compare value such that leader state is maintained
             * for only 9ms.
             */
            NecPulseTimer_WriteCompare(currentCount - LEADER_COUNTS);
            
            /* Set the next state to spacer per nec Protocol*/
            nextNecState = SPACER;
            break;
        }    
        case SPACER:
        {
            /* Make the pin state high for spacer pulse */
            NecOutPin_Write(PIN_HIGH);
            
            /* Set the compare value such that spacer state is maintained
             * for 4.5ms per NEC protocol.
             */
            NecPulseTimer_WriteCompare(currentCount - SPACER_COUNTS);
           
            /* Set the next state to low period for a bit transmission */
            nextNecState = BIT_0_1_LOW;
            break;
        }
        case BIT_0_1_LOW:
        {
            /* Make the pin state low for bit transmission */
            NecOutPin_Write(PIN_LOW);
            
            /* Set compare value for maintaining low state for a bit for 
             * 560 us.
             */
            NecPulseTimer_WriteCompare(currentCount - BIT_0_1_LOW_COUNTS);
            
            /* Find the next bit and set nec state to BIT_1_HIGH or 
             * BIT_0_HIGH depending on the next bit to be tranmitted.
             */
            nextBit = FindNextBit();   
            if(nextBit == BIT_HIGH)
            {
                nextNecState = BIT_1_HIGH;   
            }
            else if(nextBit == BIT_LOW)
            {
                nextNecState = BIT_0_HIGH;
            }
            else
            {
                nextNecState = IDLE;   
            }
            break;
        }
        case BIT_0_HIGH:
        {
            /* Make the pin state high and set compare value 
             * for 560us to indicate bit 0 
             */
            NecOutPin_Write(PIN_HIGH);
            NecPulseTimer_WriteCompare(currentCount - BIT_0_HIGH_COUNTS);
            
            /* Set the next state to low period for a bit transmission */
            nextNecState = BIT_0_1_LOW;
            break;
        }
        case BIT_1_HIGH:
        {
             /* Make the pin state high and set compare value 
             * for 1.69ms to indicate bit 1 
             */
            NecOutPin_Write(PIN_HIGH);
            NecPulseTimer_WriteCompare(currentCount - BIT_1_HIGH_COUNTS);
            
            /* Set the next state to low period for a bit transmission */
            nextNecState = BIT_0_1_LOW;
            break;
        }
        case IDLE:
        {
            /* Make the pin state high to indicate bus IDLE */
            NecOutPin_Write(PIN_HIGH);
            
            /* Set next State to leader for the next nec data */
            nextNecState = LEADER;
            
            /* Current data transmission is complete at this stage, so stop
             * the timer and set the necBusyFlag to indicate transmit complete.
             */
            NecPulseTimer_Stop();
            necBusyFlag = NEC_TX_COMPLETE;
            break;
        }
        default:
        {
            
            break;
        }
    }
    
    necState = nextNecState;
    
    /* Clear the timer's sticky interrupt. */
    NecPulseTimer_ClearInterrupt(NecPulseTimer_INTR_MASK_CC_MATCH);
}    
/* [] END OF FILE */
