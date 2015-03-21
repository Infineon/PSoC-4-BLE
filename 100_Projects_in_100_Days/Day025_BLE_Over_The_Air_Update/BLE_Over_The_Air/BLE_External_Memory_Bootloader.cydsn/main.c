/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  Simple BLE example project that demonstrates how to configure and use
*  Cypress's BLE component APIs and application layer callback. Device
*  Information service is used as an example to demonstrate configuring
*  BLE service characteristics in the BLE component.
*
* References:
*  BLUETOOTH SPECIFICATION Version 4.1
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

#include <project.h>
#include "Options.h"
#include <stdio.h>
#include "debug.h"
#include "CustomInterface.h"
#include "ExternalMemoryInterface.h"

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Starts the bootloader component.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    CyGlobalIntEnable;

    #if (DEBUG_UART_ENABLED == YES)
        UART_Start();
    #endif /* (DEBUG_UART_ENABLED == YES) */


    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("===============================================================================\r\n");
    DBG_PRINT_TEXT("=              BLE_External_Memory_Bootloader Application Started              \r\n");
    DBG_PRINT_TEXT("=              Version: 1.0                                                    \r\n");    
    DBG_PRINTF    ("=              Compile Date and Time : %s %s                                   \r\n", __DATE__,__TIME__);
    DBG_PRINT_TEXT("===============================================================================\r\n");
    DBG_PRINT_TEXT("\r\n"); 
    DBG_PRINT_TEXT("\r\n");       

	CyBle_AesCcmInit();    
    
    Bootloader_Start();
    
    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
