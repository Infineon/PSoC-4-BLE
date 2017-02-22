/*******************************************************************************
* File Name: ExternalMemoryInterface.h
*
* Version: 1.0
*
* Description:
*  Provides an API for the external memory access.
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

#if !defined(ExternalMemoryInterface_H)
#define ExternalMemoryInterface_H


#include "cytypes.h"
#include "Encryption.h"
#include <stdbool.h>

    
void EMI_Start(void);
cystatus EMI_EraseAll(void);
cystatus EMI_WriteData(uint8 instruction, uint32 addressBytes, uint32 dataSize, uint8 *data);
cystatus EMI_ReadData (uint32 addressBytes, uint32 dataSize, uint8 *data);
bool EMI_IsBusy(void);

#define ENC_BUFFER_SIZE (300)
#define META_DATA_SIZE  (128)
#define META_DATA_ADDR  (0)

/*******************************************************************************
* Communication with External Memory
*******************************************************************************/
#define EMI_SIZE_OF_WRITE_BUFFER		        (300u)

/* Instructions for the Serial NOR Flash chip */
#define NOR_FLASH_INSTRUCTION_WRITE_ENABLE      (0x06)
#define NOR_FLASH_INSTRUCTION_WRITE_DISABLE     (0x04)

#define NOR_FLASH_INSTRUCTION_READ              (0x03)
#define NOR_FLASH_INSTRUCTION_READ_SR1          (0x05)

#define NOR_FLASH_INSTRUCTION_SECTOR_ERASE      (0x20)
#define NOR_FLASH_INSTRUCTION_BLOCK_ERASE       (0xD8)
#define NOR_FLASH_INSTRUCTION_PP                (0x02)

#define EMI_EXTERNAL_MEMORY_PAGE_SIZE           (0x000100)  /* 256 B */
#define EMI_EXTERNAL_MEMORY_SECTOR_SIZE         (0x001000)  /* 4 kB */
#define EMI_EXTERNAL_MEMORY_BLOCK_SIZE          (0x010000)  /* 64 kB */

#define EMI_BUSY_BIT_FIELD                      (0x01)

/*******************************************************************************
* External Memory Layout
*******************************************************************************/

/* First sector of 4 kB to store metadata only (128 bytes); rest is blank.
 * This is done so that metadata can be erased independently of the application.
 */
#define EMI_MD_BASE_ADDR                    (0x000000u) 

/* Second sector onwards data is stored, each row size is 256 bytes but the 
 * data stored per row is only 128 bytes, so as to make things simple.
 */
#define EMI_APP_BASE_ADDR                   (EMI_MD_BASE_ADDR + EMI_EXTERNAL_MEMORY_SECTOR_SIZE)

/* Address calculation for app pages - each page size in the external memory
 * is 256 Bytes, but the actual row size for BLE chips could be 128/256 bytes,
 * so both can be accomodated. In case of 128 byte BLE chips, the first 128 
 * bytes in the external memory's page are filled, and the remaining are empty.
 */
#define EMI_APP_ABS_ADDR(row)               (EMI_APP_BASE_ADDR + ((row) * EMI_EXTERNAL_MEMORY_PAGE_SIZE))


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


/* [] END OF FILE */
