/******************************************************************************
* Project Name		: PSoC4_BLE_RTC
* File Name			: RTC.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: kris@cypress.com
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
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
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************
******************************************************************************
*                           Description
*******************************************************************************
*  Header file for RTC implementaiton on PSoC 4 BLE
*
*******************************************************************************/

#if !defined(RTC_H)
#define RTC_H
    
#include <project.h>    

/***************************************
*    Macros for RTC & CTS BLE service
***************************************/
#define CYBLE_INIT                                  (0u)      /* Initialized state */
#define CYBLE_UNCONFIG                              (1u)      /* Unconfigured state */
#define CYBLE_CONFIG                                (2u)      /* Configured state */    
    
#define BLE_CTS_CHARACTERISTIC_VALUE                (0u)
#define BLE_CTS_CHARACTERISTIC_DESCRIPTOR           (1u)
    
/* Leap Year status bit */
#define RTC_STATUS_LY                               (0x02u)

/* AM/PM status bit */
#define RTC_STATUS_AM_PM                            (0x04u)
    
/* Time elapse constants */
#define RTC_MINUTE_ELAPSED                          (59u)
#define RTC_HOUR_ELAPSED                            (59u)
#define RTC_HALF_OF_DAY_ELAPSED                     (12u)
#define RTC_DAY_ELAPSED                             (23u)
#define RTC_WEEK_ELAPSED                            (7u)
#define RTC_YEAR_ELAPSED                            (12u)
#define RTC_DAYS_IN_WEEK                            (7u)
#define RTC_YEAR_LOW_MAX                            (255u)
    
/* Days Of Week definition */
#define RTC_SUNDAY                                  (1u)
#define RTC_MONDAY                                  (2u)
#define RTC_TUESDAY                                 (3u)
#define RTC_WEDNESDAY                               (4u)
#define RTC_THURDAY                                 (5u)
#define RTC_FRIDAY                                  (6u)
#define RTC_SATURDAY                                (7u)
    
/* Month definition */
#define RTC_JANUARY                                 (1u)
#define RTC_DAYS_IN_JANUARY                         (31u)
#define RTC_FEBRUARY                                (2u)
#define RTC_DAYS_IN_FEBRUARY                        (28u)
#define RTC_MARCH                                   (3u)
#define RTC_DAYS_IN_MARCH                           (31u)
#define RTC_APRIL                                   (4u)
#define RTC_DAYS_IN_APRIL                           (30u)
#define RTC_MAY                                     (5u)
#define RTC_DAYS_IN_MAY                             (31u)
#define RTC_JUNE                                    (6u)
#define RTC_DAYS_IN_JUNE                            (30u)
#define RTC_JULY                                    (7u)
#define RTC_DAYS_IN_JULY                            (31u)
#define RTC_AUGUST                                  (8u)
#define RTC_DAYS_IN_AUGUST                          (31u)
#define RTC_SEPTEMBER                               (9u)
#define RTC_DAYS_IN_SEPTEMBER                       (30u)
#define RTC_OCTOBER                                 (10u)
#define RTC_DAYS_IN_OCTOBER                         (31u)
#define RTC_NOVEMBER                                (11u)
#define RTC_DAYS_IN_NOVEMBER                        (30u)
#define RTC_DECEMBER                                (12u)
#define RTC_DAYS_IN_DECEMBER                        (31u)    
    
#define RTC_MONTHS_IN_YEAR                          (12u)
#define RTC_DAYS_IN_YEAR                            (365u)
#define RTC_DAYS_IN_LEAP_YEAR                       (366u)
    
#define RTC_SOURCE_COUNTER                          (0u)
#define RTC_COUNTER_ENABLE                          (1u)
#define RTC_COUNT_PERIOD                            ((uint32)32767)
#define RTC_INTERRUPT_SOURCE                        CY_SYS_WDT_COUNTER0_INT    
    
/* Returns 1 if leap year, otherwise 0 */
#define RTC_LEAP_YEAR(year) \
                    (((((year) % 400u) == 0u) || ((((year) % 4u) == 0u) && \
                    (((year) % 100u) != 0u))) ? 0x01u : 0x00u)
                   
extern CYBLE_CTS_CURRENT_TIME_T currentTime;

/***************************************
*    Function declarations
***************************************/ 
void RTC_Start(void);
CYBLE_API_RESULT_T StartTimeServiceDiscovery(void); 
CYBLE_API_RESULT_T SyncTimeFromBleTimeServer(void);
void CtsCallBack(uint32 event, void* eventParam);
uint8 RTC_TickExpired(void);
void RTC_UI_Update(void);
void WDT_Handler(void);
    
#endif /* End of #if !defined(RTC_H) */

/* [] END OF FILE */
