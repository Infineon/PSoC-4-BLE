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

#ifndef _APP_I2C_H_
#define _APP_I2C_H_	

#include "main.h"      

#define SPI_READ_BUFFER_SIZE	20 
#define SPI_WRITE_BUFFER_SIZE	20 
	
// #define ENABLE_SPI_ONLY_WHEN_CONNECTED	
	
extern uint8 wrBuf[SPI_WRITE_BUFFER_SIZE]; /* SPI write buffer */
extern uint8 rdBuf[SPI_READ_BUFFER_SIZE];  /* SPI read buffer */
extern uint32 byteCnt;	

extern uint8 sendNotifications;   

extern void sendSPINotification(void);
extern void handleSPITraffic(void);

#endif /* _APP_I2C_H_ */

/* [] END OF FILE */
