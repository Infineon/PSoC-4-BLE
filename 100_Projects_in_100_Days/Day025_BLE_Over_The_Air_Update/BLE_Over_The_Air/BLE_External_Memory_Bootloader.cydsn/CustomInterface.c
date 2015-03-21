/*******************************************************************************
* File Name: CustomInterface.c
*
* Version: 1.0
*
* Description:
*  Provides an API that emulates Bootloader Host Tool application. The emulation
*  implemented within custom bootloader interface API.
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

#include "CustomInterface.h"
#include "ExternalMemoryInterface.h"
#include "cytypes.h"
#include "debug.h"
#include "CyFlash.h"

uint8  metadata[CY_FLASH_SIZEOF_ROW];



uint32 communicationState = CI_COMMUNICATION_STATE_IDLE;
uint32 numOfRxedRows;
uint16 numOfRows;
uint16 rowIdx;

uint16 flashRowTotal;
uint16 appFirstRowNum;

static uint16 CI_CalcExtMemAppChecksum(void);
static cystatus CI_WritePacket(uint8 status, uint8 buffer[], uint16 size);


/*******************************************************************************
* Function Name: CyBtldrCommRead
********************************************************************************
*
* Summary:
*
*
* Parameters:
*  None
*
* Returns:
*  None
*
*******************************************************************************/
cystatus CyBtldrCommRead(uint8* buffer, uint16 size, uint16* count, uint8 timeOut)
{
    cystatus rspCode  = CYRET_UNKNOWN;
    uint32 rspSize = 0u;
    uint16 appExtMemChecksum;
    
    
    buffer = buffer;
    size = size;
    timeOut = timeOut;

    if(CI_COMMUNICATION_STATE_IDLE == communicationState)
    {
        /* Generate Enter Bootloader packet */
        communicationState = CI_COMMUNICATION_STATE_ACTIVE;

        /* Read metadata section from external memory */
        (void) EMI_ReadData(EMI_MD_BASE_ADDR, CY_FLASH_SIZEOF_ROW , metadata);

        appFirstRowNum = ((uint16)((uint16)metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR + 1u] << 8u)) |
                                        metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR];

        flashRowTotal = ((uint16)((uint16)metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR + 1u] << 8u)) |
                                        metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR];


        appExtMemChecksum = ((uint16)((uint16)metadata[EMI_MD_APP_EM_CHECKSUM_ADDR + 1u] << 8u)) |
                                        metadata[EMI_MD_APP_EM_CHECKSUM_ADDR];

        DBG_PRINT_TEXT("\r\n");
        DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
        DBG_PRINT_TEXT("\tCyBtldrCommRead():\r\n");

        DBG_PRINT_TEXT("\t\t\t Metadata: Application Image Status: 0x");
        DBG_PRINT_HEX(metadata[EMI_MD_APP_STATUS_ADDR]);
        DBG_PRINT_TEXT("\r\n");

        DBG_PRINT_TEXT("\t\t\t Metadata: Image Size (in flash rows): 0x");
        DBG_PRINT_HEX(flashRowTotal);
        DBG_PRINT_TEXT("\r\n");

        DBG_PRINT_TEXT("\t\t\t Metadata: Start of image in flash (row #): 0x");
        DBG_PRINT_HEX(appFirstRowNum);
        DBG_PRINT_TEXT("\r\n");

        DBG_PRINT_TEXT("\t\t\t Metadata: Application checksum: 0x");
        DBG_PRINT_HEX(appExtMemChecksum);
        DBG_PRINT_TEXT("\r\n");
        
        DBG_PRINT_TEXT("\t\t\t Metadata: Encryption Enabled: 0x");
        DBG_PRINT_HEX(metadata[EMI_MD_ENCRYPTION_STATUS_ADDR]);
        DBG_PRINT_TEXT("\r\n");
        encryptionEnabled = metadata[EMI_MD_ENCRYPTION_STATUS_ADDR];

        /* Check application checksum in the external memory */
        if (CI_CalcExtMemAppChecksum() != appExtMemChecksum)
        {
            /* Mark application as invalid when checksum verification failed */
            metadata[EMI_MD_APP_STATUS_ADDR] = EMI_MD_APP_STATUS_INVALID;
        }


        if(metadata[EMI_MD_APP_STATUS_ADDR] == EMI_MD_APP_STATUS_VALID)
        {
            /* Start copying bootloadable application from external memory */

            /* Generate Enter Bootloader Command */
            buffer[CI_CMD_ADDR] = CI_COMMAND_ENTER;
            *count = CI_COMMAND_ENTER_PACKET_SIZE;
            rspSize = CI_COMMAND_ENTER_PACKET_DATA_SIZE;
            rspCode = CYRET_SUCCESS;

            /* Perform custom interface initial initializations */
            rowIdx = 0u;

            DBG_PRINT_TEXT("\r\n");
            DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
            DBG_PRINT_TEXT("\tCyBtldrCommRead():\r\n");
            DBG_PRINT_TEXT("\t\tApplication Status in External Memory: VALID\r\n");
            DBG_PRINT_TEXT("\t\t\tCopy bootloadable application from external memory.\r\n");
        }
        else if (metadata[EMI_MD_APP_STATUS_ADDR] == EMI_MD_APP_STATUS_LOADED)
        {

            /*******************************************************************
            * If the bootloadable application in the internal flash is valid,
            * schedule the specified application and perform software reset to
            * launch the bootloadable application.
            *
            * If the bootloadable application in the internal flash is not
            * valid, try to update bootloadable application from the external
            * flash memory.
            *******************************************************************/
            if(CYRET_SUCCESS == Bootloader_ValidateBootloadable(Bootloader_EXIT_TO_BTLDB))
            {
                (void) Bootloader_Exit(Bootloader_EXIT_TO_BTLDB);
            }
            else
            {
                /* Start copying bootloadable application from external memory */

                /* Generate Enter Bootloader Command */
                buffer[CI_CMD_ADDR] = CI_COMMAND_ENTER;
                *count = CI_COMMAND_ENTER_PACKET_SIZE;
                rspSize = CI_COMMAND_ENTER_PACKET_DATA_SIZE;
                rspCode = CYRET_SUCCESS;

                /* Perform custom interface initial initializations */
                rowIdx = 0u;

                DBG_PRINT_TEXT("\r\n");
                DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                DBG_PRINT_TEXT("\tCyBtldrCommRead():\r\n");
                DBG_PRINT_TEXT("\t\tApplication Status in External Memory: LOADED.\r\n");
                DBG_PRINT_TEXT("\t\tApplication in the internal FLASH: INVALID.\r\n");
                DBG_PRINT_TEXT("\t\t\tCopy bootloadable application from external memory.\r\n");
            }

            DBG_PRINT_TEXT("\r\n");
            DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
            DBG_PRINT_TEXT("\tCyBtldrCommRead():\r\n");
            DBG_PRINT_TEXT("\t\tApplication status: LOADED\r\n");
            DBG_PRINT_TEXT("\t\t\tSchedule bootloadable application and reset device.\r\n");

            DBG_PRINT_TEXT("\r\n");
            DBG_PRINT_TEXT("\r\n");
            DBG_PRINT_TEXT("===============================================================================\r\n");
            DBG_PRINT_TEXT("=    BLE_External_Memory_Bootloader Application Performs Software Reset       =\r\n");
            DBG_PRINT_TEXT("===============================================================================\r\n");
            DBG_PRINT_TEXT("\r\n");
            DBG_PRINT_TEXT("\r\n");
        }
        else
        {
            /*******************************************************************
            * The external memory metadata does not contain any information that
            * application image is present in the external memory, or the
            * application is invalid (checksum verification failed).
            *
            * Try to launch bootloadable application if it is valid. If the
            * bootloadable application in invalid, halt device.
            *******************************************************************/
            if(CYRET_SUCCESS == Bootloader_ValidateBootloadable(Bootloader_MD_BTLDB_ACTIVE_0))
            {
                (void) Bootloader_Exit(Bootloader_EXIT_TO_BTLDB);
            }
            else
            {
                DBG_PRINT_TEXT("\r\n");
                DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                DBG_PRINT_TEXT("\tCyBtldrCommRead():\r\n");
                DBG_PRINT_TEXT("\t\tApplication Status in External Memory: UNKNOWN.\r\n");
                DBG_PRINT_TEXT("\t\tApplication status: INVALID.\r\n");
                DBG_PRINT_TEXT("\t\t\tHalt Device.\r\n");
                CyHalt(0x00u);
            }
        }
    }
    else
    {
        if (CI_COMMUNICATION_STATE_ACTIVE == communicationState)
        {

            if (flashRowTotal >= rowIdx)
            {
                /* Generate Program Row Command */
                uint16 appFirstRowNumInArray;

                if (appFirstRowNum > CI_FLASH_ROWS_IN_ARRAY)
                {
                    appFirstRowNumInArray = appFirstRowNum - (CI_FLASH_ROWS_IN_ARRAY + 1u);
                }
                else
                {
                    appFirstRowNumInArray = appFirstRowNum;
                }

                buffer[CI_CMD_ADDR      ] = CI_COMMAND_PROGRAM;
                buffer[CI_DATA_ADDR     ] = CY_FLASH_GET_MACRO_FROM_ROW(appFirstRowNum);
                buffer[CI_DATA_ADDR + 1u] = LO8(appFirstRowNumInArray);
                buffer[CI_DATA_ADDR + 2u] = HI8(appFirstRowNumInArray);
                (void) EMI_ReadData(EMI_APP_ABS_ADDR(rowIdx), CY_FLASH_SIZEOF_ROW, (uint8 *) (buffer + CI_DATA_ADDR + 3u));

                if((flashRowTotal - 1u)  == rowIdx)
                {
                    /* Update metadata flash row */
                    buffer[CI_DATA_ADDR     ] = CY_FLASH_GET_MACRO_FROM_ROW(CY_FLASH_NUMBER_ROWS - 1u);
                    buffer[CI_DATA_ADDR + 1u] = LO8(CY_FLASH_NUMBER_ROWS - 2u - CI_FLASH_ROWS_IN_ARRAY);
                    buffer[CI_DATA_ADDR + 2u] = HI8(CY_FLASH_NUMBER_ROWS - 2u - CI_FLASH_ROWS_IN_ARRAY);
                }

                *count  = CI_COMMAND_PROGRAM_PACKET_SIZE;
                rspSize = CI_COMMAND_PROGRAM_PACKET_DATA_SIZE;
                rspCode = CYRET_SUCCESS;
                rowIdx++;
                appFirstRowNum++;

                DBG_PRINT_TEXT("\r\n");
                DBG_PRINT_TEXT("CustomInterface:\r\n");
                DBG_PRINT_TEXT("\tCyBtldrCommRead():\r\n");
                DBG_PRINT_TEXT("\t\tProgram Row:\r\n");

                DBG_PRINT_TEXT("\t\t\t rowIdx: 0x");
                DBG_PRINT_HEX(rowIdx - 1u);
                DBG_PRINT_TEXT("\r\n");

                DBG_PRINT_TEXT("\t\t\t appFirstRowNum: 0x");
                DBG_PRINT_HEX(appFirstRowNum - 1u);
                DBG_PRINT_TEXT("\r\n");
            }
            else
            {
                /* Schedule Bootloadable application */

                /* Mark bootloadable application as loaded */
                (void) EMI_ReadData(EMI_MD_BASE_ADDR, CY_FLASH_SIZEOF_ROW , metadata);
                metadata[EMI_MD_APP_STATUS_ADDR] = EMI_MD_APP_STATUS_LOADED;
                (void) EMI_WriteData(EMI_MD_BASE_ADDR, CY_FLASH_SIZEOF_ROW , metadata);

                /* Generate Exit Bootloader Command */
                buffer[CI_CMD_ADDR] = CI_COMMAND_EXIT;
                *count = CI_COMMAND_EXIT_PACKET_SIZE;
                rspSize = CI_COMMAND_EXIT_PACKET_DATA_SIZE;
                rspCode = CYRET_SUCCESS;

                DBG_PRINT_TEXT("\r\n");
                DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                DBG_PRINT_TEXT("\tCyBtldrCommRead():\r\n");
                DBG_PRINT_TEXT("\t\tExit Bootloader:\r\n");
            }

        }
    }

    CI_WritePacket(rspCode, buffer, rspSize);

    return (CYRET_SUCCESS);
}


