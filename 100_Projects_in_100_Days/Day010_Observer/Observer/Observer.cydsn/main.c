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
#include <stdio.h>
#include <common.h>



/***************************************
*        Function Prototypes
***************************************/
void StackEventHandler(uint32 event, void* eventParam);



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
void StackEventHandler(uint32 event, void* eventParam)
{
    /*local variables*/
   CYBLE_GAPC_ADV_REPORT_T advReport;
   uint8 i;
     
    switch(event)
    {
        
        case CYBLE_EVT_STACK_ON:    /*BLE stack ON*/
        
                printf("Bluetooth ON:\r\n");
        
                /*Start Scanning*/
                if(CYBLE_ERROR_OK==CyBle_GapcStartScan(CYBLE_SCANNING_SLOW))
                {
                    printf("Sarted to Scan\r\n");
                    
                }
                
            break;

        case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:  
                
                /* scan progress result event occurs 
                 * when it receives any advertisiment packet
                 * or scan response packet from peer device*/
                

                advReport=*(CYBLE_GAPC_ADV_REPORT_T *)eventParam;

                printf("CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:\r\n");
                 
                /* Print the Advertising Event details of peer device:*/
                printf("eventType:");
                 
                switch(advReport.eventType)
                {
                    case CYBLE_GAPC_CONN_UNDIRECTED_ADV: 
                        
                         printf("Connectable undirected advertising\r\n");
                        break;
                        
                    case  CYBLE_GAPC_CONN_DIRECTED_ADV:
                         
                         printf("Connectable directed advertising\r\n");
                        break;
                    
                    case  CYBLE_GAPC_SCAN_UNDIRECTED_ADV:
                        
                         printf("Scannable undirected advertising\r\n");
                        break;
                     
                    case CYBLE_GAPC_NON_CONN_UNDIRECTED_ADV:
                        
                          printf("Non connectable undirected advertising\r\n");
                        break;
                     
                    case CYBLE_GAPC_SCAN_RSP:
                        
                          printf("SCAN_RSP\r\n");
                        break;
                }
  
                
                /*  PEER addr type  */
                printf("    peerAddrType: ");
                
                if(advReport.peerAddrType==CYBLE_GAP_ADDR_TYPE_PUBLIC)
                {
                    printf("PUBLIC\r\n");
                }
                else if(advReport.peerAddrType==CYBLE_GAP_ADDR_TYPE_RANDOM)
                {
                    printf("RANDOM \r\n");
                }
                
                 /*  PEER Device address type  */
                printf("    peerBdAddr: %2.2x%2.2x%2.2x%2.2x%2.2x%2.2x \r\n",
                    advReport.peerBdAddr[5u], advReport.peerBdAddr[4u],
                    advReport.peerBdAddr[3u], advReport.peerBdAddr[2u],
                    advReport.peerBdAddr[1u], advReport.peerBdAddr[0u]);
                

                /*  Advertising or scan response data  */
                printf("    Peer device adveritsing/scan response data Length: %x \r\n", advReport.dataLen);
                printf("    advertising/scan response data of peer device: ");

                
                if(advReport.dataLen!=0)
                {
                    for(i = 0u; i < advReport.dataLen; i++)
                    {
                        printf("%x", advReport.data[advReport.dataLen-i]);
                    }
                
                }
                printf("\r\n");
                
                /*  RSSI of the received packet from peer Device  */
                printf("    Rssi: %i \r\n", advReport.rssi);
                
                printf("\r\n");
            break;  
                
         case  CYBLE_EVT_GAPC_SCAN_START_STOP:
                
                if(CyBle_GetState()==CYBLE_STATE_DISCONNECTED)
                {   /*Restart scanning if time out happens*/
                    CyBle_GapcStartScan(CYBLE_SCANNING_SLOW);
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
   
    /* Variable declarations */
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
    uint8 InterruptsStatus;
      
   
    /* Start communication component */
    UART_Start();
    
    /* Enable global interrupts */
    CyGlobalIntEnable;
       
    /* Internal low power oscillator is stopped as it is not used in this project */
    CySysClkIloStop();
    
    /* Set the divider for ECO, ECO will be used as source when IMO is switched off to save power,
    **  to drive the HFCLK */
    CySysClkWriteEcoDiv(CY_SYS_CLK_ECO_DIV8);
    
    CyBle_Start(StackEventHandler);
   
    /*Infinite Loop*/
    for(;;)
    {
       
        if((UART_SpiUartGetTxBufferSize() + UART_GET_TX_FIFO_SR_VALID) == 0)
        {
            
           if(CyBle_GetState() != CYBLE_STATE_INITIALIZING)
           {
                /* Put BLE sub system in DeepSleep mode when it is idle */
                 lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
                
               /* Disable global interrupts to avoid any other tasks from interrupting this section of code*/
                InterruptsStatus = CyEnterCriticalSection();
                
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
                CyExitCriticalSection(InterruptsStatus);
            
            }/*end of if(CyBle_GetState() != CYBLE_STATE_INITIALIZING)*/
             
            CyBle_ProcessEvents();
                        
        }   
    
    }
    
 }


/* [] END OF FILE */

