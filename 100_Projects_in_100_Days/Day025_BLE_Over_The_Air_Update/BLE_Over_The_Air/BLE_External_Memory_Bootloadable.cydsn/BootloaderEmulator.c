/*******************************************************************************
* File Name: BootloaderEmulator.c
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

#include "BootloaderEmulator_PVT.h"
#include "ExternalMemoryInterface.h"
#include "BleCommunication.h"
#include "debug.h"

#define BootloaderEmulator_activeApp      (BootloaderEmulator_MD_BTLDB_ACTIVE_0)

uint8  metadata[CY_FLASH_SIZEOF_ROW];

uint16 appSizeInRows;
uint16 appFirstRowNum;
uint16 appExtMemChecksum = 0u;

#if (ENCRYPTION_ENABLED == YES)
    uint8 emiKey[KEY_LENGTH];
#endif /*ENCRYPTION_ENABLED == YES*/

/***************************************
*     Function Prototypes
***************************************/
static cystatus BootloaderEmulator_WritePacket(uint8 status, uint8 buffer[], uint16 size);
static uint16   BootloaderEmulator_CalcPacketChecksum(const uint8 buffer[], uint16 size);
static void     BootloaderEmulator_HostLink(uint8 timeOut);


/*******************************************************************************
* Function Name: BootloaderEmulator_CalcPacketChecksum
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
static uint16 BootloaderEmulator_CalcPacketChecksum(const uint8 buffer[], uint16 size)
{
    #if(0u != BootloaderEmulator_PACKET_CHECKSUM_CRC)

        uint16 CYDATA crc = BootloaderEmulator_CRC_CCITT_INITIAL_VALUE;
        uint16 CYDATA tmp;
        uint8  CYDATA i;
        uint16 CYDATA tmpIndex = size;

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
                        crc = (crc >> 1u) ^ BootloaderEmulator_CRC_CCITT_POLYNOMIAL;
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

        uint16 CYDATA sum = 0u;

        while (size > 0u)
        {
            sum += buffer[size - 1u];
            size--;
        }

        return(( uint16 )1u + ( uint16 )(~sum));

    #endif /* (0u != BootloaderEmulator_PACKET_CHECKSUM_CRC) */
}


/*******************************************************************************
* Function Name: BootloaderEmulator_Calc8BitSum
********************************************************************************
*
* Summary:
*  This computes the 8 bit sum for the provided number of bytes contained in
*  FRAM.
*
* Parameters:
* rowNum :
*   Flash row number as located in the external memory.
*
* Returns:
*   The 8 bit sum for the provided data.
*
*******************************************************************************/
uint32 BootloaderEmulator_Calc8BitSum(uint32 rowNum)
{
    uint32 sum = 0u;
    uint8 flashRowFromExtMem[CY_FLASH_SIZEOF_ROW];
    uint32 size = CY_FLASH_SIZEOF_ROW;

    (void) EMI_ReadData(EMI_APP_ABS_ADDR(rowNum), CY_FLASH_SIZEOF_ROW, flashRowFromExtMem);

    DBG_PRINT_TEXT("\r\n");
    DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
    DBG_PRINT_TEXT("\tBootloaderEmulator_Calc8BitSum():\r\n");

    DBG_PRINT_TEXT("\t\trowNum  = ");
    DBG_PRINT_DEC( (uint16) rowNum);
    DBG_PRINT_TEXT("\r\n");

    DBG_PRINT_TEXT("\t\tEMI_APP_ABS_ADDR(rowNum)  = ");
    DBG_PRINT_HEX(EMI_APP_ABS_ADDR((uint16)rowNum));
    DBG_PRINT_TEXT("\r\n");

    DBG_PRINT_TEXT("\t\tEMI_ReadData():\r\n");
    DBG_PRINT_ARRAY(flashRowFromExtMem, size);
    DBG_PRINT_TEXT("\r\n");


    while (size > 0u)
    {
        size--;
        sum += flashRowFromExtMem[size];
    }

    return(sum);
}


