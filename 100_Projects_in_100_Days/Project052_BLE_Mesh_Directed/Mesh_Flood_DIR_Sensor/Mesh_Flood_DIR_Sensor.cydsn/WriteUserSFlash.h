/*******************************************************************************
* File Name: WriteUserSFlash.h
*
* Version: 1.0
*
* Description:
*  This file contains the headers and constants for user SFlash write API.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined (WRITE_USER_SFLASH_H)
#define WRITE_USER_SFLASH_H

#include <project.h>
   
/*****************************************************
*                  Enums and macros
*****************************************************/  
#define SWITCH_PRESSED                  (0u)   /* Active low user switch on BLE Pioneer kit */ 
#define USER_SFLASH_ROW_SIZE            (128u) /* SFlash row size for 128KB flash BLE device. For other PSoC 4 BLE devices 
                                                * with higher flash size, this example project might need some modification.
                                                * Please check the device datasheet and TRM before using this code on non 128KB
                                                * flash devices */
#define SFLASH_STARTING_VALUE           (0x00) /* Starting value to be stored in user SFlash to demonstrate SFlash write API */
#define USER_SFLASH_ROWS                (4u)   /* Total number of user SFlash rows supported by the device */
#define USER_SFLASH_BASE_ADDRESS        (0x0FFFF200u) /* Starting address of user SFlash row for 128KB PSoC 4 BLE device */    

#define SFLASH_NODE_ADDRESS_INDEX		(0x00)
#define NODE_ADDRESS_SFLASH_ROW			(0x01)
#define NODE_ADDRESS_MSB_INDEX			(0x01)
#define NODE_ADDRESS_LSB_INDEX			(0x00)
	
#define LOAD_FLASH				         0x80000004
#define WRITE_USER_SFLASH_ROW	         0x80000018
#define USER_SFLASH_WRITE_SUCCESSFUL     0xA0000000     

/*****************************************************
*                  Function declarations
*****************************************************/    
uint32 WriteUserSFlashRow(uint8 userRowNUmber, uint32 *dataPointer);   
void UpdateSFlashNodeAddress(uint16 node_addr);

#endif /* End of #ifndef WRITE_USER_SFLASH_H */

/* [] END OF FILE */