/*******************************************************************************
* Function Name: CyBtldrCommStart
********************************************************************************
*
* Summary:
*
*
* Parameters:
*  None
*
* Returns:
*  None
*
*******************************************************************************/
void CyBtldrCommStart(void)
{
    numOfRxedRows = 0u;
    communicationState = CI_COMMUNICATION_STATE_IDLE;

    EMI_Start();

    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("CustomInterface:\r\n");
    DBG_PRINT_TEXT("\tCyBtldrCommStart():\r\n");
    DBG_PRINT_TEXT("\r\n");

}


/*******************************************************************************
* Function Name: CyBtldrCommStop
********************************************************************************
*
* Summary:
*
*
* Parameters:
*  None
*
* Returns:
*  None
*
*******************************************************************************/
void CyBtldrCommStop(void)
{
    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("CustomInterface:\r\n");
    DBG_PRINT_TEXT("\tCyBtldrCommStop():\r\n");
    DBG_PRINT_TEXT("\r\n");
}


/*******************************************************************************
* Function Name: CyBtldrCommReset
********************************************************************************
*
* Summary:
*
*
* Parameters:
*  None
*
* Returns:
*  None
*
*******************************************************************************/
void CyBtldrCommReset(void)
{
    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("CustomInterface:\r\n");
    DBG_PRINT_TEXT("\tCyBtldrCommReset():\r\n");
    DBG_PRINT_TEXT("\r\n");
}


