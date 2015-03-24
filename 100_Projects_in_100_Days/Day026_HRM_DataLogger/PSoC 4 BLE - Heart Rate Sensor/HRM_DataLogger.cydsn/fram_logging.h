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
//roit: FRAM
#if !defined(FRAM_LOGGING_H)
#define FRAM_LOGGING_H

#define FRAM_SLAVE_ADDR			 	 0x50
#define TIMER_VALUE				   450000
#define ADDR_SIZE						4
#define FRAM_SIZE                  131072
	
uint8 WriteFRAMData(uint16, uint8*, uint16);
uint8 ReadFRAMData(uint16, uint8*, uint16);
void DataLogging(uint8 len, uint8* data_buf);
	
#endif
/* [] END OF FILE */
