/*******************************************************************************
* File Name: common.h
*
* Version 1.0
*
* Description:
*  Common BLE application header.
*
*******************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
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

#define ADC_BATTERY_CHANNEL         (0x00u)
#define ADC_TEMPERATURE_CHANNEL     (0x01u)
#define ADC_DEF_TEMP_REF            (1024u)

#define LED_TIMEOUT                 (1000u/300u)    /* Сounts depend on advertising interval parameter */

#define WDT_COUNTER                 (CY_SYS_WDT_COUNTER1)
#define WDT_COUNTER_MASK            (CY_SYS_WDT_COUNTER1_MASK)
#define WDT_INTERRUPT_SOURCE        (CY_SYS_WDT_COUNTER1_INT) 
#define WDT_COUNTER_ENABLE          (1u)
#define WDT_1SEC                    (32767u)


/***************************************
*        External Function Prototypes
***************************************/
int _write(int file, char *ptr, int len);
void DebugOut(uint32 event, void* eventParam);
void PrintHex(uint8 num);

/***************************************
* External data references
***************************************/
extern CYBLE_API_RESULT_T apiResult;
extern volatile uint32 mainTimer;


/* [] END OF FILE */
