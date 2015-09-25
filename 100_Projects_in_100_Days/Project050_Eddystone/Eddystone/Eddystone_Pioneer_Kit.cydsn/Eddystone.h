/*******************************************************************************
* File Name: Eddystone.h
* 
* Version 1.0
*
* Author - udyg@cypress.com
* 
* Description:
*  Header file for Eddystone.
*
********************************************************************************
* Copyright 2010-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <project.h>
#include <stdbool.h>
#include "Configuration.h"

extern void BLE_AppEventHandler(uint32 event, void* eventParam);
extern void ConfigureAdvPacket(bool isPacketRegular);

/* [] END OF FILE */
