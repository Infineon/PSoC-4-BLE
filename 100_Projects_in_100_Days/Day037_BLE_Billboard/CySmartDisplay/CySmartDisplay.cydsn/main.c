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
#include <main.h>

int main()
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    StandardDisplayInit();
    
    CyGlobalIntEnable;
    
    DisplayMessage("PSoC Rocks!!", 12);
    
    CyBle_Start(StackEventHandler);
    
    while(1)
    {
        /* CyBle_ProcessEvents() allows BLE stack to process pending events */
        CyBle_ProcessEvents();
    }
}

/* [] END OF FILE */
