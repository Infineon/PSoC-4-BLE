/*******************************************************************************
* File Name: tps.c
*
* Description:
*  This file contains Tx Power Service callback handler function.
* 
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <common.h>
#include <stdio.h>


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
    
    CYBLE_TPS_CHAR_VALUE_T CharValue;
   
    switch(event)
    {
    
    /***************************************
    *        TPS Client events
    ***************************************/
    case CYBLE_EVT_TPSC_NOTIFICATION:
            CharValue=*(CYBLE_TPS_CHAR_VALUE_T*)eventParam;
            printf("Notification received. Tx Power Level:%d dBm\r\n",(int8)(CharValue.value->val[0]));
        break;
            
    case CYBLE_EVT_TPSC_READ_CHAR_RESPONSE:
            CharValue=*(CYBLE_TPS_CHAR_VALUE_T*)eventParam;
            printf("Read Char Response. Tx Power Level: %d dBm\r\n",(int8)(CharValue.value->val[0]));
        break;
                      
    case CYBLE_EVT_TPSC_WRITE_DESCR_RESPONSE:
             printf("Write Descr Response\r\n");
        break;
            
     case CYBLE_EVT_GATTC_ERROR_RSP:
             printf("Error Rsp\r\n");
        break;
     
    default:                 
             printf("default:\r\n");            
        break;
    }
}

/* [] END OF FILE */
