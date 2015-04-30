/*******************************************************************************
* File Name: cscss.c
*
* Version: 1.0
*
* Description:
*  This file contains routines related to Cycling Speed and Cadence Service.
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

#include "cscs.h"


/***************************************
*        Global Variables
***************************************/
uint8                cscFlags;
uint32               wheelRev = CSC_WHEEL_REV_INIT_VAL;
uint32               lastWheelEvTime = CSC_WHEEL_EV_TIME_VAL;
uint32               crankRev = 0u;
uint32               lastCrankEvTime = CSC_CRANK_EV_TIME_VAL;
uint16               cscSpeed;
uint16               cscCadenceRpm;
uint8                speedCPresponse = 0u;
uint8                speedSimulation;
uint8                supportedSensorLoc;
uint8                sensorLocations[NUM_SUPPORTED_SENSORS] = {TOP_OF_SHOE, IN_SHOE, LEFT_CRANK, RIGHT_CRANK};

/* Data buffer for Control Point response. 
   First byte contains the length of response 
*/
uint8                scCPResponse[SC_CP_CHAR_LENGTH_4BYTES + 1u + NUM_SUPPORTED_SENSORS];



/*******************************************************************************
* Function Name: CscServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Cycling Speed and Cadence Service.
*
* Parameters:  
*  event:           CSCS event.
*  *eventParams:    Data structure specific to event received.
*
* Return: 
*  None
*
*******************************************************************************/
void CscsCallback(uint32 event, void *eventParam)
{
    uint8 i;
    uint8 sensorLocSupported = 0u;
    CYBLE_CSCS_CHAR_VALUE_T *wrReqParam;

    switch(event)
    {
    /* CSCS Server - Notifications for Cycling Speed and Cadence Service
        Characteristic was enabled. The parameter of this event is a structure of
        CYBLE_CSCS_CHAR_VALUE_T type.
    */
    case CYBLE_EVT_CSCSS_NOTIFICATION_ENABLED:
        printf("Notifications for CSC Measurement Characteristic are enabled\r\n");
        speedSimulation |= CSCS_NOTIFICATION_ENABLE;
		break;

    /* CSCS Server - Notifications for Cycling Speed and Cadence Service
        Characteristic was disabled. The parameter of this event is a structure  of
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSS_NOTIFICATION_DISABLED:
        printf("Notifications for CSC Measurement Characteristic are disabled\r\n");
		speedSimulation &= ~CSCS_NOTIFICATION_ENABLE;
        break;

    /* CSCS Server - Indication for Cycling Speed and Cadence Service Characteristic
        was enabled. The parameter of this event is a structure of
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSS_INDICATION_ENABLED:
        printf("Indications for SC Control Point Characteristic are enabled\r\n");
        speedSimulation |= CSCS_INDICATION_ENABLE;
		break;

    /* CSCS Server - Indication for Cycling Speed and Cadence Service Characteristic
        was disabled. The parameter of this event is a structure of
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSS_INDICATION_DISABLED:
        printf("Indications for SC Control Point Characteristic are disabled\r\n");
        speedSimulation &= ~CSCS_INDICATION_ENABLE;
		break;

    /* CSCS Server - Cycling Speed and Cadence Service Characteristic
        Indication was confirmed. The parameter of this event is a structure of 
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSS_INDICATION_CONFIRMATION:
        printf("Confirmation of SC Control Point Characteristic indication received\r\n");
		break;
    
    /* CSCS Server - Write Request for Cycling Speed and Cadence Service
        Characteristic was received. The parameter of this event is a structure of
        CYBLE_CSCS_CHAR_VALUE_T type.
    */
    case CYBLE_EVT_CSCSS_CHAR_WRITE:

        wrReqParam = (CYBLE_CSCS_CHAR_VALUE_T *) eventParam;
        printf("Write to SC Control Point Characteristic occurred. ");
        
        printf("Data length: %d. ", wrReqParam->value->len);
        
        printf("Received data:");
        
        for(i = 0u; i < wrReqParam->value->len; i++)
        {
             printf(" %x", wrReqParam->value->val[i]);
        }
         printf("\r\n");
        
        /* Prepare general response */
        speedCPresponse = 1u;
        
        /* Length of response */
        scCPResponse[CYBLE_CSCS_SC_CP_RESP_LEN_IDX] = SC_CP_CHAR_LENGTH_3BYTES;
        scCPResponse[CYBLE_CSCS_SC_CP_RESP_OP_CODE_IDX] = CYBLE_CSCS_RESPONSE_CODE;
        scCPResponse[CYBLE_CSCS_SC_CP_REQ_OP_CODE_IDX] = wrReqParam->value->val[CYBLE_CSCS_SC_CP_OP_CODE_IDX];
        scCPResponse[CYBLE_CSCS_SC_CP_RESP_VALUE_IDX] = CYBLE_CSCS_ERR_SUCCESS;

        switch(wrReqParam->value->val[CYBLE_CSCS_SC_CP_OP_CODE_IDX])
        {
            case CYBLE_CSCS_SET_CUMMULATIVE_VALUE:
                
                wheelRev = (wrReqParam->value->val[CYBLE_CSCS_SC_CUM_VAL_BYTE0_IDX + 3u] << 24u) |
                                (wrReqParam->value->val[CYBLE_CSCS_SC_CUM_VAL_BYTE0_IDX + 2u] << 16u) |
                                (wrReqParam->value->val[CYBLE_CSCS_SC_CUM_VAL_BYTE0_IDX + 1u] << 8u) |
                                wrReqParam->value->val[CYBLE_CSCS_SC_CUM_VAL_BYTE0_IDX];
                
                if(0ul == wheelRev)
                {
                    printf("Set cumulative value to zero.\r\n");
                }
                else
                {
                    printf("Set cumulative value to 0x%4.4x%4.4x.\r\n", HI16(wheelRev), LO16(wheelRev));
                }
                break;

            case CYBLE_CSCS_START_SENSOR_CALIBRATION:
                printf("Start Sensor calibration command received. ");
                printf("This command is not supported in this example project.\r\n");
                /* The Start Sensor Calibration command is not supported in the example */
                scCPResponse[CYBLE_CSCS_SC_CP_RESP_VALUE_IDX] = CYBLE_CSCS_ERR_OP_CODE_NOT_SUPPORTED;
                break;

            case CYBLE_CSCS_UPDATE_SENSOR_LOCATION:
                printf("Update Sensor Location \r\n");
                /* Update to the location of the Sensor with the value sent as parameter to this op code. 
                */
                switch(wrReqParam->value->val[CYBLE_CSCS_SENSOR_LOC_IDX])
                {
                    case TOP_OF_SHOE:
                    case IN_SHOE:
                    case LEFT_CRANK:
                    case RIGHT_CRANK:
                        sensorLocSupported = 1u;
                        break;
                    default:
                        break;
                }

                if(0u != sensorLocSupported)
                {
                    (void) CyBle_CscssSetCharacteristicValue(CYBLE_CSCS_SENSOR_LOCATION, 
                                                             sizeof(uint8),
                                                             &wrReqParam->value->val[CYBLE_CSCS_SENSOR_LOC_IDX]);
                    printf("Set sensor location operation completed successfully. \r\n");
                }
                else
                {
                    scCPResponse[CYBLE_CSCS_SC_CP_RESP_VALUE_IDX] = CYBLE_CSCS_ERR_INVALID_PARAMETER;
                    printf("Unsupported sensor location.\r\n");
                }
                break;

            case CYBLE_CSCS_REQ_SUPPORTED_SENSOR_LOCATION:
                printf("Request Supported Sensor Locations \r\n");
                /* Request a list of supported locations where the Sensor can be attached. */
                scCPResponse[CYBLE_CSCS_SC_CP_RESP_LEN_IDX] += NUM_SUPPORTED_SENSORS;
                
                for(i = 0u; i < NUM_SUPPORTED_SENSORS; i++)
                {
                    scCPResponse[CYBLE_CSCS_SC_CP_RESP_PARAMETER_IDX + i] = sensorLocations[i];
                }
                break;

            default:
                printf("Unsupported command.\r\n");
                scCPResponse[CYBLE_CSCS_SC_CP_RESP_VALUE_IDX] = CYBLE_CSCS_ERR_OP_CODE_NOT_SUPPORTED;
                break;
        }
		break;
    
    
    /* CSCS Client - Cycling Speed and Cadence Service Characteristic
        Notification was received. The parameter of this event is a structure of
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSC_NOTIFICATION:
        break;

    /* CSCS Client - Cycling Speed and Cadence Service Characteristic
        Indication was received. The parameter of this event is a structure of 
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSC_INDICATION:
        break;

    /* CSCS Client - Read Response for Read Request of Cycling Speed and Cadence 
        Service Characteristic value. The parameter of this event is a structure of
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSC_READ_CHAR_RESPONSE:
        break;

    /* CSCS Client - Write Response for Write Request of Cycling Speed and Cadence 
        Service Characteristic value. The parameter of this event is a structure of
        CYBLE_CSCS_CHAR_VALUE_T type
    */
    case CYBLE_EVT_CSCSC_WRITE_CHAR_RESPONSE:
        break;

    /* CSCS Client - Read Response for Read Request of Cycling Speed and Cadence
        Service Characteristic Descriptor Read request. The parameter of this event
        is a structure of CYBLE_CSCS_DESCR_VALUE_T type
    */
    case CYBLE_EVT_CSCSC_READ_DESCR_RESPONSE:
        break;

    /* CSCS Client - Write Response for Write Request of Cycling Speed and Cadence
        Service Characteristic Configuration Descriptor value. The parameter of
        this event is a structure of  CYBLE_CSCS_DESCR_VALUE_T type
    */
    case CYBLE_EVT_CSCSC_WRITE_DESCR_RESPONSE:
        break;
        
	default:
        printf("Unrecognized CSCS event.\r\n");

	    break;
    }
}


