/************************************************************************************************************************
* Project Name		: PSoC_4_BLE_Pressure_Sensor
* File Name			: main.c
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit
*************************************************************************************************************************/

/************************************************************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
************************************************************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
***********************************************************************************************************************/

/************************************************************************************************************************
*                           THEORY OF OPERATION
*************************************************************************************************************************
* This project showcases the capability of PSoC 4 BLE to communicate with an I2C digital sensor and a BLE Central device.
* The data communicated over BLE is the Temperature, Barometric Pressure and Altitude value acquired from a BMP180 Pressure Sensor.
* This project uses an I2C component (Master) to acquire the data from the sensor and reports it to the Central device.
* The sensor data sent to the Central device over a Custom Service.
* The Custom Service allows notifications to be sent to Central device when enabled.
* 
* For Adafruit's BMP180 Pressure Sensor theory of operation, please refer to the sensor datasheet.
* http://www.adafruit.com/product/1603

*************************************************************************************************************************
* Note: The programming pins in the project have been configured as SWD.
* With this settings, the PSoC 4 BLE device consumes extra power through the programming pins.
* To prevent the leakage of power, the programming pins can be configured as GPIO. To configure,
* open the PSoC_4_BLE_Pressure_Sensor.cydwr file from the Workspace Explorer, go to Systems tab,
* and set the Debug Select option to GPIO.
*************************************************************************************************************************
* Hardware connection required:
* Red LED - P2[6] (Hard-wired on the BLE Pioneer Kit)
* Plug in the sensor to the outer row of the BLE Pioneer Kit Arduino compatible header J2
* Sensor SDA - P3[0] - J2.1
* Sensor SCL - P3[1] - J2.3
* Sensor GND - P3[2] - J2.5
* Sensor VIN - P3[4] - J2.7
************************************************************************************************************************/

#include <main.h>

/*************************Variables Declaration*************************************************************************/
uint8 ChangePowerPinDriveMode = TRUE; //This flag is set when the GATT disconnect event occurs
uint8 NotificationDelayCounter = NOTIFICATION_DELAY_COUNT; //Counter to handle the periodic notification
uint8 LEDDelayCounter = LED_DELAY_COUNT; //Counter to handle the LED blinking
long LastTemperatureData; //Variable to store the previous temperature reading
long LastPressureData; //Variable to store the previous pressure reading
long LastAltitudeData; //Variable to store the previous altitude reading
extern uint8 StartAdvertisement; //This flag is used to start advertisement
extern uint8 DeviceConnected; //This flag is set when a Central device is connected
extern uint8 TemperatureNotificationEnabled; //This flag is set when the Central device writes to CCCD to enable temperature notification
extern uint8 PressureNotificationEnabled; //This flag is set when the Central device writes to CCCD to enable pressure notification
extern uint8 AltitudeNotificationEnabled; //This flag is set when the Central device writes to CCCD to enable altitude notification
extern uint8 TemperatureNotificationData[0x04]; //The temperature notification value is stored in this array
extern uint8 PressureNotificationData[0x04]; //The pressure notification value is stored in this array
extern uint8 AltitudeNotificationData[0x04]; //The altitude notification value is stored in this array
extern long Temperature; //Variable to store the true temperature
extern long Pressure; //Variable to store the true pressure
extern long Altitude; //Variable to store the true altitude
static CYBLE_LP_MODE_T SleepMode; //Variable to store the status of BLESS Hardware block
static CYBLE_BLESS_STATE_T BlessState; //Variable to store the status of BLESS Hardware block
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: main
**************************************************************************************************************************
* Summary: This is the system entrance point. This function initializes the system and
* continuously process sensor data and BLE events. It also handles the low power mode.
*
* Parameters:
*  void
*
* Return:
*  int
*

