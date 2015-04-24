/*******************************************************************************
* File Name: main.c
*
* Version: 1.0
*
* Author: SSHH
*
* Description:
*  ProximityGesturesWithBLE example project demonstrates proximity gestures 
*  simualting keyboard left and right arrows. It supports both input and output reports
*  in boot and protocol mode. The example also demonstrates handling suspend 
*  event from the central device and enters low power mode when suspended.
*
* References:
*  BLUETOOTH SPECIFICATION Version 4.1
*  HID Usage Tables spec ver 1.12
*  AN92239 -  Proximity Sensing with CapSense
*
* Hardware Dependency:
*  CY8CKIT-042 BLE, CY8CKIT-024 CAPSENSE PROXIMITY SHIELD
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include <project.h>
#include <stdbool.h>
#include <gestures.h>

#include "common.h"
#include "hids.h"
#include "bas.h"

/*******************************************************************************
*   Macros and #define Constants
*******************************************************************************/
/* Macro to enable Tx8 data out on P0[1], sends out serial data when macro is 
 * set to '1' 
 */
/* Note the following before enabling Tx8:
 *     In CY8CKIT-042, P0[1] on header J2, has to be manually connected to
 *     P12[6] on header J8. 
 *     P12[6] on header J8 is the Rx pin of onboard USB-UART hardware. Since  
 *     this pin is not hardwired to any of the PSoC4A pins, a manual connection 
 *     is required between PSoC4A Tx8 pin and P12[6] on J8.
 *     Refer AN92239 "Proximity Sensing with CapSense" for more details on 
 *     using Tx8 output to view data on Bridge Control Panel.
 */
#define TX8_ENABLE                  (1u)
/*Enable tuner during tuning CapSense sensors only. BLE gets disabled if tuner is enabled*/
#define TUNER_ENABLE                (0u)

/* Macro indicating the device is in slow scan mode -Corresponds to watch dog 
   refresh interval of WDT_TIMEOUT_SLOW_SCAN*/
#define SLOW_SCAN_MODE              (0u)

/* Macro indicating the device is in fast scan mode -Corresponds to watch dog 
   refresh interval of WDT_TIMEOUT_FAST_SCAN*/
#define FAST_SCAN_MODE              (1u)

/* This timeout is for device scan mode shift from fast scan to slow scan if there is
   no proximity sensor detected */
#define SCANMODE_TIMEOUT_VALUE      (1000u)


/*******************************************************************************
*   Variable and Function Declarations
*******************************************************************************/
#if TX8_ENABLE
    #define NO_OF_SAMPLES(array)	    (sizeof(array)/sizeof(array[0]))    
        
    /* Multichart header and tail */
    const uint8 multichartHeader[] =    {0x0D, 0x0A};      /* Header = CR, LF */
    const uint8 multichartTail[] =      {0x00, 0xFF, 0xFF};  /* Tail = 0x00, 0xFF, 0xFF */

    void SendUint16ToMultichart(uint16 value);
    void SendDataToMultichart(uint8, uint8);
#endif

#if !TUNER_ENABLE
	/*Sensor numbers*/
	SENSOR_NAMES startSensorNumber;
	SENSOR_NAMES endSensorNumber;
#endif

void CapSenseInitialization(void);
void DeviceInit(void);
void CapSense_HandleCapSense(void);
#if !TUNER_ENABLE
    void BLE_HandleBLE(void);
#endif
void HandleDeviceScanMode(void);

#if !TUNER_ENABLE
    
CYBLE_GAP_CONN_UPDATE_PARAM_T connectionParameters = 
{
    8,                 /* Minimum connection interval - 8 x 1.25 = 10 ms */
    8,                 /* Maximum connection interval - 8 x 1.25 = 10 ms */
    25,                /* Slave latency - 25 */
    500               /* Supervision timeout - 500 x 10 = 5000 ms */
};


