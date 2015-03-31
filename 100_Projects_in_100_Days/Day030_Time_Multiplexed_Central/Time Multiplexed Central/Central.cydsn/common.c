/*******************************************************************************
* File Name: debug.c
*
* Version: 1.0
*
* Description:
*  This file contains functions for printf functionality.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include <stdio.h>
#include "common.h"

#if defined(__ARMCC_VERSION)

/* For MDK/RVDS compiler revise fputc function for printf functionality */
struct __FILE
{
    int handle;
};

enum
{
    STDIN_HANDLE,
    STDOUT_HANDLE,
    STDERR_HANDLE
};

FILE __stdin = {STDIN_HANDLE};
FILE __stdout = {STDOUT_HANDLE};
FILE __stderr = {STDERR_HANDLE};

int fputc(int ch, FILE *file)
{
    int ret = EOF;

    switch( file->handle )
    {
        case STDOUT_HANDLE:
            UART_DEB_UartPutChar(ch);
            ret = ch ;
            break ;

        case STDERR_HANDLE:
            ret = ch ;
            break ;

        default:
            file = file;
            break ;
    }
    return ret ;
}

#elif defined (__ICCARM__)      /* IAR */

/* For IAR compiler revise __write() function for printf functionality */
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    size_t nChars = 0;

    if (buffer == 0)
    {
        /*
         * This means that we should flush internal buffers.  Since we
         * don't we just return.  (Remember, "handle" == -1 means that all
         * handles should be flushed.)
         */
        return (0);
    }

    for (/* Empty */; size != 0; --size)
    {
        UART_DEB_UartPutChar(*buffer++);
        ++nChars;
    }

    return (nChars);
}

#else  /* (__GNUC__)  GCC */

/* For GCC compiler revise _write() function for printf functionality */
int _write(int file, char *ptr, int len)
{
    int i;
    file = file;
    for (i = 0; i < len; i++)
    {
        UART_UartPutChar(*ptr++);
    }
    return len;
}


#endif  /* (__ARMCC_VERSION) */

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
