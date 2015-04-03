/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This code demonstrates Out-of-Band (OOB) pairing using PSoC4 BLE kit. This
*  code is based on the example project 'BLE_FindMe'. In this project, PSoC4 is
*  configured as a Find Me Target with GAP role as peripheral.
*
* Note:
*
* Owners:
*  VAIR@CYPRESS.COM
*
* Related Document:
*
* Hardware Dependency:
*  1. PSoC 4 BLE device
*  2. CY8CKIT-042 BLE Pioneer Kit
*
* Code Tested With:
*  1. PSoC Creator 3.1
*  2. ARM CM0-GCC
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
#include "stdio.h"
#include "stdlib.h"

/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/
#define LED_ON             (0u)
#define LED_OFF            (1u)


/*******************************************************************************
*   Module Variable and Constant Declarations with Applicable Initializations
*******************************************************************************/

/* Security key should be 16 bytes in length. Since we use a null terminated
 * string, the array size is one byte extra to store the null character.
 * OOB pairing with this device will be successful only if the pairing initiator
 * uses the exact key while setting the OOB data before initiating the pairing
 * request. OOB data is set using CyBle_GapSetOobData() API. 
 */
static uint8 securityKey[16  + 1] = "BleOobPairingDem";


/***************************************
*        Function Prototypes
***************************************/
void StackEventHandler(uint32 event, void* eventParam);

/*******************************************************************************
* Function Name: StackEventHandler
********************************************************************************
*
* Summary:
*  This is an event callback function to receive events from the BLE Component.
*
* Parameters:
*  uint8 event:       Event from the CYBLE component
*  void* eventParams: A structure instance for corresponding event type. The
*                     list of event structure is described in the component
*                     datasheet.
*
* Return:
*  None
*
*******************************************************************************/
void StackEventHandler(uint32 event, void *eventParam)
{
	char authFailReasonCode[3];
	CYBLE_GAP_AUTH_FAILED_REASON_T *authFailReason;
	
    switch(event)
    {
    /* Mandatory events to be handled by Find Me Target design */
    case CYBLE_EVT_STACK_ON:	
    case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
        /* Start BLE advertisement for 30 seconds and update link
         * status on LEDs */
    	CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
        Advertising_LED_Write(LED_ON);
        PWM_WriteCompare(LED_NO_ALERT);
        break;

    case CYBLE_EVT_GAP_DEVICE_CONNECTED:
    	UART_UartPutString("GAP Device Connected\r\n");
    	
        /* BLE link is established */
        Advertising_LED_Write(LED_OFF);			
        break;

    case CYBLE_EVT_TIMEOUT:
        if(*(uint8 *) eventParam == CYBLE_GAP_ADV_MODE_TO)
        {
            /* Advertisement event timed out, go to low power
             * mode (Hibernate mode) and wait for an external
             * user event to wake up the device again */
            Advertising_LED_Write(LED_OFF);
            Hibernate_LED_Write(LED_ON);
            PWM_Stop();
            Wakeup_SW_ClearInterrupt();
            Wakeup_Interrupt_ClearPending();
            Wakeup_Interrupt_Start();
            CySysPmHibernate();
        }
        break;

    /**********************************************************
    *                       GAP Events
    ***********************************************************/
    case CYBLE_EVT_GAP_AUTH_REQ:						
		UART_UartPutString("Authorization Requested\r\n");
        break;
		
    case CYBLE_EVT_GAP_AUTH_COMPLETE:
		UART_UartPutString("Pairing is Successful!\r\n");
        break;
		
    case CYBLE_EVT_GAP_AUTH_FAILED:
		authFailReason = ((CYBLE_GAP_AUTH_FAILED_REASON_T *)eventParam);
		UART_UartPutString("Authentication Failed with Reason Code: ");
		snprintf(authFailReasonCode, sizeof(authFailReasonCode), "%lu", (uint32)(*authFailReason));
		UART_UartPutString(authFailReasonCode);
		UART_UartPutChar("\r\n");			
        break;

    /**********************************************************
    *                       GATT Events
    ***********************************************************/
    case CYBLE_EVT_GATT_CONNECT_IND:
		UART_UartPutString("GATT Connection Indication\r\n");
		
		/* Set OOB data after the connection indication but before the authorization
		 * request is received. 
		 */
		
		if(CyBle_GapSetOobData(cyBle_connHandle.bdHandle, CYBLE_GAP_OOB_ENABLE, securityKey, NULL, NULL)  != CYBLE_ERROR_OK)
		{
			UART_UartPutString("Error in Setting OOB Data\r\n");
		}
		else
		{
			UART_UartPutString("OOB Data is Set\r\n");
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
    CYBLE_API_RESULT_T apiResult;
    CYBLE_STATE_T bleState;

    CyGlobalIntEnable;
	
    PWM_Start();
	UART_Start();
	UART_UartPutString("Welcome to BLE OOB Pairing Demo\r\n");

    apiResult = CyBle_Start(StackEventHandler);

    if(apiResult != CYBLE_ERROR_OK)
    {
        /* BLE stack initialization failed, check your configuration */
        CYASSERT(0);
    }

    CyBle_IasRegisterAttrCallback(IasEventHandler);

    for(;;)
    {
        /* Single API call to service all the BLE stack events. Must be
         * called at least once in a BLE connection interval */
        CyBle_ProcessEvents();

        bleState = CyBle_GetState();

        if(bleState != CYBLE_STATE_STOPPED &&
            bleState != CYBLE_STATE_INITIALIZING)
        {
            /* Configure BLESS in DeepSleep mode  */
            CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);

            /* Configure PSoC 4 BLE system in sleep mode */
            CySysPmSleep();

            /* BLE link layer timing interrupt will wake up the system */
        }
    }
}


/* [] END OF FILE */
