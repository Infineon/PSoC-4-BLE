/*******************************************************************************
* File Name: OTAOptional.c
*
* Version: 1.30
*
* Description:
*  Provides an API that implement optional functionality of OTA.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
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

#include "OTAOptional.h"
#include "debug.h"    

#if (ENCRYPTION_ENABLED == YES)

#if (CY_IP_SPCIF_SYNCHRONOUS)
    static CY_SYS_FLASH_CLOCK_BACKUP_STRUCT cySysFlashBackup;
#endif /* (CY_IP_SPCIF_SYNCHRONOUS) */

static cystatus SF_CySysFlashClockBackup(void);
static cystatus SF_CySysFlashClockRestore(void);
static cystatus SF_CySysFlashClockConfig(void);
    

/*******************************************************************************
* Function Name: CR_Initialization
********************************************************************************
*
* Summary:
*  Run BLE stack initialization function for encryption/decryption usage. Prefix 
*  CR stands for en/decryption to show that it is part of encryption module.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void CR_Initialization(void)
{
    CyBle_AesCcmInit();
}


/*******************************************************************************
* Function Name: CR_Encrypt
********************************************************************************
*
* Summary:
*  Encrypts the specified data with the specified key. Prefix 
*  CR stands for en/decryption to show that it is part of encryption module.
*
* Parameters:
*  uint8 * plain:       Pointer to an array of bytes to be encrypted. Size of 
*                       the array should be equal to the value of 'length' 
*                       parameter.
*  uint16 length:        Length of the data to be encrypted, in Bytes.
*  uint8 * key:         Pointer to an array of bytes holding the key. The array 
*                       length to be allocated by the application should be 16 
*                       bytes.
*  uint8 * nonce        Pointer to an array of bytes. The array length to be 
*                       allocated by the application is 13 Bytes.
*  uint8 * encrypted:   Pointer to an array of size 'length' where the encrypted 
*                       data is stored.
*  uint8 * out_mic:     Pointer to an array of bytes (4 Bytes) to store the   
*                       Message Integrity Check (MIC) value generated during 
*                       encryption.                        
*
* Return:
*   CYBLE_API_RESULT_T: Return value indicates if the function succeeded or
*   failed. Following are the possible error codes.
*   CYBLE_ERROR_OK                    On successful operation.
*       CYBLE_ERROR_INVALID_PARAMETER     One of the inputs is a null pointer or 
*									 the 'length' value is invalid	
*******************************************************************************/
CYBLE_API_RESULT_T CR_Encrypt(
    uint8 * plain, 
    uint16 length, 
    uint8 * key, 
    uint8 * nonce, 
    uint8 * encrypted, 
    uint8 * out_mic)
{
    CYBLE_API_RESULT_T retval = CYBLE_ERROR_OK;
    uint16 blockLeft;
    uint16 blockLength = 0;
    uint16 offset = 0;
    uint8 inBuf[EBCRYPTION_BLOCK_LENGTH];
    uint8 outBuf[EBCRYPTION_BLOCK_LENGTH];
    
    /*Input parameters check*/
    if ((plain == NULL) || (key == NULL) || (nonce == NULL) || \
        (encrypted == NULL) || out_mic == NULL)
    {
        return (CYBLE_ERROR_INVALID_PARAMETER);
    }
    
    #if (CYDEV_BOOTLOADER_ENABLE == 1)
        if (!encryptionEnabled)
        {
            memcpy(encrypted,plain,length);
            DBG_PRINT_TEXT("Encryption skipped");
            return (CYBLE_ERROR_OK);
        }
    #endif /*(CYDEV_BOOTLOADER_ENABLE == 1)*/
    
    /* Split plain text in to acceptable number of blocks */  
    for(offset=0;offset<length;offset+=EBCRYPTION_BLOCK_LENGTH)
    {

        blockLeft = length - offset;
        blockLength = blockLeft < EBCRYPTION_BLOCK_LENGTH ? blockLeft : EBCRYPTION_BLOCK_LENGTH;
        memcpy(inBuf, plain+offset, blockLength);
        retval = CyBle_AesCcmEncrypt(key, nonce, inBuf, blockLength, outBuf, out_mic);
        
        /*Do not store MIC and check MIC authorization, due to memory consumption*/
        if (retval == CYBLE_ERROR_OK || retval == CYBLE_ERROR_MIC_AUTH_FAILED)
        {
            memcpy(encrypted+offset, outBuf, blockLength);
        }
        
    }
    return (retval);
}


