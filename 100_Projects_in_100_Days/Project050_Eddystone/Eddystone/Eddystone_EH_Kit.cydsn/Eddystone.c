/*******************************************************************************
* File Name: Eddystone.c
* 
* Version 1.0
*
* Author - udyg@cypress.com
* 
* Description:
*  Application for Eddystone beacon.
*
********************************************************************************
* Copyright 2010-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <project.h>
#include <stdbool.h>
#include "Eddystone.h"
#include "WatchdogTimer.h"

static bool isRegularAdvScheduled = true;

/* BLE stack event handler */
void BLE_AppEventHandler(uint32 event, void* eventParam)
{
    CYBLE_API_RESULT_T apiResult;

    switch (event)
	{
        /**********************************************************
        *                       General Events
        ***********************************************************/

        /* This event is received when component is Started */
        case CYBLE_EVT_STACK_ON: 
            
            /* Start with a regular UID/URL packet */
            isRegularAdvScheduled = true;
            ConfigureAdvPacket(isRegularAdvScheduled);

            initCounter = 0;
            WDT_EnableWcoCounter();     /* Enable WDT's WCO counter (counter 0) */
        break;
            
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            if(CyBle_GetState() != CYBLE_STATE_ADVERTISING)
            {
                /* On advertisement timeout, restart advertisement */

            #if (IS_EDDYSTONE_TLM_PRESENT)
                if(true == isRegularAdvScheduled)
                {
                    /* Configure for TLM packet */
                    isRegularAdvScheduled = false;
                }
                else
                {
                    /* Configure for regular UID/URL packet */
                    isRegularAdvScheduled = true;
                }
                
                ConfigureAdvPacket(isRegularAdvScheduled);
            #endif
            
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_CUSTOM);
            }
        break;
            
		default:
        break;
	}
}

