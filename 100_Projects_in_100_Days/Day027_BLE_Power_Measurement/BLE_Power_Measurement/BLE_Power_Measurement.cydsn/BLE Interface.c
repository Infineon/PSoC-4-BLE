/******************************************************************************
* Project Name		: BLE_Power_Measurement
* File Name			: BLE Interface.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 SP1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
* Owner				: kris@cypress.com
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international treaty
* provisions. Cypress hereby grants to licensee a personal, non-exclusive,
* non-transferable license to copy, use, modify, create derivative works of,
* and compile the Cypress Source Code and derivative works for the sole
* purpose of creating custom software in support of licensee product to be
* used only in conjunction with a Cypress integrated circuit as specified in
* the applicable agreement. Any reproduction, modification, translation,
* compilation, or representation of this software except as specified above 
* is prohibited without the express written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. 
*
* Use of this Software may be limited by and subject to the applicable Cypress
* software license agreement. 
*******************************************************************************/

/********************************************************************************
* Description -  This file contains routines that handle all the BLE interface  
* connection establishment procedures and event handlers.
********************************************************************************/

#include <BLE Interface.h>
#include <Configuration.h>
#include <project.h>
#include <stdio.h>

/***************************************
*    Function declarations
***************************************/
void BLE_StackEventHandler(uint32 event, void* eventParam);
void BLE_HeartRateCallBack(uint32 event, void* eventParam);

/***************************************
*    Global variables
***************************************/
uint8 connUpdate = 0;
uint8 notificationState = 0;
uint8 initCount = 0;

#if SEND_NOTIFICATIONS
uint8 notifyValue = INITIAL_HEART_RATE_VALUE;
uint8 heartRatePacket[2] = {0x01, INITIAL_HEART_RATE_VALUE};
#endif /* End of #if SEND_NOTIFICATIONS */

/*******************************************************************************
* Function Name: BLE_Engine_Start
********************************************************************************
*
* Summary:
*  Application level API for starting the BLE interface. The API internally calls
*  other BLE interface init APIs to setup the system.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
CYBLE_API_RESULT_T BLE_Engine_Start(void)
{
    CYBLE_API_RESULT_T apiResult;
    
    apiResult = CyBle_Start(BLE_StackEventHandler);
    
    /* HRS service initialization */
    CyBle_HrsRegisterAttrCallback(BLE_HeartRateCallBack);
   
    return apiResult;
}

/*******************************************************************************
* Function Name: BLE_Run
********************************************************************************
*
* Summary:
*  BLE interface processing engine. This API should be continuously called by 
*  the application in its main loop.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
inline void BLE_Run(void)
{
    CyBle_ProcessEvents();
    
    if(CyBle_GetState() == CYBLE_STATE_CONNECTED && connUpdate)
    {
        CYBLE_API_RESULT_T conResult;
        static CYBLE_GAP_CONN_UPDATE_PARAM_T hrmConnectionParam =
        {
            MINIMUM_CONNECTION_INTERVAL,        /* Minimum connection interval of 987 ms */
            MAXIMUM_CONNECTION_INTERVAL,        /* Maximum connection interval of 1000 ms */
            SLAVE_LATENCY,                      /* Slave latency */
            SUPERVISION_TIMEOUT                 /* Supervision timeout of 5 seconds */
        };

        conResult = CyBle_L2capLeConnectionParamUpdateRequest(cyBle_connHandle.bdHandle, &hrmConnectionParam);
        
        if(conResult != CYBLE_ERROR_OK)
        {
            connUpdate = 0;
        }
    }
    
#if SEND_NOTIFICATIONS
    if(CyBle_GetBleSsState() == CYBLE_BLESS_STATE_ECO_STABLE && notificationState)
    {
        if(initCount < NOTIFICATION_OFFSET) /* Wait for 'X' connection intervals before starting notifications */
        {
            initCount++;
        }
        else
        {
            notifyValue++;
            
#if NOTIF_INTERVAL_FOUR_SEC
            if(notifyValue % 4 == 0)    
#endif /* End of #if NOTIF_INTERVAL_FOUR_SEC */
            {
                if(notifyValue > 160)
                {
                    notifyValue = 60;   
                }
                heartRatePacket[1] = notifyValue;
               
                CyBle_HrssSendNotification(cyBle_connHandle, CYBLE_HRS_HRM, sizeof(heartRatePacket), heartRatePacket);
            }
        }
    }
#endif /* End of #if SEND_NOTIFICATIONS */

    /* Add your code to perform other BLE operations in main loop. Storing bonding information to flash is one example */
}

/*******************************************************************************
* Function Name: BLE_StackEventHandler
********************************************************************************
*
* Summary:
*   BLE stack generic event handler routine for handling connection, discovery, 
*   security etc. events.
*
* Parameters:  
*  event - event that triggered this callback
*  eventParam - parameters for the event.
*
* Return: 
*  None
*******************************************************************************/
void BLE_StackEventHandler(uint32 event, void* eventParam)
{
    CYBLE_BLESS_CLK_CFG_PARAMS_T clockConfig;
    CYBLE_API_RESULT_T apiResult;
    
    (void) eventParam;
    
    switch(event)
    {
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            notificationState = 0;
            /* Fall through */
        case CYBLE_EVT_STACK_ON:
        
#if ENABLE_900_XTAL_STARTUP
            clockConfig.bleLlClockDiv = CYBLE_LL_ECO_CLK_DIV_2; /* Set LL clock at 12MHz */
            clockConfig.bleLlSca = CYBLE_LL_SCA_031_TO_050_PPM; /* Set the slave clock accuracy to 50ppm */
            clockConfig.ecoXtalStartUpTime = CRYSTAL_STARTUP_TIME; /* 24 MHz crystal startup time to 900us */
            CyBle_SetBleClockCfgParam(&clockConfig);
#endif /* End of #if ENABLE_900_XTAL_STARTUP */

            /* Put the device into discoverable mode so that remote can search it. */
            apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            (void) apiResult;
        break;
            
        /* Add your code to handle other generic BLE stack events */
                
        default:
        
        break;
    }
}

/*******************************************************************************
* Function Name: BLE_HeartRateCallBack
********************************************************************************
*
* Summary:
*   This is an event callback function to receive events from the Heart Rate Service.
*
* Parameters:
*  event - the event code
*  *eventParam - the event parameters
*
*******************************************************************************/
void BLE_HeartRateCallBack(uint32 event, void* eventParam)
{
    (void) eventParam;
    
    switch(event)
    {
        case CYBLE_EVT_HRSS_NOTIFICATION_ENABLED:
#if CON_PARAM_UPDATE
            connUpdate = 1;
#endif  /* End of #if CON_PARAM_UPDATE */
            notificationState = 1;
        break;
        
        case CYBLE_EVT_HRSS_NOTIFICATION_DISABLED:
            notificationState = 0;
            connUpdate = 0;
        break;
    }
}

/* [] END OF FILE */