/*******************************************************************************
* Function Name: CR_Decrypt
********************************************************************************
*
* Summary:
*  Decrypts the specified data with the specified key. Prefix 
*  CR stands for en/decryption to show that it is part of encryption module.
*
* Parameters:
*  uint8 * encrypted:   Pointer to an array of bytes to be decrypted. Size of 
*                       the array should be equal to the value of 'length' 
*                       parameter.
*  uint16 length:        Length of the data to be encrypted, in Bytes.
*  uint8 * key:         Pointer to an array of bytes holding the key. The array 
*                       length to be allocated by the application should be 16 
*                       bytes.
*  uint8 * nonce        Pointer to an array of bytes. The array length to be 
*                       allocated by the application is 13 Bytes.
*  uint8 * decrypted:   Pointer to an array of size 'length' where the decrypted 
*                       data is stored.
*  uint8 * out_mic:     Pointer to an array of bytes (4 Bytes) to check the MIC 
*                       value generated during encryption. 
* 
* Return:
*   CYBLE_API_RESULT_T: Return value indicates if the function succeeded or
*   failed. Following are the possible error codes.
*       CYBLE_ERROR_OK                    On successful operation.
*       CYBLE_ERROR_INVALID_PARAMETER     One of the inputs is a null 
*		    pointer or the 'length' value is invalid
*       CYBLE_ERROR_MIC_AUTH_FAILED		Data decryption has been done  
*		    successfully but MIC based authorization check has failed. This 
*			error can be ignored if MIC based authorization was not intended.
*
*******************************************************************************/
CYBLE_API_RESULT_T CR_Decrypt(uint8 * encrypted, 
    uint16 length, 
    uint8 * key, 
    uint8 * nonce, 
    uint8 * decrypted , 
    uint8 * out_mic)
{
    CYBLE_API_RESULT_T retval = CYBLE_ERROR_OK;
    uint16 blockLeft;
    uint16 blockLength = 0;
    uint16 offset = 0; 
    uint8 inBuf[EBCRYPTION_BLOCK_LENGTH];
    uint8 outBuf[EBCRYPTION_BLOCK_LENGTH];
    
    /*Input parameters check*/
    if ((encrypted == NULL) || (key == NULL) || (nonce == NULL) || \
        (decrypted == NULL) || out_mic == NULL)
    {
        return (CYBLE_ERROR_INVALID_PARAMETER);
    }
    
    #if (CYDEV_BOOTLOADER_ENABLE == 1)
        if (!encryptionEnabled)
        {
            DBG_PRINT_TEXT("\r\nDecryption skipped\r\n");
            memcpy(decrypted,encrypted,length);
            return (CYBLE_ERROR_OK);
        }
    #endif /*(CYDEV_BOOTLOADER_ENABLE == 1)*/
    
    /* Split plain text in to acceptable number of blocks */

    for(offset=0;offset<length;offset+=EBCRYPTION_BLOCK_LENGTH)
    {

        blockLeft = length - offset;
        blockLength = blockLeft < EBCRYPTION_BLOCK_LENGTH ? blockLeft : EBCRYPTION_BLOCK_LENGTH;
        
        memcpy(inBuf, encrypted+offset, blockLength);
        
        retval = CyBle_AesCcmDecrypt(key, nonce, inBuf, blockLength, outBuf, out_mic);
        
        if (retval != CYBLE_ERROR_INVALID_PARAMETER)
        {
            memcpy(decrypted+offset, outBuf, blockLength);;
        }
    }
    return (retval);
}


