/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
*  This is the source code for the PSoC 4 BLE Finde Me Code Example.
*
* Hardware Dependency:
*  CY8CKIT-042 BLE Pioneer Kit
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <project.h>

/***************************************
*        API Constants
***************************************/
#define NO_ALERT           (0u)
#define MILD_ALERT         (1u)
#define HIGH_ALERT         (2u)

#define NO_ALERT_COMPARE   (0u)
#define MILD_ALERT_COMPARE (250u)
#define HIGH_ALERT_COMPARE (500u)

/***************************************
*        Function Prototypes
***************************************/
void StackEventHandler(uint32 event, void* eventParam);
void IasEventHandler(uint32 event, void* eventParam);
void HandleAlertLEDs(uint8 status);


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

    /* Start the BLE component and register StackEventHandler function */
    CyBle_Start(StackEventHandler);
    
    /* Start the PWM component */
    PWM_Start();

    /* Register IAS event handler function */
    CyBle_IasRegisterAttrCallback(IasEventHandler);
        
    while(1)
    {
        /* Process all the pending BLE tasks. This single API call to 
         * will service all the BLE stack events. This API MUST be called at least once
         * in a BLE connection interval */
        CyBle_ProcessEvents();
    }
}

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
    switch(event)
    {
        /* Mandatory events to be handled by Find Me Target design */
        case CYBLE_EVT_STACK_ON:
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            /* ADD_CODE - Start the BLE fast advertisement. */
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
        
        default:
    	    break;
    }
}

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
    uint8 alertLevel;
    
    /* Alert Level Characteristic write event */
    if(event == CYBLE_EVT_IASS_WRITE_CHAR_CMD)
    {
        /* Extract Alert Level value from the GATT DB using the 
		 * CYBLE_IAS_ALERT_LEVEL as a parameter to CyBle_IassGetCharacteristicValue
		 * routine. Store the Alert Level Characteristic value in "alertLevel"
		 * variable */
        CyBle_IassGetCharacteristicValue(CYBLE_IAS_ALERT_LEVEL, sizeof(alertLevel), &alertLevel);
        
        /*Based on alert Level level recieved, Drive LED*/
        HandleAlertLEDs(alertLevel);
    }
}

/*******************************************************************************
* Function Name: HandleAlertLEDs
********************************************************************************
*
* Summary:
*  This function drives the LED based on the alert level
*
* Parameters:  
*  uint8 status:      Alert level 
*
* Return: 
*  None
*
*******************************************************************************/
void HandleAlertLEDs(uint8 status)
{
    /* Update Alert LED status based on IAS Alert level characteristic. */
    switch(status)
    {
        case NO_ALERT:
            PWM_WriteCompare(NO_ALERT_COMPARE);
            break;

        case MILD_ALERT:
            PWM_WriteCompare(MILD_ALERT_COMPARE);
            break;
            
        case HIGH_ALERT:
            PWM_WriteCompare(HIGH_ALERT_COMPARE);
            break;                
    }
}


/* [] END OF FILE */
