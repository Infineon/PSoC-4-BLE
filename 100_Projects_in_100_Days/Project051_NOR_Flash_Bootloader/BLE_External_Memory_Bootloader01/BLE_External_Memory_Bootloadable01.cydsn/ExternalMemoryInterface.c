/*******************************************************************************
* File Name: ExternalMemoryInterface.c
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

#include "Options.h"
#include "ExternalMemoryInterface.h"
#include "cytypes.h"
#include "debug.h"
#include <project.h>


uint8 emiWriteBuffer[EMI_SIZE_OF_WRITE_BUFFER];



/*******************************************************************************
* Function Name: EMI_Start
********************************************************************************
*
* Summary:
*  Starts the external memory interface.
*
* Parameters:
*  None
*
* Return:
*  None
*******************************************************************************/
void EMI_Start(void)
{
    /* Start the SCB component */
    EMI_SPIM_Start();
}


/*******************************************************************************
* Function Name: EMI_WriteData
********************************************************************************
*
* Summary:
*  Write data to the external memory.
*
* Parameters:
*  uint8 instruction:
*   Whether the master wants to read/write/erase/program the NOR Flash etc.
*   The instruction set is as per the Serial NOR Flash part's datasheet.
*  uint32 addressBytes:
*   Address of the location in the Serial NOR Flash to write to.
*  uint32 dataSize:
*   Amount of data to write.
*  uint8 *data:
*   Pointer to data that is written to external memory
*
* Return:
*  Status
*     Value               Description
*    CYRET_SUCCESS           Successful
*    Other non-zero          Failure
*******************************************************************************/
cystatus EMI_WriteData(uint8 instruction, uint32 addressBytes, uint32 dataSize, uint8 *data)
{
    uint32 i;
    uint32 dataPointer = 1;

    /* Enable write on the Serial NOR Flash. */
    {
        EMI_SPIM_SpiUartClearRxBuffer();
        EMI_SPIM_SpiUartClearTxBuffer();

        /* Pull Slave select low */
        EMI_SPIM_SlaveSelect_Write(0);
    
        EMI_SPIM_SpiUartWriteTxData(NOR_FLASH_INSTRUCTION_WRITE_ENABLE);
        while(1 != EMI_SPIM_SpiUartGetRxBufferSize())
        {
            /* Wait until the RX FIFO has the same entries as the amount 
             * transmitted - signifies a completed transfer. 
             */
        }
        
        /* Pull Slave select high */
        EMI_SPIM_SlaveSelect_Write(1);
    
    }
    
    /* Instruction to write to the Serial NOR flash */
    emiWriteBuffer[0] = instruction;
    dataPointer = 1;
    
    /* Address bytes if required */
    if((instruction == NOR_FLASH_INSTRUCTION_SECTOR_ERASE) ||
       (instruction == NOR_FLASH_INSTRUCTION_BLOCK_ERASE) ||
       (instruction == NOR_FLASH_INSTRUCTION_PP))
    {
        emiWriteBuffer[1] = (uint8) (addressBytes >> 16u);
        emiWriteBuffer[2] = (uint8) (addressBytes >> 8u);
        emiWriteBuffer[3] = (uint8) (addressBytes);
        
        dataPointer += 3;
    }
    
    #if (ENCRYPTION_ENABLED == YES)
        if ((addressBytes >= EMI_APP_BASE_ADDR) && (dataSize>0) && (data != NULL))
        {
            uint8 key[KEY_LENGTH] = {0};
            uint8 nonce[NONCE_LENGTH] = {0};
            uint8 encrypted[ENC_BUFFER_SIZE] = {0};
            uint8 out_mic[MIC_DATA_LENGTH];
            CYBLE_API_RESULT_T result;
            
            CR_ReadKey(key);
            CR_ReadNonce(nonce);
            result = CR_Encrypt(data, dataSize, key, nonce, encrypted, out_mic);
            
            if (result == CYBLE_ERROR_OK)
            {
                memcpy(data, encrypted, dataSize);
            }
            else
            {
                if (result == CYBLE_ERROR_INVALID_PARAMETER)
                {
                    DBG_PRINT_TEXT("===============================================================================\r\n");                    
                    DBG_PRINT_TEXT("=              ENCRYPTION ERROR: CYBLE_ERROR_INVALID_PARAMETER                 \r\n");
                    DBG_PRINT_TEXT("===============================================================================\r\n");                    
                }
                else 
                {
                    DBG_PRINT_TEXT("===============================================================================\r\n");
                    DBG_PRINT_TEXT("=              ENCRYPTION ERROR:UNKNOWN:  ");
                    DBG_PRINT_HEX(result);
                    DBG_PRINT_TEXT("=\r\n");
                    DBG_PRINT_TEXT("===============================================================================\r\n");                    
                }
                return (result);
            }
            
        }
    #endif /* (ENCRYPTION_ENABLED == YES) */
    
    /* Data when required to be written - for PP command */
    if((instruction == NOR_FLASH_INSTRUCTION_PP) && (data != NULL))
    {
        for (i = 0; i < dataSize; i++)
        {        
            emiWriteBuffer[dataPointer + i] = data[i];   
        }
        dataPointer += dataSize;
    }
    
    /* Clear Rx and Tx Buffers */
    EMI_SPIM_SpiUartClearRxBuffer();
    EMI_SPIM_SpiUartClearTxBuffer();
    
    /* Pull Slave select low */
    EMI_SPIM_SlaveSelect_Write(0);
    
	/* Initiate write - the first byte defines whether this is a 
     * write or erase or PP for the Serial NOR Flash.
     */
    (void) EMI_SPIM_SpiUartPutArray(emiWriteBuffer, dataPointer);
    
    while(dataPointer != EMI_SPIM_SpiUartGetRxBufferSize())
    {
        /* Wait until the RX FIFO has the same entries as the amount 
         * transmitted - signifies a completed transfer. 
         */
    }

    /* Pull Slave select high */
    EMI_SPIM_SlaveSelect_Write(1);
    
    while(EMI_IsBusy())
    {
        /* Wait while the page program/erase operation is complete. */
    }
    
    return (CYRET_SUCCESS);
}