#if (CYDEV_BOOTLOADER_ENABLE == 0)

    /***************************************************************************
    * Function Name: CR_GenerateKey
    ****************************************************************************
    *
    * Summary:
    *  Generates encryption key. Available only for the bootloadable application.
    *  As the function requires BLE component for the random number generation.
    *  Prefix CR stands for en/decryption to show that it is part of encryption
    *  module.
    *
    * Parameters:
    *   uint8 * key: Pointer to an array of bytes to output the generated key.   
    *                The array length to be allocated by the application should  
    *                be 16 bytes.
    * 
    * Return:
    *  None
    *
    ***************************************************************************/
    void CR_GenerateKey(uint8 * key)
    {
        uint8 temp_key[16];
        
        /* Begin: Used from cy_boot 5.0 function CyGetUniqueId*/
        temp_key[0u]  =  (uint32)(* (reg8 *) CYREG_SFLASH_DIE_LOT0  );
        temp_key[1u] |= ((uint32)(* (reg8 *) CYREG_SFLASH_DIE_LOT1  ) <<  8u);
        temp_key[2u] |= ((uint32)(* (reg8 *) CYREG_SFLASH_DIE_LOT2  ) << 16u);
        temp_key[3u] |= ((uint32)(* (reg8 *) CYREG_SFLASH_DIE_WAFER ) << 24u);
        temp_key[4u]  =  (uint32)(* (reg8 *) CYREG_SFLASH_DIE_X     );
        temp_key[5u] |= ((uint32)(* (reg8 *) CYREG_SFLASH_DIE_Y     ) <<  8u);
        temp_key[6u] |= ((uint32)(* (reg8 *) CYREG_SFLASH_DIE_SORT  ) << 16u);
        temp_key[7u] |= ((uint32)(* (reg8 *) CYREG_SFLASH_DIE_MINOR ) << 24u);
        /* End */


        CyBle_GenerateRandomNumber(&temp_key[8]);

        memcpy(key, temp_key, sizeof(temp_key));
    }
#endif /* (CYDEV_BOOTLOADER_ENABLE == 0) */


/*******************************************************************************
* Function Name: CR_GenerateNonce
********************************************************************************
*
* Summary:
*  Generates encryption nonce vector. Prefix CR stands for en/decryption to show 
*  that it is part of encryption module.
*
* Parameters:
*   uint8 * nonce:  Pointer to an array of bytes for generated nonce output. The  
*                   array length to be allocated by the application is 13 Bytes.
* 
* Return:
*  None
*
*******************************************************************************/
void CR_GenerateNonce(uint8 * nonce)
{
    static uint8 nonceBase[NONCE_LENGTH] = NONCE_INIT_VECTOR;
    uint8 len = NONCE_LENGTH;
    for(;len>0;len--)
    {
        nonce[len-1] = nonceBase[len-1]++;
    }    
}


/*******************************************************************************
* Function Name: CR_ReadNonce
********************************************************************************
*
* Summary:
*  Read nonce vector. Prefix CR stands for en/decryption to show that it is part 
*  of encryption module. 
*
* Parameters:
*   uint8 * nonce:  Pointer to an array of bytes for read nonce output. The  
*                   array length to be allocated by the application is 13 Bytes.
* 
* Return:
*  None
*
*******************************************************************************/
void CR_ReadNonce(uint8 * nonce)
{
    /*Temporary generate constant*/
    uint8 len = NONCE_LENGTH;
    uint8 readNonce = 1u;
    for(;len>0;len--)
    {
        nonce[len-1] = readNonce++;
    }        
}