void ConfigureAdvPacket(bool isPacketRegular)
{
    /* Configure Eddystone packets at run-time. */
    
    /* Flags are untouched */

    /* Service Solicitation data */
    cyBle_discoveryData.advData[3] = 0x03;      /* Length */
    cyBle_discoveryData.advData[4] = 0x03;      /* Complete List of Service UUIDs */
    cyBle_discoveryData.advData[5] = 0xAA;      /* LSB - Eddystone service */
    cyBle_discoveryData.advData[6] = 0xFE;      /* MSB - Eddystone service */

    if(isPacketRegular == true)
    {
        /* ADV packet timeout */
    #if (IS_EDDYSTONE_TLM_PRESENT)
        cyBle_discoveryModeInfo.advTo = EDDYSTONE_REGULAR_TIMEOUT_SECONDS;
    #else
        cyBle_discoveryModeInfo.advTo = 0;  /* No timeout */
    #endif
        
    #if (EDDYSTONE_IMPLEMENTATION == EDDYSTONE_UID)

        /* Service Data */
        cyBle_discoveryData.advData[7] = 0x17;      /* Length */
        cyBle_discoveryData.advData[8] = 0x16;      /* Service Data */
        cyBle_discoveryData.advData[9] = 0xAA;      /* LSB - Eddystone Service */
        cyBle_discoveryData.advData[10] = 0xFE;     /* MSB - Eddystone Service */

        cyBle_discoveryData.advData[11] = 0x00;     /* Signifies Eddystone UID */

        cyBle_discoveryData.advData[12] = 0xF2;     /* Ranging data: -14dB*/

        /* SHA-1 hash of the FQDN (cypress.com) is calculated and its 
         * first 10 bytes are placed here as the Namespace ID, MSB first.
         */
        cyBle_discoveryData.advData[13] = 0xCB;     /* NID[0] */
        cyBle_discoveryData.advData[14] = 0x6F;     /* NID[1] */
        cyBle_discoveryData.advData[15] = 0x15;     /* NID[2] */
        cyBle_discoveryData.advData[16] = 0xCE;     /* NID[3] */
        cyBle_discoveryData.advData[17] = 0x20;     /* NID[4] */
        cyBle_discoveryData.advData[18] = 0x2A;     /* NID[5] */
        cyBle_discoveryData.advData[19] = 0xCE;     /* NID[6] */
        cyBle_discoveryData.advData[20] = 0x15;     /* NID[7] */
        cyBle_discoveryData.advData[21] = 0x6F;     /* NID[8] */
        cyBle_discoveryData.advData[22] = 0xCB;     /* NID[9] */

        /* Instance ID - randomly created */
        cyBle_discoveryData.advData[23] = 0x01;     /* BID[0] */
        cyBle_discoveryData.advData[24] = 0x01;     /* BID[1] */
        cyBle_discoveryData.advData[25] = 0x01;     /* BID[2] */
        cyBle_discoveryData.advData[26] = 0x01;     /* BID[3] */
        cyBle_discoveryData.advData[27] = 0x01;     /* BID[4] */
        cyBle_discoveryData.advData[28] = 0x01;     /* BID[5] */
        
        cyBle_discoveryData.advData[29] = 0x00;     /* Reserved */
        cyBle_discoveryData.advData[30] = 0x00;     /* Reserved */
        
        /* ADV packet length */
        cyBle_discoveryData.advDataLen = 31;
        
    #elif (EDDYSTONE_IMPLEMENTATION == EDDYSTONE_URL)
        
        /* Service Data */
        cyBle_discoveryData.advData[7] = 0x0E;      /* Length */
        cyBle_discoveryData.advData[8] = 0x16;      /* Service Data */
        cyBle_discoveryData.advData[9] = 0xAA;      /* LSB - Eddystone Service */
        cyBle_discoveryData.advData[10] = 0xFE;     /* MSB - Eddystone Service */

        cyBle_discoveryData.advData[11] = 0x10;     /* Signifies Eddystone URL */

        cyBle_discoveryData.advData[12] = 0xF2;     /* Ranging data: -14dB*/

        cyBle_discoveryData.advData[13] = 0x00;     /* URL scheme- http://www. */

        cyBle_discoveryData.advData[14] = 0x63;     /* Encoded URL - 'c' */
        cyBle_discoveryData.advData[15] = 0x79;     /* Encoded URL - 'y' */
        cyBle_discoveryData.advData[16] = 0x70;     /* Encoded URL - 'p' */
        cyBle_discoveryData.advData[17] = 0x72;     /* Encoded URL - 'r' */
        cyBle_discoveryData.advData[18] = 0x65;     /* Encoded URL - 'e' */
        cyBle_discoveryData.advData[19] = 0x73;     /* Encoded URL - 's' */
        cyBle_discoveryData.advData[20] = 0x73;     /* Encoded URL - 's' */
        cyBle_discoveryData.advData[21] = 0x00;     /* Expansion - .com */
        
        /* ADV packet length */
        cyBle_discoveryData.advDataLen = 22;

    #endif
    }

    else
    {
        /* ADV packet timeout */
        cyBle_discoveryModeInfo.advTo = EDDYSTONE_TLM_TIMEOUT_SECONDS;
        
    #if (IS_EDDYSTONE_TLM_PRESENT)
        
        /* Telemetry packets */
    
        /* Service Data */
        cyBle_discoveryData.advData[7] = 0x11;      /* Length */
        cyBle_discoveryData.advData[8] = 0x16;      /* Service Data */
        cyBle_discoveryData.advData[9] = 0xAA;      /* LSB - Eddystone Service */
        cyBle_discoveryData.advData[10] = 0xFE;     /* MSB - Eddystone Service */

        cyBle_discoveryData.advData[11] = 0x20;     /* Signifies Eddystone TLM */

        cyBle_discoveryData.advData[12] = 0x00;     /* TLM version */
        
        /* Battery voltage in mV (1 mV per bit) */
        cyBle_discoveryData.advData[13] = 0x00;     
        cyBle_discoveryData.advData[14] = 0x00;     

        /* Beacon temperature in Celsius (8.8 fixed point notation) */
        cyBle_discoveryData.advData[15] = 0x80;     
        cyBle_discoveryData.advData[16] = 0x00;     

        /* Advertising PDU count since power-up or reboot */
        cyBle_discoveryData.advData[17] = 0x00;     
        cyBle_discoveryData.advData[18] = 0x00;     
        cyBle_discoveryData.advData[19] = 0x00;     
        cyBle_discoveryData.advData[20] = 0x00;     
        
        /* Time since power-on or reboot */
        cyBle_discoveryData.advData[21] = 0x00;     
        cyBle_discoveryData.advData[22] = 0x00;     
        cyBle_discoveryData.advData[23] = 0x00;     
        cyBle_discoveryData.advData[24] = 0x00;     
        
        
        /* ADV packet length */
        cyBle_discoveryData.advDataLen = 25;
        
    #endif /* #if (IS_EDDYSTONE_TLM_PRESENT) */
    }
}


/* [] END OF FILE */
