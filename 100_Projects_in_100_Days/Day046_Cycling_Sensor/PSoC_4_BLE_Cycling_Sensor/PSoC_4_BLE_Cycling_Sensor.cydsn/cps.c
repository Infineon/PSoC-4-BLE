/*******************************************************************************
* File Name: cps.c
*
* Version: 1.0
*
* Description:
*  This file contains CPS callback handler function.
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

#include "common.h"
#include "cps.h"

uint32 powerTimer = 1u;
uint16 powerSimulation;
uint16 powerCPResponse;

CYBLE_CPS_POWER_MEASURE_T powerMeasure;
CYBLE_CPS_POWER_VECTOP_T powerVector;

/* Data buffer for Control Point response. 
   First byte contains the length of response 
*/
uint8 powerCPData[CYBLE_GATT_DEFAULT_MTU - 2u] = {3, CYBLE_CPS_CP_OC_RC, CYBLE_CPS_CP_OC_SCV, CYBLE_CPS_CP_RC_SUCCESS};


/*******************************************************************************
* Function Name: CpsCallBack()
********************************************************************************
*
* Summary:
*   This is an event callback function to receive service specific events from 
*   Cycling Power Service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
* Return:
*  None.
*
*******************************************************************************/
void CpsCallback(uint32 event, void *eventParam)
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    uint8 i;
    uint8 locCharIndex;
    locCharIndex = ((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->charIndex;
    printf("CPS event: %lx, ", event);

    switch(event)
    {
        /* CPS Server - Notifications for Cycling Power Service Characteristic
        was enabled. The parameter of this event is a structure of
        CYBLE_CPS_CHAR_VALUE_T type.
        */
        case CYBLE_EVT_CPSS_NOTIFICATION_ENABLED:
            printf("CYBLE_EVT_CPSS_NOTIFICATION_ENABLED: char: %x\r\n", locCharIndex);
            if(locCharIndex == CYBLE_CPS_POWER_MEASURE)
            {
                powerSimulation |= CPS_NOTIFICATION_MEASURE_ENABLE;
            }
            if(locCharIndex == CYBLE_CPS_POWER_VECTOR)
            {
                powerSimulation |= CPS_NOTIFICATION_VECTOR_ENABLE;
            }
            break;
        
        /* CPS Server - Notifications for Cycling Power Service Characteristic
            was disabled. The parameter of this event is a structure 
            of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSS_NOTIFICATION_DISABLED:
            printf("CYBLE_EVT_CPSS_NOTIFICATION_DISABLED: char: %x\r\n", locCharIndex);
            if(locCharIndex == CYBLE_CPS_POWER_MEASURE)
            {
                powerSimulation &= ~CPS_NOTIFICATION_MEASURE_ENABLE;
            }
            if(locCharIndex == CYBLE_CPS_POWER_VECTOR)
            {
                powerSimulation &= ~CPS_NOTIFICATION_VECTOR_ENABLE;
            }
            break;
        
        /* CPS Server - Indication for Cycling Power Service Characteristic
            was enabled. The parameter of this event is a structure 
            of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSS_INDICATION_ENABLED:
            printf("CYBLE_EVT_CPSS_INDICATION_ENABLED: char: %x\r\n", locCharIndex);
            powerSimulation |= CPS_INDICATION_ENABLE;
            break;
        
        /* CPS Server - Indication for Cycling Power Service Characteristic
            was disabled. The parameter of this event is a structure 
            of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSS_INDICATION_DISABLED:
            printf("CYBLE_EVT_CPSS_INDICATION_DISABLED: char: %x\r\n", locCharIndex);
            powerSimulation &= ~CPS_INDICATION_ENABLE;
            break;
        
        /* CPS Server - Cycling Power Service Characteristic
            Indication was confirmed. The parameter of this event
            is a structure of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSS_INDICATION_CONFIRMED:
            printf("CYBLE_EVT_CPSS_INDICATION_CONFIRMED: char: %x\r\n", locCharIndex);
            break;
        
        /* CPS Server - Broadcast for Cycling Power Service Characteristic
            was enabled. The parameter of this event
            is a structure of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSS_BROADCAST_ENABLED:
            printf("CYBLE_EVT_CPSS_BROADCAST_ENABLED: char: %x\r\n", locCharIndex);
            powerSimulation |= CPS_BROADCAST_ENABLE;
            break;
        
        /* CPS Server - Broadcast for Cycling Power Service Characteristic
            was disabled. The parameter of this event
            is a structure of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSS_BROADCAST_DISABLED:
            printf("CYBLE_EVT_CPSS_BROADCAST_DISABLED: char: %x\r\n", locCharIndex);
            powerSimulation &= ~CPS_BROADCAST_ENABLE;
            CyBle_CpssStopBroadcast();
            printf("Stop Broadcast \r\n");
            break;
        
        /* CPS Server - Write Request for Cycling Power Service Characteristic 
            was received. The parameter of this event is a structure
            of CYBLE_CPS_CHAR_VALUE_T type.
        */
        case CYBLE_EVT_CPSS_CHAR_WRITE:
            printf("CYBLE_EVT_CPSS_CHAR_WRITE: %x ", locCharIndex);
            ShowValue(((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value);
            if(locCharIndex == CYBLE_CPS_POWER_CP)
            {
                uint8 cpOpCode;
                cpOpCode = ((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[0];
                /* Prepare general response */
                powerCPResponse = 1u;
                powerCPData[CYBLE_CPS_CP_RESP_LENGTH] = 3u; /* Length of response */
                powerCPData[CYBLE_CPS_CP_RESP_OP_CODES] = CYBLE_CPS_CP_OC_RC;
                powerCPData[CYBLE_CPS_CP_RESP_REQUEST_OP_CODE] = cpOpCode;
                powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_SUCCESS;
            
                printf("CP Opcode %x: ", cpOpCode);
                switch(cpOpCode)
                {
                    case CYBLE_CPS_CP_OC_SCV:
                        printf("Set Cumulative Value \r\n");
                        /* Initiate the procedure to set a cumulative value. The new value is sent as parameter 
                           following op code (parameter defined per service). The response to this control point is 
                           Op Code 0x20 followed by the appropriate Response Value. */
                        if(((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->len == (sizeof(uint32) + 1u))
                        {
                            powerMeasure.cumulativeWheelRevolutions = *(uint32 *)&((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CYBLE_CPS_CP_OC_USL:      
                        printf("Update Sensor Location \r\n");
                        /* Update to the location of the Sensor with the value sent as parameter to this op code. 
                           The response to this control point is Op Code 0x20 followed by the appropriate Response 
                           Value. */
                        if(((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1] < CYBLE_CPS_SL_COUNT)
                        {
                            apiResult = CyBle_CpssSetCharacteristicValue(CYBLE_CPS_SENSOR_LOCATION, sizeof(uint8), &((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1]);
                        }
                        else
                        {
                            powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        printf("CyBle_CpssSetCharacteristicValue SENSOR_LOCATION, API result: %x \r\n", apiResult);
                        break;
                    case CYBLE_CPS_CP_OC_RSSL: 
                        printf("Request Supported Sensor Locations \r\n");
                        /* Request a list of supported locations where the Sensor can be attached. The response to this
                           control point is Op Code 0x20 followed by the appropriate Response Value, including a list
                           of supported Sensor locations in the Response Parameter. */
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += CYBLE_CPS_SL_COUNT; /* Length of response */
                        for(i = 0; i < CYBLE_CPS_SL_COUNT; i++)
                        {
                            powerCPData[CYBLE_CPS_CP_RESP_PARAMETER + i] = i;
                        }
                        
                        break;
                    case CYBLE_CPS_CP_OC_SCRL: 
                        printf("Set Crank Length \r\n");
                        /* Initiate the procedure to set the crank length value to Sensor. The new value is sent as a 
                           parameter with preceding Op Code 0x04 operand. The response to this control point is Op Code
                           0x20 followed by the appropriate Response Value. */
                        if(((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->len == (sizeof(uint16) + 1u))
                        {
                            cyBle_cpssAdjustment.crankLength =  *(uint16 *)&((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CYBLE_CPS_CP_OC_RCRL:
                        printf(" Request Crank Length \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += sizeof(cyBle_cpssAdjustment.crankLength); /* Length of response */
                        CyBle_Set16ByPtr(powerCPData + CYBLE_CPS_CP_RESP_PARAMETER, cyBle_cpssAdjustment.crankLength);
                        break;
                    case CYBLE_CPS_CP_OC_SCHL:
                        printf("Set Chain Length \r\n");
                        if(((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->len == (sizeof(uint16) + 1u))
                        {
                            cyBle_cpssAdjustment.chainLength =  *(uint16 *)&((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CYBLE_CPS_CP_OC_RCHL:
                        printf("Request Chain Length \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += sizeof(cyBle_cpssAdjustment.chainLength); /* Length of response */
                        CyBle_Set16ByPtr(powerCPData + CYBLE_CPS_CP_RESP_PARAMETER, cyBle_cpssAdjustment.chainLength);
                        break;
                    case CYBLE_CPS_CP_OC_SCHW:
                        printf("Set Chain Weight \r\n");
                        if(((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->len == (sizeof(uint16) + 1u))
                        {
                            cyBle_cpssAdjustment.chainWeight =  *(uint16 *)&((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CYBLE_CPS_CP_OC_RCHW:
                        printf("Request Chain Weight \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += sizeof(cyBle_cpssAdjustment.chainWeight); /* Length of response */
                        CyBle_Set16ByPtr(powerCPData + CYBLE_CPS_CP_RESP_PARAMETER, cyBle_cpssAdjustment.chainWeight);
                        break;
                    case CYBLE_CPS_CP_OC_SSL:
                        printf("Set Span Length \r\n");
                        if(((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->len == (sizeof(uint16) + 1u))
                        {
                            cyBle_cpssAdjustment.spanLength =  *(uint16 *)&((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1];
                        }
                        else
                        {
                            powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_INVALID_PARAMETER;                            
                        }
                        break;
                    case CYBLE_CPS_CP_OC_RSL:
                        printf(" Request Span Length \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += sizeof(cyBle_cpssAdjustment.spanLength); /* Length of response */
                        CyBle_Set16ByPtr(powerCPData + CYBLE_CPS_CP_RESP_PARAMETER, cyBle_cpssAdjustment.spanLength);
                        break;
                    case CYBLE_CPS_CP_OC_SOC:
                        printf("Start Offset Compensation \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += sizeof(cyBle_cpssAdjustment.offsetCompensation); /* Length of response */
                        CyBle_Set16ByPtr(powerCPData + CYBLE_CPS_CP_RESP_PARAMETER, cyBle_cpssAdjustment.offsetCompensation);
                        break;
                    case CYBLE_CPS_CP_OC_MCPMCC:
                        printf("Mask Cycling Power Measurement Characteristic Content \r\n");
                        { 
                            uint16 mask = *(uint16 *)&((CYBLE_CPS_CHAR_VALUE_T *)eventParam)->value->val[1];
                            if((mask & CYBLE_CPS_CP_ENERGY_RESERVED) != 0u)
                            {
                                powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_INVALID_PARAMETER;                            
                            }

                            if((mask & CYBLE_CPS_CP_PEDAL_PRESENT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_PEDAL_PRESENT_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_TORQUE_PRESENT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_TORQUE_PRESENT_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_WHEEL_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_WHEEL_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_CRANK_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_CRANK_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_MAGNITUDES_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_FORCE_MAGNITUDES_BIT;
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_TORQUE_MAGNITUDES_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_ANGLES_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_ANGLES_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_TOP_DEAD_SPOT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_TOP_DEAD_SPOT_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_BOTTOM_DEAD_SPOT_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_BOTTOM_DEAD_SPOT_BIT;
                            }
                            if((mask & CYBLE_CPS_CP_ENERGY_BIT) != 0u) 
                            {
                                powerMeasure.flags &= ~CYBLE_CPS_CPM_ENERGY_BIT;
                            }
                        }
                        break;
                    case CYBLE_CPS_CP_OC_RSR: 
                        printf("Request Sampling Rate \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += sizeof(cyBle_cpssAdjustment.samplingRate); /* Length of response */
                        powerCPData[CYBLE_CPS_CP_RESP_PARAMETER] = cyBle_cpssAdjustment.samplingRate;
                        break;
                    case CYBLE_CPS_CP_OC_RFCD:
                        printf("Request Factory Calibration Date \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_LENGTH] += sizeof(cyBle_cpssAdjustment.factoryCalibrationDate); /* Length of response */
                        memcpy(powerCPData + CYBLE_CPS_CP_RESP_PARAMETER, &cyBle_cpssAdjustment.factoryCalibrationDate, 
                               sizeof(cyBle_cpssAdjustment.factoryCalibrationDate));
                        break;
                    case CYBLE_CPS_CP_OC_RC:
                        printf("Response Code \r\n");
                        break;
                    default:
                        printf("Op Code Not supported \r\n");
                        powerCPData[CYBLE_CPS_CP_RESP_VALUE] = CYBLE_CPS_CP_RC_NOT_SUPPORTED;
                        break;
                }
            }
            break;
        
        /* CPS Client - Cycling Power Service Characteristic
            Notification was received. The parameter of this event
            is a structure of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSC_NOTIFICATION:
            break;
        
        /* CPS Client - Cycling Power Service Characteristic
            Indication was received. The parameter of this event
            is a structure of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSC_INDICATION:
            break;
        
        /* CPS Client - Read Response for Read Request of Cycling Power Service
            Characteristic value. The parameter of this event
            is a structure of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSC_READ_CHAR_RESPONSE:
            break;

        /* CPS Client - Write Response for Write Request of Cycling Power Service
            Characteristic value. The parameter of this event
            is a structure of CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSC_WRITE_CHAR_RESPONSE:
            break;
        
        /* CPS Client - Read Response for Read Request of Cycling Power
            Service Characteristic Descriptor Read request. The 
            parameter of this event is a structure of
            CYBLE_CPS_DESCR_VALUE_T type
        */
        case CYBLE_EVT_CPSC_READ_DESCR_RESPONSE:
            break;
        
        /* CPS Client - Write Response for Write Request of Cycling Power
            Service Characteristic Configuration Descriptor value.
            The parameter of this event is a structure of 
            CYBLE_CPS_DESCR_VALUE_T type
        */
        case CYBLE_EVT_CPSC_WRITE_DESCR_RESPONSE:
            break;

        /* CPS Client - This event is triggered every time a device receive
            non-connectable undirected advertising event.
            The parameter of this event is a structure of 
            CYBLE_CPS_CHAR_VALUE_T type
        */
        case CYBLE_EVT_CPSC_SCAN_PROGRESS_RESULT:
            break;
            
		default:
            printf("Not supported event\r\n");
			break;
    }
}


/*******************************************************************************
* Function Name: CpsInit()
********************************************************************************
*
* Summary:
*   Initializes the SCP service.
*
*******************************************************************************/
void CpsInit(void)
{
    CYBLE_API_RESULT_T apiResult;
    uint16 cccdValue;
    CYBLE_CPS_SL_VALUE_T sensorLocation = CYBLE_CPS_SL_TOP_OF_SHOE;
    
    /* Register service specific callback function */
    CyBle_CpsRegisterAttrCallback(&CpsCallback);
    /* Read CCCD configurations from flash */
    apiResult = CyBle_CpssGetCharacteristicDescriptor(CYBLE_CPS_POWER_MEASURE, CYBLE_CPS_CCCD, 
        CYBLE_CCCD_LEN, (uint8 *)&cccdValue);
    if((apiResult == CYBLE_ERROR_OK) && (cccdValue != 0u))
    {
        powerSimulation |= CPS_NOTIFICATION_MEASURE_ENABLE;
    }
    apiResult = CyBle_CpssGetCharacteristicDescriptor(CYBLE_CPS_POWER_VECTOR, CYBLE_CPS_CCCD, 
        CYBLE_CCCD_LEN, (uint8 *)&cccdValue);
    if((apiResult == CYBLE_ERROR_OK) && (cccdValue != 0u))
    {
        powerSimulation |= CPS_NOTIFICATION_VECTOR_ENABLE;
    }
    
    /* Read flags of Power Measure characteristic default value */
    apiResult = CyBle_CpssGetCharacteristicValue(CYBLE_CPS_POWER_MEASURE, sizeof(powerMeasure.flags), (uint8 *)&powerMeasure);
    if((apiResult != CYBLE_ERROR_OK))
    {
        printf("CyBle_CpssGetCharacteristicValue API Error: %x \r\n", apiResult);
    }
    
    /* Read flags of Power Vector characteristic default value */
    apiResult = CyBle_CpssGetCharacteristicValue(CYBLE_CPS_POWER_VECTOR, sizeof(powerVector.flags), (uint8 *)&powerVector);
    if((apiResult != CYBLE_ERROR_OK))
    {
        printf("CyBle_CpssGetCharacteristicValue API Error: %x \r\n", apiResult);
    }

    /* Set default values which might be placed in EEPROM by application */
    cyBle_cpssAdjustment.factoryCalibrationDate.year = 2014;
    cyBle_cpssAdjustment.factoryCalibrationDate.day = 12;
    cyBle_cpssAdjustment.factoryCalibrationDate.month = 12;
    
    powerMeasure.instantaneousPower = CPS_SIM_POWER_INIT; 
    powerMeasure.accumulatedTorque = CPS_SIM_TORQUE_INIT;
    powerMeasure.cumulativeWheelRevolutions = CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INIT;
    powerMeasure.lastWheelEventTime = CPS_SIM_WHEEL_EVENT_TIME_INIT;
    powerMeasure.accumulatedEnergy = CPS_SIM_ACCUMULATED_ENERGY_INIT;
    
    powerVector.cumulativeCrankRevolutions += CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INIT;
    powerVector.lastCrankEventTime += CPS_SIM_CRANK_EVENT_TIME_INIT;
    
    CyBle_CpssSetCharacteristicValue(CYBLE_CPS_SENSOR_LOCATION, sizeof(uint8), (uint8 *)&sensorLocation);
}


/*******************************************************************************
* Function Name: SimulateCyclingPower()
********************************************************************************
*
* Summary:
*   This function measures the die temperature and sends it to the client.
*
*******************************************************************************/
void SimulateCyclingPower(void)
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    if(CyBle_GattGetBusyStatus() == CYBLE_STACK_STATE_FREE)
    {
        uint8 powerMeasureData[CYBLE_GATT_DEFAULT_MTU - 3];
        uint8 length = 0;
        
        /* Prepare data array */
        CyBle_Set16ByPtr(powerMeasureData, (CYBLE_CPS_CPM_TORQUE_PRESENT_BIT |
                                            CYBLE_CPS_CPM_TORQUE_SOURCE_BIT |
                                            CYBLE_CPS_CPM_WHEEL_BIT |
                                            CYBLE_CPS_CPM_ENERGY_BIT) & powerMeasure.flags);
        length += sizeof(powerMeasure.flags);
        CyBle_Set16ByPtr(powerMeasureData + length, powerMeasure.instantaneousPower);
        length += sizeof(powerMeasure.instantaneousPower);
        if((CYBLE_CPS_CPM_TORQUE_PRESENT_BIT & powerMeasure.flags) != 0u)
        {
            CyBle_Set16ByPtr(powerMeasureData + length, powerMeasure.accumulatedTorque);
            length += sizeof(uint16);
            CyBle_Set16ByPtr(powerMeasureData + length, (uint16)powerMeasure.cumulativeWheelRevolutions);
            CyBle_Set16ByPtr(powerMeasureData + length+2, (uint16)(powerMeasure.cumulativeWheelRevolutions >> 16u));
            length += sizeof(powerMeasure.cumulativeWheelRevolutions);
            CyBle_Set16ByPtr(powerMeasureData + length, powerMeasure.lastWheelEventTime);
            length += sizeof(powerMeasure.lastWheelEventTime);
        }
        if((CYBLE_CPS_CPM_ENERGY_BIT & powerMeasure.flags) != 0u)
        {
            CyBle_Set16ByPtr(powerMeasureData + length, powerMeasure.accumulatedEnergy);
            length += sizeof(uint16);
        }
            
        /* Send data */
        if((powerSimulation & CPS_NOTIFICATION_MEASURE_ENABLE) != 0u)
        {
            apiResult = CyBle_CpssSendNotification(cyBle_connHandle, CYBLE_CPS_POWER_MEASURE, length, powerMeasureData);
            printf("CpssSendNotification POWER_MEASURE, Power: %d W, ", powerMeasure.instantaneousPower);
            printf("Torque: %ld, Wheel Revolution: %ld, Time: %d s, Speed: %3.2f km/h, ", 
                powerMeasure.accumulatedTorque / 32u,
                powerMeasure.cumulativeWheelRevolutions,
                powerMeasure.lastWheelEventTime / CPS_WHEEL_EVENT_TIME_PER_SEC,
                (float)CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INCREMENT * CPS_WHEEL_CIRCUMFERENCE /
                CPS_SIM_WHEEL_EVENT_TIME_INCREMENT * CPS_WHEEL_EVENT_TIME_PER_SEC * CPS_SEC_IN_HOUR
            );
            printf("Energy: %ld kJ \r\n", powerMeasure.accumulatedEnergy);
            
            if((apiResult != CYBLE_ERROR_OK))
            {
                printf("CpssSendNotification API Error: %x \r\n", apiResult);
            }
        }
        
        if((powerSimulation & CPS_BROADCAST_ENABLE) != 0u)
        {
            apiResult = CyBle_CpssStartBroadcast(CYBLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN, length, powerMeasureData);
            printf("CyBle_CpssStartBroadcast, API result: %x \r\n", apiResult);
        }
        
        if((powerSimulation & CPS_NOTIFICATION_VECTOR_ENABLE) != 0u)
        {
            uint8 powerVectorData[CPS_POWER_VECTOR_DATA_MAX_SIZE];
            length = 0;
            
            CyBle_Set16ByPtr(powerVectorData,  powerVector.flags);
            length += sizeof(powerVector.flags);
            CyBle_Set16ByPtr(powerVectorData + length, powerVector.cumulativeCrankRevolutions);
            length += sizeof(powerVector.cumulativeCrankRevolutions);
            CyBle_Set16ByPtr(powerVectorData + length, powerVector.lastCrankEventTime);
            length += sizeof(powerVector.lastCrankEventTime);

            apiResult = CyBle_CpssSendNotification(cyBle_connHandle, CYBLE_CPS_POWER_VECTOR, length , powerVectorData);
            printf("CpssSendNotification POWER_VECTOR, Crank Revolution: %d W, ", powerVector.cumulativeCrankRevolutions);
            printf("Time: %d s, Cadence: %d rpm \r\n", powerVector.lastCrankEventTime / CPS_CRANK_EVENT_TIME_PER_SEC,
                CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INCREMENT / (CPS_SIM_CRANK_EVENT_TIME_INCREMENT / CPS_CRANK_EVENT_TIME_PER_SEC)
            );
            if((apiResult != CYBLE_ERROR_OK))
            {
                printf("CpssSendNotification API Error: %x \r\n", apiResult);
            }
            
        }
        
        if(((powerSimulation & CPS_INDICATION_ENABLE) != 0u) && (powerCPResponse != 0u))
        {
            apiResult = CyBle_CpssSendIndication(cyBle_connHandle, CYBLE_CPS_POWER_CP, 
                                                 powerCPData[CYBLE_CPS_CP_RESP_LENGTH], powerCPData + 1u);
            printf("CyBle_CpssSendIndication POWER_CP, API result: %x \r\n", apiResult);
            powerCPResponse = 0u;
        }
        
        /* Simulate data */
        powerMeasure.instantaneousPower++;
        powerMeasure.accumulatedTorque += CPS_SIM_TORQUE_INCREMENT;
        powerMeasure.cumulativeWheelRevolutions += CPS_SIM_CUMULATIVE_WHEEL_REVOLUTION_INCREMENT;
        powerMeasure.lastWheelEventTime += CPS_SIM_WHEEL_EVENT_TIME_INCREMENT;
        powerMeasure.accumulatedEnergy += CPS_SIM_ACCUMULATED_ENERGY_INCREMENT;
        
        powerVector.cumulativeCrankRevolutions += CPS_SIM_CUMULATIVE_CRANK_REVOLUTION_INCREMENT;
        powerVector.lastCrankEventTime += CPS_SIM_CRANK_EVENT_TIME_INCREMENT;
    }
}

/* [] END OF FILE */