/*******************************************************************************
* Function Name:  AppCallBack()
****************************************************************************//**
* @par Summary
*     This is an event callback function to receive events from the BLE Component.
*
* @return
*    None
*
* @param[in] 
*    Event: Event code
*    * EventParam: Address of any associated parameters of an event 
*
* @par Theory of Operation
*    Service specific events are handled here.
*
* @par Notes
*    None
*
**//***************************************************************************/
void AppCallBack(uint32 event, void* eventParam)
{
    CYBLE_GAP_BD_ADDR_T localAddr;
    
    switch (event)
	{
        /**********************************************************
        *                       General Events
        ***********************************************************/
		case CYBLE_EVT_STACK_ON: /* This event is received when the component is Started */
            /* Enter into discoverable mode so that remote can search it. */
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            CyBle_GetDeviceAddress(&localAddr);
            break;          
        /**********************************************************
        *                       GAP Events
        ***********************************************************/
        case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
             if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
            {   
                /* Fast and slow advertising period complete, go to low power  
                 * mode (Hibernate mode) and wait for an external
                 * user event to wake up the device again */
                Advertising_LED_Write(LED_OFF);
                Disconnect_LED_Write(LED_ON);
                SW2_ClearInterrupt();
                Wakeup_Interrupt_ClearPending();
                Wakeup_Interrupt_Start();
                CySysPmHibernate();
            }
            break;
        case CYBLE_EVT_GAP_DEVICE_CONNECTED:
            Advertising_LED_Write(LED_OFF);
           
            break;
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
            break;
        case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
            { 
                uint16 mtu;
                CyBle_GattGetMtuSize(&mtu);
            }
            break;
        /**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
            /* Register service specific callback functions */
            HidsInit();
            BasInit();
            break;
            
        case CYBLE_EVT_GAP_AUTH_COMPLETE:
            CyBle_L2capLeConnectionParamUpdateRequest(cyBle_connHandle.bdHandle, &connectionParameters);
            break;
        /**********************************************************
        *                       Other Events
        ***********************************************************/
		default:
			break;
	}

}


/*******************************************************************************
* Function Name: Timer_Interrupt
********************************************************************************
*
* Summary:
*  Handles the Interrupt Service Routine for the WDT timer.
*
* @return
*    None
*
* @param[in] 
*    None
*
* @par Theory of Operation
*    Clears the watchdog timer interrupt and toggles the advertising LED if the
*    device is in advertising mode.
*
* @par Notes
*    None
*
**//***************************************************************************/
CY_ISR(Timer_Interrupt)
{
    if(CySysWdtGetInterruptSource() & WDT_INTERRUPT_SOURCE)
    {
        static uint8 led = LED_OFF;
        static uint32 ledTimer = LED_TIMEOUT;
    
        /* Blink green LED to indicate that device advertises */
        if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
        {
            if(--ledTimer == 0u) 
            {
                ledTimer = LED_TIMEOUT;
                led ^= LED_OFF;
                Advertising_LED_Write(led);
            }
        }
        
        /* Clears interrupt request  */
        CySysWdtClearInterrupt(WDT_INTERRUPT_SOURCE);
    }
}

/*******************************************************************************
* Function Name: WDT_Start
********************************************************************************
*
* Summary:
*  Configures WDT to trigger an interrupt.
* @return
*    None
*
* @param[in] 
*    None
*
* @par Theory of Operation
*    The device starts with slow scan mode. Hence watchdog timer match value is
*    set accordingly.
*
* @par Notes
*    None
*
**//***************************************************************************/

void WDT_Start(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    /* Setup ISR */
    WDT_Interrupt_StartEx(&Timer_Interrupt);
    /* Write the mode to generate interrupt on match */
    CySysWdtWriteMode(WDT_COUNTER, CY_SYS_WDT_MODE_INT);
    /* Configure the WDT counter clear on a match setting */
    CySysWdtWriteClearOnMatch(WDT_COUNTER, WDT_COUNTER_ENABLE);
    /* Configure the WDT counter match comparison value. 
     The device is in slow scan mode upon power up */
    CySysWdtWriteMatch(WDT_COUNTER, WDT_TIMEOUT_SLOW_SCAN);
    /* Reset WDT counter */
    CySysWdtResetCounters(WDT_COUNTER);
    /* Enable the specified WDT counter */
    CySysWdtEnable(WDT_COUNTER_MASK);
    /* Lock out configuration changes to the Watchdog timer registers */
    CySysWdtLock();    
}