/*******************************************************************************
* Function Name: BootloaderEmulator_Start
********************************************************************************
* Summary:
*  This function is called in order to execute the following algorithm:
*
*  - Identify the active bootloadable application (applicable only to
*    the Multi-application bootloader)
*
*  - Validate the bootloader application (design-time configurable, Bootloader
*    application validation option of the component customizer)
*
*  - Validate the active bootloadable application. If active bootloadable
*    application is not valid, and the other bootloadable application (inactive)
*    is valid, the last one is started.
*
*  - Run a communication subroutine (design-time configurable, Wait for command
*    option of the component customizer)
*
*  - Schedule the bootloadable and reset the device
*
* Parameters:
*  None
*
* Return:
*  This method will never return. It will either load a new application and
*  reset the device or jump directly to the existing application. The CPU is
*  halted, if validation failed when "Bootloader application validation" option
*  is enabled.
*  PSoC 3/PSoC 5: The CPU is halted if Flash initialization fails.
*
* Side Effects:
*  If Bootloader application validation option is enabled and this method
*  determines that the bootloader application itself is corrupt, this method
*  will not return, instead it will simply hang the application.
*
*******************************************************************************/
void BootloaderEmulator_Start(void)
{
    EMI_Start();
    BootloaderEmulator_HostLink(BootloaderEmulator_WAIT_FOR_COMMAND_FOREVER);
}


/*******************************************************************************
* Function Name: BootloaderEmulator_ValidateBootloadable
********************************************************************************
* Summary:
*  Performs the bootloadable application validation by calculating the
*  application image checksum and comparing it with the checksum value stored
*  in the Bootloadable Application Checksum field of the metadata section.
*
*  If the Fast bootloadable application validation option is enabled in the
*  component customizer and bootloadable application successfully passes
*  validation, the Bootloadable Application Verification Status field of the
*  metadata section is updated. Refer to the Metadata Layout section for the
*  details.
*
*  If the Fast bootloadable application validation option is enabled and
*  Bootloadable Application Verification Status field of the metadata section
*  claims that bootloadable application is valid, the function returns
*  CYRET_SUCCESS without further checksum calculation.
*
* Parameters:
*  appId:
*  The number of the bootloadable application should be 0 for the normal
*  bootloader and 0 or 1 for the Multi-Application bootloader.
*
* Returns:
*  Returns CYRET_SUCCESS if the specified bootloadable application is valid.
*
*******************************************************************************/
cystatus BootloaderEmulator_ValidateBootloadable(void)
{
    uint8 appChecksum;
    uint8 calcedChecksum = 0u;
    uint8   appFlashRow[CY_FLASH_SIZEOF_ROW];
    uint32 row;
    uint32 i;
    uint32 valid = 0u;

    cystatus status = CYRET_SUCCESS;

    if (0u != appSizeInRows)
    {
        /* Get bootloadable application checksum from external memory */
        (void) EMI_ReadData(EMI_APP_ABS_ADDR(appSizeInRows - 1u), CY_FLASH_SIZEOF_ROW, appFlashRow);


        DBG_PRINT_TEXT("\t\t Metadata Row: ");
        DBG_PRINT_ARRAY(appFlashRow, CY_FLASH_SIZEOF_ROW);
        DBG_PRINT_TEXT("\r\n");

        appChecksum = appFlashRow[BootloaderEmulator_MD_APP_CHECKSUM];


        for (row = 0u; row < (appSizeInRows - 1u); row++)
        {
            /* Read flash row data from external memory */
            (void) EMI_ReadData(EMI_APP_ABS_ADDR(row) , CY_FLASH_SIZEOF_ROW, appFlashRow);

            /* Calculate checksum of application row */
            for(i = 0u; i < CY_FLASH_SIZEOF_ROW; i++)
            {
                if((appFlashRow[i] != 0u) && (appFlashRow[i] != 0xFFu))
                {
                    valid = 1u;
                }
                calcedChecksum += appFlashRow[i];
            }
        }

        calcedChecksum = ( uint8 )1u + ( uint8 )(~calcedChecksum);

        if((calcedChecksum != appChecksum) || (0u == valid))
        {
            status = CYRET_BAD_DATA;
        }
        
        DBG_PRINT_TEXT("\r\n");
        DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
        DBG_PRINT_TEXT("\tBootloaderEmulator_ValidateBootloadable() Status: ");
        DBG_PRINT_DEC( (uint16) status);
        DBG_PRINT_TEXT("\r\n");

        DBG_PRINT_TEXT("\t\tStored Bootloadable Application Checksum  = ");
        DBG_PRINT_DEC( (uint16) appChecksum);
        DBG_PRINT_TEXT("\r\n");

        DBG_PRINT_TEXT("\t\tCalculated Bootloadable Application Checksum  = ");
        DBG_PRINT_DEC( (uint16) calcedChecksum);
        DBG_PRINT_TEXT("\r\n");
    }
    else
    {
        status = CYRET_BAD_DATA;
        DBG_PRINT_TEXT("\r\n");
        DBG_PRINT_TEXT("Error: Stored data length = 0.");
        DBG_PRINT_TEXT("\r\n");
    }
    

    return(status);
}


