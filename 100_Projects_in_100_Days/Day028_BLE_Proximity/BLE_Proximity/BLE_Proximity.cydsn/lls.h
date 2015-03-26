/*******************************************************************************
* File Name: lls.h
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
*       Function Prototypes
***************************************/
void TpsServiceAppEventHandler(uint32 event, void *eventParam);


/***************************************
*      External data references
***************************************/
extern uint8                llsAlertTOCounter;
extern uint8                alertLevel;

/* [] END OF FILE */
