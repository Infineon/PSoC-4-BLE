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


/* [] END OF FILE */