/*******************************************************************************
* Function Name: CyBtldrCommWrite
********************************************************************************
*
* Summary:
*
*
* Parameters:
*  None
*
* Returns:
*  None
*
*******************************************************************************/
cystatus CyBtldrCommWrite(uint8* buffer, uint16 size, uint16* count, uint8 timeOut)
{
    buffer = buffer;
    size = size;
    count = count;
    timeOut = timeOut;

    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("CustomInterface:\r\n");
    DBG_PRINT_TEXT("\tCyBtldrCommWrite():\r\n");
    DBG_PRINT_TEXT("\r\n");

    return (CYRET_SUCCESS);
}


/*******************************************************************************
* Function Name: CI_CalcPacketChecksum
********************************************************************************
*
* Summary:
*  This computes the 16 bit checksum for the provided number of bytes contained
*  in the provided buffer
*
* Parameters:
*  buffer:
*     The buffer containing the data to compute the checksum for
*  size:
*     The number of bytes in the buffer to compute the checksum for
*
* Returns:
*  16 bit checksum for the provided data
*
*******************************************************************************/
static uint16 CI_CalcPacketChecksum(const uint8 buffer[], uint16 size)
{
    #if(0u != CI_PACKET_CHECKSUM_CRC)

        uint16 crc = CI_CRC_CCITT_INITIAL_VALUE;
        uint16 tmp;
        uint8  i;
        uint16 tmpIndex = size;

        if(0u == size)
        {
            crc = ~crc;
        }
        else
        {
            do
            {
                tmp = buffer[tmpIndex - size];

                for (i = 0u; i < 8u; i++)
                {
                    if (0u != ((crc & 0x0001u) ^ (tmp & 0x0001u)))
                    {
                        crc = (crc >> 1u) ^ CI_CRC_CCITT_POLYNOMIAL;
                    }
                    else
                    {
                        crc >>= 1u;
                    }

                    tmp >>= 1u;
                }

                size--;
            }
            while(0u != size);

            crc = ~crc;
            tmp = crc;
            crc = ( uint16 )(crc << 8u) | (tmp >> 8u);
        }

        return(crc);

    #else

        uint16 sum = 0u;

        while (size > 0u)
        {
            sum += buffer[size - 1u];
            size--;
        }

        return(( uint16 )1u + ( uint16 )(~sum));

    #endif /* (0u != Bootloader_PACKET_CHECKSUM_CRC) */
}


