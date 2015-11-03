/*******************************************************************************
* File Name: OTAMandatory.h
*
* Version: 1.30
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project. They are mandatory for OTA functionality.
*
********************************************************************************
* Copyright 2014-2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#if !defined(OTAMandatory_H)
#define OTAMandatory_H

#include "cytypes.h"
#include "Options.h"
#include "cytypes.h"
#include "CyFlash.h"
#include "OTAOptional.h"

#define BootloaderEmulator_activeApp      (BootloaderEmulator_MD_BTLDB_ACTIVE_0)

#if !defined(ExternalMemoryInterface_H)
#define ExternalMemoryInterface_H


void EMI_Start(void);
cystatus EMI_SetPointer(uint32 dataAddr);
cystatus EMI_EraseAll(void);
cystatus EMI_WriteData(uint32 dataAddr, uint32 dataSize, uint8 *data);
cystatus EMI_ReadData (uint32 dataAddr, uint32 dataSize, uint8 *data);


#define ENC_BUFFER_SIZE (300)
#define META_DATA_SIZE  (128)
#define META_DATA_ADDR  (0)

/*******************************************************************************
* Communication with External Memory
*******************************************************************************/
#define EMI_SIZE_OF_WRITE_BUFFER		    (300u)


#define EMI_HIGHEST_ADDR_OF_LOW_BLOCK	    (0x0FFFFu)
#define EMI_HIGHEST_ADDR_OF_HIGH_BLOCK	    (0x1FFFFu)

#define EMI_I2C_SLAVE_ADDR_LOW_64K	        (0x50u)
#define EMI_I2C_SLAVE_ADDR_HIGH_64K	        (0x51u)

#define EMI_DATA_ADDR_MSB_INDX			    (0u)
#define EMI_DATA_ADDR_LSB_INDX			    (1u)
#define EMI_DATA_INDX                       (2u)

#define EMI_ADDR_SIZE					    (2u)	/* Size of RAM address in bytes */
#define EMI_NO_DATA_SIZE                    (0u)
#define EMI_EXTERNAL_MEMORY_PAGE_SIZE       (64u)


/*******************************************************************************
* External Memory Layout
*******************************************************************************/
#define EMI_MD_BASE_ADDR                    (0x00u)
#define EMI_APP_BASE_ADDR                   (CY_FLASH_SIZEOF_ROW)
#define EMI_APP_ABS_ADDR(row)               (EMI_APP_BASE_ADDR + ((row) * CY_FLASH_SIZEOF_ROW))


/*******************************************************************************
* External Memory Metadata
*******************************************************************************/
#define EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR   (EMI_MD_BASE_ADDR + 0x14u)
#define EMI_MD_APP_FIRST_ROW_NUM_ADDR           (EMI_MD_BASE_ADDR + 0x10u)
#define EMI_MD_APP_SIZE_IN_ROWS_ADDR            (EMI_MD_BASE_ADDR + 0x0Cu)
#define EMI_MD_APP_EM_CHECKSUM_ADDR             (EMI_MD_BASE_ADDR + 0x08u)
#define EMI_MD_ENCRYPTION_STATUS_ADDR           (EMI_MD_BASE_ADDR + 0x04u)
#define EMI_MD_APP_STATUS_ADDR                  (EMI_MD_BASE_ADDR + 0x00u)


/*******************************************************************************
* External Memory Metadata Fields Values
*******************************************************************************/
#define EMI_MD_APP_STATUS_VALID             (0x56u)
#define EMI_MD_APP_STATUS_LOADED            (0x4Cu)
#define EMI_MD_APP_STATUS_INVALID           (0x00u)

#endif /* ExternalMemoryInterface_H */

#if !defined(BootloaderEmulator_H)
#define BootloaderEmulator_H

/*******************************************************************************
* This variable is used by Bootloader/Bootloadable components to schedule what
* application will be started after software reset.
*******************************************************************************/
#if (CY_PSOC4)
    #if defined(__ARMCC_VERSION)
        __attribute__ ((section(".bootloaderruntype"), zero_init))
    #elif defined (__GNUC__)
        __attribute__ ((section(".bootloaderruntype")))
   #elif defined (__ICCARM__)
        #pragma location=".bootloaderruntype"
    #endif  /* defined(__ARMCC_VERSION) */
    extern volatile uint32 cyBtldrRunType;
#endif  /* (CY_PSOC4) */


#define BootloaderEmulator_DUAL_APP_BOOTLOADER        (0u)  /* Not supported  */
#define BootloaderEmulator_BOOTLOADER_APP_VERSION     (0u)
#define BootloaderEmulator_FAST_APP_VALIDATION        (0u)  /* Not supported  */
#define BootloaderEmulator_PACKET_CHECKSUM_CRC        (CI_PACKET_CHECKSUM_CRC)  /* Configurable   */
#define BootloaderEmulator_WAIT_FOR_COMMAND           (1u)  /* Wait           */
#define BootloaderEmulator_WAIT_FOR_COMMAND_TIME      (0u)  /*      forever   */
#define BootloaderEmulator_BOOTLOADER_APP_VALIDATION  (0u)  /* Not supported  */

#define BootloaderEmulator_CMD_GET_FLASH_SIZE_AVAIL   (1u)
#define BootloaderEmulator_CMD_ERASE_ROW_AVAIL        (0u)
#define BootloaderEmulator_CMD_VERIFY_ROW_AVAIL       (1u)
#define BootloaderEmulator_CMD_SYNC_BOOTLOADER_AVAIL  (0u)
#define BootloaderEmulator_CMD_SEND_DATA_AVAIL        (1u)
#define BootloaderEmulator_CMD_GET_METADATA           (0u)  /* Not supported  */


/*******************************************************************************
* Bootloadable applications identification
*******************************************************************************/
#define BootloaderEmulator_MD_BTLDB_ACTIVE_0          (0x00u)

/* Mask used to indicate starting application */
#define BootloaderEmulator_SCHEDULE_BTLDB             (0x80u)
#define BootloaderEmulator_SCHEDULE_BTLDR             (0x40u)


/*******************************************************************************
* Schedule Bootloader/Bootloadable to be run after software reset
*******************************************************************************/
#define BootloaderEmulator_SET_RUN_TYPE(x)                (cyBtldrRunType = (x))


/*******************************************************************************
* External References
*******************************************************************************/
void     BootloaderEmulator_Start(void);
cystatus BootloaderEmulator_ValidateBootloadable(void);
uint32   BootloaderEmulator_Calc8BitSum(uint32 addr);


#endif /* BootloaderEmulator_H */

#if !defined(CY_BOOTLOADER_BootloaderEmulator_PVT_H)
#define CY_BOOTLOADER_BootloaderEmulator_PVT_H

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

#if !defined(PROGRAMFLASH_H)
#define PROGRAMFLASH_H
    
#if (ENCRYPTION_ENABLED == YES)

#define CY_FLASH_API_OPCODE_WRITE_SFLASH_ROW    (0x18u)
#define WRITE_KEY_ERROR                (1)
	
uint32 SF_WriteUserSFlashRow(uint32 rowNum, uint32 rowData[]); 


#endif /*(ENCRYPTION_ENABLED == YES)*/

    
#endif /*PROGRAMFLASH_H*/

#endif /* OTAMandatory_H */


/* [] END OF FILE */
