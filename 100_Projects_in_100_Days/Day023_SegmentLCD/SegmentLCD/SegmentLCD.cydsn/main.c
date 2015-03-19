/******************************************************************************
* Project Name		: SegmentLCD
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1 Service Pack 1
* Compiler    		: ARM GCC 4.8.4, ARM RVDS Generic, ARM MDK Generic
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit 
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
#include <project.h>

/* General Macros */
#define TRUE							1
#define FALSE							0
#define ZERO							0

/* Commands from CySmart tool */
#define START 1 /* Starts the Timer */
#define STOP 2 /* Stops the Timer */
#define RESET_COMMAND 0

/* Client Characteristic Configuration descriptor data length. This is defined
* as per BLE spec. */
#define CCCD_DATA_LEN						2

/* Bit mask for notification bit in CCCD (Client Characteristic 
* Configuration Descriptor) written by Client device. */
#define CCCD_NTF_BIT_MASK					0x01

/* Connection Update Parameter values to modify connection interval. These values
* are sent as part of CyBle_L2capLeConnectionParamUpdateRequest() which requests
* Client to update the existing Connection Interval to new value. Increasing 
* connection interval will reduce data rate but will also reduce power consumption.
* These numbers will influence power consumption */

/* Minimum connection interval = CONN_PARAM_UPDATE_MIN_CONN_INTERVAL * 1.25 ms*/
#define CONN_PARAM_UPDATE_MIN_CONN_INTERVAL	100        	
/* Maximum connection interval = CONN_PARAM_UPDATE_MAX_CONN_INTERVAL * 1.25 ms */
#define CONN_PARAM_UPDATE_MAX_CONN_INTERVAL	110        	
/* Slave latency = Number of connection events */
#define CONN_PARAM_UPDATE_SLAVE_LATENCY		0          
/* Supervision timeout = CONN_PARAM_UPDATE_SUPRV_TIMEOUT * 10*/
#define CONN_PARAM_UPDATE_SUPRV_TIMEOUT		200     

#define MILLISECONDS 1000 /* Down counter value for seconds. 1000 msecs = 1 second */
#define MAX_SECONDS 60 
#define DENOMINATOR 10 /* Denominator for hexadecimal to decimal conversion for seconds display */

#define FIRST_POSITION 0
#define SECOND_POSITION 1
#define THIRD_POSITION 2
#define FOURTH_POSITION 3
/**************************Variable Declarations*****************************/

/* 'connectionHandle' stores connection parameters */
CYBLE_CONN_HANDLE_T  connectionHandle;

/* This flag is used by application to know whether a Central 
* device has been connected. This is updated in BLE event callback 
* function*/
uint8 deviceConnected = FALSE;
/* 'restartAdvertisement' flag provided the present state of power mode in firmware */
uint8 restartAdvertisement = FALSE;
/* Status flag for the Stack Busy state. This flag is used to notify the application 
* whether there is stack buffer free to push more data or not */
uint8 busyStatus = 0;

/* Byte to store blue tooth command for CySmart tool */
uint8 segLcdCommand;

int mscount;  /* Millisecond down counter */
uint8 seccount; /* Seconds counter */
uint8 intoccured; /* Falg to check if timer interrupt occured */


/****************************************************************************/

void WritePSoC(void);
void CustomEventHandler(uint32 event, void * eventParam);
uint8 ConvertHexToDec(uint8 value);

/*******************************************************************************
* Function Name: timer_interrupt
********************************************************************************
* Summary:
*        This is the timer interrupt service routine. The timer interrupt occurs every millisecond.
* A variable is decremented (down counter) from 1000 until 0 for every timer interrupt. 
* In for loop in main, the down counter is checked for 0 to determine passing of 1 second.
*
* Parameters:
*  None
*
* Return:
*  void
*
*******************************************************************************/

CY_ISR(timer_interrupt)
{
   	Timer_1_STATUS;
	if (mscount > 0) mscount--;
	intoccured = TRUE;
}

