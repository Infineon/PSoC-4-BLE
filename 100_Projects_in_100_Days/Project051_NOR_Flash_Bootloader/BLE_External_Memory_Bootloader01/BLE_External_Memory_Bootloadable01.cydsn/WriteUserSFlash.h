/*******************************************************************************
* File Name: WriteUserSFlash.c
*
* Version: 1.0
*
* Description:
*  Provides an API for the writing to SFlash.
*
*
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

#if !defined(PROGRAMFLASH_H)
#define PROGRAMFLASH_H
    
#if (ENCRYPTION_ENABLED == YES)

#include "CyFlash.h"
    
#define CY_FLASH_API_OPCODE_WRITE_SFLASH_ROW    (0x18u)
#define WRITE_KEY_ERROR                (1)
	
uint32 SF_WriteUserSFlashRow(uint32 rowNum, uint32 rowData[]); 


#endif /*(ENCRYPTION_ENABLED == YES)*/

    
#endif /*PROGRAMFLASH_H*/
/* [] END OF FILE */
