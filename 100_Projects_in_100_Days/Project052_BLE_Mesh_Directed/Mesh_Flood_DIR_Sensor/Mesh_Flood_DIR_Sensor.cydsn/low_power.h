/*******************************************************************************
* File Name: low_power.h
*
* Version: 1.0
*
* Description:
*  This file contains the headers and constants for low power API.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(LOW_POWER_H)
#define LOW_POWER_H
#include "project.h"

/*****************************************************
*                  Enums and macros
*****************************************************/
#define IMO_FREQUENCY_3MHZ			(3)
#define IMO_FREQUENCY_12MHZ			(12)

/*****************************************************
*                  Function Declarations
*****************************************************/
void HandleLowPowerMode(void);
	
#endif
/* [] END OF FILE */
