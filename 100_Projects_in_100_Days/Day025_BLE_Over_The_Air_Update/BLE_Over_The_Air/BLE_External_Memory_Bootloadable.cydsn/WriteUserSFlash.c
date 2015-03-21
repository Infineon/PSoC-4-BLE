/*******************************************************************************
* File Name: WriteUserSFlash.c
*
* Version: 1.0
*
* Description:
*  Provides an API for the writing to SFlash.
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

#include "WriteUserSFlash.h"

#if (ENCRYPTION_ENABLED == YES)
    
static CY_SYS_FLASH_CLOCK_BACKUP_STRUCT cySysFlashBackup;

static cystatus SF_CySysFlashClockBackup(void);
static cystatus SF_CySysFlashClockRestore(void);
static cystatus SF_CySysFlashClockConfig(void);

/* Function created based on CyFlash.c ver. 4.20 
*  Modified: comand in in SF_WriteUserSFlashRow to work with SFlash, removed 
*  unused defines.
*/
/*******************************************************************************
* Function Name: SF_WriteUserSFlashRow
********************************************************************************
*
* Summary:
*   Write data to SFlash   
*
* Parameters:
*   uint32 rowNum:  Number of SFlash row to write in 
*   uint32 rowData[]: Pointer to array of 32 word of 32-bit with data to write 
*   in
*
* Return:
*  The same as CySysFlashWriteRow().
*
*******************************************************************************/
uint32 SF_WriteUserSFlashRow(uint32 rowNum, uint32 rowData[])
{
    volatile uint32 retValue = CY_SYS_FLASH_SUCCESS;
    volatile uint32 clkCnfRetValue = CY_SYS_FLASH_SUCCESS;
    volatile uint32   parameters[(CY_FLASH_SIZEOF_ROW + CY_FLASH_SRAM_ROM_DATA)/4u];
    uint8  interruptState;
    

    if ((rowNum < CY_FLASH_NUMBER_ROWS) && (rowData != 0u))
    {
        /* Load Flash Bytes */
        parameters[0u] = (uint32) (CY_FLASH_GET_MACRO_FROM_ROW(rowNum)        << CY_FLASH_PARAM_MACRO_SEL_OFFSET) |
                         (uint32) (CY_FLASH_PAGE_LATCH_START_ADDR             << CY_FLASH_PARAM_ADDR_OFFSET     ) |
                         (uint32) (CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_LOAD) << CY_FLASH_PARAM_KEY_TWO_OFFSET  ) |
                         CY_FLASH_KEY_ONE;
        parameters[1u] = CY_FLASH_SIZEOF_ROW - 1u;

        (void)memcpy((void *)&parameters[2u], rowData, CY_FLASH_SIZEOF_ROW);
        CY_FLASH_CPUSS_SYSARG_REG = (uint32) &parameters[0u];
        CY_FLASH_CPUSS_SYSREQ_REG = CY_FLASH_CPUSS_REQ_START | CY_FLASH_API_OPCODE_LOAD;
        retValue = CY_FLASH_API_RETURN;
        
        if(retValue == CY_SYS_FLASH_SUCCESS)
        {
            /***************************************************************
            * Mask all the exceptions to guarantee that Flash write will
            * occur in the atomic way. It will not affect system call
            * execution (flash row write) since it is executed in the NMI
            * context.
            ***************************************************************/
            interruptState = CyEnterCriticalSection();

            clkCnfRetValue = SF_CySysFlashClockBackup();

        #if(CY_IP_FMLT)
            if(clkCnfRetValue == CY_SYS_FLASH_SUCCESS)
            {
                retValue = SF_CySysFlashClockConfig();
            }
        #endif  /* (CY_IP_FMLT) */
            
            if(retValue == CY_SYS_FLASH_SUCCESS)
            {
                /* Write User Sflash Row */
                parameters[0u]  = (uint32) (((uint32) CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_WRITE_SFLASH_ROW) <<  CY_FLASH_PARAM_KEY_TWO_OFFSET) | CY_FLASH_KEY_ONE);
                parameters[1u] = (uint32) rowNum;

                CY_FLASH_CPUSS_SYSARG_REG = (uint32) &parameters[0u];
                CY_FLASH_CPUSS_SYSREQ_REG = CY_FLASH_CPUSS_REQ_START | CY_FLASH_API_OPCODE_WRITE_SFLASH_ROW;
                retValue = CY_FLASH_API_RETURN;
            }

            if(clkCnfRetValue == CY_SYS_FLASH_SUCCESS)
            {
                clkCnfRetValue = SF_CySysFlashClockRestore();
            }
            CyExitCriticalSection(interruptState); 
        }
    }
    else
    {
        retValue = CY_SYS_FLASH_INVALID_ADDR;
    }

    return (retValue);    
}

	
/*******************************************************************************
* Function Name: SF_CySysFlashClockBackup
********************************************************************************
*
* Summary:
*  Backups the device clock configuration.
*
* Parameters:
*  None
*
* Return:
*  The same as CySysFlashWriteRow().
*
*******************************************************************************/
static cystatus SF_CySysFlashClockBackup(void)
{
    cystatus retValue = CY_SYS_FLASH_SUCCESS;
    volatile uint32   parameters[2u];

    /* FM-Lite Clock Backup System Call */
    parameters[0u] = (uint32) ((CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_CLK_BACKUP) <<  CY_FLASH_PARAM_KEY_TWO_OFFSET) | CY_FLASH_KEY_ONE);
    parameters[1u] = (uint32) &cySysFlashBackup.clockSettings[0u];
    CY_FLASH_CPUSS_SYSARG_REG = (uint32) &parameters[0u];
    CY_FLASH_CPUSS_SYSREQ_REG = CY_FLASH_CPUSS_REQ_START | CY_FLASH_API_OPCODE_CLK_BACKUP;
    retValue = CY_FLASH_API_RETURN;

    return (retValue);
}


