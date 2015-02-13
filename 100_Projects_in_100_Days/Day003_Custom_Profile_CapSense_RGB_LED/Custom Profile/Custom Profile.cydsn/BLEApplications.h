/*****************************************************************************
* File Name: BleApplications.h
*
* Version: 1.0
*
* Description:
* This file defines the macros and function prototypes for BLE functionality.
*
* Hardware Dependency:
* CY8CKIT-042 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/

#if !defined(_BLE_APPLICATIONS_H)
#define _BLE_APPLICATIONS_H

/*****************************************************************************
* Included headers
*****************************************************************************/
#include <project.h>


/*****************************************************************************
* Macros 
*****************************************************************************/
#define CAPSENSE_SERVICE_INDEX          (0x00)
#define RGB_LED_SERVICE_INDEX           (0x01)

#define CAPSENSE_SLIDER_CHAR_INDEX      (0x00)
#define RGB_LED_CHAR_INDEX              (0x00)
#define CAPSENSE_SLIDER_CHAR_HANDLE		(0x000E)
#define RGB_LED_CHAR_HANDLE				(0x0013)

#define CAPSENSE_CCC_HANDLE				(0x000F)

#define CCC_DATA_LEN					(2)
#define CAPSENSE_CHAR_DATA_LEN			(1)
#define RGB_CHAR_DATA_LEN				(4)

#define CAPSENSE_SLIDER_CCC_INDEX		(0u)
#define CCC_DATA_INDEX					(0u)

#define BLE_STATE_ADVERTISING			(0x01)
#define BLE_STATE_CONNECTED				(0x02)
#define BLE_STATE_DISCONNECTED			(0x00)

#define PASSIVE_LED_STATUS				(0xFF)

#define LED_ADV_BLINK_PERIOD			(40000)
#define LED_CONN_ON_PERIOD				(145000)

#define MTU_XCHANGE_DATA_LEN			(0x0020)


/*****************************************************************************
* Extern variables
*****************************************************************************/
extern uint8 deviceConnected;
extern uint8 sendCapSenseSliderNotifications;


/*****************************************************************************
* Public functions
*****************************************************************************/
void CustomEventHandler(uint32 event, void * eventParam);
void UpdateNotificationCCCD(void);
void UpdateRGBled(void);
void SendCapSenseNotification(uint8 CapSenseSliderData);


#endif  /* #if !defined(_BLE_APPLICATIONS_H) */

/* [] END OF FILE */
