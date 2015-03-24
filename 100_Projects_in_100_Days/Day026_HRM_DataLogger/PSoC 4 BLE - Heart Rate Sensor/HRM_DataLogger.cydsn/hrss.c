/*******************************************************************************
* File Name: hrss.c
*
* Version 1.0
*
* Description:
*  This file contains HRS service related code.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "hrss.h"
#include "fram_logging.h"
#include "common.h"

uint16 energyExpended = 0u;
uint16 fram_addr = 0u;


/* Heart Rate Measurement characteristic data structure */
CYBLE_HRS_HRM_T hrsHeartRate;

/* Pointer and counter for RR-Interval data */
uint8 hrssRrIntPtr;
uint8 hrssRrIntCnt;
uint8 heartRateSimulation = DISABLED;

/* Heart Rate Service callback */
void HeartRateCallBack(uint32 event, void* eventParam)
{
    if(0u != eventParam)
    {
        /* This dummy operation is to avoid warning about unused eventParam */
    }

    switch(event)
    {
        case CYBLE_EVT_HRSS_NOTIFICATION_ENABLED:
            heartRateSimulation = ENABLED;
            printf("Heart Rate Measurement Notification is Enabled \r\n");
            break;

        case CYBLE_EVT_HRSS_NOTIFICATION_DISABLED:
            heartRateSimulation = DISABLED;
            printf("Heart Rate Measurement Notification is Disabled \r\n");
            break;

        /* This event is received when client writes Control Point characteristic value */
        case CYBLE_EVT_HRSS_ENERGY_EXPENDED_RESET:
            energyExpended = 0u;
            HrssSetEnergyExpended(energyExpended);
            printf("Energy Expended Reset \r\n");
            break;

        default:
            printf("Unknown HRS event: %lx \r\n", event);
            break;
    }
}

/*******************************************************************************
* Function Name: HrsInit
********************************************************************************
*
* Summary:
*  Initializes the Heart Rate application global variables.
*
* Parameters:
*  None
*
* Return:
*  None.
*
*******************************************************************************/

void HrsInit(void)
{
    uint16 i;
    
    CyBle_HrsRegisterAttrCallback(HeartRateCallBack);

    hrsHeartRate.flags = 0u;
    hrsHeartRate.heartRateValue = 0u;
    hrsHeartRate.energyExpendedValue = 0u;

    for(i = 0; i < CYBLE_HRS_HRM_RRSIZE; i++)
    {
        hrsHeartRate.rrInterval[i] = 0u;
    }
}

/***************************************
*        Functions
***************************************/


/*******************************************************************************
* Function Name: HrssSetBodySensorLocation
********************************************************************************
*
* Summary:
*  Set the Body Sensor Location characteristic value.
*
* Parameters:
*  CYBLE_HRS_BSL_T location:  Body Sensor Location characteristic value.
* Return:
*  CYBLE_API_RESULT_T: API result will state if API succeeded
*                      (CYBLE_ERROR_OK)
*                      or failed with error codes.
*
*******************************************************************************/
void HrssSetBodySensorLocation(CYBLE_HRS_BSL_T location)
{
    apiResult = CyBle_HrssSetCharacteristicValue(CYBLE_HRS_BSL, CYBLE_HRS_BSL_CHAR_LEN, (uint8*)&location);
    
    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("HrssSetBodySensorLocation API Error: %x \r\n", (int) apiResult);
    }
    else
    {
        printf("Body Sensor Location is set successfully: %d \r\n", location);
    }
}

/*******************************************************************************
* Function Name: CyBle_HrssSetEnergyExpended
********************************************************************************
*
* Summary:
*  Sets Energy Expended value into the Heart Rate Measurument characteristic
*
* Parameters:
*  uint16 energyExpended - Energy Expended value to be set.
*
* Return:
*  None
*
*******************************************************************************/
void HrssSetEnergyExpended(uint16 energyExpended)
{
    hrsHeartRate.energyExpendedValue = energyExpended;
    hrsHeartRate.flags |= CYBLE_HRS_HRM_ENEXP;
}