/*******************************************************************************
* Function Name: WDT_Stop
********************************************************************************
*
* Summary:
*  This API stops the WDT timer.
*
*******************************************************************************/
void WDT_Stop(void)
{
    /* Unlock the WDT registers for modification */
    CySysWdtUnlock(); 
    /* Disable the specified WDT counter */
    CySysWdtDisable(WDT_COUNTER_MASK);
    /* Locks out configuration changes to the Watchdog timer registers */
    CySysWdtLock();    
}
#endif


int main(void)
{
    
	/*Initializes system variables, global interrupts and serial interface*/	
	DeviceInit();
    #if TUNER_ENABLE
      /*Only sensors are scanned when tuner is enabled*/
	    CapSenseInitialization();
    #else
        /*Only sensors are scanned when tuner is enabled*/
	    CapSenseInitialization();

        /*Based on the board and gesture direction assign the proper sensor numbers*/
    	#if(GESTURE_AXIS)
    		startSensorNumber = LEFT_SENSOR;
    		endSensorNumber = RIGHT_SENSOR;	
    	#else
    		startSensorNumber = BOTTOM_SENSOR;
    		endSensorNumber = TOP_SENSOR;
    	#endif
    
    #endif
    
	#if TUNER_ENABLE
		CapSense_TunerStart();
    #endif
    
    /* swtich off the ILO to save power */
    CySysClkIloStop();
    
    while(1)
    {
        #if !TUNER_ENABLE
            if((CyBle_GetState() == CYBLE_STATE_CONNECTED) && (suspend != CYBLE_HIDS_CP_SUSPEND))
        #endif
            {
                /* Scan the CapSense sensors here and process the CapSense sensor data*/ 
                CapSense_HandleCapSense();
            }
        
        #if !TUNER_ENABLE
            /* BLE, Keyboard simulation, low power mode handling */
            BLE_HandleBLE();
       
            /* Handle the scan mode of the device based on the proximity sensor status */
            HandleDeviceScanMode();
        #endif
        
	}
}  

/*******************************************************************************
* Function Name: DeviceInit
****************************************************************************//**
* @par Summary
*    Initializes system variables, global interrupts and serial interface
*
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*    Initializes system variables, global interrupts and serial interface
*
* @par Notes
*    None
*
**//***************************************************************************/
void DeviceInit(void)
{	
    
	CyGlobalIntEnable;    
    #if !TUNER_ENABLE
        
        /* BLE functionality indicators*/
        Disconnect_LED_Write(LED_OFF);
        Advertising_LED_Write(LED_OFF);

        /* Start CYBLE component and register generic event handler */
        CyBle_Start(AppCallBack);
        WDT_Start();  
        
    #endif  
     
    /*Initialize the variables needed for gestures*/
	GestureVariableInit(&XAxis);
    
	#if TX8_ENABLE
    	Tx8_Start();
	#endif
	
}


/*******************************************************************************
* Function Name: CapSenseInitialization
****************************************************************************//**
* @par Summary
*    Initializes the capacitive sensing engine. 
*
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*    Enables proximity sensors, Starts the Capacitive sensing hardware block,
*	 Initializes the sensor decision logic reference
*
* @par Notes
*    None
*
**//***************************************************************************/
void CapSenseInitialization(void)
{
	uint8 sensor;
	for(sensor= 0; sensor < CapSense_TOTAL_SENSOR_COUNT; sensor++)
	{
		CapSense_EnableWidget(sensor);
	}
	
	CapSense_Start();        
    /*Set the reference*/
	CapSense_InitializeEnabledBaselines();       

}

