/*****************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
* This is the main file for the application.
*
* Note:
* 
* Owner:
* UDYG
*
* Related Document:
* PSoC 4 BLE Heart Rate Monitor Solution Demo IROS: 001-92353
*
* Hardware Dependency:
* PSoC 4 BLE HRM Solution Demo Board
*
* Code Tested With:
* 1. PSoC Creator 3.1
* 2. ARM-GCC Compiler
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


/*****************************************************************************
* Included headers
*****************************************************************************/
#include "CompileTimeOptions.h"
#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h"
#include "CyPm.h"
#include <stdbool.h>

#if (BLE_EN)
    #include "BleProcessing.h"
    #include "BLE.h"
    #include "BLE_hrs.h"
    #include "BLE_eventHandler.h"
    #include "BLE_bas.h"
#endif  /* #if (BLE_EN) */

#if (BATTERY_EN)
    #include "BatteryLevel.h"
#endif  /* #if (BATTERY_EN) */

#if (HRM_EN)
    #include "HeartRateDetection.h"
    #include "LedInfrared1.h"
    #include "LedInfrared2.h"
#endif  /* #if (HRM_EN) */

#if (HRM_EN || BATTERY_EN)
    #include "ADC.h"
    #include "ADC_IRQ.h"
#endif  /* #if (HRM_EN || BATTERY_EN) */

#if (FIR_FILTER_EN)
    #include "filter.h"
#endif  /* #if (FIR_FILTER_EN) */

#if (UART_DEBUG_EN)
    #include "UART.h"
#endif  /* #if (UART_DEBUG_EN) */

#if (STATUS_LED_EN)
    #include "LEDStatus.h"
#endif  /* #if (STATUS_LED_EN) */

#include "MyIsr.h"
#include "WatchdogTimer.h"


/*****************************************************************************
* Macros and constants
*****************************************************************************/
#define WDT_PERIOD_MS                       (5)
#define BAS_SERVICE_INDEX                   (0)


/*****************************************************************************
* Global variables
*****************************************************************************/
bool stopDeviceFlag = false;


/*****************************************************************************
* Function Definitions
*****************************************************************************/

/*****************************************************************************
* Function Name: OpAmpTrim()
******************************************************************************
* Summary:
* Trims the offset value of the Differential Amplifier opamp.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* This function implements a binary search method to find the appropriate
* value for the offset trim register of the Differential Amplifier opamp.
* This is needed in case the silicon is not factory-trimmed, and the routine
* can be disabled from CompileTimeOptions.h. 
* There are two ranges for the trim values - 0 to 31, and 32 to 63. 
* A binary search is performed in the first range and if that does not give
* us the right value then another binary search is performed in the second
* range. The ranges were separated after an observation that the opamp trim 
* could work in both ways - increasing the trim value could tilt offset
* towards more positive value on some silicons, while tilting it towards more 
* negative value on other silicons. For positive tilt, the trim value lies
* between 32 and 63, while for negative tilt it lies between 0 and 31.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
#if (OPAMP_TRIM_EN)
static void OpAmpTrim(void)
{
    uint16 adcOut = 0;
    uint8 lowerTrimLimit = 0;
    uint8 upperTrimLimit = 31;
    uint8 nextTrimValue = 0;
    bool tuneFlag = 0;
    
    /* Binary search for the trim value */
    while(1)
    {
        #if (0)
            UART_PutChar(0x0D);
            UART_PutChar(0x0A);
            UART_PutChar(adcOut >> 8);
            UART_PutChar(adcOut);
            UART_PutChar(CY_GET_REG32(CYREG_CTBM1_OA0_OFFSET_TRIM));
            UART_PutChar(lowerTrimLimit);
            UART_PutChar(upperTrimLimit);
            UART_PutChar(0x0F);
        #endif
        
        nextTrimValue = (lowerTrimLimit + upperTrimLimit) >> 1;
        CY_SET_REG32(CYREG_CTBM1_OA0_OFFSET_TRIM, nextTrimValue);
        CyDelay(250);
        
        canMeasureHeartRate = false;
        ADC_StartConvert();
        while(true != canMeasureHeartRate);
        adcOut = ADC_GetResult16(0);
        
        if(tuneFlag == 0)
        {
            if(adcOut < 15000)
            {
                upperTrimLimit = nextTrimValue;
            }
            else if(adcOut > 17000)
            {
                lowerTrimLimit = nextTrimValue;
            }
            else
            {
                break;
            }
        }
        else
        {
            if(adcOut < 15000)
            {
                lowerTrimLimit = nextTrimValue;
            }
            else if(adcOut > 17000)
            {
                upperTrimLimit = nextTrimValue;
            }
            else
            {
                break;
            }
        }
        
        if((upperTrimLimit - lowerTrimLimit) <= 1)
        {
            if(tuneFlag == 0) 
            {
                tuneFlag = 1;
                lowerTrimLimit = 32;
                upperTrimLimit = 63;
            }
            else
            {
                break;
            }
        }
    }
    
    
    #if (0)
        while(1)
        {
            UART_PutChar(0x0D);
            UART_PutChar(0x0A);
            UART_PutChar(adcOut >> 8);
            UART_PutChar(adcOut);
            UART_PutChar(CY_GET_REG32(CYREG_CTBM1_OA0_OFFSET_TRIM));
            UART_PutChar(lowerTrimLimit);
            UART_PutChar(upperTrimLimit);
            UART_PutChar(0x0F);
        }
    #endif
}
#endif  /* #if (OPAMP_TRIM_EN) */


