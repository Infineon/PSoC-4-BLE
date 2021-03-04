/*******************************************************************************
* File Name: CustomInterface.h
*
* Version: 1.0
*
* Description:
*  Provides an API that emulates Bootloader Host Tool application. The emulation
*  implemented within custom bootloader interface API.
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

#if !defined(CustomInterface_H)
#define CustomInterface_H

#include "Options.h"
#include "Bootloader_PVT.h"
#include "cytypes.h"


void CyBtldrCommStart(void);
void CyBtldrCommStop(void);
void CyBtldrCommReset(void);
cystatus CyBtldrCommWrite(uint8* buffer, uint16 size, uint16* count, uint8 timeOut);
cystatus CyBtldrCommRead (uint8* buffer, uint16 size, uint16* count, uint8 timeOut);

extern int8 encryptionEnabled;
/*******************************************************************************
* Bootloader_CalcPacketChecksum()
*******************************************************************************/
#if(0u != CI_PACKET_CHECKSUM_CRC)
    #define CI_CRC_CCITT_POLYNOMIAL       (0x8408u)       /* x^16 + x^12 + x^5 + 1 */
    #define CI_CRC_CCITT_INITIAL_VALUE    (0xffffu)
#endif /* (0u != CI_PACKET_CHECKSUM_CRC) */


#define CI_COMMUNICATION_STATE_IDLE   (0u)
#define CI_COMMUNICATION_STATE_ACTIVE (1u)


/*******************************************************************************
* Bootloader packet byte addresses:
* [1-byte] [1-byte ] [2-byte] [n-byte] [ 2-byte ] [1-byte]
* [ SOP  ] [Command] [ Size ] [ Data ] [Checksum] [ EOP  ]
*******************************************************************************/
#define CI_SOP                  (0x01u)    /* Start of Packet */
#define CI_EOP                  (0x17u)    /* End of Packet */

#define CI_SOP_ADDR             (0x00u)         /* Start of packet offset from beginning     */
#define CI_CMD_ADDR             (0x01u)         /* Command offset from beginning             */
#define CI_SIZE_ADDR            (0x02u)         /* Packet size offset from beginning         */
#define CI_DATA_ADDR            (0x04u)         /* Packet data offset from beginning         */
#define CI_CHK_ADDR(x)          (0x04u + (x))   /* Packet checksum offset from end           */
#define CI_EOP_ADDR(x)          (0x06u + (x))   /* End of packet offset from end             */
#define CI_MIN_PKT_SIZE         (7u)            /* The minimum number of bytes in a packet   */


/* Bootloader command definitions. */
#define CI_COMMAND_ENTER        (0x38u)    /* Enter the bootloader                               */
#define CI_COMMAND_PROGRAM      (0x39u)    /* Program the specified row                          */
#define CI_COMMAND_EXIT         (0x3Bu)    /* Exits the bootloader & resets the chip             */


/* Program the specified row */
#define CI_COMMAND_PROGRAM_ARRAY_ID           (0x00u)     /* Always zero. Real array id is get from row number. */
#define CI_COMMAND_PROGRAM_PARAM_SIZE         (3u)        /* Flash Array ID (1) and Flash Row Number (2) */
#define CI_COMMAND_PROGRAM_PACKET_SIZE        (CI_MIN_PKT_SIZE + CI_COMMAND_PROGRAM_PARAM_SIZE + CY_FLASH_SIZEOF_ROW)
#define CI_COMMAND_PROGRAM_PACKET_DATA_SIZE   (CI_COMMAND_PROGRAM_PARAM_SIZE + CY_FLASH_SIZEOF_ROW)


/* Enter the bootloader */
#define CI_COMMAND_ENTER_PACKET_SIZE        (CI_MIN_PKT_SIZE)
#define CI_COMMAND_ENTER_PACKET_DATA_SIZE   (0u)


/* Exits the bootloader */
#define CI_COMMAND_EXIT_PACKET_SIZE        (CI_MIN_PKT_SIZE)
#define CI_COMMAND_EXIT_PACKET_DATA_SIZE   (0u)



#define CI_FLASH_ROWS_IN_ARRAY              (0x1FFu)

#endif /* CustomInterface_H */


/* [] END OF FILE */
