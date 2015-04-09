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

extern uint8 sendNotifications;
extern CYBLE_CONN_HANDLE_T ConnHandle;    
    
extern void AppCallBack(uint32 , void *);  
extern void SendNotification(uint8 *, uint8);
    
#endif /* _APP_BLE_H_ */


/* [] END OF FILE */