/*****************************************************************************
* Function Name: InitializeAllBlocks()
******************************************************************************
* Summary:
* Initializes all the blocks of the firmware.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* This function initializes the different blocks of the firmware.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
static void InitializeAllBlocks(void)
{
    uint32 loopCounter;
    
#if (HRM_EN || BATTERY_EN)
    /* Start ADC */
    ADC_Start();
    ADC_IRQ_StartEx(MyAdcIsr);
#endif  /* #if (HRM_EN || BATTERY_EN) */

#if (HRM_EN)
    /* Start heart rate measurement */
    HeartRate_Start();
    
    #if (OPAMP_TRIM_EN)
        /* Trim the differential amplifier opamp */
        OpAmpTrim();
    #endif  /* #if (OPAMP_TRIM_EN) */

    /* Initialize FIR filter */
    #if (FIR_FILTER_EN)
        for(loopCounter = 0; loopCounter < FIR_TAPS; loopCounter++)
        {
            canMeasureHeartRate = false;
            ADC_StartConvert();
            while(true != canMeasureHeartRate);
            FirFilter(ADC_GetResult16(0));
        }
    #endif  /* #if (FIR_FILTER_EN) */
    
#endif  /* #if (HRM_EN) */

#if (BLE_EN)
    CyBle_Start(BleProcessing_GeneralCallBack);
    CyBle_HrsRegisterAttrCallback(BleProcessing_HeartRateCallback);
    CyBle_BasRegisterAttrCallback(BleProcessing_BatteryCallback);
#endif  /* #if (BLE_EN) */

    /* Watchdog timer */
    WatchdogTimer_Start(WDT_PERIOD_MS);

#if (STATUS_LED_EN)
    /* Turn on status LED */
    LedStatus_Write(false);
#endif  /* #if (STATUS_LED_EN) */

}


/*****************************************************************************
* Function Name: GoToDeepSleep()
******************************************************************************
* Summary:
* Does the pre-processing before and then enters deep sleep.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Stops the heart rate measurement block. Then enters silicon deep sleep.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
#if (DEEP_SLEEP_EN)
    static void GoToDeepSleep(void)
    {
        uint32 timestamp;
        uint8 interruptStatus;
        
    #if (BLE_EN)
        CYBLE_LP_MODE_T bleMode;
    #endif  /* #if (BLE_EN) */
    
    #if (HRM_EN || BATTERY_EN)
        ADC_Sleep();
    #endif  /* #if (HRM_EN || BATTERY_EN) */

    #if (BLE_EN)
        timestamp = WatchdogTimer_GetTimestamp();
        
        /* Stay in lowest power mode possible until next WDT interrupt. */
        while(timestamp == WatchdogTimer_GetTimestamp())
        {
            CyBle_ProcessEvents();
            bleMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
            interruptStatus = CyEnterCriticalSection();
            if(CYBLE_BLESS_DEEPSLEEP == bleMode)
            {
                if((CyBle_GetBleSsState() == CYBLE_BLESS_STATE_ECO_ON) ||
                   (CyBle_GetBleSsState() == CYBLE_BLESS_STATE_DEEPSLEEP))
                {
                    CySysPmDeepSleep();
                }
            }
            else
            {
                if(CyBle_GetBleSsState() != CYBLE_BLESS_STATE_EVENT_CLOSE)
                {
                    CySysPmSleep();
                }
            }
            CyExitCriticalSection(interruptStatus);
        }
    #else
        CySysClkEcoStop();
        CySysPmDeepSleep();
    #endif  /* #if (BLE_EN) */
    }
