/*******************************************************************************
* File Name: tps.h
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/***************************************
*          API Constants
***************************************/
#define TX_POWER_LEVEL_MAX                  (0)
#define TX_POWER_LEVEL_MIN                  (-18)


/***************************************
*       Function Prototypes
***************************************/
void LlsServiceAppEventHandler(uint32 event, void * eventParam);
int ConvertTxPowerlevelToInt8(CYBLE_BLESS_PWR_LVL_T pwrLevel);
void DecreaseTxPowerLevelValue(CYBLE_BLESS_PWR_LVL_T * pwrLevel);


/***************************************
*      External data references
***************************************/


/* [] END OF FILE */
