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

extern uint16 I2CWriteDataCharHandle;                /* Handle for the I2CWrite characteristic */

extern uint16 I2CReadDataCharHandle;                /* Handle for the I2CRead characteristic */

/*******************************************************************************
* Function Name: HandleI2CNotifications
********************************************************************************
* Summary:
*    This function handles the I2C notifications received from the peripheral
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void HandleI2CNotifications(CYBLE_GATTC_HANDLE_VALUE_NTF_PARAM_T *I2CDataNotification)
{
	uint8 i;
	
	/* Check if its I2C read notitifications */
    if(I2CDataNotification->handleValPair.attrHandle == I2CReadDataCharHandle)
    {
		/* Disable I2C interrupts before updating the data */
		I2C_DisableInt();
        
		/* Update I2C read registers with the notification data*/
		for(i=0;i<I2CDataNotification->handleValPair.value.len;i++)
			rdBuf[i] = I2CDataNotification->handleValPair.value.val[i];
		
		/* Enable I2C interrupts after updating the data */	
		I2C_EnableInt();	
    }
}

/*******************************************************************************
* Function Name: HandleI2CWrite
********************************************************************************
* Summary:
*    This function writes the data written by I2C master to the Gatt DB of the 
*     peripheral
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/

void HandleI2CWrite(void)
{	

	CYBLE_GATTC_WRITE_CMD_REQ_T     I2CDataWriteCmd;
  
	/* Check if the peripheral is connected before trying to write*/
	if(CyBle_GetState() == CYBLE_STATE_CONNECTED)
	{
		/* update the attribute handle to be written */
	    I2CDataWriteCmd.attrHandle = I2CWriteDataCharHandle;
	    
		I2CDataWriteCmd.value.len  = byteCnt;
	    
		I2CDataWriteCmd.value.val  = wrBuf;

	    do
	    {
	        apiResult = CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &I2CDataWriteCmd);
			
			CyBle_ProcessEvents();
			
	    }
	    while((CYBLE_ERROR_OK != apiResult) && (CYBLE_STATE_CONNECTED == cyBle_state));
	}
}

/*******************************************************************************
* Function Name: handle_I2C_traffic
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

void HandleI2CTraffic(void)
{
	/* Wait for I2C master to complete a write */
	if(0u != (I2C_I2CSlaveStatus() & I2C_I2C_SSTAT_WR_CMPLT))
	{
		/* Read the number of bytes transferred */
		byteCnt = I2C_I2CSlaveGetWriteBufSize();
		
		/* Clear the write status bits*/
		I2C_I2CSlaveClearWriteStatus();

		if(CyBle_GattGetBusStatus() != CYBLE_STACK_STATE_BUSY)
		{
			HandleI2CWrite();
			
			/* Clear the write buffer pointer so that the next write operation will
			start from index 0 */
			I2C_I2CSlaveClearWriteBuf();
		}
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
/* [] END OF FILE */

