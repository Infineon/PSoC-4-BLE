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

#include "app_SPI.h"

/*******************************************************************************
* Function Name: handleSPITraffic
********************************************************************************
* Summary:
*    This function handles the SPI read or write processing
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void handleSPITraffic(void)
{
	uint8 i;
		
	while(SPI_SpiIsBusBusy())
	{
		/* Wait till SPI read/write to complete */
	}
	
	/* Read the number of bytes transferred */
	byteCnt = SPI_SpiUartGetRxBufferSize();
	
	if(byteCnt)
	{
		for(i=0;i<byteCnt;i++)
			wrBuf[i] = SPI_SpiUartReadRxData();
			
		SPI_SpiUartClearRxBuffer();
	
		sendSPINotification();	
	}
}

/*******************************************************************************
* Function Name: sendSPINotification
********************************************************************************
* Summary:
*    This function notifies the SPI data written by SPI master to the Client
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void sendSPINotification(void)
{
	/* stores  notification data parameters */
	CYBLE_GATTS_HANDLE_VALUE_NTF_T		SPIHandle;	
	
	if(sendNotifications)
	{
		/* Package the notification data as part of I2C_read Characteristic*/
		SPIHandle.attrHandle = CYBLE_SPI_READ_SPI_READ_DATA_CHAR_HANDLE;				
		
		SPIHandle.value.val = wrBuf;
		
		SPIHandle.value.len = byteCnt;

	    /* Send the I2C_read Characteristic to the client only when notification is enabled */
		do
		{
		    apiResult = CyBle_GattsNotification(cyBle_connHandle,&SPIHandle);
			
			CyBle_ProcessEvents();
		
		}while((CYBLE_ERROR_OK != apiResult)  && (CYBLE_STATE_CONNECTED == cyBle_state));
		
	}		
}
/* [] END OF FILE */
