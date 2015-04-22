/*******************************************************************************
* File Name: hrsc.c
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
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "hrsc.h"
#include "basc.h"

uint8 hrsEeReset = 0;
uint8 hrsNotification = 0;

/* Heart Rate Measurement characteristic data structure */
CYBLE_HRS_HRM_T hrsHeartRate;

/* Heart Rate Service callback */
void HeartRateCallBack(uint32 event, void* eventParam)
{
    uint16 rrInt;
    uint16 i;
    uint16 attrValue;

    switch(event)
    {
        case CYBLE_EVT_HRSC_READ_CHAR_RESPONSE:
            printf("Body Sensor Location: ");
            switch(*(((CYBLE_HRS_CHAR_VALUE_T*)eventParam)->value->val))
            {
                case CYBLE_HRS_BODY_SENSOR_LOCATION_OTHER:
                    printf("OTHER (0) \r\n");
                    break;

                case CYBLE_HRS_BODY_SENSOR_LOCATION_CHEST:
                    printf("CHEST (1) \r\n");
                    break;

                case CYBLE_HRS_BODY_SENSOR_LOCATION_WRIST:
                    printf("WRIST (2) \r\n");
                    break;

                default:
                    printf("default \r\n");
                    break;
            }

            /* Enable heart rate and battery notification */
            hrsNotification = 2u;
            basNotification = 3u;
            apiResult = HrscConfigHeartRateNtf(CYBLE_HRS_HRM_NTF_ENABLE);
            if(apiResult != CYBLE_ERROR_OK)
            {
                printf("HrscConfigHeartRateNtf API Error: %xd \r\n", apiResult);
            }
            else
            {
                printf("HRM CCCD Write Request is sent \r\n");
            }
            break;


        case CYBLE_EVT_HRSC_WRITE_DESCR_RESPONSE:
            if(0u == hrsNotification)
            {
                printf("Heart Rate Measurement Notification is Disabled \r\n");
            }
            else
            {
                printf("Heart Rate Measurement Notification is Enabled \r\n");
            }
            break;

        case CYBLE_EVT_HRSC_NOTIFICATION:
            printf("Heart Rate Notification: ");
            
            HrscUnPackHrm(((CYBLE_HRS_CHAR_VALUE_T*)eventParam)->value);

            if(!HrscIsSensorContactSupported() || HrscIsSensorContactDetected())
            {
                printf("Heart Rate: %d    ", (int) HrscGetHeartRate());
                printf("EnergyExpended: %d", (int) HrscGetEnergyExpended());

                for(i = 0; i < CYBLE_HRS_HRM_RRSIZE; i++)
                {
                    rrInt = HrscGetRRInterval(i);
                    if(0u != rrInt)
                    {
                        printf("    RR-Interval %d: %d", (int) i, (int) HrscGetRRInterval(i));
                    }
                }

                printf("\r\n");
            }
            else
            {
                printf("Sensor Contact is supported but not detected \r\n");
            }

            if(2u == hrsNotification)
            {
                hrsNotification = 1u;
                apiResult = CyBle_HrscGetCharacteristicDescriptor(cyBle_connHandle, CYBLE_HRS_HRM, CYBLE_HRS_HRM_CCCD);
                if(apiResult != CYBLE_ERROR_OK)
                {
                    printf("CyBle_HrscGetCharacteristicDescriptor API Error: %xd \r\n", apiResult);
                }
                else
                {
                    printf("HRM CCCD Read Request is sent \r\n");
                }
            }
            else if(3u == basNotification)
            {
                basNotification = 2u;
                attrValue = CYBLE_CCCD_NOTIFICATION;
                apiResult = CyBle_BascSetCharacteristicDescriptor(cyBle_connHandle, 0, CYBLE_BAS_BATTERY_LEVEL,
                    CYBLE_BAS_BATTERY_LEVEL_CCCD, sizeof(attrValue), (uint8 *)&attrValue);
                if(apiResult != CYBLE_ERROR_OK)
                {
                    printf("CyBle_BascSetCharacteristicDescriptor API Error: %x \r\n", apiResult);
                }
                else
                {
                    printf("Battery Level CCCD Write Request is sent \r\n");
                }
            }
            else if((1u == hrsEeReset) && (30u > HrscGetEnergyExpended()))
            {
                hrsEeReset = 0u;
            }
            else if((0u == hrsEeReset) && (300u < HrscGetEnergyExpended()))
            {
                hrsEeReset = 1u;

                apiResult = HrscResetEnergyExpendedCounter();
                if(apiResult != CYBLE_ERROR_OK)
                {
                    printf("HrscResetEnergyExpendedCounter API Error: %x \r\n", apiResult);
                }
                else
                {
                    printf("Heart Rate Control Point Write Request is sent \r\n");
                }
            }
            
            Notification_LED_Write(LED_ON);
            break;

        case CYBLE_EVT_HRSC_READ_DESCR_RESPONSE:
            printf("HRM CCCD Read Response: %4.4x \r\n", CyBle_Get16ByPtr(((CYBLE_HRS_DESCR_VALUE_T*)eventParam)->value->val));
            break;

        case CYBLE_EVT_HRSC_WRITE_CHAR_RESPONSE:
            printf("CPT Write Response: energy expended counter is reset \r\n");
            break;

        default:
            printf("OTHER HRS event: %lx \r\n", event);
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

    hrsHeartRate.flags = 0u;
    hrsHeartRate.heartRateValue = 0u;
    hrsHeartRate.energyExpendedValue = 0u;

    for(i = 0; i < CYBLE_HRS_HRM_RRSIZE; i++)
    {
        hrsHeartRate.rrInterval[i] = 0u;
    }
    
    CyBle_HrsRegisterAttrCallback(HeartRateCallBack);
}

/***************************************
*        Functions
***************************************/


/*******************************************************************************
* Function Name: CyBle_HrscConfigHeartRateNtf
********************************************************************************
*
* Summary:
*   Modifies the Heart Rate Measurument characteristic configuration descriptor.
*
* Parameters:
*  uint16 configuration:   Notification flag, can be either:
*                          CYBLE_HRS_HRM_NTF_ENABLE or
*                          CYBLE_HRS_HRM_NTF_DISABLE
*
* Return:
*   CYBLE_API_RESULT_T: API result will state if API succeeded
*                               (CYBLE_ERROR_OK) or
*                               failed with error codes:
*   CYBLE_ERROR_OK - The request is sent successfully;
*   CYBLE_ERROR_INVALID_PARAMETER - Validation of input parameter is failed.
*******************************************************************************/
CYBLE_API_RESULT_T HrscConfigHeartRateNtf(uint16 configuration)
{
    uint8 config[CYBLE_CCCD_LEN];

    configuration &= CYBLE_CCCD_NOTIFICATION;

    CyBle_Set16ByPtr(config, configuration);

    return (CyBle_HrscSetCharacteristicDescriptor(cyBle_connHandle, CYBLE_HRS_HRM,
                        CYBLE_HRS_HRM_CCCD , CYBLE_CCCD_LEN, config));
}


/*******************************************************************************
* Function Name: CyBle_HrscConfigHeartRateNtf
********************************************************************************
*
* Summary:
*   Modifies the Heart Rate Measurument characteristic configuration descriptor.
*
* Parameters:
*  uint16 configuration:   Notification flag, can be either:
*                   CYBLE_HRS_HRM_NTF_ENABLE or
*                   CYBLE_HRS_HRM_NTF_DISABLE
*
* Return:
*   CYBLE_API_RESULT_T: API result will state if API succeeded
*                               (CYBLE_ERROR_OK) or
*                               failed with error codes:
*   CYBLE_ERROR_OK - The request is sent successfully;
*   CYBLE_ERROR_INVALID_PARAMETER - Validation of input parameter is failed.
*******************************************************************************/
void HrscUnPackHrm(CYBLE_GATT_VALUE_T* value)
{
    uint8 nextPtr;
    uint8 rrInt;
    uint8 i;
    uint8 * pdu;

    pdu = value->val;

    /* flags field is always the first byte */
    hrsHeartRate.flags = pdu[0u];

    if((hrsHeartRate.flags & CYBLE_HRS_HRM_HRVAL16) != 0u)
    {
        /* Heart Rate Measurement Value is also located beginning from the first byte */
        hrsHeartRate.heartRateValue =
            CyBle_Get16ByPtr(&pdu[1u]);
        /* the next data will be located beginning from 3rd byte */
        nextPtr = 3u;
    }
    else
    {
        /* Heart Rate Measurement Value is also located in the first byte */
        hrsHeartRate.heartRateValue = (uint16) pdu[1u];
        /* the next data will be located beginning from the 2nd byte */
        nextPtr = 2u;
    }

    if((hrsHeartRate.flags & CYBLE_HRS_HRM_ENEXP) != 0u)
    {
        hrsHeartRate.energyExpendedValue =
            CyBle_Get16ByPtr(&pdu[nextPtr]);
        /* add 2 bytes: Energy Expended field is uint16 */
        nextPtr += 2u;
    }

    if((hrsHeartRate.flags & CYBLE_HRS_HRM_RRINT) != 0u)
    {
        /* Calculate how many RR-Intervals are in this pdu */
        rrInt = (uint8)(((uint8)(value->len) - nextPtr) >> 1);

        for(i = 0u; i < CYBLE_HRS_HRM_RRSIZE; i++)
        {
            if(i < rrInt) /* While there are RR-Interval values in this pdu */
            {
                hrsHeartRate.rrInterval[i] =
                    CyBle_Get16ByPtr(&pdu[nextPtr]);
                /* add 2 bytes: RR-Interval field is uint16 */
                nextPtr += 2u;
            }
            else /* Fill fhe rest of RR-Interval buffer with zeros */
            {
                hrsHeartRate.rrInterval[i] = 0u;
            }
        }
    }
}


/*******************************************************************************
* Function Name: HrscGetRRInterval
********************************************************************************
*
* Summary:
*   Gets the specified RR-Interval from the
*   Heart Rate Measurument characteristic structure
*
* Parameters:
*   uint8 rrIntervalNumber - number of desired RR-Intervals
*
* Return:
*   uint16 rrInterval.
*
*******************************************************************************/
uint16 HrscGetRRInterval(uint8 rrIntervalNumber)
{
    uint16 retVal;

    if((hrsHeartRate.flags & CYBLE_HRS_HRM_RRINT) != 0u)
    {
        retVal = hrsHeartRate.rrInterval[rrIntervalNumber];
    }
    else
    {
        retVal = 0u; /* If Energy Expended field is not present then return 0 */
    }

    return (retVal);
}

/*******************************************************************************
* Function Name: CyBle_HrscResetEnergyExpendedCounter
********************************************************************************
*
* Summary:
*   Writes "1" into the Heart Rate Control Point characteristic
*       on the peripheral device, that is to reset the Energy Expended counter
*
* Parameters:
*   None.
*
* Return:
*   CYBLE_API_RESULT_T: API result will state if API succeeded
*                               (CYBLE_ERROR_OK) or
*                               failed with error codes:
*   CYBLE_ERROR_OK - The request is sent successfully;
*   CYBLE_ERROR_INVALID_PARAMETER - Validation of input parameter is failed.
*******************************************************************************/
CYBLE_API_RESULT_T HrscResetEnergyExpendedCounter()
{
    uint8 value = CYBLE_HRS_RESET_ENERGY_EXPENDED;

    return (CyBle_HrscSetCharacteristicValue(cyBle_connHandle,
                CYBLE_HRS_CPT, CYBLE_HRS_CPT_CHAR_LEN, &value));
}



/* [] END OF FILE */