/*******************************************************************************
* Function Name: CapSense_HandleCapSense
****************************************************************************//**
* @par Summary
*    Capacitive sensors are scanned and sensor data are processed 
*
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*    Scans the sensors, detects gestures, drives LEDs based on gestures detection
*
* @par Notes
*    None
*
**//***************************************************************************/

void CapSense_HandleCapSense(void)
{   
    #if !TUNER_ENABLE
		/*Sensor numbers*/
		uint8 sensorNo;
	#endif
    #if TUNER_ENABLE
        CapSense_TunerComm();
    #else

        for(sensorNo = startSensorNumber ;sensorNo <= endSensorNumber; sensorNo++)
		{
			/*Scan sensor*/
			CapSense_ScanSensor(sensorNo);
			/*Wait till sensor is scanned*/
	    	while(CapSense_IsBusy())
			{
                /* CPU Sleep */
                CySysPmSleep();
                
			} 
	        /*Update the Sensor digital count reference*/
	        CapSense_UpdateSensorBaseline(sensorNo);	
		}			
	
		/*Detailed gesture algorithm is explained in gesture.c file*/
		/*Detect x-axis gestures*/
		#if(GESTURE_AXIS)
			GestureDetection(startSensorNumber,endSensorNumber, &XAxis); 
		#endif	
		
		/*CapSense digital data with reference and signal is sent out serially*/
		#if TX8_ENABLE
        	SendDataToMultichart(startSensorNumber, endSensorNumber); 
		#endif  
    #endif        
}

/*******************************************************************************
* Function Name: HandleDeviceScanMode
****************************************************************************//**
* @par Summary
*    This function switches between the slow scan mode and fast scan mode 
*    depending on CapSense proximity sensor status.
*
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*    If any CapSense proximity sensor is triggered, the device mode is switched to
*    fast scan mode. If none of the sensors are triggered, after timeout period,
*    the device mode is changed to slow scan mode until a new proximity
*    event is detected.
*
* @par Notes
*    None
*
**//***************************************************************************/
void HandleDeviceScanMode(void)
{
    static uint16 timeoutCounter = 0;
    static uint8 deviceScanMode = SLOW_SCAN_MODE;
    switch(deviceScanMode)
    {
        case SLOW_SCAN_MODE:
            if(CapSense_CheckIsAnyWidgetActive())
            {
               
                /* Configure the WDT counter match comparison value */
                CySysWdtWriteMatch(WDT_COUNTER, WDT_TIMEOUT_FAST_SCAN);
                /* Reset WDT counter */
                CySysWdtResetCounters(WDT_COUNTER);
                
                /*Change the device mode to fast since a proximity sensor is triggered */
                deviceScanMode = FAST_SCAN_MODE; 
            }
            break;
        case FAST_SCAN_MODE:     
            if(0x00 == CapSense_CheckIsAnyWidgetActive())
            {
                 timeoutCounter++;
            } 
            else
            {
                timeoutCounter = 0x00;      
            }
            if(timeoutCounter >= SCANMODE_TIMEOUT_VALUE)
            {
               /* Configure the WDT counter match comparison value */
               CySysWdtWriteMatch(WDT_COUNTER, WDT_TIMEOUT_SLOW_SCAN); 
               /* Reset WDT counter */
               CySysWdtResetCounters(WDT_COUNTER);
            
               deviceScanMode = SLOW_SCAN_MODE;
               timeoutCounter = 0x00;
            }
            break;
        default:
            break;           
    }    
}


 #if !TUNER_ENABLE
/*******************************************************************************
* Function Name: BLE_HandleBLE
****************************************************************************//**
* @par Summary
*    BLE is handled here 
*
* @return
*    None
*
* @param[in] 
*    none
*
* @par Theory of Operation
*    BLE, Keyboard simulation, low power mode handling is done here.
*
* @par Notes
*    None
*
**//***************************************************************************/

