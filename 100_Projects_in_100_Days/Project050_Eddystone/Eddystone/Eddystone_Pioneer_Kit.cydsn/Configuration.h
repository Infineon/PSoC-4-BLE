/*******************************************************************************
* File Name: Configuration.h
* 
* Version 1.0
*
* Author - udyg@cypress.com
* 
* Description:
*  File to configure various parameters in an Eddystone application.
*
********************************************************************************
* Copyright 2010-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <project.h>

#define EDDYSTONE_UID                       (1)
#define EDDYSTONE_URL                       (2)

/* Macros to set the advertisement timeout intervals */
#define EDDYSTONE_REGULAR_TIMEOUT_SECONDS   (25)   /* Forced to 0 if no TLM */
#define EDDYSTONE_TLM_TIMEOUT_SECONDS       (5)

/* Change this macro to decide whether to have TLM packets interleaved or not */
#define IS_EDDYSTONE_TLM_PRESENT            (1)

/* Change this macro value for corresponding implementation - UID or URL */
#define EDDYSTONE_IMPLEMENTATION            (EDDYSTONE_URL)


/* [] END OF FILE */