/*******************************************************************************
* Function Name: SF_CySysFlashClockConfig
********************************************************************************
*
* Summary:
*  Configures the device clocks for the flash writing.
*
* Parameters:
*  None
*
* Return:
*  The same as CySysFlashWriteRow().
*
*******************************************************************************/
static cystatus SF_CySysFlashClockConfig(void)
{
    cystatus retValue = CY_SYS_FLASH_SUCCESS;

    /***************************************************************************
    * The FM-Lite IP uses the IMO at 48MHz for the pump clock and SPC timer
    * clock. The PUMP_SEL and HF clock must be set to IMO before calling Flash
    * write or erase operation.
    ***************************************************************************/

    /* FM-Lite Clock Configuration */
    CY_FLASH_CPUSS_SYSARG_REG = (uint32) ((CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_CLK_CONFIG) <<  CY_FLASH_PARAM_KEY_TWO_OFFSET) | CY_FLASH_KEY_ONE);
    CY_FLASH_CPUSS_SYSREQ_REG = CY_FLASH_CPUSS_REQ_START | CY_FLASH_API_OPCODE_CLK_CONFIG;
    retValue = CY_FLASH_API_RETURN;

    return (retValue);
}


/*******************************************************************************
* Function Name: SF_CySysFlashClockRestore
********************************************************************************
*
* Summary:
*  Restores the device clock configuration.
*
* Parameters:
*  None
*
* Return:
*  The same as CySysFlashWriteRow().
*
*******************************************************************************/
static cystatus SF_CySysFlashClockRestore(void)
{
    cystatus retValue = CY_SYS_FLASH_SUCCESS;
    volatile uint32   parameters[2u];

    /* FM-Lite Clock Restore */
    parameters[0u] = (uint32) ((CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_CLK_RESTORE) <<  CY_FLASH_PARAM_KEY_TWO_OFFSET) | CY_FLASH_KEY_ONE);
    parameters[1u] = (uint32) &cySysFlashBackup.clockSettings[0u];
    CY_FLASH_CPUSS_SYSARG_REG = (uint32) &parameters[0u];
    CY_FLASH_CPUSS_SYSREQ_REG = CY_FLASH_CPUSS_REQ_START | CY_FLASH_API_OPCODE_CLK_RESTORE;
    retValue = CY_FLASH_API_RETURN;

    return (retValue);
}

#endif /*(ENCRYPTION_ENABLED == YES)*/

/* [] END OF FILE */