/*******************************************************************************
* Function Name: SimulateCyclingSpeed
********************************************************************************
*
* Summary:
*  Simulates Cycling speed data and send them to the Client device.
*
* Parameters:  
*  None.
*
* Return: 
*  None
*
*******************************************************************************/
void SimulateCyclingSpeed(void)
{
    CYBLE_API_RESULT_T apiResult;
    uint8 csValue[CSC_CHAR_LENGTH];
    uint8 len = 0u;
    
    /*  Updates CSC Measurement Characteristic data. */
    wheelRev += CSC_WHEEL_REV_VAL;
    lastWheelEvTime += CSC_WHEEL_EV_TIME_VAL;
    crankRev += CSC_CRANK_REV_VAL;
    lastCrankEvTime += CSC_CRANK_EV_TIME_VAL;
    
    if((speedSimulation & CSCS_NOTIFICATION_ENABLE) != 0u)
    {
        /* Pack SCS Measurement Characteristic data */
        csValue[len++] = cscFlags;
        
        if(0u != (cscFlags & WHEEL_REV_DATA_PRESENT))
        {
            csValue[len++] = LO8(LO16(wheelRev));
            csValue[len++] = HI8(LO16(wheelRev));
            csValue[len++] = LO8(HI16(wheelRev));
            csValue[len++] = HI8(HI16(wheelRev));
            
            csValue[len++] = LO8(lastWheelEvTime);
            csValue[len++] = HI8(lastWheelEvTime);
            
            /* Calculate instantaneous Cadence */
            cscSpeed = ((((((uint32)WHEEEL_CIRCUMFERENCE_CM) * (CSC_WHEEL_REV_VAL))
                        * ((uint32) WHEEL_TIME_EVENT_UNIT)) / ((uint32) CSC_WHEEL_EV_TIME_VAL))
                            * ((uint32) MS_TO_KMH_COEFITIENT)) / ((uint32) INT_DIVIDER);
        }

        if(0u != (cscFlags & CRANK_REV_DATA_PRESENT))
        {
            csValue[len++] = LO8(crankRev);
            csValue[len++] = HI8(crankRev);
            
            csValue[len++] = LO8(lastCrankEvTime);
            csValue[len++] = HI8(lastCrankEvTime);

            cscCadenceRpm = (((uint32) CSC_CRANK_REV_VAL) * ((uint32) WHEEL_TIME_EVENT_UNIT) * 60u) /
                               ((uint32) CSC_CRANK_EV_TIME_VAL);
        }
        
        /* Send Characteristic value to peer device */
        apiResult = CyBle_CscssSendNotification(cyBle_connHandle, CYBLE_CSCS_CSC_MEASUREMENT, len, csValue);

        if(CYBLE_ERROR_OK == apiResult)
        {
            printf("CscssSendNotification, ");
            printf("Wheel Revolution: %ld, ", wheelRev);
            printf("Wheel Time: %ld s, ", lastWheelEvTime / CSC_TIME_PER_SEC);
            printf("Crank Revolution: %ld, ", crankRev);
            printf("Crank Time: %ld s, ", lastCrankEvTime / CSC_TIME_PER_SEC);
            printf("Speed: %d.%2.2d km/h, ", cscSpeed/100u, cscSpeed%100u);
            printf("Cadence: %d rpm\r\n", cscCadenceRpm);
        }
        else
        {
            printf("Error #%d while sending CSC Measurement Characteristic notification\r\n", apiResult);
        }
    }
    
    if(((speedSimulation & CSCS_INDICATION_ENABLE) != 0u) && (speedCPresponse != 0u))
    {
        apiResult = CyBle_CscssSendIndication(cyBle_connHandle, CYBLE_CSCS_SC_CONTROL_POINT, 
                                                 scCPResponse[CYBLE_CSCS_SC_CP_RESP_LEN_IDX], scCPResponse + 1u);
        printf("CyBle_CscsSendIndication(CYBLE_CSCS_SC_CONTROL_POINT), API result: %x \r\n", apiResult);
        speedCPresponse = 0u;
    }
}


/*******************************************************************************
* Function Name: CscsInit()
********************************************************************************
*
* Summary:
*   Initializes the CSC service.
*
*******************************************************************************/
void CscsInit(void)
{
    uint8 cscMeasurementData[CSC_CHAR_LENGTH];
    
    /* Register service specific callback function */
    CyBle_CscsRegisterAttrCallback(CscsCallback);

    /* Get initial value of CSC Measurement Characteristic */
    CyBle_CscssGetCharacteristicValue(CYBLE_CSCS_CSC_MEASUREMENT, CSC_CHAR_LENGTH, cscMeasurementData);
    
    /* Set initial CSC Characteristic flags as per values set in the customizer */
    cscFlags = cscMeasurementData[0u];
}


/* [] END OF FILE */
