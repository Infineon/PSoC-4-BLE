/*******************************************************************************
* File Name: Encryption.h
*
* Version: 1.0
*
* Description:
*  Provides an API for the encryption.
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

#if !defined(Encryption_H)
#define Encryption_H
    
#include "Options.h"


#if (ENCRYPTION_ENABLED == YES)

    #include <BLE_Stack.h>
    #include <string.h>
    #include "WriteUserSFlash.h"
    
    #define NONCE_LENGTH        (13)  
    #define NONCE_INIT_VECTOR   {17,18,19,20,21,22,23,24,25,26,27,28,29}
    #define KEY_LENGTH          (16)
    #define EBCRYPTION_BLOCK_LENGTH (27)
    #define MIC_DATA_LENGTH     (4)
    
    #define SFASH_START_ROW     (4)

    #include "cytypes.h"

    void CR_Initialization(void);
    CYBLE_API_RESULT_T CR_Encrypt(uint8 * plain, uint8 length, uint8 * key, \
        uint8 * nonce, uint8 * encrypted, uint8 * out_mic);
    CYBLE_API_RESULT_T CR_Decrypt(uint8 * encrypted, uint16 length, uint8 * key, \
        uint8 * nonce, uint8 * decrypted, uint8 * out_mic);
    void CR_GenerateKey(uint8 * key);
    void CR_GenerateNonce(uint8 * nonce);
    void CR_ReadNonce(uint8 * nonce);
    uint32 CR_WriteKey(uint8 * key);
    void CR_ReadKey(uint8 * key);


    #define CR_SILICON_ID_REG              (*(reg32 *) CYREG_SFLASH_SILICON_ID)
    #define CR_DIE_LOT_NUM0_REG            (*(reg32 *) CYREG_SFLASH_DIE_LOT0)


#endif /* (ENCRYPTION_ENABLED == YES) */

#endif /* Encryption_H */


/* [] END OF FILE */
