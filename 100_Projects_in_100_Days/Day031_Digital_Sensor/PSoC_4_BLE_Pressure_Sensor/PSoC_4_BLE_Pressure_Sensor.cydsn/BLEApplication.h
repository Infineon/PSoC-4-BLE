/******************************************************************************
* Project Name		: PSoC_4_BLE_Pressure_Sensor
* File Name			: BLEApplication.h
* Version 			: 1.10
* Device Used		: CY8C4248LQI-BL583
* Software Used		: PSoC Creator 4.2
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit
*******************************************************************************/

/***********************************************************************************
* Copyright (2015-2018), Cypress Semiconductor Corporation. All Rights Reserved.
************************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
************************************************************************************/

#if !defined(BLEAPPLICATION)
#define BLEAPPLICATION

#include <common.h>

/*************************Macro Definitions**********************************/
#define CONN_PARAM_UPDATE_MIN_CONN_INTERVAL 0x64 //Minimum connection interval
#define CONN_PARAM_UPDATE_MAX_CONN_INTERVAL 0x6E //Maximum connection interval
#define CONN_PARAM_UPDATE_SLAVE_LATENCY 0x00 //Slave latency
#define CONN_PARAM_UPDATE_SUPRV_TIMEOUT 0xC8 //Supervision timeout
/*****************************************************************************/

/*************************Function Prototypes*******************************/
void CustomEventHandler(uint32 Event, void * EventParameter);
void SendTemperatureOverNotification(void);
void SendPressureOverNotification(void);
void SendAltitudeOverNotification(void);
void UpdateNotificationCCCDAttribute(void);
void UpdateConnectionParameters(void);
/*****************************************************************************/
#endif
/* [] END OF FILE */
