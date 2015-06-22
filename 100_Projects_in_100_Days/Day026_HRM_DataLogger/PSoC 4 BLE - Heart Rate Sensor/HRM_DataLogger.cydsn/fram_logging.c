/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include "common.h"
#include "fram_logging.h"

/* Attribute CY_NOINIT puts SRAM variable in memory section which is retained in low power modes */
uint32 addr;

uint8 WriteFRAMData(uint16 addr_buf, uint8* write_buf, uint16 len)
{
	uint8 err;
	uint8 temp_buf[len+ADDR_SIZE];
	
	temp_buf[0] = (addr_buf & 0x000000ff);
	temp_buf[1] = ((addr_buf >> 8) & 0x000000ff);
    temp_buf[2] = ((addr_buf >> 16) & 0x000000ff);
    temp_buf[3] = ((addr_buf >> 24) & 0x000000ff);
    
	memcpy(&temp_buf[4], write_buf, len);
	
	err = I2C_M_I2CMasterWriteBuf(FRAM_SLAVE_ADDR, temp_buf, len+ADDR_SIZE, I2C_M_I2C_MODE_COMPLETE_XFER);	
	
	while(0u == (I2C_M_I2CMasterStatus() & I2C_M_I2C_MSTAT_WR_CMPLT))
	{
	/* Wait until master complete write */
	}
	/* Clear I2C master status */
	(void) I2C_M_I2CMasterClearStatus();
	
	return(err);
}

uint8 ReadFRAMData(uint16 addr_buf, uint8* read_buf, uint16 len)
{
	uint8 err;

    
	err = I2C_M_I2CMasterWriteBuf(FRAM_SLAVE_ADDR, (uint8 *)&addr_buf, ADDR_SIZE, I2C_M_I2C_MODE_COMPLETE_XFER);	
	
	while(0u == (I2C_M_I2CMasterStatus() & I2C_M_I2C_MSTAT_WR_CMPLT))
	{
	/* Wait until master complete write */
	}
	/* Clear I2C master status */
	(void) I2C_M_I2CMasterClearStatus();
	
	err = I2C_M_I2CMasterReadBuf(FRAM_SLAVE_ADDR, &read_buf[0], len, I2C_M_I2C_MODE_COMPLETE_XFER);
				
	while(0u == (I2C_M_I2CMasterStatus() & I2C_M_I2C_MSTAT_RD_CMPLT))
	{
	/* Wait until master complete reading */
	}
	
	/* Clear I2C master status */
	(void) I2C_M_I2CMasterClearStatus();
	
	return(err);
}

void DataLogging(uint8 len, uint8* data_buf)
{
    uint8 read_buf[20];
    uint8 temp;
    uint16 temp1;
    
    if((addr + len) < FRAM_SIZE)
    {
        addr = addr + len;
        WriteFRAMData(addr, data_buf, len);
        printf("Data logged successfully \r\n");
	    ReadFRAMData(addr, read_buf, len);
	    printf("Data read back: ");
        for(temp=0; temp<len; temp++)
        {
	        PrintHex(read_buf[temp]);
	        UART_UartPutString(" ");
        }
        UART_UartPutString("\r\n");
    }
    else
    {
        temp = (uint8)UART_UartGetChar();
        if(temp == 'r' || temp == 'R')
        {
            printf("Address reset\r\n");
            addr = 0;
        }
        else
        {
            printf("Memory full. Press r or R to reset the memory.\r\n");
        }
    }
}

/* [] END OF FILE */
