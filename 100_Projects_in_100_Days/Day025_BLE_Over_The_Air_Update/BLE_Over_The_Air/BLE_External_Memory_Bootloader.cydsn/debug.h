/*******************************************************************************
* File Name: Debug.h
*
* Version: 1.0
*
* Description:
*  Provides debug API.
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

#include <stdio.h>

#include "Options.h"

#if (DEBUG_UART_ENABLED == YES)
    #define DBG_PRINT_TEXT(a)           do\
                                        {\
                                            printf((a));\
                                        } while (0)

    #define DBG_PRINT_DEC(a)         do\
                                        {\
                                           printf("%02d ", a);\
                                        } while (0)


    #define DBG_PRINT_HEX(a)         do\
                                        {\
                                           printf("%08X ", a);\
                                        } while (0)


    #define DBG_PRINT_ARRAY(a,b)     do\
                                        {\
                                            uint32 i;\
                                            \
                                            for(i = 0u; i < (b); i++)\
                                            {\
                                                printf("%02X ", *(a+i));\
                                            }\
                                        } while (0)

    #define DBG_PRINTF(...)          (printf(__VA_ARGS__))
    
    
#else
    #define DBG_PRINT_TEXT(a)
    #define DBG_PRINT_DEC(a)
    #define DBG_PRINT_HEX(a)
    #define DBG_PRINT_ARRAY(a,b)
    #define DBG_PRINTF(...)
#endif /* (DEBUG_UART_ENABLED == YES) */

/* [] END OF FILE */
