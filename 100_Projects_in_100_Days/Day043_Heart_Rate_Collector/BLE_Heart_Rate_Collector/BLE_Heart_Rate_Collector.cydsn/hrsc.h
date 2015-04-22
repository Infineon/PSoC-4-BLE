/*******************************************************************************
* File Name: hrsc.h
*
* Version 1.0
*
* Description:
*  HRS service related code header.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <project.h>
#include "common.h"


/***************************************
*        Constant definitions
***************************************/
/* Definitions for Heart Rate Measurement characteristic */
#define CYBLE_HRS_HRM_HRVAL16           (0x01u)
#define CYBLE_HRS_HRM_SC_SPRT           (0x04u)
#define CYBLE_HRS_HRM_SC_STAT           (0x02u)
#define CYBLE_HRS_HRM_ENEXP             (0x08u)
#define CYBLE_HRS_HRM_RRINT             (0x10u)
#define CYBLE_HRS_HRM_CHAR_LEN          (20u)        /* for default 23-byte MTU */
/* RR-Interval buffer size = ((char size)-(flags: 1 byte)-(Heart Rate: min 1 byte))/(RR-Interval size: 2 bytes) */
#define CYBLE_HRS_HRM_RRSIZE            ((CYBLE_HRS_HRM_CHAR_LEN - 2u) / 2u)
#define CYBLE_ENERGY_EXPENDED_MAX_VALUE (0xFFFFu)   /* kilo Joules */
#define CYBLE_HRS_RRCNT_OL              (0x80u)

#define CYBLE_HRS_HRM_NTF_ENABLE        CYBLE_CCCD_NOTIFICATION
#define CYBLE_HRS_HRM_NTF_DISABLE       (0x0000u)


/***************************************
*            Data Types
***************************************/
/* Heart Rate Measurement characteristic data structure type */
typedef struct
{
    uint8 flags;
    uint16 heartRateValue;
    uint16 energyExpendedValue;
    uint16 rrInterval[CYBLE_HRS_HRM_RRSIZE];
}CYBLE_HRS_HRM_T;

/* Body Sensor Location characteristic value type */
typedef enum
{
    CYBLE_HRS_BODY_SENSOR_LOCATION_OTHER,
    CYBLE_HRS_BODY_SENSOR_LOCATION_CHEST,
    CYBLE_HRS_BODY_SENSOR_LOCATION_WRIST,
    CYBLE_HRS_BODY_SENSOR_LOCATION_FINGER,
    CYBLE_HRS_BODY_SENSOR_LOCATION_HAND,
    CYBLE_HRS_BODY_SENSOR_LOCATION_EAR_LOBE,
    CYBLE_HRS_BODY_SENSOR_LOCATION_FOOT
}CYBLE_HRS_BSL_T;

/***************************************
*        Function Prototypes
***************************************/

void HeartRateCallBack(uint32 event, void* eventParam);
void HrsInit(void);
CYBLE_API_RESULT_T HrscConfigHeartRateNtf(uint16 configuration);
void HrscUnPackHrm(CYBLE_GATT_VALUE_T* value);
uint16 HrscGetRRInterval(uint8 rrIntervalNumber);
CYBLE_API_RESULT_T HrscResetEnergyExpendedCounter(void);


/* Functions generated in macros */


/*******************************************************************************
* Function Name: CyBle_HrscGetHeartRate
********************************************************************************
*
* Summary:
*   Gets the Heart Rate value from the
*   Heart Rate Measurument characteristic structure
*
* Parameters:
*   None
*
* Return:
*   uint16 heartRate.
*
*******************************************************************************/
#define HrscGetHeartRate()\
            (hrsHeartRate.heartRateValue)


/*******************************************************************************
* Function Name: CyBle_HrscGetEnergyExpended
********************************************************************************
*
* Summary:
*   Gets the Energy Expended value from the
*   Heart Rate Measurument characteristic structure.
*
* Parameters:
*   None
*
* Return:
*   uint16 energyExpended value.
*
*******************************************************************************/
#define HrscGetEnergyExpended()\
            (hrsHeartRate.energyExpendedValue)


/*******************************************************************************
* Function Name: CyBle_HrscIsSensorContactSupported
********************************************************************************
*
* Summary:
*  Checks if the Sensor Contact feature is supported.
*
* Parameters:
*  None.
*
* Return:
*  bool: TRUE  - if the Sensor Contact feature is supported.
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrscIsSensorContactSupported()\
            (0u != (hrsHeartRate.flags & CYBLE_HRS_HRM_SC_SPRT))


/*******************************************************************************
* Function Name: CyBle_HrscIsSensorContactDetected
********************************************************************************
*
* Summary:
*  Checks if the Sensor Contact detected.
*
* Parameters:
*  None.
*
* Return:
*  bool: TRUE  - if the Sensor Contact feature is supported and
*                the sensor contact is detected.
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrscIsSensorContactDetected()\
            (HrscIsSensorContactSupported() &&\
                (0u != (hrsHeartRate.flags & CYBLE_HRS_HRM_SC_STAT)))


/*******************************************************************************
* Function Name: HrscReadBodySensorLocation
********************************************************************************
*
* Summary:
*   Initiates the Read Characteristic Request for Heart Rate Service Body Sensor
*   Location characteristic
*
* Parameters:
*   CYBLE_CONN_HANDLE_T cyBle_connHandle: connection handle which
*                           consists of device ID and ATT connection ID.
*
* Return:
*   uint16: API result will state if API succeeded
*           (CYBLE_ERROR_OK) or failed with error codes:
*   CYBLE_ERROR_OK - The request is sent successfully;
*   CYBLE_ERROR_INVALID_PARAMETER - Validation of input parameter failed.
*******************************************************************************/
#define HrscReadBodySensorLocation()\
            (CyBle_HrscGetCharacteristicValue(cyBle_connHandle, CYBLE_HRS_BSL))

/*******************************************************************************
* Function Name: HrscGetBodySensorLocation
********************************************************************************
*
* Summary:
*   Gets the Body Sensor Location characteristic value from the internal
*   variable.
*
* Parameters:
*   None.
*
* Return:
*   CYBLE_HRS_BSL_T Body Sensor Location value.
*
*******************************************************************************/
#define HrscGetBodySensorLocation() (cyBle_hrscBodySensorLocation)


/***************************************
*      External data references
***************************************/
extern CYBLE_HRS_HRM_T hrsHeartRate;     
extern uint8 hrsNotification;


/* [] END OF FILE */