*************************************************************************************************************************/
int main()
{
    InitializeSystem(); //Initialize the Components and enable interrupts
    
    for(;;)
    {
        /* Process event CallBack to handle BLE events.
        The events generated and used in this application are inside the CustomEventHandler routine */
        CyBle_ProcessEvents();
        
        HandleStatusLED(); //Update the status LED depending upon the BLE state
        
        if(DeviceConnected)
		{
			UpdateConnectionParameters(); //Update the Connection Parameters to run the BLE communication on desired interval
            
            UpdateNotificationCCCDAttribute(); //Update the CCCD writing by the Central device
            
            if(--NotificationDelayCounter == ZERO)
            {
                NotificationDelayCounter = NOTIFICATION_DELAY_COUNT; //Reset the notification counter
                ReadSensorDataAndNotify(); //Read the sensor and send notifications if enabled when the counter is reset
            }
		}
		
        EnterExitLowPowerMode(); //Enter low power mode
        
        if(StartAdvertisement)
        {
            StartAdvertisement = FALSE; //Clear the advertisement flag
            
            CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST); //Start advertisement and enter Discoverable mode
            
            LED_Write(ZERO); //Turn on the status LED
            LED_SetDriveMode(LED_DM_STRONG); //Set the LED pin drive mode to Strong
        }
    }
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: InitializeSystem
**************************************************************************************************************************
* Summary: This function starts the components and initialize system.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void InitializeSystem(void)
{
    CyGlobalIntEnable; //Enable global interrupts
    
    /* Start BLE component and register the CustomEventHandler function.
    This function exposes the events from BLE component for application use */
    CyBle_Start(CustomEventHandler);
    
    I2C_Start(); //Start the I2C component
    
    Btn_Interrupt_StartEx(ButtonISR); //Start the button ISR to allow wakeup from sleep
    
    GND_Write(ZERO); //Turn off the GND pin    
    GND_SetDriveMode(GND_DM_STRONG); //Set the GND pin drive mode to Strong
    
    VDD_Write(ONE); //Turn on the power pin    
    VDD_SetDriveMode(VDD_DM_STRONG); //Set the VDD pin drive mode to Strong
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: ReadSensorDataAndNotify
**************************************************************************************************************************
* Summary: This function checks whether any of the notifications is enabled and sends the sensor data as BLE notification.
* If any notification is enabled and the sensor data has changed from the previous readings, it sends the notification.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void ReadSensorDataAndNotify(void)
{
    /* Start sensor data processing if any notification is enabled */
    if((TemperatureNotificationEnabled || PressureNotificationEnabled || AltitudeNotificationEnabled))
    {
        ReadAndCalculateSensorData(); //Read and calculate the sensor data
    }
    
    /* Check whether the temperature notification is enabled and the data has changed from the previous reading */
    if(TemperatureNotificationEnabled && (Temperature != LastTemperatureData))
    {
        /* Convert the temperature data into notification type */
        TemperatureNotificationData[0x00] = Temperature;
        TemperatureNotificationData[0x01] = Temperature >> 0x08;
        TemperatureNotificationData[0x02] = Temperature >> 0x10;
        TemperatureNotificationData[0x03] = Temperature >> 0x18;
        
        LastTemperatureData = Temperature; //Store the current temperature data for reference
        
        SendTemperatureOverNotification(); //Send temperature data as notification
    }
    
    /* Check whether the pressure notification is enabled and the data has changed from the previous reading */
    if(PressureNotificationEnabled && (Pressure != LastPressureData))
    {
        /* Convert the pressure data into notification type */
        PressureNotificationData[0x00] = Pressure;
        PressureNotificationData[0x01] = Pressure >> 0x08;
        PressureNotificationData[0x02] = Pressure >> 0x10;
        PressureNotificationData[0x03] = Pressure >> 0x18;
        
        LastPressureData = Pressure; //Store the current pressure data for reference
        
        SendPressureOverNotification(); //Send pressure data as notification
    }
    
    /* Check whether the altitude notification is enabled and the data has changed from the previous reading */
    if(AltitudeNotificationEnabled && (Altitude != LastAltitudeData))
    {
        /* Convert the altitude data into notification type */
        AltitudeNotificationData[0x00] = Altitude;
        AltitudeNotificationData[0x01] = Altitude >> 0x08;
        AltitudeNotificationData[0x02] = Altitude >> 0x10;
        AltitudeNotificationData[0x03] = Altitude >> 0x18;
        
        LastAltitudeData = Altitude; //Store the current altitude data for reference
        
        SendAltitudeOverNotification(); //Send altitude data as notification
    }
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: HandleStatusLED
**************************************************************************************************************************
* Summary: This function controls the status LED depending upon the BLE state.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void HandleStatusLED(void)
{
    /* Check whether the device is advertising and blink the LED */
    if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
    {
        if(--LEDDelayCounter == ZERO)
        {
            LEDDelayCounter = LED_DELAY_COUNT; //Reset the LED state counter
            LED_Write(!LED_Read()); //Toggle the status LED when the counter is reset
        }
    }    
    else
    {
        LED_Write(ONE); //Turn off the status LED
        LED_SetDriveMode(LED_DM_ALG_HIZ); //Set the LED pin drive mode to Hi-Z
    }
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: EnterExitLowPowerMode
**************************************************************************************************************************
* Summary: This function puts the BLESS and CPU to deep sleep mode. 
* System will resume from here when it wakes up from user button press.
*
* Parameters:
*  void
*
* Return:
*  void
*
*************************************************************************************************************************/
void EnterExitLowPowerMode(void)
{
	I2C_Sleep(); //Put I2C to sleep
    
    /* Change the pin drive mode to save power consumption */
    if(ChangePowerPinDriveMode)
    {
        GND_SetDriveMode(GND_DM_ALG_HIZ); //Set the GND pin drive mode to Hi-Z
        VDD_SetDriveMode(VDD_DM_ALG_HIZ); //Set the VDD pin drive mode to Hi-Z
    }
	
	SleepMode = CyBle_EnterLPM(CYBLE_BLESS_DEEPSLEEP); //Put the BLESS to sleep
    
    CyGlobalIntDisable; //Disable global interrupts
    
    BlessState = CyBle_GetBleSsState(); //Check the Status of BLESS
    
    if(SleepMode == CYBLE_BLESS_DEEPSLEEP)
    {
        if(BlessState == CYBLE_BLESS_STATE_ECO_ON || BlessState == CYBLE_BLESS_STATE_DEEPSLEEP)
        {
            CySysPmDeepSleep(); //Put the CPU to Deep Sleep mode when the ECO has started
        }
    }
    else
    {    
        if(BlessState != CYBLE_BLESS_STATE_EVENT_CLOSE)
        {
            CySysPmSleep(); //Put the CPU to Sleep mode if all the BLE events are not closed
        }
    }
    
    CyGlobalIntEnable; //Enable global interrupts
    
    /* Revert the pin drive mode back to the original set in the initialization */
    if(ChangePowerPinDriveMode)
    {
        GND_SetDriveMode(GND_DM_STRONG); //Set the GND pin drive mode to Strong
        VDD_SetDriveMode(VDD_DM_STRONG); //Set the VDD pin drive mode to Strong
    }
    
    I2C_Wakeup(); //Wakeup the I2C
}
/***********************************************************************************************************************/

/*************************************************************************************************************************
* Function Name: CY_ISR
**************************************************************************************************************************
* Summary: ISR routine for button interrupt. The system enters here after the CPU wakeup.
* It also Clears pending interrupt.
*
* Parameters:
*  ButtonISR - Address of the ISR to set in the interrupt vector table
*
* Return:
*  void
*
*************************************************************************************************************************/
CY_ISR(ButtonISR)
{
    /* Clear the pending interrupts */
    Btn_Interrupt_ClearPending();    
    Button_ClearInterrupt();
}
/***********************************************************************************************************************/

/* [] END OF FILE */