/*******************************************************************************
* Function Name: CI_WritePacket
********************************************************************************
*
* Summary:
*  Creates a bootloader response packet and transmits it back to the bootloader
*  host application over the already established communications protocol.
*
* Parameters:
*  status:
*      The status code to pass back as the second byte of the packet
*  buffer:
*      The buffer containing the data portion of the packet
*  size:
*      The number of bytes contained within the buffer to pass back
*
* Return:
*   CYRET_SUCCESS if successful. Any other non-zero value if failure occurred.
*
*******************************************************************************/
static cystatus CI_WritePacket(uint8 status, uint8 buffer[], uint16 size)
{
    uint16 CYDATA checksum;

    /* Start of packet. */
    buffer[CI_SOP_ADDR]       = CI_SOP;
    buffer[CI_SIZE_ADDR]      = LO8(size);
    buffer[CI_SIZE_ADDR + 1u] = HI8(size);

    /* Compute checksum. */
    checksum = CI_CalcPacketChecksum(buffer, size + CI_DATA_ADDR);

    buffer[CI_CHK_ADDR(size)]      = LO8(checksum);
    buffer[CI_CHK_ADDR(1u + size)] = HI8(checksum);
    buffer[CI_EOP_ADDR(size)]      = CI_EOP;

    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("CustomInterface:\r\n");
    DBG_PRINT_TEXT("\tCI_WritePacket():\r\n");

    DBG_PRINT_TEXT("\t\t SOP:       0x");
    DBG_PRINT_HEX(buffer[CI_SOP_ADDR]);
    DBG_PRINT_TEXT("\r\n");

    DBG_PRINT_TEXT("\t\t CMD:       0x");
    DBG_PRINT_HEX(buffer[CI_CMD_ADDR]);
    DBG_PRINT_TEXT("\r\n");

    DBG_PRINT_TEXT("\t\t SIZE:      0x");
    DBG_PRINT_HEX(size);
    DBG_PRINT_TEXT("\r\n");

    if (buffer[CI_CMD_ADDR] == 0x39)
    {
        DBG_PRINT_TEXT("\t\t Flash Array: 0x");
        DBG_PRINT_HEX(buffer[CI_DATA_ADDR ]);
        DBG_PRINT_TEXT("\r\n");

        DBG_PRINT_TEXT("\t\t Flash Row: 0x");
        DBG_PRINT_HEX(buffer[CI_DATA_ADDR + 2] << 8 | buffer[CI_DATA_ADDR + 1]);
        DBG_PRINT_TEXT("\r\n");
    }

    DBG_PRINT_TEXT("\t\t Checksum:  0x");
    DBG_PRINT_HEX(checksum);
    DBG_PRINT_TEXT("\r\n");

    DBG_PRINT_TEXT("\t\t EOP:       0x");
    DBG_PRINT_HEX(buffer[CI_EOP_ADDR(size)]);
    DBG_PRINT_TEXT("\r\n");

    DBG_PRINT_TEXT("\t\t Packet:    ");
    DBG_PRINT_ARRAY(buffer, CI_EOP_ADDR(size) + 1u);
    DBG_PRINT_TEXT("\r\n");

    return (CYRET_SUCCESS);
}


