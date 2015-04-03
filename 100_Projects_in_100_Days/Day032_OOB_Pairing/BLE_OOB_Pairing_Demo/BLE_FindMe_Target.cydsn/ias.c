/*******************************************************************************
* File Name: ias.c
*
* Description:
*  This file contains Immediate Alert Service callback handler function.
* 
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


/*******************************************************************************
*   Included Headers
*******************************************************************************/
#include <project.h>
#include "ias.h"


/*******************************************************************************
* Function Name: IasEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Immediate Alert Service.
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
void IasEventHandler(uint32 event, void *eventParam)
{
    /* Alert Level Characteristic write event */
    if(event == CYBLE_EVT_IASS_WRITE_CHAR_CMD)
    {
        uint8 alertLevel;

        /* Data structure that is returned as eventParam */
        CYBLE_IAS_CHAR_VALUE_T *charValue = (CYBLE_IAS_CHAR_VALUE_T *)eventParam;

        /* Extract Alert Level value from the data structure */
        alertLevel = *((charValue->value->val));

        switch(alertLevel)
        {
            case NO_ALERT:
            PWM_WriteCompare(LED_NO_ALERT);
            break;

            case MILD_ALERT:
            PWM_WriteCompare(LED_MILD_ALERT);
            break;

            case HIGH_ALERT:
            PWM_WriteCompare(LED_HIGH_ALERT);
            break;
        }
    }
}


/* [] END OF FILE */
