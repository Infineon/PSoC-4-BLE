/*******************************************************************************
* File Name: BootloaderEmulator_PVT.h
*
* Version 1.0
*
*  Description:
*   TBD
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_BOOTLOADER_BootloaderEmulator_PVT_H)
#define CY_BOOTLOADER_BootloaderEmulator_PVT_H

#include "BootloaderEmulator.h"


typedef struct
{
    uint32 SiliconId;
    uint8  Revision;
    uint8  BootLoaderVersion[3u];

} BootloaderEmulator_ENTER;


#define BootloaderEmulator_VERSION        {\
                                            (uint8) 30, \
                                            (uint8) 1, \
                                            (uint8)0x01u \
                                        }

/* Packet framing constants. */
#define BootloaderEmulator_SOP            (0x01u)    /* Start of Packet */
#define BootloaderEmulator_EOP            (0x17u)    /* End of Packet */


/* Bootloader command responses */
#define BootloaderEmulator_ERR_KEY       (0x01u)  /* The provided key does not match the expected value          */
#define BootloaderEmulator_ERR_VERIFY    (0x02u)  /* The verification of flash failed                            */
#define BootloaderEmulator_ERR_LENGTH    (0x03u)  /* The amount of data available is outside the expected range  */
#define BootloaderEmulator_ERR_DATA      (0x04u)  /* The data is not of the proper form                          */
#define BootloaderEmulator_ERR_CMD       (0x05u)  /* The command is not recognized                               */
#define BootloaderEmulator_ERR_DEVICE    (0x06u)  /* The expected device does not match the detected device      */
#define BootloaderEmulator_ERR_VERSION   (0x07u)  /* The bootloader version detected is not supported            */
#define BootloaderEmulator_ERR_CHECKSUM  (0x08u)  /* The checksum does not match the expected value              */
#define BootloaderEmulator_ERR_ARRAY     (0x09u)  /* The flash array is not valid                                */
#define BootloaderEmulator_ERR_ROW       (0x0Au)  /* The flash row is not valid                                  */
#define BootloaderEmulator_ERR_PROTECT   (0x0Bu)  /* The flash row is protected and can not be programmed        */
#define BootloaderEmulator_ERR_APP       (0x0Cu)  /* The application is not valid and cannot be set as active    */
#define BootloaderEmulator_ERR_ACTIVE    (0x0Du)  /* The application is currently marked as active               */
#define BootloaderEmulator_ERR_UNK       (0x0Fu)  /* An unknown error occurred                                   */


/* Bootloader command definitions. */
#define BootloaderEmulator_COMMAND_CHECKSUM     (0x31u)    /* Verify the checksum for the bootloadable project   */
#define BootloaderEmulator_COMMAND_REPORT_SIZE  (0x32u)    /* Report the programmable portions of flash          */
#define BootloaderEmulator_COMMAND_APP_STATUS   (0x33u)    /* Gets status info about the provided app status     */
#define BootloaderEmulator_COMMAND_ERASE        (0x34u)    /* Erase the specified flash row                      */
#define BootloaderEmulator_COMMAND_SYNC         (0x35u)    /* Sync the bootloader and host application           */
#define BootloaderEmulator_COMMAND_APP_ACTIVE   (0x36u)    /* Sets the active application                        */
#define BootloaderEmulator_COMMAND_DATA         (0x37u)    /* Queue up a block of data for programming           */
#define BootloaderEmulator_COMMAND_ENTER        (0x38u)    /* Enter the bootloader                               */
#define BootloaderEmulator_COMMAND_PROGRAM      (0x39u)    /* Program the specified row                          */
#define BootloaderEmulator_COMMAND_VERIFY       (0x3Au)    /* Compute flash row checksum for verification        */
#define BootloaderEmulator_COMMAND_EXIT         (0x3Bu)    /* Exits the bootloader & resets the chip             */
#define BootloaderEmulator_COMMAND_GET_METADATA (0x3Cu)    /* Reports the metadata for a selected application    */


/*******************************************************************************
* Bootloader packet byte addresses:
* [1-byte] [1-byte ] [2-byte] [n-byte] [ 2-byte ] [1-byte]
* [ SOP  ] [Command] [ Size ] [ Data ] [Checksum] [ EOP  ]
*******************************************************************************/
#define BootloaderEmulator_SOP_ADDR             (0x00u)         /* Start of packet offset from beginning     */
#define BootloaderEmulator_CMD_ADDR             (0x01u)         /* Command offset from beginning             */
#define BootloaderEmulator_SIZE_ADDR            (0x02u)         /* Packet size offset from beginning         */
#define BootloaderEmulator_DATA_ADDR            (0x04u)         /* Packet data offset from beginning         */
#define BootloaderEmulator_CHK_ADDR(x)          (0x04u + (x))   /* Packet checksum offset from end           */
#define BootloaderEmulator_EOP_ADDR(x)          (0x06u + (x))   /* End of packet offset from end             */
#define BootloaderEmulator_MIN_PKT_SIZE         (7u)            /* The minimum number of bytes in a packet   */


/*******************************************************************************
* BootloaderEmulator_Start()
*******************************************************************************/
#define BootloaderEmulator_MD_BTLDB_IS_ACTIVE         (0x01u)
#define BootloaderEmulator_WAIT_FOR_COMMAND_FOREVER   (0x00u)


 /* The maximum number of bytes accepted in a packet plus some */
#define BootloaderEmulator_SIZEOF_COMMAND_BUFFER      (300u)


/*******************************************************************************
* BootloaderEmulator_HostLink()
*******************************************************************************/
#define BootloaderEmulator_COMMUNICATION_STATE_IDLE   (0u)
#define BootloaderEmulator_COMMUNICATION_STATE_ACTIVE (1u)


/*******************************************************************************
* BootloaderEmulator_CalcPacketChecksum()
*******************************************************************************/
#if(0u != BootloaderEmulator_PACKET_CHECKSUM_CRC)
    #define BootloaderEmulator_CRC_CCITT_POLYNOMIAL       (0x8408u)       /* x^16 + x^12 + x^5 + 1 */
    #define BootloaderEmulator_CRC_CCITT_INITIAL_VALUE    (0xffffu)
#endif /* (0u != BootloaderEmulator_PACKET_CHECKSUM_CRC) */


#define BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY                ((uint16)(CY_FLASH_SIZEOF_ARRAY/CY_FLASH_SIZEOF_ROW))
#define BootloaderEmulator_FIRST_ROW_IN_ARRAY                     (0u)


/*******************************************************************************
* Metadata addresses and pointer defines
*******************************************************************************/
#define BootloaderEmulator_MD_SIZEOF                        (64u)
#define BootloaderEmulator_MD_APP_CHECKSUM                  (CY_FLASH_SIZEOF_ROW - BootloaderEmulator_MD_SIZEOF)

#endif /* CY_BOOTLOADER_BootloaderEmulator_PVT_H */


/* [] END OF FILE */