/*******************************************************************************
* Function Name: HrssGetHrmDescriptor
********************************************************************************
*
* Summary:
*   Gets the Heart Rate Measurement characteristic
*   Configuration Descriptor value
*
* Parameters:
*   None.
*
* Return:
*   uint16 value of the Heart Rate Measurement characteristic
*   configuration Descriptor.
*
*******************************************************************************/
uint16 HrssGetHrmDescriptor(void)
{
    uint8 val[CYBLE_CCCD_LEN];
    uint16 retVal;

    if(CYBLE_ERROR_OK == CyBle_HrssGetCharacteristicDescriptor(CYBLE_HRS_HRM,
        CYBLE_HRS_HRM_CCCD, CYBLE_CCCD_LEN, val))
    {
        /* Using the length as a temporary variable with suitable type to return */
        retVal = CyBle_Get16ByPtr(val);
    }
    else
    {
        retVal = 0u;
    }

    return (retVal);
}


/*******************************************************************************
* Function Name: HrssAddRrInterval
********************************************************************************
*
* Summary:
*  Adds the next RR-Interval into the
*  Heart Rate Measurument characteristic structure.
*
* Parameters:
*  uint16 rrIntervalValue: RR-Interval value to be set.
*
* Return:
*  None
*
*******************************************************************************/
void HrssAddRrInterval(uint16 rrIntervalValue)
{
    hrsHeartRate.flags |= CYBLE_HRS_HRM_RRINT;

    if(hrssRrIntPtr == CYBLE_HRS_HRM_RRSIZE - 1)
    {
        hrssRrIntPtr = 0;
    }
    else
    {
        hrssRrIntPtr++;
    }

    if(hrssRrIntCnt <= CYBLE_HRS_HRM_RRSIZE)
    {
        hrssRrIntCnt++;
    }

    hrsHeartRate.rrInterval[hrssRrIntPtr] = rrIntervalValue;
}


