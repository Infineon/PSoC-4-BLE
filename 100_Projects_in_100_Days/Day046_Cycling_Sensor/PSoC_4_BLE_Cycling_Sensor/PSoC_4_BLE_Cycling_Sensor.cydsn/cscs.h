/*******************************************************************************
* File Name: cscs.h
*
* Version 1.0
*
* Description:
*  Contains the function prototypes and constants used by Cycling Speed and 
*  Cadence Service.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "common.h"


/***************************************
*          Constants
***************************************/
#define WHEEL_REV_DATA_PRESENT              (0x01)
#define CRANK_REV_DATA_PRESENT              (0x02)

/* SC CP Characteristic constants */
#define SC_CP_CHAR_LENGTH_3BYTES            (3u)
#define SC_CP_CHAR_LENGTH_4BYTES            (4u)

/* RSC Characteristic constants */
#define CSC_CHAR_LENGTH                     (11u)

#define CSC_CHAR_FLAGS_OFFSET               (0u)
#define CSC_CHAR_CUMULATIVE_WHEEL_REV       (1u)
#define CSC_CHAR_LAST_WHEEL_EV_TIME         (5u)
#define CSC_CHAR_CUMULATIVE_CRANK_REV       (7u)
#define CSC_CHAR_LAST_CRANK_EV_TIME         (9u)

#define CSC_TIME_PER_SEC                    (1024u) 
#define CSC_CRANK_EV_TIME_VAL               (1024u)
#define CSC_WHEEL_EV_TIME_VAL               (CSC_CRANK_EV_TIME_VAL)

#define CSC_CRANK_REV_VAL                   (0x02u)
#define CSC_WHEEL_REV_VAL                   (0x05u)

#define CSC_WHEEL_REV_INIT_VAL              (0x000077FFul)

/* SC Control Point Characteristic fields indexes */
#define CYBLE_CSCS_SC_CP_OP_CODE_IDX        (0u)
#define CYBLE_CSCS_SC_CUM_VAL_BYTE0_IDX     (1u)
#define CYBLE_CSCS_SC_CUM_VAL_BYTE1_IDX     (2u)
#define CYBLE_CSCS_SC_CUM_VAL_BYTE2_IDX     (3u)
#define CYBLE_CSCS_SC_CUM_VAL_BYTE3_IDX     (4u)
#define CYBLE_CSCS_SENSOR_LOC_IDX           (1u)


#define CYBLE_CSCS_SC_CP_RESP_LEN_IDX       (0u)
#define CYBLE_CSCS_SC_CP_RESP_OP_CODE_IDX   (1u)
#define CYBLE_CSCS_SC_CP_REQ_OP_CODE_IDX    (2u)
#define CYBLE_CSCS_SC_CP_RESP_VALUE_IDX     (3u)
#define CYBLE_CSCS_SC_CP_RESP_PARAMETER_IDX (4u)

#define CSCS_NOTIFICATION_ENABLE            (1u)
#define CSCS_INDICATION_ENABLE              (2u)

/* Speed calculation coefficients */
#define WHEEEL_CIRCUMFERENCE_CM             (210u)
#define MS_TO_KMH_COEFITIENT                (36u)
#define INT_DIVIDER                         (10u)
#define WHEEL_TIME_EVENT_UNIT               (1024u)

/* Supported sensor locations */
#define TOP_OF_SHOE                         (1u)
#define IN_SHOE                             (2u)
#define LEFT_CRANK                          (5u)
#define RIGHT_CRANK                         (6u)

#define NUM_SUPPORTED_SENSORS               (4u)

/***************************************
*       Function Prototypes
***************************************/
void CscsCallback(uint32 event, void *eventParam);
void CscsInit(void);
void SimulateCyclingSpeed(void);