void BLE_HandleBLE(void)
{
   
    CYBLE_LP_MODE_T lpMode;
    CYBLE_BLESS_STATE_T blessState;
    uint8 interruptState;
    
    if((CyBle_GetState() == CYBLE_STATE_CONNECTED) && (suspend != CYBLE_HIDS_CP_SUSPEND))
    {              
        if(keyboardSimulation == ENABLED)
        {
            /* Keyboard functionality is simulated */
            SimulateKeyboard(); 
        }      
    }
    if(!(CySysWdtGetInterruptSource() & WDT_INTERRUPT_SOURCE))
    {
        
        if(CyBle_GetState() != CYBLE_STATE_INITIALIZING)
        {
            CapSense_Sleep();
            /* Enter DeepSleep mode between connection intervals */
            lpMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP);
             interruptState = CyEnterCriticalSection();
            blessState = CyBle_GetBleSsState();

            if(lpMode == CYBLE_BLESS_DEEPSLEEP) 
            {   
                if(blessState == CYBLE_BLESS_STATE_ECO_ON || blessState == CYBLE_BLESS_STATE_DEEPSLEEP)
                {
                    
                    /* Put the device into the Deep Sleep mode */
                    CySysPmDeepSleep();
                    
                }
            }
            else
            {
                if(blessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
                {
                    CySysPmSleep();
                }
            }
           CyExitCriticalSection(interruptState);
        
           CapSense_Wakeup();
        }
        
    }
        
    CyBle_ProcessEvents();
}

#endif  

/*******************************************************************************
* Function Name: SendDataToMultichart
****************************************************************************//**
* @par Summary
*    This function sends data of either x-axis or y-axis sensor based on macro setting
*    to Tx8 in multichart format.
*
* @return
*    None
*
* @param[in] 
*    startSensorNumber: start sensor number index to send TX8 data
*	 endSensorNumber : TX8 data is sent till this index number
*
* @par Theory of Operation
*    Multichart tool expects a header followed by 3x number of data 
*    words (16 bit, big endian) and a pre-defined tail. This function sends 
*    following data:
*       a. rawcounts
*       b. baseline
*       c. difference counts
*
* @par Notes
*    None
*
**//***************************************************************************/
#if TX8_ENABLE
void SendDataToMultichart(uint8 startSensorNumber, uint8 endSensorNumber)
{
    uint8 sensor;
    
    /* Send header expected by multichart tool */
    Tx8_PutArray(multichartHeader, NO_OF_SAMPLES(multichartHeader));
    
    /* Send 3x words(uint16) of data (as expected by multichart) */
    /* Send Raw Counts, These are labeled as Raw Count 0  
     * to Raw Count (endSensorNumber-startSensorNumber)
     */
    for(sensor= startSensorNumber; sensor <= endSensorNumber; sensor++)
    {
        SendUint16ToMultichart(CapSense_ReadSensorRaw(sensor));
    }

	/* Send Baseline, These are labeled as Baseline 0  
     * to Baseline (endSensorNumber-startSensorNumber)
     */
    for(sensor= startSensorNumber; sensor <= endSensorNumber; sensor++)
    {
        SendUint16ToMultichart(CapSense_GetBaselineData(sensor));
    }
	/* Send Signal, These are labeled  as Signal 0  
     * to Signal (endSensorNumber-startSensorNumber)
     */
	for(sensor= startSensorNumber; sensor <= endSensorNumber; sensor++)
    {
        SendUint16ToMultichart(CapSense_GetDiffCountData(sensor));
    }
    /* Send the Tail as expected by multichart */
    Tx8_PutArray(multichartTail, NO_OF_SAMPLES(multichartTail));
	
}

void SendUint16ToMultichart(uint16 value)
{
    /* Send MSB */
    Tx8_PutChar(HI8(value));
    
    /* Send LSB */
    Tx8_PutChar(LO8(value));
    
}

#endif
/* [] END OF FILE */



