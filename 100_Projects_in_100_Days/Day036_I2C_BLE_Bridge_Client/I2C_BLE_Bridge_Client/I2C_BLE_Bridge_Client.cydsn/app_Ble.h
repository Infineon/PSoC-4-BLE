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

#ifndef _APP_BLE_H_
#define _APP_BLE_H_   
	
#include "main.h"	
    
    
/***************************************
*   Enumerated Types and Structs
***************************************/
typedef enum 
{
    INFO_EXCHANGE_START             	= 0x00,
    
	I2C_READ_SERVICE_HANDLE_FOUND   	= 0x01,
    
	I2C_READ_DATA_ATTR_HANDLE_FOUND		= 0x02,		
    
	I2C_READ_DATA_CCCD_HANDLE_FOUND     = 0x04,		
	
	I2C_WRITE_SERVICE_HANDLE_FOUND  	= 0x08,
    
	I2C_WRITE_DATA_ATTR_HANDLE_FOUND 	= 0x10,
    
	MTU_XCHNG_COMPLETE             		= 0x20,
    
    SERVICE_AND_CHAR_HANDLES_FOUND  = I2C_READ_SERVICE_HANDLE_FOUND  | \
                                      I2C_WRITE_SERVICE_HANDLE_FOUND | \
									  I2C_READ_DATA_ATTR_HANDLE_FOUND     | \
	  									I2C_WRITE_DATA_ATTR_HANDLE_FOUND,
    
    ALL_HANDLES_FOUND               = SERVICE_AND_CHAR_HANDLES_FOUND | \
                                            I2C_READ_DATA_CCCD_HANDLE_FOUND,
    
    INFO_EXCHANGE_COMPLETE          = ALL_HANDLES_FOUND | \
                                            MTU_XCHNG_COMPLETE,
} INFO_EXCHANGE_STATE_T;
    
	
	
void HandleBleProcessing(void);

void AppCallBack(uint32 , void *);  

void SendNotification(uint8 *, uint8);

void attrHandleInit(void);

void enableNotifications(void);
    
#endif /* _APP_BLE_H_ */

