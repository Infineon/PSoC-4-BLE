/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates BLE component as peripheral.Initially advertising
*  type is CYBLE_GAPP_CONNECTABLE_UNDIRECTED_ADV. After bonding it changes the 
*  advertising type to CYBLE_GAPP_CONNECTABLE_LOW_DC_DIRECTED_ADV
*  The device would connect to a peer device, which maintains the connection 
*  for a particular amount of time.
*
*  Hardware Dependency:
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
#include "common.h"

/***************************************
*        Global Variables
***************************************/
CYBLE_GAP_BONDED_DEV_ADDR_LIST_T  BondedDeviceList;
uint8 state=DISCONNECTED;
uint8 RemoveBondedDevice=FALSE;


/*******************************************************************************
* Function Name: StackEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the CYBLE Component.
*
* Parameters:
*  uint8 event:       Event from the CYBLE component.
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of event structure is described in the component
*                     datasheet.
*
* Return:
*  None
*
*******************************************************************************/
void StackEventHandler(uint32 event,void* eventparam)
{    
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
                /*Set advertising Time out as Infinite*/
                cyBle_discoveryModeInfo.advTo=0x00;
                CyBle_GapGetBondedDevicesList(&BondedDeviceList);
                if(BondedDeviceList.count!=0)
                {
                    cyBle_discoveryModeInfo.advParam->advType=CYBLE_GAPP_CONNECTABLE_LOW_DC_DIRECTED_ADV;
                    /*Set the type of address*/
                    cyBle_discoveryModeInfo.advParam->directAddrType=BondedDeviceList.bdAddrList->type;
                    /*Use bonded address for direct advertising*/
                    cyBle_discoveryModeInfo.advParam->directAddr[5]=BondedDeviceList.bdAddrList->bdAddr[5];
                    cyBle_discoveryModeInfo.advParam->directAddr[4]=BondedDeviceList.bdAddrList->bdAddr[4];
                    cyBle_discoveryModeInfo.advParam->directAddr[3]=BondedDeviceList.bdAddrList->bdAddr[3];
                    cyBle_discoveryModeInfo.advParam->directAddr[2]=BondedDeviceList.bdAddrList->bdAddr[2];
                    cyBle_discoveryModeInfo.advParam->directAddr[1]=BondedDeviceList.bdAddrList->bdAddr[1];
                    cyBle_discoveryModeInfo.advParam->directAddr[0]=BondedDeviceList.bdAddrList->bdAddr[0];
                }
                else
                {
                    cyBle_discoveryModeInfo.advParam->advType=CYBLE_GAPP_CONNECTABLE_UNDIRECTED_ADV;
                }
                CyBle_GappEnterDiscoveryMode(&cyBle_discoveryModeInfo);
            break;
                       
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
                if(state==DISCONNECTED)
                {
                    state=ADVERTISING;
                }
                else if(state==ADVERTISING)
                {
                    state=DISCONNECTED;
                }
            break;
                
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
                state=CONNECTED;   
                CyBle_GapAuthReq(cyBle_connHandle.bdHandle,&cyBle_authInfo);
            break;
                
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:        
                state=DISCONNECTED;
                /*After disconnection check the bonding information*/
                CyBle_GapGetBondedDevicesList(&BondedDeviceList);
                
                if(BondedDeviceList.count!=0)    /*If number of bonded devices is one then start Direct advertisement*/
                {                    
                    /*Set the Advertising type*/
                    cyBle_discoveryModeInfo.advParam->advType=CYBLE_GAPP_CONNECTABLE_LOW_DC_DIRECTED_ADV;
                    /*Set the type of address*/
                    cyBle_discoveryModeInfo.advParam->directAddrType=BondedDeviceList.bdAddrList->type;
                    /*Use bonded address for direct advertising*/
                    cyBle_discoveryModeInfo.advParam->directAddr[5]=BondedDeviceList.bdAddrList->bdAddr[5];
                    cyBle_discoveryModeInfo.advParam->directAddr[4]=BondedDeviceList.bdAddrList->bdAddr[4];
                    cyBle_discoveryModeInfo.advParam->directAddr[3]=BondedDeviceList.bdAddrList->bdAddr[3];
                    cyBle_discoveryModeInfo.advParam->directAddr[2]=BondedDeviceList.bdAddrList->bdAddr[2];
                    cyBle_discoveryModeInfo.advParam->directAddr[1]=BondedDeviceList.bdAddrList->bdAddr[1];
                    cyBle_discoveryModeInfo.advParam->directAddr[0]=BondedDeviceList.bdAddrList->bdAddr[0];                                                                                                   
                }
                else
                {
                    cyBle_discoveryModeInfo.advParam->advType=CYBLE_GAPP_CONNECTABLE_UNDIRECTED_ADV;
                }
                /*Enter discovery mode*/
                CyBle_GappEnterDiscoveryMode(&cyBle_discoveryModeInfo);
            break;
        
        default:
            break;
    }
    
}

/*******************************************************************************
* Function Name: HandleLeds
********************************************************************************
*
* Summary:
*  This is used to TURN ON and TURN OFF the LEDs based on the "state".
*
* Parameters:
*  None

* Return:
*  None
*
*******************************************************************************/
void HandleLeds()
{
    if(state==DISCONNECTED)
    {
        Advertising_LED_Write(LED_OFF);     /*TUNR OFF GREEN and RED LEDs*/
        Connection_LED_Write(LED_OFF);       
    }
    else if(state==ADVERTISING)
    {
        Advertising_LED_Write(LED_ON);      /*TURN ON RED LED*/
        Connection_LED_Write(LED_OFF);
    }
    else if(state==CONNECTED)
    {
        Advertising_LED_Write(LED_OFF);
        Connection_LED_Write(LED_ON);       /*TURN ON GREEN LED*/
    }
}


/*******************************************************************************
* Function Name: CY_ISR
********************************************************************************
*
* Summary:
*  Handles the interrupt routine interrupt-SW2.
*
* Parameters:
*  None
*
* Return:
*  None
*
*Theory:
* set the variable RemoveBondedDevice=TRUE
*******************************************************************************/
CY_ISR(interrupt)
{
    SW2_ClearInterrupt();
    RemoveBondedDevice=TRUE;    
}


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{
    
    CyGlobalIntEnable;  
    
    CyBle_Start(StackEventHandler);/*Start BLE*/
    
    isr_removebondedDevice_StartEx(interrupt);

    for(;;)
    {
        CyBle_ProcessEvents();/*Process BLE events*/
        HandleLeds();

        /*Store bonding information*/
        if((cyBle_pendingFlashWrite != 0u))
        {   /*If bonding is successful then enable whitelist*/                                    
            if(CYBLE_ERROR_OK== CyBle_StoreBondingData(0u))
            {   
                cyBle_discoveryModeInfo.advParam->advFilterPolicy=CYBLE_GAPP_SCAN_CONN_WHITELIST_ONLY;               
            }                           
        } 
        
        /*Remode the oldest bonded device and change the advertising type*/
        if(RemoveBondedDevice==TRUE && state!=CONNECTED)
        {
            RemoveBondedDevice=FALSE;
            CyBle_GappExitDiscoveryMode();
            CyBle_GapRemoveOldestDeviceFromBondedList();
            cyBle_discoveryModeInfo.advParam->advType=CYBLE_GAPP_CONNECTABLE_UNDIRECTED_ADV;
            CyBle_GappEnterDiscoveryMode(&cyBle_discoveryModeInfo);
            
        }
                
    }
}

/* [] END OF FILE */
