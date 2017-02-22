/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This project demonstrates simultaneous usage of the Cypress BLE in
*  GAP Broadcaster role. The filter policy is set to Scan Request: Whitelist.
*  Only the device in the whitelist will be able to receive the scan response.
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
#include <stdio.h>

/* Array to hold whitelist address
 * Each array element has a format - {{Device Addrress},Adress type}
 * The First Element is initialized with Device address = 0x00A500B2A230,Address type = CYBLE_GAP_ADDR_TYPE_PUBLIC
 * Rest of the elements can be initialized to support other addresses    */

CYBLE_GAP_BD_ADDR_T WhitelistDevice[8] = { {{0x30,0xA2,0xB2,0x00,0xA5,0x00},CYBLE_GAP_ADDR_TYPE_PUBLIC},  
                                           {{0x00,0x00,0x00,0x00,0x00,0x00},0},
                                           {{0x00,0x00,0x00,0x00,0x00,0x00},0},
                                           {{0x00,0x00,0x00,0x00,0x00,0x00},0},
                                           {{0x00,0x00,0x00,0x00,0x00,0x00},0},
                                           {{0x00,0x00,0x00,0x00,0x00,0x00},0},
                                           {{0x00,0x00,0x00,0x00,0x00,0x00},0},
                                           {{0x00,0x00,0x00,0x00,0x00,0x00},0},

                                          };
                                                                                  


/***************************************
*        Function Prototypes
***************************************/
void StackEventHandler(uint32 event,void* eventParam);


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

void StackEventHandler(uint32 event,void* eventParam)
{
    
    switch(event)
    {
        case CYBLE_EVT_STACK_ON:

              /*add device to whitelist.Maximum number of devices can be only 8 inlcuding bonded devices*/                      
              CyBle_GapAddDeviceToWhiteList(&WhitelistDevice[0]);
              
              
             /*Start advertising*/
              CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_SLOW);
             
            break;
                
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            
            if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
            {
                /* On advertisement timeout, restart advertisement */
                CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_SLOW);

            }
            
            break;
        
        default:
            break;
        
    }
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
    /*Declarig the variables*/
    uint8 InterruptStatus;
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
    
   /* Enable global interrupts */
    CyGlobalIntEnable;
       
    /* Internal low power oscillator is stopped as it is not used in this project */
    CySysClkIloStop();
    
    /* Set the divider for ECO, ECO will be used as source when IMO is switched off to save power,
    **  to drive the HFCLK */
    CySysClkWriteEcoDiv(CY_SYS_CLK_ECO_DIV8);
    
        
    CyBle_Start(StackEventHandler);
    
      /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    while(1)
    {
    
         if(CyBle_GetState() != CYBLE_STATE_INITIALIZING)
        {
            /* Put BLE sub system in DeepSleep mode when it is idle */
            lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
            
           /* Disable global interrupts to avoid any other tasks from interrupting this section of code*/
            InterruptStatus = CyEnterCriticalSection();
            
            /* Get current state of BLE sub system to check if it has successfully entered deep sleep state */
            blessState = CyBle_GetBleSsState();

            /* If BLE sub system has entered deep sleep, put chip into deep sleep for reducing power consumption */
            if(lpMode == CYBLE_BLESS_DEEPSLEEP)
            {   
                if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
                {
                   /* Put the chip into the deep sleep state as there are no pending tasks and BLE has also
                   ** successfully entered BLE DEEP SLEEP mode */
                   CySysPmDeepSleep();
                }
            }
            
            /* BLE sub system has not entered deep sleep, wait for completion of radio operations */
            else if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
            {
                
                /* change HF clock source from IMO to ECO, as IMO can be stopped to save power */
                CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_ECO); 
                /* stop IMO for reducing power consumption */
                CySysClkImoStop(); 
                /* put the CPU to sleep */
                CySysPmSleep();
                /* starts execution after waking up, start IMO */
                CySysClkImoStart();
                /* change HF clock source back to IMO */
                CySysClkWriteHfclkDirect(CY_SYS_CLK_HFCLK_IMO);
                
            }
            /*Enable interrupts */
            CyExitCriticalSection(InterruptStatus);
        }
        /*******************************************************************
        *  Process all pending BLE events in the stack
        *******************************************************************/
        CyBle_ProcessEvents();       
        
    }
    
}
/* [] END OF FILE */