#endif  /* #if (DEEP_SLEEP_EN) */


/*****************************************************************************
* Function Name: PostDeepSleep()
******************************************************************************
* Summary:
* Does the post-processing after wakeup from deep sleep.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* Turns on the heart rate measurement block.
*
* Side Effects:
* None
*
* Note:
*
*****************************************************************************/
#if (DEEP_SLEEP_EN)
    static void PostDeepSleep(void)
    {
    #if (HRM_EN || BATTERY_EN)
        ADC_Wakeup();
    #endif  /* #if (HRM_EN || BATTERY_EN) */
    }
#endif  /* #if (DEEP_SLEEP_EN) */


/*****************************************************************************
* Function Name: main()
******************************************************************************
* Summary:
* This is the main function for the program.
*
* Parameters:
* None
*
* Return:
* Not intended to return from here.
*
* Theory:
* Calls the initialization function to start all blocks, and then runs into
* an infinite loop where heart rate and battery level are processed and deep
* sleep is handled.
*
* Side Effects:
* Global interrupts are enabled. IOs are unfrozen.
*
* Note:
*
*****************************************************************************/
int main()
{
#if (BATTERY_EN)
    uint32 previousBatteryTime = 0;
    uint32 currentBatteryTime = 0;
    bool firstTimeBattery = true;
#endif  /* #if (BATTERY_EN) */

    stopDeviceFlag = false;
    CySysPmUnfreezeIo();
    CyGlobalIntEnable;
    InitializeAllBlocks();
    
#if (BATTERY_EN)
    previousBatteryTime = WatchdogTimer_GetTimestamp();
#endif  /* #if (BATTERY_EN) */

    while(1)
    {
#if (HRM_EN)
        canMeasureHeartRate = false;
        ADC_StartConvert();
        while(false == canMeasureHeartRate);
        HeartRate_Measure();
#endif  /* #if (HRM_EN) */


#if (BATTERY_EN)
        if(firstTimeBattery)
        {
            BatteryLevel_Measure();
    #if (BLE_EN)
            CyBle_BassSetCharacteristicValue(BAS_SERVICE_INDEX, CYBLE_BAS_BATTERY_LEVEL, sizeof(batteryLevel), (uint8 *)(&batteryLevel));
    #endif  /* #if (BLE_EN) */
            
            firstTimeBattery = false;
        }
        else
        {
            /* Measure battery level every 10 seconds */
            currentBatteryTime = WatchdogTimer_GetTimestamp();
            if((currentBatteryTime - previousBatteryTime) >= (uint32)10000) 
            {
                previousBatteryTime = currentBatteryTime;
                BatteryLevel_Measure();
        #if (BLE_EN)
                CyBle_BassSetCharacteristicValue(BAS_SERVICE_INDEX, CYBLE_BAS_BATTERY_LEVEL, sizeof(batteryLevel), &batteryLevel);
        #endif  /* #if (BLE_EN) */
            }
        }
#endif  /* #if (BATTERY_EN) */


#if (BLE_EN)
        BleProcessing_Main();
#endif  /* #if (BLE_EN) */


#if (WAKEUP_EN)
        if(stopDeviceFlag)                   
        {
    #if (STATUS_LED_EN)
            LedStatus_Write(true);
    #endif  /* #if (STATUS_LED_EN) */
    
    #if (HRM_EN)
            LedInfrared1_Write(true);
            LedInfrared2_Write(true);
    #endif  /* #if (HRM_EN) */
    
    #if (BLE_EN)
            CyBle_Stop();
            CySysPmStop();
    #else
            CySysPmStop();
    #endif  /* #if (BLE_EN) */
        }
#endif  /* #if (WAKEUP_EN) */
        
#if (DEEP_SLEEP_EN)
        /* Implement deep sleep algorithm */
        GoToDeepSleep();
        
        /* Re-enable everything upon wakeup */
        PostDeepSleep();
#endif  /* #if (DEEP_SLEEP_EN) */
    }
}


/* [] END OF FILE */