/*******************************************************************************
* Function Name: CR_WriteKey
********************************************************************************
*
* Summary:
*  Write input encryption key to SFlash. Prefix CR stands for en/decryption to 
*  show that it is part of encryption module.
*
* Parameters:
*   uint8 * key:  Pointer to an array of bytes holding the key. The array length 
*                 to be allocated by the application should be 16 bytes.
*
* Return:
*  Status
*    Value                          Description
*    CY_SYS_FLASH_SUCCESS           Successful
*    CY_SYS_FLASH_INVALID_ADDR      Specified flash row address is invalid
*    CY_SYS_FLASH_PROTECTED         Specified flash row is protected
*    WRITE_KEY_ERROR                Failure
*
*******************************************************************************/
uint32 CR_WriteKey(uint8 * key)
{
    uint32 result = WRITE_KEY_ERROR;
    
    if(key != NULL)
    {
        result = SF_WriteUserSFlashRow(KEY_ROW_NUM, (uint32 *) key);  
    }
        
    return (result);
}


/*******************************************************************************
* Function Name: CR_ReadKey
********************************************************************************
*
* Summary:
*  Reed encryption key from SFlash. Prefix CR stands for en/decryption to show 
*  that it is part of encryption module.
*
* Parameters:
*   uint8 * key:  Pointer to an array of bytes holding the key. The array length 
*                 to be allocated by the application should be 16 bytes.
*
* Return:
*   None
*
*******************************************************************************/
void CR_ReadKey(uint8 * key)
{
    uint8 *keyAddr;
    uint32 offset = CYDEV_FLS_ROW_SIZE * (SFASH_START_ROW+KEY_ROW_NUM);
    keyAddr = (uint8*) CYDEV_SFLASH_BASE + offset;
    if (key != NULL)
    {
        memcpy(key, keyAddr, KEY_LENGTH);      
    }
}


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

            if(clkCnfRetValue == CY_SYS_FLASH_SUCCESS)
            {
                retValue = SF_CySysFlashClockConfig();
            }
            
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
    
    #if (CY_IP_SPCIF_SYNCHRONOUS)
        volatile uint32   parameters[2u];

        parameters[0u] =
                (uint32) ((CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_CLK_BACKUP) <<  CY_FLASH_PARAM_KEY_TWO_OFFSET) |
                        CY_FLASH_KEY_ONE);
        parameters[1u] = (uint32) &cySysFlashBackup.clockSettings[0u];
    
        CY_FLASH_CPUSS_SYSARG_REG = (uint32) &parameters[0u];
        CY_FLASH_CPUSS_SYSREQ_REG = CY_FLASH_CPUSS_REQ_START | CY_FLASH_API_OPCODE_CLK_BACKUP;
    #endif /* (CY_IP_SPCIF_SYNCHRONOUS) */
    
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

    /* FM-Lite Clock Configuration */
    CY_FLASH_CPUSS_SYSARG_REG =
        (uint32) ((CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_CLK_CONFIG) <<  CY_FLASH_PARAM_KEY_TWO_OFFSET) |
                    CY_FLASH_KEY_ONE);
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
    
    #if (CY_IP_SPCIF_SYNCHRONOUS)
	    volatile uint32   parameters[2u];

	    /* FM-Lite Clock Restore */
	    parameters[0u] =
	        (uint32) ((CY_FLASH_KEY_TWO(CY_FLASH_API_OPCODE_CLK_RESTORE) <<  CY_FLASH_PARAM_KEY_TWO_OFFSET) |
	                    CY_FLASH_KEY_ONE);
	    parameters[1u] = (uint32) &cySysFlashBackup.clockSettings[0u];
	    CY_FLASH_CPUSS_SYSARG_REG = (uint32) &parameters[0u];
	    CY_FLASH_CPUSS_SYSREQ_REG = CY_FLASH_CPUSS_REQ_START | CY_FLASH_API_OPCODE_CLK_RESTORE;
	    retValue = CY_FLASH_API_RETURN;
    #endif /* (CY_IP_SPCIF_SYNCHRONOUS) */

    return (retValue);
}

#endif /*(ENCRYPTION_ENABLED == YES)*/


/* [] END OF FILE */
