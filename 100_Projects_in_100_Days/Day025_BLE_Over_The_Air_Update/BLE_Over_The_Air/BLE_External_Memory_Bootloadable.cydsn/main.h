/*******************************************************************************
* File Name: main.h
*
* Version: 1.0
*
* Description:
*  Simple BLE example project that demonstrates how to configure and use
*  Cypress's BLE component APIs and application layer callback. Device
*  Information service is used as an example to demonstrate configuring
*  BLE service characteristics in the BLE component.
*
* References:
*  BLUETOOTH SPECIFICATION Version 4.1
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include <project.h>
#include "Options.h"
#include <stdio.h>
#include "debug.h"
#include "BootloaderEmulator_PVT.h"
#include "Encryption.h"

#define LED_GREEN                           (0u)
#define LED_BLUE                            (1u)
#define LED_ADV_COLOR                       (LED_Green)

#if (LED_ADV_COLOR == LED_GREEN)
    #define LED_WRITE_MACRO(a)              (Advertising_LED_1_Write(a))
#else
    #define LED_WRITE_MACRO(a)              (Advertising_LED_2_Write(a))
#endif /* (LED_ADV_COLOR == LED_GREEN) */

#define LED_TIMEOUT                         (1000u)   /* Сounts of 1 millisecond */
#define LED_ON                              (0u)
#define LED_OFF                             (1u)

#define BLE_PACKET_SIZE_MAX                 (144u)

/*WDT setup values*/
#define WDT_COUNTER                         (CY_SYS_WDT_COUNTER1)
#define WDT_COUNTER_MASK                    (CY_SYS_WDT_COUNTER1_MASK)
#define WDT_INTERRUPT_SOURCE                (CY_SYS_WDT_COUNTER1_INT) 
#define WDT_COUNTER_ENABLE                  (1u)
#define WDT_TIMEOUT                         (32767u/1000u) /* 1 ms @ 32.768kHz clock */

#define SERIAL_NUMBER                       ("123456")

extern void generateKey(uint8 * key);
void AppCallBack(uint32 event, void* eventParam);
void HandleLeds(void);
void WriteAttrServChanged(void);

/* [] END OF FILE */
