/*****************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Description:
* This is the top level application file.
*
* Hardware Dependency:
* CY8CKIT-042 BLE Pioneer Kit
*
******************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
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
*****************************************************************************/


/*****************************************************************************
* Included headers
*****************************************************************************/
#include <main.h>
#include <BLEApplications.h>


/*****************************************************************************
* Function Prototypes
*****************************************************************************/
static void InitializeSystem(void);
static void HandleCapSenseSlider(void);


/*****************************************************************************
* Public functions
*****************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* System entrance point. This calls the initializing function and continuously
* process BLE and CapSense events.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main()
{
	/* This function will initialize the system resources such as BLE and CapSense */
    InitializeSystem();
	
    for(;;)
    {
        /*Process event callback to handle BLE events. The events generated and 
		* used for this application are inside the 'CustomEventHandler' routine*/
        CyBle_ProcessEvents();
		
		if(TRUE == deviceConnected)
		{
            /* When the Client Characteristic Configuration descriptor (CCCD) is
             * written by Central device for enabling/disabling notifications, 
             * then the same descriptor value has to be explicitly updated in 
             * application so that it reflects the correct value when the 
             * descriptor is read */
			UpdateNotificationCCCD();
			
			/* Send CapSense Slider data when respective notification is enabled */
			if(TRUE == sendCapSenseSliderNotifications)
			{
				/* Check for CapSense slider swipe and send data accordingly */
				HandleCapSenseSlider();
			}
		}
    }	
}


/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
* Summary:
* Start the components and initialize system.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitializeSystem(void)
{
	/* Enable global interrupt mask */
	CyGlobalIntEnable; 
		
	/* Start BLE component and register the CustomEventHandler function. This 
	 * function exposes the events from BLE component for application use */
    CyBle_Start(CustomEventHandler);	
    
	/* Start both the PrISM components for LED control*/
    PRS_1_Start();
    PRS_2_Start();
	
	/* The RGB LED on BLE Pioneer kit are active low. Drive HIGH on 
	 * pin for OFF and drive LOW on pin for ON*/
	PRS_1_WritePulse0(RGB_LED_OFF);
	PRS_1_WritePulse1(RGB_LED_OFF);
	PRS_2_WritePulse0(RGB_LED_OFF);
	
	/* Set Drive mode of output pins from HiZ to Strong */
	RED_SetDriveMode(RED_DM_STRONG);
	GREEN_SetDriveMode(GREEN_DM_STRONG);
	BLUE_SetDriveMode(BLUE_DM_STRONG);
	
	/* ADD_CODE to initialize CapSense component and initialize baselines*/
	CapSense_Start();
	CapSense_InitializeAllBaselines();
}


/*******************************************************************************
* Function Name: HandleCapSenseSlider
********************************************************************************
* Summary:
* This function scans for finger position on CapSense slider, and if the  
* position is different, triggers separate routine for BLE notification.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void HandleCapSenseSlider(void)
{
	/* Last read CapSense slider position value */
	static uint16 lastPosition;	
	
	/* Present slider position read by CapSense */
	uint16 sliderPosition;
		
	/* Update CapSense baseline for next reading*/
	CapSense_UpdateEnabledBaselines();	
		
	/* ADD_CODE to scan the slider widget */
	CapSense_ScanEnabledWidgets();			
	
	/* Wait for CapSense scanning to be complete. This could take about 5 ms */
	while(CapSense_IsBusy());
	
	/* ADD_CODE to read the finger position on the slider */
	sliderPosition = CapSense_GetCentroidPos(CapSense_LINEARSLIDER0__LS);	

	/*If finger is detected on the slider*/
	if((sliderPosition != NO_FINGER) && (sliderPosition <= SLIDER_MAX_VALUE))
	{
        /* If finger position on the slider is changed then send data as BLE 
         * notifications */
        if(sliderPosition != lastPosition)
		{
			/* Update global variable with present finger position on slider*/
			lastPosition = sliderPosition;

			SendCapSenseNotification((uint8)sliderPosition);

		}	
	}	
}

/* [] END OF FILE */
