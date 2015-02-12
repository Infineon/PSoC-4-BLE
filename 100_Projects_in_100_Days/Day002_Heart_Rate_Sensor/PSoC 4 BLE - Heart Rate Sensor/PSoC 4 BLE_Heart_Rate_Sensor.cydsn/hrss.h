/*******************************************************************************
* File Name: hrss.h
*
* Version 1.0
*
* Description:
*  HRS service related code header.
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
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

/* Energy expended is typically only included in the Heart Rate Measurement characteristic
*  once every 10 measurements at a regular interval.
*/
#define ENERGY_EXPECTED_TIMEOUT         (10u)       /* 10 seconds */

#define SIM_HEART_RATE_MIN              (60u)       /* Minimum simulated heart rate measurement */
#define SIM_HEART_RATE_MAX              (300u)      /* Maximum simulated heart rate measurement */
#define SIM_HEART_RATE_INCREMENT        (12u)       /* Value by which the heart rate is incremented */
#define SIM_ENERGY_EXPENDED_INCREMENT   (20u)       /* Value by which the Energy is incremented */
#define SENSOR_CONTACT_DETECTED         (0x01u)


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
void SimulateHeartRate(void);
void HrsInit(void);
void HrssSetEnergyExpended(uint16 energyExpended);
void HrssAddRrInterval(uint16 rrIntervalValue);
void HrssSendHeartRateNtf(void);
void HrssSetBodySensorLocation(CYBLE_HRS_BSL_T location);
uint16 HrssGetHrmDescriptor(void);

/* Functions generated in macros */

/*******************************************************************************
* Function Name: CyBle_HrssSetHeartRate
********************************************************************************
*
* Summary:
*  Sets Heart Rate value into the Heart Rate Measurument characteristic.
*
* Parameters:
*  uint16 heartRate - Heart Rate value to be set.
*
* Return:
*  None
*
*******************************************************************************/
#define HrssSetHeartRate(heartRate)\
            (hrsHeartRate.heartRateValue = (heartRate))


/*******************************************************************************
* Function Name: CyBle_HrssSensorContactSupportEnable
********************************************************************************
*
* Summary:
*  Sets the Sensor Contact Feature Support bit.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
#define HrssSensorContactSupportEnable()\
            (hrsHeartRate.flags |= CYBLE_HRS_HRM_SC_SPRT)


/*******************************************************************************
* Function Name: CyBle_HrssSensorContactSupportDisable
********************************************************************************
*
* Summary:
*  Clears both Sensor Contact Feature bits.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
#define HrssSensorContactSupportDisable()\
            (hrsHeartRate.flags &= (uint8) ~(CYBLE_HRS_HRM_SC_SPRT | CYBLE_HRS_HRM_SC_STAT))


/*******************************************************************************
* Function Name: CyBle_HrssSensorContactIsDetected
********************************************************************************
*
* Summary:
*  Sets the Sensor Contact Status bit.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
#define HrssSensorContactIsDetected()\
            (hrsHeartRate.flags |= CYBLE_HRS_HRM_SC_SPRT | CYBLE_HRS_HRM_SC_STAT)


/*******************************************************************************
* Function Name: CyBle_HrssSensorContactIsUndetected
********************************************************************************
*
* Summary:
*  Clears the Sensor Contact Status bit.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
#define HrssSensorContactIsUndetected()\
            (hrsHeartRate.flags &= (uint8) ~CYBLE_HRS_HRM_SC_STAT)


/*******************************************************************************
* Function Name: CyBle_HrssIsRrIntervalBufferFull
********************************************************************************
*
* Summary:
*  Checks if the RR-Interval buffer is full.
*
* Parameters:
*  None.
*
* Return:
*  bool: TRUE  - if the buffer is full,
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrssIsRrIntervalBufferFull()\
            (hrssRrIntCnt >= CYBLE_HRS_HRM_RRSIZE)

/*******************************************************************************
* Function Name: CyBle_HrssAreThereRrIntervals
********************************************************************************
*
* Summary:
*  Checks if there are any RR-Intervals in the buffer.
*
* Parameters:
*  None.
*
* Return:
*  bool: TRUE  - if there is at least one RR-Interval,
*        FALSE - otherwise.
*
*******************************************************************************/
#define HrssAreThereRrIntervals()\
            ((hrsHeartRate.flags & CYBLE_HRS_HRM_RRINT) != 0u)


/***************************************
*      External data references
***************************************/
/* Heart Rate Measurement characteristic data structure */
extern CYBLE_HRS_HRM_T hrsHeartRate;
extern uint8 hrssRrIntPtr;
extern uint8 hrssRrIntCnt;
extern uint8 heartRateSimulation;

/* [] END OF FILE */