/*******************************************************************************
* Function Name: BootloaderEmulator_HostLink
********************************************************************************
*
* Summary:
*  Causes the bootloader to attempt to read data being transmitted by the
*  host application.  If data is sent from the host, this establishes the
*  communication interface to process all requests.
*
* Parameters:
*  timeOut:
*   The amount of time to listen for data before giving up. Timeout is
*   measured in 10s of ms.  Use 0 for an infinite wait.
*
* Return:
*   None
*
*******************************************************************************/
static void BootloaderEmulator_HostLink(uint8 timeOut)
{
    uint16    CYDATA numberRead;
    uint16    CYDATA rspSize;
    uint8     CYDATA ackCode;
    uint16    CYDATA pktChecksum;
    cystatus  CYDATA readStat;
    uint16    CYDATA pktSize    = 0u;
    uint16    CYDATA dataOffset = 0u;
    uint8     CYDATA timeOutCnt = 10u;

    #if(0u != BootloaderEmulator_FAST_APP_VALIDATION)
        uint8 CYDATA clearedMetaData = 0u;
    #endif  /* (0u != BootloaderEmulator_FAST_APP_VALIDATION) */

    CYBIT     communicationState = BootloaderEmulator_COMMUNICATION_STATE_IDLE;

    uint8     packetBuffer[BootloaderEmulator_SIZEOF_COMMAND_BUFFER];
    uint8     dataBuffer  [BootloaderEmulator_SIZEOF_COMMAND_BUFFER];



    /* Initialize communications channel. */
    CyBtldrCommStart();

    /* Enable global interrupts */
    CyGlobalIntEnable;

    do
    {
        ackCode = CYRET_SUCCESS;

        do
        {
            readStat = CyBtldrCommRead(packetBuffer,
                                        BootloaderEmulator_SIZEOF_COMMAND_BUFFER,
                                        &numberRead,
                                        (0u == timeOut) ? 0xFFu : timeOut);
            if (0u != timeOut)
            {
                timeOutCnt--;
            }

        } while ( (0u != timeOutCnt) && (readStat != CYRET_SUCCESS) );


        if( readStat != CYRET_SUCCESS )
        {
            continue;
        }

        if((numberRead < BootloaderEmulator_MIN_PKT_SIZE) ||
           (packetBuffer[BootloaderEmulator_SOP_ADDR] != BootloaderEmulator_SOP))
        {
            ackCode = BootloaderEmulator_ERR_DATA;
        }
        else
        {
            pktSize = ((uint16)((uint16)packetBuffer[BootloaderEmulator_SIZE_ADDR + 1u] << 8u)) |
                               packetBuffer[BootloaderEmulator_SIZE_ADDR];

            pktChecksum = ((uint16)((uint16)packetBuffer[BootloaderEmulator_CHK_ADDR(pktSize) + 1u] << 8u)) |
                                   packetBuffer[BootloaderEmulator_CHK_ADDR(pktSize)];

            if((pktSize + BootloaderEmulator_MIN_PKT_SIZE) > numberRead)
            {
                ackCode = BootloaderEmulator_ERR_LENGTH;
            }
            else if(packetBuffer[BootloaderEmulator_EOP_ADDR(pktSize)] != BootloaderEmulator_EOP)
            {
                ackCode = BootloaderEmulator_ERR_DATA;
            }
            else if(pktChecksum != BootloaderEmulator_CalcPacketChecksum(packetBuffer,
                                                                        pktSize + BootloaderEmulator_DATA_ADDR))
            {
                ackCode = BootloaderEmulator_ERR_CHECKSUM;
            }
            else
            {
                /* Empty section */
            }
        }

        rspSize = 0u;
        if(ackCode == CYRET_SUCCESS)
        {
            uint8 CYDATA btldrData = packetBuffer[BootloaderEmulator_DATA_ADDR];

            ackCode = BootloaderEmulator_ERR_DATA;
            switch(packetBuffer[BootloaderEmulator_CMD_ADDR])
            {

            /***************************************************************************
            *   Verify checksum
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_CHECKSUM:

                if((BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState) && (pktSize == 0u))
                {
                    packetBuffer[BootloaderEmulator_DATA_ADDR] =
                            (uint8)(BootloaderEmulator_ValidateBootloadable() == CYRET_SUCCESS);

                    DBG_PRINT_TEXT("\r\n");
                    DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                    DBG_PRINT_TEXT("\tVerify Checksum Command:\r\n");
                    DBG_PRINT_HEX(packetBuffer[BootloaderEmulator_DATA_ADDR]);
                    DBG_PRINT_TEXT("\r\n");

                    rspSize = 1u;
                    ackCode = CYRET_SUCCESS;
                }
                break;


            /***************************************************************************
            *   Get flash size
            ***************************************************************************/

            #if(0u != BootloaderEmulator_CMD_GET_FLASH_SIZE_AVAIL)

                case BootloaderEmulator_COMMAND_REPORT_SIZE:

                    if((BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState) && (pktSize == 1u))
                    {
                        packetBuffer[BootloaderEmulator_DATA_ADDR]      = LO8(BootloaderEmulator_FIRST_ROW_IN_ARRAY);
                        packetBuffer[BootloaderEmulator_DATA_ADDR + 1u] = HI8(BootloaderEmulator_FIRST_ROW_IN_ARRAY);

                        packetBuffer[BootloaderEmulator_DATA_ADDR + 2u] =
                                    LO8(BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY - 1u);

                        packetBuffer[BootloaderEmulator_DATA_ADDR + 3u] =
                                    HI8(BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY - 1u);

                        rspSize = 4u;
                        ackCode = CYRET_SUCCESS;

                        DBG_PRINT_TEXT("\r\n");
                        DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                        DBG_PRINT_TEXT("\tGet Flash Size:\r\n");

                        DBG_PRINT_TEXT("\t\tFirst Available Row in Array: 0x");
                        DBG_PRINT_HEX(HI8(BootloaderEmulator_FIRST_ROW_IN_ARRAY));
                        DBG_PRINT_HEX(LO8(BootloaderEmulator_FIRST_ROW_IN_ARRAY));
                        DBG_PRINT_TEXT("\r\n");

                        DBG_PRINT_TEXT("\t\tLast Available Row in Array: 0x");
                        DBG_PRINT_HEX(HI8(BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY - 1u));
                        DBG_PRINT_HEX(LO8(BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY - 1u));
                        DBG_PRINT_TEXT("\r\n");
                        DBG_PRINT_TEXT("\r\n");

                    }
                    break;

            #endif  /* (0u != BootloaderEmulator_CMD_GET_FLASH_SIZE_AVAIL) */


            /***************************************************************************
            *   Program / Erase row
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_PROGRAM:

            /* The btldrData variable holds Flash Array ID */

        #if (0u != BootloaderEmulator_CMD_ERASE_ROW_AVAIL)

            case BootloaderEmulator_COMMAND_ERASE:
                if (BootloaderEmulator_COMMAND_ERASE == packetBuffer[BootloaderEmulator_CMD_ADDR])
                {
                    if ((BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState) && (pktSize == 3u))
                    {
                        /* Size of FLASH row (no ECC available) */
                        dataOffset = BootloaderEmulator_FROW_SIZE;

                        (void) memset(dataBuffer, 0, (uint32) dataOffset);
                    }
                    else
                    {
                        break;
                    }
                }

        #endif  /* (0u != BootloaderEmulator_CMD_ERASE_ROW_AVAIL) */

                if((BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState) && (pktSize >= 3u))
                {
                    /* The command may be sent along with the last block of data, to program the row. */
                    (void) memcpy(&dataBuffer[dataOffset],
                                &packetBuffer[BootloaderEmulator_DATA_ADDR + 3u],
                                (uint32) pktSize - 3u);

                    dataOffset += (pktSize - 3u);

                    /* Size of FLASH row (no ECC available) */
                    pktSize = CY_FLASH_SIZEOF_ROW;


                    /* Check if we have all data to program */
                    if(dataOffset == pktSize)
                    {
                        uint16 row;
                        uint32 size = CY_FLASH_SIZEOF_ROW;

                        /* Save 1st bootloadable application flash row number to the metadata in external memory */
                        if (appSizeInRows == 0u)
                        {
                            /* Get Flash row number inside of the array */
                            dataOffset = ((uint16)((uint16)packetBuffer[BootloaderEmulator_DATA_ADDR + 2u] << 8u)) |
                                                  packetBuffer[BootloaderEmulator_DATA_ADDR + 1u];

                            /* btldrData  - holds flash array Id sent by host */
                            /* dataOffset - holds flash row Id sent by host   */
                            row = (uint16)(btldrData * BootloaderEmulator_NUMBER_OF_ROWS_IN_ARRAY) + dataOffset;

                            /* Save number of the bootloadable application first flash row */
                            appFirstRowNum = row;
                        }

                        /* Erase metadata row */
                        if (appSizeInRows == 0u)
                        {
                            /* Ersase content of the external memory */
                            uint8  erase[CY_FLASH_SIZEOF_ROW] = {0u};
                            #if (DEBUG_UART_ENABLED == YES)
                                uint8  tmp[CY_FLASH_SIZEOF_ROW];
                            #endif /* #if (DEBUG_UART_ENABLED == YES) */

                            #if (DEBUG_UART_ENABLED == YES)
                                (void) EMI_ReadData(EMI_MD_BASE_ADDR, CY_FLASH_SIZEOF_ROW, tmp);
                            #endif /* #if (DEBUG_UART_ENABLED == YES) */
                            DBG_PRINT_TEXT("\t\t Metadata Row Before Erase: ");
                            DBG_PRINT_ARRAY(tmp, CY_FLASH_SIZEOF_ROW);
                            DBG_PRINT_TEXT("\r\n");

                            (void) EMI_WriteData(EMI_MD_BASE_ADDR , CY_FLASH_SIZEOF_ROW, erase);

                            #if (DEBUG_UART_ENABLED == YES)
                                (void) EMI_ReadData(EMI_MD_BASE_ADDR, CY_FLASH_SIZEOF_ROW, tmp);
                            #endif /* #if (DEBUG_UART_ENABLED == YES) */
                            DBG_PRINT_TEXT("\t\t Metadata Row After Erase: ");
                            DBG_PRINT_ARRAY(tmp, CY_FLASH_SIZEOF_ROW);
                            DBG_PRINT_TEXT("\r\n");
                        }


                        /* External memory application checksum calculation */
                        while (size > 0u)
                        {
                            size--;
                            appExtMemChecksum += dataBuffer[size];
                        }


                        /* Write row to the external memory */
                        (void) EMI_WriteData(EMI_APP_ABS_ADDR(appSizeInRows), CY_FLASH_SIZEOF_ROW, dataBuffer);
                        appSizeInRows++;


                        DBG_PRINT_TEXT("\r\n");
                        DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                        DBG_PRINT_TEXT("\tProgram Row Command:\r\n");
                        DBG_PRINT_TEXT("\t\tEMI Address: 0x");
                        DBG_PRINT_HEX(EMI_APP_ABS_ADDR(appSizeInRows - 1u));
                        DBG_PRINT_TEXT("\r\n");

                        DBG_PRINT_TEXT("\t\tnumOfTxedRows = 0x");
                        DBG_PRINT_HEX(appSizeInRows - 1u);
                        DBG_PRINT_TEXT("\r\n");

                        ackCode = CYRET_SUCCESS;

                    }
                    else
                    {
                        ackCode = BootloaderEmulator_ERR_LENGTH;
                    }

                    dataOffset = 0u;
                }
                break;


            /***************************************************************************
            *   Sync bootloader
            ***************************************************************************/
            #if(0u != BootloaderEmulator_CMD_SYNC_BOOTLOADER_AVAIL)

            case BootloaderEmulator_COMMAND_SYNC:

                if(BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState)
                {
                    /* If something failed the host would send this command to reset the bootloader. */
                    dataOffset = 0u;

                    /* Don't acknowledge the packet, just get ready to accept the next one */
                    continue;
                }
                break;

            #endif  /* (0u != BootloaderEmulator_CMD_SYNC_BOOTLOADER_AVAIL) */


            /***************************************************************************
            *   Send data
            ***************************************************************************/
            #if (0u != BootloaderEmulator_CMD_SEND_DATA_AVAIL)

                case BootloaderEmulator_COMMAND_DATA:
                    DBG_PRINTF("Send data\r\n\r\n");
                    if(BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState)
                    {
                        /*  Make sure that dataOffset is valid before copying the data */
                        if((dataOffset + pktSize) <= BootloaderEmulator_SIZEOF_COMMAND_BUFFER)
                        {
                            ackCode = CYRET_SUCCESS;

                            #if(CY_PSOC3)
                                (void) memcpy(&dataBuffer[dataOffset],
                                              &packetBuffer[BootloaderEmulator_DATA_ADDR],
                                              ( int16 )pktSize);
                            #else
                                (void) memcpy(&dataBuffer[dataOffset],
                                              &packetBuffer[BootloaderEmulator_DATA_ADDR],
                                              (uint32) pktSize);
                            #endif  /* (CY_PSOC3) */

                            dataOffset += pktSize;
                        }
                        else
                        {
                            ackCode = BootloaderEmulator_ERR_LENGTH;
                        }
                    }

                    break;

            #endif  /* (0u != BootloaderEmulator_CMD_SEND_DATA_AVAIL) */


            /***************************************************************************
            *   Enter bootloader
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_ENTER:

                if(pktSize == 0u)
                {
                    uint32 i;

                    BootloaderEmulator_ENTER CYDATA BtldrVersion =
                        {CYDEV_CHIP_JTAG_ID, CYDEV_CHIP_REV_EXPECT, BootloaderEmulator_VERSION};

                    communicationState = BootloaderEmulator_COMMUNICATION_STATE_ACTIVE;

                    rspSize = sizeof(BootloaderEmulator_ENTER);

                    (void) memcpy(&packetBuffer[BootloaderEmulator_DATA_ADDR],
                                &BtldrVersion,
                                (uint32) rspSize);


                    /* Perform initializations */
                    appSizeInRows = 0u;
                    appExtMemChecksum = 0u;
                    for (i = 0u; i < CY_FLASH_SIZEOF_ROW; i++ )
                    {
                        metadata[i] = 0u;
                    }


                    DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                    DBG_PRINT_TEXT("\tEnter bootloader:\r\n");
                    DBG_PRINT_TEXT("\r\n");

                    #if (ENCRYPTION_ENABLED == YES)
                        /*Generate key*/
                        CR_GenerateKey(emiKey);
                        DBG_PRINT_TEXT("Generated Key: ");
                        DBG_PRINT_ARRAY(emiKey, KEY_LENGTH);
                        DBG_PRINT_TEXT("\r\n");
                        CR_WriteKey(emiKey);
                        CR_ReadKey(emiKey);
                        DBG_PRINT_TEXT("Read Key     : ");
                        DBG_PRINT_ARRAY(emiKey, KEY_LENGTH);
                        DBG_PRINT_TEXT("\r\n");
                        
                    #endif /*(ENCRYPTION_ENABLED == YES)*/
                    
                    ackCode = CYRET_SUCCESS;
                }
                break;


            /***************************************************************************
            *   Verify row
            ***************************************************************************/
            #if (0u != BootloaderEmulator_CMD_VERIFY_ROW_AVAIL)

            case BootloaderEmulator_COMMAND_VERIFY:

                if((BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState) && (pktSize == 3u))
                {
                    uint8 CYDATA checksum;

                    checksum = BootloaderEmulator_Calc8BitSum(appSizeInRows - 1u);

                    packetBuffer[BootloaderEmulator_DATA_ADDR] = (uint8)1u + (uint8)(~checksum);
                    ackCode = CYRET_SUCCESS;
                    rspSize = 1u;

                    DBG_PRINT_TEXT("\r\n");
                    DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                    DBG_PRINT_TEXT("\tVerify Row:\r\n");
                    DBG_PRINT_TEXT("\t\tRow Checksum: 0x");
                    DBG_PRINT_HEX(packetBuffer[BootloaderEmulator_DATA_ADDR]);
                    DBG_PRINT_TEXT("\r\n");
                    DBG_PRINT_TEXT("\r\n");
                }
                break;

            #endif /* (0u != BootloaderEmulator_CMD_VERIFY_ROW_AVAIL) */


            /***************************************************************************
            *   Exit bootloader
            ***************************************************************************/
            case BootloaderEmulator_COMMAND_EXIT:

                if(CYRET_SUCCESS == BootloaderEmulator_ValidateBootloadable())
                {
                    BootloaderEmulator_SET_RUN_TYPE(BootloaderEmulator_SCHEDULE_BTLDR);

                    DBG_PRINT_TEXT("\r\n");
                    DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                    DBG_PRINT_TEXT("\tExit Bootloader:\r\n");
                    DBG_PRINT_TEXT("\t\tBootloadable Application is valid.\r\n");
                    metadata[EMI_MD_APP_STATUS_ADDR] = EMI_MD_APP_STATUS_VALID;
                    metadata[EMI_MD_ENCRYPTION_STATUS_ADDR] = ENCRYPTION_ENABLED;
                }
                else
                {
                    DBG_PRINT_TEXT("\r\n");
                    DBG_PRINT_TEXT("BootloaderEmulator:\r\n");
                    DBG_PRINT_TEXT("\tExit Bootloader:\r\n");
                    DBG_PRINT_TEXT("\t\tBootloadable Application is invalid.\r\n");
                    metadata[EMI_MD_APP_STATUS_ADDR] = EMI_MD_APP_STATUS_INVALID;
                }

                /* Update metadata section and save it to the external memory */
                
                metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR      ]    = LO8(appSizeInRows);
                metadata[EMI_MD_APP_SIZE_IN_ROWS_ADDR + 1u ]    = HI8(appSizeInRows);
                metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR     ]    = LO8(appFirstRowNum);
                metadata[EMI_MD_APP_FIRST_ROW_NUM_ADDR + 1u]    = HI8(appFirstRowNum);
                appExtMemChecksum = ( uint16 )1u + ( uint16 )(~appExtMemChecksum);
                metadata[EMI_MD_APP_EM_CHECKSUM_ADDR     ]      = LO8(appExtMemChecksum);
                metadata[EMI_MD_APP_EM_CHECKSUM_ADDR + 1u]      = HI8(appExtMemChecksum);
                metadata[EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR] = EMI_EXTERNAL_MEMORY_PAGE_SIZE;


                (void) EMI_WriteData(EMI_MD_BASE_ADDR, CY_FLASH_SIZEOF_ROW, metadata);


                DBG_PRINT_TEXT("\t\tApplication Status: 0x");
                DBG_PRINT_HEX(metadata[EMI_MD_APP_STATUS_ADDR]);
                DBG_PRINT_TEXT("\r\n");

                DBG_PRINT_TEXT("\t\tSize Application in Rows: 0x");
                DBG_PRINT_HEX(appSizeInRows);
                DBG_PRINT_TEXT("\r\n");

                DBG_PRINT_TEXT("\t\tFirst Flash Row Number: 0x");
                DBG_PRINT_HEX(appFirstRowNum);
                DBG_PRINT_TEXT("\r\n");

                DBG_PRINT_TEXT("\t\tExternal Memory Page Size: 0x");
                DBG_PRINT_HEX(metadata[EMI_MD_EXTERNAL_MEMORY_PAGE_SIZE_ADDR]);
                DBG_PRINT_TEXT(" KB");
                DBG_PRINT_TEXT("\r\n");

                DBG_PRINT_TEXT("\r\n");
                DBG_PRINT_TEXT("\r\n");
                DBG_PRINT_TEXT("===============================================================================\r\n");
                DBG_PRINT_TEXT("=    BLE_External_Memory_Bootloadable Application Performs Software Reset     =\r\n");
                DBG_PRINT_TEXT("===============================================================================\r\n");
                DBG_PRINT_TEXT("\r\n");
                DBG_PRINT_TEXT("\r\n");

                CySoftwareReset();

                /* Will never get here */
                break;


            /***************************************************************************
            *   Unsupported command
            ***************************************************************************/
            default:
                ackCode = BootloaderEmulator_ERR_CMD;
                break;
            }
        }

        /* Reply with acknowledge or not acknowledge packet */
        (void) BootloaderEmulator_WritePacket(ackCode, packetBuffer, rspSize);

    } while ((0u == timeOut) || (BootloaderEmulator_COMMUNICATION_STATE_ACTIVE == communicationState));
}


/*******************************************************************************
* Function Name: BootloaderEmulator_WritePacket
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
static cystatus BootloaderEmulator_WritePacket(uint8 status, uint8 buffer[], uint16 size)
{
    uint16 CYDATA checksum;

    /* Start of packet. */
    buffer[BootloaderEmulator_SOP_ADDR]      = BootloaderEmulator_SOP;
    buffer[BootloaderEmulator_CMD_ADDR]      = status;
    buffer[BootloaderEmulator_SIZE_ADDR]     = LO8(size);
    buffer[BootloaderEmulator_SIZE_ADDR + 1u] = HI8(size);

    /* Compute checksum. */
    checksum = BootloaderEmulator_CalcPacketChecksum(buffer, size + BootloaderEmulator_DATA_ADDR);

    buffer[BootloaderEmulator_CHK_ADDR(size)]     = LO8(checksum);
    buffer[BootloaderEmulator_CHK_ADDR(1u + size)] = HI8(checksum);
    buffer[BootloaderEmulator_EOP_ADDR(size)]     = BootloaderEmulator_EOP;

    /* Start packet transmit. */
    return(CyBtldrCommWrite(buffer, size + BootloaderEmulator_MIN_PKT_SIZE, &size, 150u));
}


/* [] END OF FILE */
