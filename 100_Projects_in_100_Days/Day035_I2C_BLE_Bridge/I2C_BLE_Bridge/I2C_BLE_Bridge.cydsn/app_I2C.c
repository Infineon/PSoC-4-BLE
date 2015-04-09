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

#include "app_I2C.h"

/*******************************************************************************
* Function Name: handleI2CTraffic
********************************************************************************
* Summary:
*    This function handles the I2C read or write processing
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void handleI2CTraffic(void)
{
	/* Wait for I2C master to complete a write */
	if(0u != (I2C_I2CSlaveStatus() & I2C_I2C_SSTAT_WR_CMPLT))
	{
		/* Read the number of bytes transferred */
		byteCnt = I2C_I2CSlaveGetWriteBufSize();

		/* Clear the write status bits*/
		I2C_I2CSlaveClearWriteStatus();

		sendI2CNotification();
		
		/* Clear the write buffer pointer so that the next write operation will
		start from index 0 */
		I2C_I2CSlaveClearWriteBuf();
	
	}
	/* If the master has read the data , reset the read buffer pointer to 0
	and clear the read status */
	if(0u != (I2C_I2CSlaveStatus() & I2C_I2C_SSTAT_RD_CMPLT))
	{
		/* Clear the read buffer pointer so that the next read operations starts
		from index 0 */
		I2C_I2CSlaveClearReadBuf();
		
#ifdef	RESET_I2C_READ_DATA
	    uint8 i;
	       
		for(i=0;i<I2C_READ_BUFFER_SIZE;i++)
			rdBuf[i] = 0;
#endif  /* RESET_I2C_READ_DATA */	
	
		/* Clear the read status bits */
		I2C_I2CSlaveClearReadStatus();
	}
}

/*******************************************************************************
* Function Name: sendI2CNotification
********************************************************************************
* Summary:
*    This function notifies the I2C data written by I2C master to the Client
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void sendI2CNotification(void)
{
	/* stores  notification data parameters */
	CYBLE_GATTS_HANDLE_VALUE_NTF_T		I2CHandle;	
	
	if(sendNotifications)
	{
		/* Package the notification data as part of I2C_read Characteristic*/
		I2CHandle.attrHandle = CYBLE_I2C_READ_I2C_READ_DATA_CHAR_HANDLE;				
		
		I2CHandle.value.val = wrBuf;
		
		I2CHandle.value.len = byteCnt;

	    /* Send the I2C_read Characteristic to the client only when notification is enabled */
		do
		{
		    apiResult = CyBle_GattsNotification(cyBle_connHandle,&I2CHandle);
			
			CyBle_ProcessEvents();
		
		}while((CYBLE_ERROR_OK != apiResult)  && (CYBLE_STATE_CONNECTED == cyBle_state));
		
	}		
}
/* [] END OF FILE */
