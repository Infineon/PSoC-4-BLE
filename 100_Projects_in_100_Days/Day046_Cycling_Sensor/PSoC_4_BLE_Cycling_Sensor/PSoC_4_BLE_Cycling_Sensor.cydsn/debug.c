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
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#include <project.h>
#include <stdio.h>


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
        UART_DEB_UartPutChar(*ptr++);
    }
    return len;
}


#endif  /* (__ARMCC_VERSION) */   


void ShowValue(CYBLE_GATT_VALUE_T *value)
{
    int16 i;
    
    for(i = 0; i < value->len; i++)
    {
        printf("%2.2x ", value->val[i]);
    }
    printf("\r\n");
}


void Set32ByPtr(uint8 ptr[], uint32 value)
{
    ptr[0u] = (uint8) value;
    ptr[1u] = (uint8) (value >> 8u);    
    ptr[2u] = (uint8) (value >> 16u);    
    ptr[3u] = (uint8) (value >> 24u);
}


/* [] END OF FILE */
