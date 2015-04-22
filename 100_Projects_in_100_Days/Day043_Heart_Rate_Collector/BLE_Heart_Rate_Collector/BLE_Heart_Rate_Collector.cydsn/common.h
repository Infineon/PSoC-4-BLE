/*******************************************************************************
* File Name: common.h
*
* Version: 1.0
*
* Description:
*  Common BLE application header.
*
*******************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include <stdio.h>

#if(CYBLE_GATT_ROLE_SERVER)
#include "server.h"
#endif /* (CYBLE_GATT_ROLE_SERVER) */
#if(CYBLE_GATT_ROLE_CLIENT)
#include "client.h"
#endif /* (CYBLE_GATT_ROLE_CLIENT) */

#define DEBUG_OUT

#define ENABLED                     (1u)
#define DISABLED                    (0u)

#define LED_ON                      (0u)
#define LED_OFF                     (1u)

#define LED_TIMEOUT                 (1000u/300u)    /* Counts depend on advertising interval parameter */


/***************************************
*        External Function Prototypes
***************************************/
int _write(int file, char *ptr, int len);
void DebugOut(uint32 event, void* eventParam);


/***************************************
* External data references
***************************************/
extern CYBLE_API_RESULT_T apiResult;


/* [] END OF FILE */