/*******************************************************************************
* Function Name: HrssSendHeartRateNtf
********************************************************************************
*
* Summary:
*  Packs the Heart Rate Measurement characteristic structure into the
*  uint8 array prior to sending it to the collector. Also clears the
*  CYBLE_HRS_HRM_HRVAL16, CYBLE_HRS_HRM_ENEXP and CYBLE_HRS_HRM_RRINT flags.
*
* Parameters:
*  attHandle:  Pointer to the handle which consists of device ID and ATT
*              connection ID.
*
* Return:
*  CYBLE_API_RESULT_T: API result will state if API succeeded
*                      (CYBLE_ERROR_OK)
*                      or failed with error codes.
*
*******************************************************************************/
void HrssSendHeartRateNtf()
{
    uint8 pdu[CYBLE_HRS_HRM_CHAR_LEN];
    uint8 nextPtr;
    uint8 length;
    uint8 rrInt;

    /* Flags field is always the first byte */
    pdu[0u] = hrsHeartRate.flags & (uint8) ~CYBLE_HRS_HRM_HRVAL16;

    /* If the Heart Rate value exceeds one byte */
    if(hrsHeartRate.heartRateValue > 0x00FFu)
    {
        /* then set the CYBLE_HRS_HRM_HRVAL16 flag */
        pdu[0u] |= CYBLE_HRS_HRM_HRVAL16;
        /* and set the full 2-bytes Heart Rate value */
        CyBle_Set16ByPtr(&pdu[1u], hrsHeartRate.heartRateValue);
        /* The next data will be located beginning from 3rd byte */
        nextPtr = 3u;
    }
    else
    {
        /* Else leave the CYBLE_HRS_HRM_HRVAL16 flag remains being cleared */
        /* and set only LSB of the  Heart Rate value */
        pdu[1u] = (uint8) hrsHeartRate.heartRateValue;
        /* The next data will be located beginning from 2nd byte */
        nextPtr = 2u;
    }

    /* If the Energy Expended flag is set */
    if(0u != (hrsHeartRate.flags & CYBLE_HRS_HRM_ENEXP))
    {
        /* clear the CYBLE_HRS_HRM_ENEXP flag */
        hrsHeartRate.flags &= (uint8) ~CYBLE_HRS_HRM_ENEXP;
        /* and set the 2-bytes Energy Expended value */
        CyBle_Set16ByPtr(&pdu[nextPtr], hrsHeartRate.energyExpendedValue);
        /* add 2 bytes: Energy Expended value is uint16 */
        nextPtr += 2u;
    }

    if(HrssAreThereRrIntervals())
    {
        /* Calculate the actual length of pdu: the RR-interval block length should be an even number */
        length = ((CYBLE_HRS_HRM_CHAR_LEN - nextPtr) & ~0x01) + nextPtr;

        rrInt = hrssRrIntPtr;

        while(nextPtr < length)
        {
            /* Increment the rrInterval array pointer in RR-Interval buffer size loop */
            rrInt++;
            if(rrInt >= CYBLE_HRS_HRM_RRSIZE)
            {
                rrInt = 0;
            }

            if(hrsHeartRate.rrInterval[rrInt] != 0)
            {
                /* Copy the non-zero RR-Interval into the pdu */
                CyBle_Set16ByPtr(&pdu[nextPtr], hrsHeartRate.rrInterval[rrInt]);
                /* Clear the current RR-Interval in the buffer */
                hrsHeartRate.rrInterval[rrInt] = 0;
                /* Add 2 bytes: RR-Interval value is uint16 */
                nextPtr += 2u;
            }

            if(rrInt == hrssRrIntPtr)
            {
                hrsHeartRate.flags &= (uint8) ~CYBLE_HRS_HRM_RRINT;
                break;
            }
        }

        hrssRrIntCnt = 0;
    }
	
	apiResult = CyBle_HrssSendNotification(cyBle_connHandle, CYBLE_HRS_HRM, nextPtr, pdu);
    
    if(apiResult != CYBLE_ERROR_OK)
    {
        printf("HrssSendHeartRateNtf API Error: %x \r\n", apiResult);
    }
    else
    {
        printf("Heart Rate Notification is sent successfully, Heart Rate = %d \r\n", hrsHeartRate.heartRateValue);
    }
    
    /* Store heart rate data */
    DataLogging(nextPtr, pdu);
}

/*******************************************************************************
* Function Name: SimulateHeartRate
********************************************************************************
*
* Summary:
*   Simulates all constituents of the Heart Rate Measurement characteristic.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void SimulateHeartRate(void)
{
    static uint32 energyExpendedTimer   = ENERGY_EXPECTED_TIMEOUT;
    static uint16 heartRate             = SIM_HEART_RATE_MIN;
    
    uint16 rrInterval;
    uint8  rrIntCnt;

    /* Heart Rate simulation */
    heartRate += SIM_HEART_RATE_INCREMENT;
    if(heartRate > SIM_HEART_RATE_MAX)
    {
        heartRate = SIM_HEART_RATE_MIN;
    }

    HrssSetHeartRate(heartRate);
    
    /* RR-Interval calculation */    
    /* rrInterval = 60 000 mSec (1 min) / heartRate */
    rrInterval = 60000u / heartRate;
    
    /* count of RR-intervals per one second (1000 mSec) */
    rrIntCnt = (uint8)(1000 / rrInterval);

    while(rrIntCnt > 0u)
    {
        if(!HrssIsRrIntervalBufferFull())
        {
            HrssAddRrInterval(rrInterval);
        }
        rrInterval++;
        rrIntCnt--;
    }
    
    /* Energy Expended simulation */
    if(--energyExpendedTimer == 0)
    {
        energyExpendedTimer = ENERGY_EXPECTED_TIMEOUT;

        HrssSetEnergyExpended(energyExpended);

        if(energyExpended < (CYBLE_ENERGY_EXPENDED_MAX_VALUE - SIM_ENERGY_EXPENDED_INCREMENT))
        {
            energyExpended += SIM_ENERGY_EXPENDED_INCREMENT;
        }
    }
    
    HrssSendHeartRateNtf();
}

/* [] END OF FILE */
