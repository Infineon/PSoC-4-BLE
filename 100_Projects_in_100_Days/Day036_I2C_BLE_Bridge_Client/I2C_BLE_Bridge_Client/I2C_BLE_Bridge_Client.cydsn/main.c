/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "main.h"

uint8 wrBuf[I2C_WRITE_BUFFER_SIZE]; /* I2C write buffer */

uint8 rdBuf[I2C_READ_BUFFER_SIZE];  /* I2C read buffer */

uint32 byteCnt; /* variable to store the number of bytes written by I2C mater */

CYBLE_API_RESULT_T	apiResult; /* Variable to store BLE api retruns */

/*******************************************************************************
* Function Name: I2CBLEClientInit
********************************************************************************
* Summary:
*    This function initializes the BLE and I2C component
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void I2CBLEClientInit(void)
{
	
#ifndef	 ENABLE_I2C_ONLY_WHEN_CONNECTED	
	
	/* Start I2C Slave operation */
	I2C_Start();
	
	/* Initialize I2C write buffer */
	I2C_I2CSlaveInitWriteBuf((uint8 *) wrBuf, I2C_WRITE_BUFFER_SIZE);
	
	/* Initialize I2C read buffer */
	I2C_I2CSlaveInitReadBuf((uint8 *) rdBuf, I2C_READ_BUFFER_SIZE);
	
#endif
    
	/* Start BLE operation */	
   	apiResult = CyBle_Start(AppCallBack);
	
	if(apiResult != CYBLE_ERROR_OK)
	{
		
#ifdef LED_INDICATION	
	
		DISCON_LED_ON();
		
#endif /* LOW_POWER_MODE */		
		
		while(1);
		/* Failed to initialize stack */
	}
}

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*        System entrance point. This calls the initializing function and
* continuously process BLE and I2C events.
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
	/* Enable the Global Interrupt */	
    CyGlobalIntEnable; 
    
#ifdef LED_INDICATION		

	/*Turn off all LEDS */
	ALL_LED_OFF();

#else
	/* Set the LED pins drive mode to Strong drive */
	DISCON_LED_SetDriveMode(DISCON_LED_DM_ALG_HIZ);
	
	CONNECT_LED_SetDriveMode(CONNECT_LED_DM_ALG_HIZ);
	
	SCAN_LED_SetDriveMode(SCAN_LED_DM_ALG_HIZ);
	
#endif	
    
    I2CBLEClientInit();
        
    for(;;)
    {  
		
#ifdef LOW_POWER_MODE	
	
		handleLowPowerMode();
		
#endif /* LOW_POWER_MODE */		
		
		/* Process queued BLE events */
        CyBle_ProcessEvents();        
        
		/* Handle I2C read and write */
		HandleBleProcessing();
		
#ifndef ENABLE_I2C_ONLY_WHEN_CONNECTED
	
		HandleI2CTraffic();
		
#endif	
		
    }
}

/* [] END OF FILE */

