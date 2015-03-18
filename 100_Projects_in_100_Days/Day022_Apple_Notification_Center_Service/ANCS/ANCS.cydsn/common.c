/*******************************************************************************
* File Name: common.c
*
* Version: 1.0
*
* Description:
*  This file implements the common functionality in the ANCS project.
*
* Hardware Dependency:
*  CY8CKIT-042-BLE
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
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
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*******************************************************************************/


/*******************************************************************************
* Included headers
*******************************************************************************/
#include <project.h>


/*******************************************************************************
* Function definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: HexToDecimal()
********************************************************************************
* Summary:
* Hexadecimal to decimal converter
*
* Parameters:
* uint32 value: Hexadecimal value
* uint8 digit: Which digit to be obtained
*
* Return:
* char: the decimal equivalent of that digit 
*
* Theory:
* Converts hexadecimal to decimal and returns digit-by-digit value.
*
*******************************************************************************/
char HexToDecimal(uint32 value, uint8 digit)
{
    if(digit == 5)
    {
        value = (value%1000000)/100000;
        value = value + '0';
    }
    else if(digit == 4)
    {
        value = (value%100000)/10000;
        value = value + '0';
    }
    else if(digit == 3)
    {
        value = (value%10000)/1000;
        value = value + '0';
    }
    else if(digit == 2)
    {
        value = (value%1000)/100;
        value = value + '0';
    }
    else if(digit == 1)
    {
        value = (value%100)/10;
        value = value + '0';
    }
    else if (digit == 0)
    {
        value = value%10;
        value = value + '0';
    }
    else
    {
        value = ' ';    
    }
    
    return value;
}


/*******************************************************************************
* Function Name: HexToAscii()
********************************************************************************
* Summary:
* Hexadecimal to ASCII converter
*
* Parameters:
* uint32 value: Hexadecimal value
* uint8 digit: Which nibble to be obtained
*
* Return:
* char: the ASCII equivalent of that nibble
*
* Theory:
* Converts hexadecimal to ASCII
*
*******************************************************************************/
char HexToAscii(uint8 value, uint8 nibble)
{
    if(nibble == 1)
    {
        value = value & 0xf0;
        value = value >> 4;
        
        /*bit-shift the result to the right by four bits (i.e. quickly divides by 16)*/
        if (value >9)
        {
            value = value - 10 + 'A';
        }
        else
        {
            value = value + '0';
        }
    }
    else if (nibble == 0)
    {
        /*means use a bitwise AND to take the bottom four bits from the byte,
        0x0F is 00001111 in binary*/
        value = value & 0x0F;
        if (value >9)
        {
            value = value - 10 + 'A';
        }
        else
        {
            value = value + '0';
        }
    }
    else
    {
        value = ' ';    
    }
    
    return value;
}

/* [] END OF FILE */
