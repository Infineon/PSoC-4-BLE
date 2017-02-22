/*******************************************************************************
* File Name: Debug.c
*
* Version: 1.0
*
* Description:
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include "cytypes.h"
#include "debug.h"
#include "project.h"

#if (DEBUG_UART_ENABLED == YES)

    #if defined(__ARMCC_VERSION)

        /* For MDK/RVDS compiler revise fputc() for the printf() */
        struct __FILE
        {
            int handle;
        };

        enum
        {
            STDIN_HANDLE,
            STDOUT_HANDLE,
            STDERR_HANDLE
        } ;

        FILE __stdin = {STDIN_HANDLE};
        FILE __stdout = {STDOUT_HANDLE};
        FILE __stderr = {STDERR_HANDLE};

        int fputc(int ch, FILE *file)
        {
            int ret = EOF;

            switch( file->handle )
            {
                case STDOUT_HANDLE:
                    UART_PutChar(ch);
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
        UART_PutChar(*buffer++);
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
                UART_PutChar(*ptr++);
            }
            return len;
        }

    #endif  /* (__ARMCC_VERSION) */

#endif /* (DEBUG_UART_ENABLED == YES) */

/* [] END OF FILE */
