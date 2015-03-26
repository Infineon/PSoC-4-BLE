/*******************************************************************************
* File Name: lls.c
*
* Description:
*  This file contains Link Loss Service callback handler function.
* 
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <common.h>
#include "lls.h"


/***************************************
*        Global Variables
***************************************/
uint8                llsAlertTOCounter;
uint8                alertLevel;


/*******************************************************************************
* Function Name: LlsServiceAppEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component,
*  which are specific to Link Loss Service.
*
* Parameters:
*  uint8 event:       Write Request event from the CYBLE component.
*  void* eventParams: A structure instance of CYBLE_LLS_CHAR_VALUE_T type.
*
* Return:
*  None
*
*******************************************************************************/
void LlsServiceAppEventHandler(uint32 event, void *eventParam)
{
    CYBLE_LLS_CHAR_VALUE_T * llsWrReqValueParam;
    
    /* There is only one event present in Link Loss Service for Server GATT role.
    *  'CYBLE_EVT_LLSS_WRITE_CHAR_REQ' - write request to set new alert level. 
    */
    if (event == CYBLE_EVT_LLSS_WRITE_CHAR_REQ)
    {
        printf("Write LLS Alert Level request received \r\n");

        llsWrReqValueParam = (CYBLE_LLS_CHAR_VALUE_T *) eventParam;

        if(CYBLE_LLS_ALERT_LEVEL_SIZE == llsWrReqValueParam->value->len)
        {
            alertLevel = llsWrReqValueParam->value->val[0u];

            switch (alertLevel)
            {
            case CYBLE_NO_ALERT:
                printf("Alert Level for LLS is set to \"No Alert\" \r\n");
                break;

            case CYBLE_MILD_ALERT:
                printf("Alert Level for LLS is set to \"Mild Alert\" \r\n");
                break;

            case CYBLE_HIGH_ALERT:
                printf("Alert Level for LLS is set to \"High Alert\" \r\n");
                break;

            default:
                printf("Incorrect value of Alert Level for LLS was received\r\n");
                break;
            }
        }
    }
}



/* [] END OF FILE */
