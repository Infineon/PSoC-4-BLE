/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates how to read and write user SFlash locations from
*  firmware.
*
* Owner:
*  kris@cypress.com
*
* Hardware Dependency:
*  1. PSoC 4 BLE device
*
* Code Tested With:
*  1. PSoC Creator 3.1 SP1
*  2. ARM GCC 4.8.4
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include <WriteUserSFlash.h>

/***************************************
*    Function declarations
***************************************/
char HexToAscii(uint8 value, uint8 nibble);

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        System entry point. This calls the user SFlash write API when switch SW2
* is pressed during system bootup.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main()
{
    uint32 data[USER_SFLASH_ROW_SIZE/4];
    uint8 *sflashPtr;
    uint8 rowIndex;
    uint32 dataIndex;
            
    CyGlobalIntEnable; /* Enable system interrupts required for UART operation */
    
    UART_Console_Start(); /* UART is used for UI on the serial port terminal */
    
    if(Write_Switch_Read() == SWITCH_PRESSED) /* If SW2 on BLE pioneer kit baseboard is pressed during startup */
    {
        for(dataIndex = 0; dataIndex < (USER_SFLASH_ROW_SIZE/4); dataIndex++)
        {
            data[dataIndex] = SFLASH_STARTING_VALUE + dataIndex;  /* Fill the user SFlash write buffer with known data */
        }
        
        for(rowIndex = 0; rowIndex < USER_SFLASH_ROWS; rowIndex++) /* Continuously write all the 4 user FLASH rows */
        {
            uint32 status;
            
            UART_Console_UartPutString("\r\nWriting user SFlash row");
            UART_Console_UartPutChar('0' + rowIndex);
            
            /* User SFlash write API will change the IMO frequency to 48MHz internally (which is not desired for any
             * peripherals that are operating on IMO based clock (UART in this example). Wait for UART data transfer
             * to complete before calling the user SFlash write API */
            while((UART_Console_SpiUartGetTxBufferSize() + UART_Console_GET_TX_FIFO_SR_VALID) != 0u);
            
            status  = WriteUserSFlashRow(rowIndex, &data[0]);
            
            if(status == USER_SFLASH_WRITE_SUCCESSFUL)
            {
                UART_Console_UartPutString(" successful");
            }
            else
            {
                UART_Console_UartPutString(" failed - ");
                UART_Console_UartPutChar(HexToAscii(HI8(HI16(status)),1));
                UART_Console_UartPutChar(HexToAscii(HI8(HI16(status)),0));
                UART_Console_UartPutChar(HexToAscii(LO8(HI16(status)),1));
                UART_Console_UartPutChar(HexToAscii(LO8(HI16(status)),0));
            }
        }
        
        UART_Console_UartPutString("\r\nUser SFlash write complete\r\n");
    }
    
    sflashPtr = (uint8 *)USER_SFLASH_BASE_ADDRESS; /* User SFlash read is direct memory read using pointers */

    /* Read all the 512 bytes of user configurable SFlash content and display on UART console */
    for(rowIndex = 0; rowIndex < USER_SFLASH_ROWS; rowIndex++)
    {
        UART_Console_UartPutString("\r\n\nUser SFlash row ");
        UART_Console_UartPutChar('0' + rowIndex);
        UART_Console_UartPutString(" data:\r\n");
        
        for(dataIndex = 0; dataIndex < USER_SFLASH_ROW_SIZE; dataIndex++)
        {
            uint8 readData;
            
            readData = *sflashPtr++;
            
            UART_Console_UartPutChar(HexToAscii(readData,1));
            UART_Console_UartPutChar(HexToAscii(readData,0));
            UART_Console_UartPutChar(' ');
        }
    }

    while(1); /* halt the system */
}

/*******************************************************************************
* Function Name: HexToAscii
********************************************************************************
* Summary:
*        Converts either the higher or lower nibble of a hex byte to its
* corresponding ASCII.
*
* Parameters:
*  value - hex value to be converted to ASCII
*  nibble - 0 = lower nibble, 1 = higher nibble
*
* Return:
*  char - hex value for the value/nibble specified in the parameters
*
*******************************************************************************/
char HexToAscii(uint8 value, uint8 nibble)
{
    if(nibble == 1)
    {
        /* bit-shift the result to the right by four bits */
        value = value & 0xF0;
        value = value >> 4;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else if (nibble == 0)
    {
        /* extract the lower nibble */
        value = value & 0x0F;
        
        if (value >9)
        {
            value = value - 10 + 'A'; /* convert to ASCII character */
        }
        else
        {
            value = value + '0'; /* convert to ASCII number */
        }
    }
    else
    {
        value = ' ';  /* return space for invalid inputs */
    }
    
    return value;
}

/* [] END OF FILE */
