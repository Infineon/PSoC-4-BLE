/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates simultaneous usage of the BLE GAP Peripheral and 
*  Broadcaster roles. The device would connect to a peer device, while also 
*  broadcasting (non-connectable advertising) at the same time.
*
* Hardware Dependency:
*  CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit
*
********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
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
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*******************************************************************************/

/*******************************************************************************
* Included headers
*******************************************************************************/
#include <project.h>


/*******************************************************************************
* Function Definitions
*******************************************************************************/


/*******************************************************************************
* Function Name: StartBroadcast()
********************************************************************************
* Summary:
* This function starts the broadcast advertisements.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function starts a non-connectable, undirected advertisement on all the 
* advertisement channels (37, 38, 39).
*
*******************************************************************************/
void StartBroadcast(void)
{
    /* Define the Broadcast advertisement parameters - including the BD address,
     * advertisement type, advertisement interval etc.
     */
    CYBLE_GAPP_DISC_PARAM_T myAdvParameters = 
    {
        /* Minimum Advertisement interval = 100 ms */
        CYBLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN,
        
        /* Maximum Advertisement interval = Minimum = 100 ms */
        CYBLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN,
        
        /* Advertisement type - Non-connectable undirected (broadcasting) */
        CYBLE_GAPP_NON_CONNECTABLE_UNDIRECTED_ADV,
        
        /* Own BD address type - Public */
        CYBLE_GAP_ADDR_TYPE_PUBLIC,
        
        /* Address type of peer for directed advertisement - not applicable */
        CYBLE_GAP_ADDR_TYPE_PUBLIC,
        
        /* Address for directed advertisement - not applicable */
        {0},        
        
        /* Channel map - advertise on all 3 channels */
        0x07,       
        
        /* Advertisement filter policy - not applicable */
        0           
    };

    /* Define the advertisement data for the broadcast - this includes the 
     * advertisement flags and the device name advertised. The device name
     * used for Broadcasting is different from the device name used when 
     * we want to connect to another device.
     */
    CYBLE_GAPP_DISC_DATA_T myAdvData = 
    {
        {   
            2,      /* Length for Flags field */
            1,      /* Identifier that following data is Flags field */
            4,      /* Advertisement Flags - BR/EDR not supported */
            17,     /* Length for Complete Name field */
            0x09,   /* Identifier that following data is Complete Name field */
            
            /* Name */
            'B', 'r', 'o', 'a', 'd', 'c', 'a', 's', 't', 'e', 'r', ' ', 'O', 'n', 'l', 'y'
        },
        21          /* Total bytes in the advertisement packet */
    };
    
    /* There is no scan response data configured for this broadcast */
    CYBLE_GAPP_SCAN_RSP_DATA_T myScanRespData = 
    {
        {0},        /* Scan response packet */
        0           /* Length of the scan response packet */
    };
    
    /* Top level structure for holding all advertisement parameters */
    CYBLE_GAPP_DISC_MODE_INFO_T discoveryModeInfo = 
    {
        CYBLE_GAPP_NONE_DISC_BROADCAST_MODE,   /* Discoverable mode */
        &myAdvParameters,                      /* Advertisement parameters */
        &myAdvData,                            /* Advertisement data */
        &myScanRespData,                       /* Scan response data */
        0                                      /* Advertisement timeout: none */
    };
    
    /* Start broadcasting */
    CyBle_GappEnterDiscoveryMode(&discoveryModeInfo);
}


/*******************************************************************************
* Function Name: StopBroadcast()
********************************************************************************
* Summary:
* This function stops the broadcast advertisements.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* The function instructs the BLE stack to exit the Discovery mode and switch
* off all advertisement. The event CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP is 
* generated after a call to this function.
*
*******************************************************************************/
void StopBroadcast(void)
{
    /* Stop broadcasting (basically stop all forms of advertisement) */
    CyBle_GappExitDiscoveryMode();
}


/*******************************************************************************
* Function Name: StackEventHandler()
********************************************************************************
* Summary:
* Application callback function for the BLE events processing.
*
* Parameters:
* uint32 eventCode: The event to be processed
* void * eventParam: Pointer to hold the additional information associated 
*                    with an event
*
* Return:
* None
*
* Theory:
* The function is responsible for handling the events generated by the stack.
*
*******************************************************************************/
void StackEventHandler(uint32 event, void* eventParam)
{
    switch (event)
	{
		case CYBLE_EVT_STACK_ON: 
            /* BLE is initialized. Ready for a connectable undirected
             * advertisement. 
             */
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
            
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            /* This event comes when the advertisement either started or
             * stopped. Check to see whether the advertisement stopped.
             * If it did, start a connectable undirected advertisement again.
             */
            if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            }
            break;

        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            /* Device is connected; start broadcasting as well. */
            StartBroadcast();
            break;
            
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            /* Disconnected from the peer device; stop the broadcast. When
             * the broadcast is stopped, CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP
             * event is received, upon which a new connectable undirected
             * advertisement would be started.
             */
            StopBroadcast();
            break;
            
		default:
			break;
	}
}


/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
* Main function for the application.
*
* Parameters:
* None
*
* Return:
* None
*
* Theory:
* This function initializes the BLE component and then processes the BLE events 
* routinely, while also implementing low power in the system.
*
*******************************************************************************/
int main()
{
    CYBLE_LP_MODE_T bleMode;
    uint8 interruptStatus;
    
    /* Enable global interrupts */
    CyGlobalIntEnable; 

    /* Initialize BLE component */
    CyBle_Start(StackEventHandler);

	while(1) 
    {   
        /* Process incoming BLE events */
        CyBle_ProcessEvents();
        
        /* Low power implementation for BLE and the system */
        bleMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
        interruptStatus = CyEnterCriticalSection();
        if(bleMode == CYBLE_BLESS_DEEPSLEEP)
        {
            if((CyBle_GetBleSsState() == CYBLE_BLESS_STATE_ECO_ON) ||
               (CyBle_GetBleSsState() == CYBLE_BLESS_STATE_DEEPSLEEP))
            {
                /* Either BLE is in deep sleep or the crystal is 
                 * just starting up. Enter system deep sleep.
                 */
                CySysPmDeepSleep();
            }
        }
        else
        {
            if(CyBle_GetBleSsState() != CYBLE_BLESS_STATE_EVENT_CLOSE)
            {
                /* BLE is active but it is safe to enter CPU sleep */
                CySysPmSleep();
            }
        }
        CyExitCriticalSection(interruptStatus);
	}   
}  


/* [] END OF FILE */

