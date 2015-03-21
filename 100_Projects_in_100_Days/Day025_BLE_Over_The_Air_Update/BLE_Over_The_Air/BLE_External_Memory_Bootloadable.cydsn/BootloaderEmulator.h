/*******************************************************************************
* File Name: BootloaderEmulator.h
*
* Version: 1.0
*
* Description:
*  Provides an API for the bootloader emulator. The bootloader emulator is
*  a light-weight version of the Bootloader component that updates external
*  memory with the new application code and data.
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

#if !defined(BootloaderEmulator_H)
#define BootloaderEmulator_H

#include "Options.h"
#include "cytypes.h"
#include "CyFlash.h"
#include "Encryption.h"

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
#define BootloaderEmulator_CMD_SEND_DATA_AVAIL        (0u)
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

extern void CyBtldrCommStart(void);
extern void CyBtldrCommStop (void);
extern void CyBtldrCommReset(void);

extern cystatus CyBtldrCommRead (uint8* buffer, uint16 size, uint16* count, uint8 timeOut);
extern cystatus CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);


#endif /* BootloaderEmulator_H */


/* [] END OF FILE */