int main()
{
    uint8 dispsec;
	CyGlobalIntEnable; 
	/* Start BLE component and register the CustomEventHandler function. This 
	* function exposes the events from BLE component for application use */
    CyBle_Start(CustomEventHandler);
	
	LCD_Seg_1_Start();
	segLcdCommand = RESET_COMMAND;
	intoccured = FALSE;		
    for(;;)
    {
		CyBle_ProcessEvents();
		if (restartAdvertisement == TRUE)
		{
			/* Reset 'restartAdvertisement' flag*/
			restartAdvertisement = FALSE;
			
			LCD_Seg_1_ClearDisplay();
			/* Start Advertisement and enter Discoverable mode*/
			CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);				
		}
		if (deviceConnected == TRUE)
		{
			/* Start timer command received from CySmart tool */
			if (segLcdCommand == START)
			{
				/* Reset command */
				segLcdCommand = RESET_COMMAND;
				/* Start Segment LCD component */
				LCD_Seg_1_Start();
				/* Start timer */
				Timer_1_Start();
				/* Start timer interrupt */
				isr_1_StartEx(timer_interrupt);	
				/* Set down counter value for 1 sec */
				mscount = MILLISECONDS;
			
			}
			
			/* Stop timer command received from CySmart tool */
			if (segLcdCommand == STOP)
			{
				/* Reset command */
				segLcdCommand = RESET_COMMAND;
				/* Stop timer */
				Timer_1_Stop();
			}
		}
		/* Timer Interrupt occured */
		if (intoccured == TRUE)
		{
			/* Clear timer interrupt flag */
			intoccured = FALSE;
			if (mscount == 0)
			{
				/* 1 sec downcounter value has become 0. 1 sec elapsed */
				mscount = MILLISECONDS; /* Reset the down counter value */
				seccount++; /* increment second count */
				if (seccount > MAX_SECONDS)
				{
					seccount = 0; /* Reset seconds value to 0. If 60 seconds are elapsed */
				}
				dispsec = ConvertHexToDec(seccount); /* Convert seconds value to decimal before displaying */
				/* Clear display before displaying seconds value */
				LCD_Seg_1_ClearDisplay();
				/* Extract only LSB and display in first position */
				LCD_Seg_1_Write7SegDigit_0((dispsec & 0x0F), FIRST_POSITION);
				/* Extract only MSB and display in second position */
				LCD_Seg_1_Write7SegDigit_0((dispsec & 0xF0)>>4, SECOND_POSITION);
			}
		} 
	}
}
/*******************************************************************************
* Function Name: ConvertHexToDec
********************************************************************************
* Summary:
*        This function converts the given hexadecimal number to decimal number. The given hexadecimal
* number is divided by 10 until either qoutient and remainder are less than 10. The remainder at each 
* division is stored. For example, to convert 0x6E to decimal. First, divide by 0x0A. The qoutient 
* is 0x0B and remainder is 0. Store 0 at LSB. Now since qoutient is still greater than 0x0A, divide
* 0x0B by 0x0A. Now remainder is 0x01 and qoutient is 0x01. Move the current remainder to right by
* 4 bit positions and OR it along with previous remainder. So, final remainder is 0x10.
* Next, now since the current remainder and qoutient are less than 0x0A, move quotient and OR it along with 
* remainder value to get final decimal number. So, by moving qoutient by 8 bit positions, we get 0x100.
* By Or'ing with remainder value, we get 0x110 which is decimal value of 0x6E.

* Note: The bit position to which the remainder or qoutient value to be moved is always + 4 bit positions of 
* previously moved bit position.
*
* Parameters:
*  value - Hex value to be converted.
*
* Return:
*  Decimal value of the specified hexadecimal number.
*
*******************************************************************************/
uint8 ConvertHexToDec(uint8 value)
{
	uint8 temp, rem, i, quos;
	uint8 decnum;
	temp = value;
	rem = 1;
	decnum = 0;
	i = 0;
	do
	{
		rem = temp % DENOMINATOR;	/* Get remainder */
		quos = temp / DENOMINATOR; /* Get qoutient */
		decnum = decnum  | (rem << i); /* store current remainder value */
		i = i +4; /* Add 4 to get next bit position to which remainder or qoutient to be moved */
		temp = quos;
	}
	while ((rem >= DENOMINATOR) || (quos >= DENOMINATOR)); /* Exit loop if remainder, quotient is less than 0x0A */
	decnum = decnum  | (quos << i); /* Add qoutient value to stored remainder value */
	
	return decnum; /* return the decimal value */
}
/*******************************************************************************
* Function Name: CustomEventHandler
********************************************************************************
* Summary:
*        Call back event function to handle various events from BLE stack
*
* Parameters:
*  event:		event returned
*  eventParam:	link to value of the events returned
*
* Return:
*  void
*
*******************************************************************************/
void CustomEventHandler(uint32 event, void * eventParam)
{
	/* Local variable to store the data received as part of the Write request 
	* events */
	CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;

    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
			/* This event is received when component is Started */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = TRUE;
			
			break;
			
		case CYBLE_EVT_TIMEOUT:
			/* Event Handling for Timeout  */
	
			break;
        
		/**********************************************************
        *                       GAP Events
        ***********************************************************/
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
			
			/* If the current BLE state is Disconnected, then the Advertisement
			* Start Stop event implies that advertisement has stopped */
			if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
			{
				/* Set restartAdvertisement flag to allow calling Advertisement 
				* API from main function */
				restartAdvertisement = TRUE;
			}
			break;
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED: 					
			/* This event is received when device is connected over GAP layer */
			break;

			
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			/* This event is received when device is disconnected */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = TRUE;
			
			break;
        
		/**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
			/* This event is received when device is connected over GATT level */
			
			/* Update attribute handle on GATT Connection*/
            connectionHandle = *(CYBLE_CONN_HANDLE_T  *)eventParam;
			
			/* This flag is used in application to check connection status */
			deviceConnected = TRUE;
			break;
        
        case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* This event is received when device is disconnected */
			
			/* Update deviceConnected flag*/
			deviceConnected = FALSE;
			

			
			break;
            
        case CYBLE_EVT_GATTS_WRITE_REQ:
			/* This event is received when Central device sends a Write command on an Attribute */
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
            
			/* When this event is triggered, the peripheral has received a write command on the custom characteristic */
			/* Check if command is for correct attribute and update the flag for sending Notifications */
            if(CYBLE_SEGLCD_SEGLCD_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_HANDLE == wrReqParam->handleValPair.attrHandle)
			{
				/* Extract the Write value sent by the Client for CapSense Slider CCCD */
                segLcdCommand = wrReqParam->handleValPair.value.val[CYBLE_SEGLCD_SEGLCD_CLIENT_CHARACTERISTIC_CONFIGURATION_DESC_INDEX];
				
            }
			
			
			/* Send the response to the write request received. */
			CyBle_GattsWriteRsp(connectionHandle);
			
			break;
			
		case CYBLE_EVT_STACK_BUSY_STATUS:
			/* This event is generated when the internal stack buffer is full and no more
			* data can be accepted or the stack has buffer available and can accept data.
			* This event is used by application to prevent pushing lot of data to stack. */
			
			/* Extract the present stack status */
            busyStatus = * (uint8*)eventParam;
            break;
			
        default:

       	 	break;
    }   	/* switch(event) */
}

/* [] END OF FILE */