static uint16 CI_CalcExtMemAppChecksum(void)
{
    uint8  extMemRow[CY_FLASH_SIZEOF_ROW];
    uint16 extMemRowIdx;
    uint16 extMemAppRowsTotal;
    uint16 appExtMemChecksum = 0u;
    uint16 size;

    /* Get total number of the written flash rows to the external memory */
    (void) EMI_ReadData(EMI_MD_BASE_ADDR, CY_FLASH_SIZEOF_ROW , metadata);
    extMemAppRowsTotal = ((uint16)((uint16)metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR + 1u] << 8u)) |
                                      metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR];

    for (extMemRowIdx = 0u; extMemRowIdx < extMemAppRowsTotal; extMemRowIdx++)
    {
        (void) EMI_ReadData(EMI_APP_ABS_ADDR(extMemRowIdx), CY_FLASH_SIZEOF_ROW, extMemRow);

        size = CY_FLASH_SIZEOF_ROW;
        while (size > 0u)
        {
            size--;
            appExtMemChecksum += extMemRow[size];
        }
    }

    appExtMemChecksum = ( uint16 )1u + ( uint16 )(~appExtMemChecksum);

    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("CustomInterface:\r\n");
    DBG_PRINT_TEXT("\tCI_CalcExtMemAppChecksum():\r\n");
    DBG_PRINT_TEXT("\r\n");

    DBG_PRINT_TEXT("\t\t Calculated checksum: 0x");
    DBG_PRINT_HEX(appExtMemChecksum);
    DBG_PRINT_TEXT("\r\n");

    return(appExtMemChecksum);
}


/* [] END OF FILE */