/*******************************************************************************
* Function Name: EMI_IsBusy
********************************************************************************
*
* Summary:
*  Read whether the external memory is busy.
*
* Parameters:
*  None   
*
* Return:
*  bool
*     Value               Description
*     false                 Not busy
*     true                  Busy
*
*******************************************************************************/
bool EMI_IsBusy(void)
{
    bool status = false;
    uint8 dataPointer = 1;
    uint8 tmpBuffer[2];
    
    emiWriteBuffer[0] = NOR_FLASH_INSTRUCTION_READ_SR1;
    emiWriteBuffer[1] = 0;
    dataPointer = 2;
    
    /* Clear Rx and Tx Buffer */
    EMI_SPIM_SpiUartClearRxBuffer();
    EMI_SPIM_SpiUartClearTxBuffer();
    
    /* Pull Slave select low */
    EMI_SPIM_SlaveSelect_Write(0);
    
    /* Write dummy content so MOSI is busy while MISO takes data */
    EMI_SPIM_SpiUartPutArray(emiWriteBuffer, dataPointer);
    
    while(dataPointer != EMI_SPIM_SpiUartGetRxBufferSize())
    {
        /* Wait until the RX FIFO has the same entries as the amount 
         * transmitted - signifies a completed transfer. 
         */
    }
    
    /* Pull Slave select high */
    EMI_SPIM_SlaveSelect_Write(1);
    
    /* Read 2 bytes data from the RX buffer */
    tmpBuffer[0] = (uint8) EMI_SPIM_SpiUartReadRxData();
    tmpBuffer[1] = (uint8) EMI_SPIM_SpiUartReadRxData();
    
    if(tmpBuffer[1] & EMI_BUSY_BIT_FIELD)
    {
        /* External memory is busy */
        status = true;
    }
    else
    {
        /* External memory is available to accept next command */
        status = false;
    }
    
    return status;
}


