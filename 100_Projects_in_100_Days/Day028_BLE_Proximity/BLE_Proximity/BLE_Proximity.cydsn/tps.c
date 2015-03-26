/*******************************************************************************
* File Name: tps.c
*
* Description:
*  This file contains Tx Power Service callback handler function.
* 
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <common.h>
#include "tps.h"


/***************************************
*        Global Variables
***************************************/
volatile uint8 tps_notification_enabled = 0;

/*******************************************************************************
* Function Name: TpsServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Tx Power Service.
*
* Parameters:
*  uint8 event:       Write Command event from the CYBLE component.
*  void* eventParams: A structure instance of CYBLE_GATT_HANDLE_VALUE_PAIR_T
*                     type.
*
* Return:
*  None
*
*******************************************************************************/
void TpsServiceAppEventHandler(uint32 event, void *eventParam)
{
    switch(event)
    {
    /***************************************
    *        TPS Server events
    ***************************************/
    case CYBLE_EVT_TPSS_NOTIFICATION_ENABLED:
        printf("TPS notification enabled\r\n");
		/* Set TPS notification enabled flag to indicate to application
		* that TPS notifications can be sent */
		tps_notification_enabled = 1;
        break;
    case CYBLE_EVT_TPSS_NOTIFICATION_DISABLED:
        printf("TPS notification disabled\r\n");
		/* Reset TPS notification enabled flag */
		tps_notification_enabled = 0;
        break;

    /***************************************
    *        TPS Client events
    ***************************************/
    case CYBLE_EVT_TPSC_NOTIFICATION:
        break;
    case CYBLE_EVT_TPSC_READ_CHAR_RESPONSE:
        break;
    case CYBLE_EVT_TPSC_READ_DESCR_RESPONSE:
        break;
    case CYBLE_EVT_TPSC_WRITE_DESCR_RESPONSE:
        break;
    default:
        break;
    }
}


/*******************************************************************************
* Function Name: ConvertTxPowerlevelToInt8
********************************************************************************
*
* Summary:
*  Converts Tx Power Level from CYBLE_BLESS_PWR_LVL_T to int8.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
int ConvertTxPowerlevelToInt8(CYBLE_BLESS_PWR_LVL_T pwrLevel)
{
    int8 intPwrLevel = 0;

    switch (pwrLevel)
    {
    case CYBLE_LL_PWR_LVL_NEG_18_DBM:
        intPwrLevel = -18;
        break;
    case CYBLE_LL_PWR_LVL_NEG_12_DBM:
        intPwrLevel = -12;
        break;
    case CYBLE_LL_PWR_LVL_NEG_6_DBM:
        intPwrLevel = -6;
        break;
    case CYBLE_LL_PWR_LVL_NEG_3_DBM:
        intPwrLevel = -3;
        break;
    case CYBLE_LL_PWR_LVL_NEG_2_DBM:
        intPwrLevel = -2;
        break;
    case CYBLE_LL_PWR_LVL_NEG_1_DBM:
        intPwrLevel = -1;
        break;
    case CYBLE_LL_PWR_LVL_0_DBM:
        intPwrLevel = 0;
        break;
    case CYBLE_LL_PWR_LVL_3_DBM:
        intPwrLevel = 3;
        break;

    default:
        break;
    }

    return(intPwrLevel);
}


/*******************************************************************************
* Function Name: DecreaseTxPowerLevelValue
********************************************************************************
*
* Summary:
*  Decreases the Tx Power level by one scale lower.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void DecreaseTxPowerLevelValue(CYBLE_BLESS_PWR_LVL_T * pwrLevel)
{
    switch (*pwrLevel)
    {
    case CYBLE_LL_PWR_LVL_NEG_18_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_3_DBM;
        break;
    case CYBLE_LL_PWR_LVL_NEG_12_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_NEG_18_DBM;
        break;
    case CYBLE_LL_PWR_LVL_NEG_6_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_NEG_12_DBM;
        break;
    case CYBLE_LL_PWR_LVL_NEG_3_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_NEG_6_DBM;
        break;
    case CYBLE_LL_PWR_LVL_NEG_2_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_NEG_3_DBM;
        break;
    case CYBLE_LL_PWR_LVL_NEG_1_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_NEG_2_DBM;
        break;
    case CYBLE_LL_PWR_LVL_0_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_NEG_1_DBM;
        break;
    case CYBLE_LL_PWR_LVL_3_DBM:
        *pwrLevel = CYBLE_LL_PWR_LVL_0_DBM;
        break;

    default:
        break;
    }
}


/* [] END OF FILE */
