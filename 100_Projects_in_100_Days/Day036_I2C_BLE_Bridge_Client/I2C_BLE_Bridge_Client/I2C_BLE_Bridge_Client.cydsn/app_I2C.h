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

#define I2C_READ_BUFFER_SIZE	61			/* Max supported by BCP */

#define I2C_WRITE_BUFFER_SIZE	61			/* Max supported by BCP */
	
// #define RESET_I2C_READ_DATA	

// #define ENABLE_I2C_ONLY_WHEN_CONNECTED		
	
extern uint8 wrBuf[I2C_WRITE_BUFFER_SIZE]; /* I2C write buffer */

extern uint8 rdBuf[I2C_READ_BUFFER_SIZE];  /* I2C read buffer */

extern uint8 indexCntr;

extern uint32 byteCnt;	
    
void HandleI2CWrite(void);

void HandleI2CTraffic(void);

void HandleI2CNotifications(CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *);
   
#endif /* _APP_I2C_H_ */
/* [] END OF FILE */
