/*******************************************************************************
* File Name: BleCommunication.h
*
* Version: 1.0
*
* Description:
*
*
* Hardware Dependency:
*  CY8CKIT-042 BLE
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#if !defined(BleCommunicaton_H)
#define BleCommunicaton_H

#include "Options.h"
#include "cytypes.h"
#include "project.h"

extern void EMI_I2CM_Start(void);
extern void CyBle_ProcessEvents(void);
extern void HandleLeds(void);


void CyBtldrCommStart(void);
cystatus CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);
cystatus CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut);

#endif /* BleCommunicaton_H */


/* [] END OF FILE */