/*******************************************************************************
* Function Name: EMI_ReadData
********************************************************************************
*
* Summary:
*  Read data from the external memory.
*
* Parameters:
*  uint32 addressBytes: The address in external memory from where to start.
*   
*  uint32 dataSize: Amount of data to be read
*   
*  uint8 *data:     Data is copied to this array.
*   
*
* Return:
*  Status
*     Value               Description
*    CYRET_SUCCESS           Successful
*    Other non-zero          Failure
*    CYBLE_ERROR_INVALID_PARAMETER - problems with decryption
*******************************************************************************/
cystatus EMI_ReadData(uint32 addressBytes, uint32 dataSize, uint8 *data)
{
    cystatus status;
    uint32 dataPointer = 1;
    uint32 i = 0;
    
    /* Instruction to write to the Serial NOR flash */
    emiWriteBuffer[0] = NOR_FLASH_INSTRUCTION_READ;
    dataPointer = 1;
    
    /* Address bytes */
    emiWriteBuffer[1] = (uint8) (addressBytes >> 16u);
    emiWriteBuffer[2] = (uint8) (addressBytes >> 8u);
    emiWriteBuffer[3] = (uint8) (addressBytes);
    dataPointer += 3;
    
    for(i = 0; i < dataSize; i++)
    {
        emiWriteBuffer[dataPointer + i] = 0;
    }
    dataPointer += dataSize;
    
    /* Clear Rx and Tx buffers */
    EMI_SPIM_SpiUartClearRxBuffer();
    EMI_SPIM_SpiUartClearTxBuffer();

    /* Pull Slave select low */
    EMI_SPIM_SlaveSelect_Write(0);
    
    /* Write dummy content so MOSI is busy while MISO takes data */
    EMI_SPIM_SpiUartPutArray(emiWriteBuffer, dataPointer);
    
    while(dataPointer != EMI_SPIM_SpiUartGetRxBufferSize())
    {
        /* Wait until the RX FIFO has the same entries as the amount 
         * transmitted - signifies a completed transfer. 
         */
    }

    /* Pull Slave select high */
    EMI_SPIM_SlaveSelect_Write(1);
    
    /* Get the incoming data, except the first 4 bytes */
    (void) EMI_SPIM_SpiUartReadRxData();
    (void) EMI_SPIM_SpiUartReadRxData();
    (void) EMI_SPIM_SpiUartReadRxData();
    (void) EMI_SPIM_SpiUartReadRxData();
    for(i = 0; i < dataSize; i++)
    {
        data[i] = (uint8) EMI_SPIM_SpiUartReadRxData();
    }

    status = CYRET_SUCCESS;
    
    #if (ENCRYPTION_ENABLED == YES)
    /*Encrypt data except to metadata*/
    
    if ((addressBytes >= EMI_APP_BASE_ADDR) && (dataSize>0))
    {
        uint8 key[KEY_LENGTH] = {0};
        uint8 nonce[NONCE_LENGTH] = {0};
        uint8 decrypted[ENC_BUFFER_SIZE] = {0};
        uint8 out_mic[MIC_DATA_LENGTH]={0};
        CYBLE_API_RESULT_T result;
        
        CR_ReadKey(key);
        CR_ReadNonce(nonce);
        result = CR_Decrypt(data, dataSize, key, nonce, decrypted, out_mic);
        /* Invalid MIC_AUTH not checked  as it will consume additional memory and 
           was not required.*/
        if (result == CYBLE_ERROR_INVALID_PARAMETER)
        {
            DBG_PRINT_TEXT("DECRYPTION ERROR: CYBLE_ERROR_INVALID_PARAMETER            \r\n");
            status = CYBLE_ERROR_INVALID_PARAMETER;
        }
        else
        {
            memcpy(data, decrypted, dataSize);
        }                                            
    }
    #endif /* (ENCRYPTION_ENABLED == YES) */

    return status;
}

/*******************************************************************************
* Function Name: EMI_EraseAll
********************************************************************************
*
* Summary:
*  Erases content of the external memory.
*  Caution: All data is erased, including metadata.
*
* Parameters:
*  None
*
* Return:
*  Status
*     Value                     Description
*    CYRET_SUCCESS                  Successful
*    Other non-zero                 Failure
*
* Side Effects:
*  None
*
*******************************************************************************/
cystatus EMI_EraseAll(void)
{
    /* Erase in terms of blocks - each block is of 64 kB.
     * So erasing four blocks should suffice for 256 kB chips as well.
     * Caution: All data is erased, including metadata.
     */
    EMI_WriteData(NOR_FLASH_INSTRUCTION_BLOCK_ERASE, 0                                       , 0, NULL);
    EMI_WriteData(NOR_FLASH_INSTRUCTION_BLOCK_ERASE, 0 + (1 * EMI_EXTERNAL_MEMORY_BLOCK_SIZE), 0, NULL);
    EMI_WriteData(NOR_FLASH_INSTRUCTION_BLOCK_ERASE, 0 + (2 * EMI_EXTERNAL_MEMORY_BLOCK_SIZE), 0, NULL);
    EMI_WriteData(NOR_FLASH_INSTRUCTION_BLOCK_ERASE, 0 + (3 * EMI_EXTERNAL_MEMORY_BLOCK_SIZE), 0, NULL);
    
    return (CYRET_SUCCESS);
}


/* [] END OF FILE */
