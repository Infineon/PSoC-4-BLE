/*******************************************************************************
* File Name: Options.h
*
* Version: 1.30
*
* Description:
*  Provides project configuration options.
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

#if !defined(Options_H)
#define Options_H

#include "cytypes.h"

#define NO                      (0u)
#define YES                     (1u)


/*******************************************************************************
* Following section contains bootloadable project compile-time options.
*******************************************************************************/
#define ENCRYPT_ENABLED         (NO)
#define DEBUG_UART_ENABLED      (NO)
#define CI_PACKET_CHECKSUM_CRC  (NO)


/*******************************************************************************
* The next option is for configuring row number of SFLASH that will be used
* for storing of the encryption key that was used for the external memory
* encryption if it is enabled. Minimal value is 1.
* Row#0 is reserved for BLE component.
*******************************************************************************/
#define KEY_ROW_NUM             (1u)

#define ENCRYPTION_ENABLED      (ENCRYPT_ENABLED && CYDEV_BOOTLOADER_ENABLE)

#endif /* Options_H */


/* [] END OF FILE */
