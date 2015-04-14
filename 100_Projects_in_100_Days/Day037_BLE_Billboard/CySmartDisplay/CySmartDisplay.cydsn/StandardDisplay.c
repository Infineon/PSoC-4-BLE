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
#include <font.h>

const uint8 MAX7219_REG_NOOP        = 0x0;
const uint8 MAX7219_REG_DIGIT0      = 0x1;
const uint8 MAX7219_REG_DIGIT1      = 0x2;
const uint8 MAX7219_REG_DIGIT2      = 0x3;
const uint8 MAX7219_REG_DIGIT3      = 0x4;
const uint8 MAX7219_REG_DIGIT4      = 0x5;
const uint8 MAX7219_REG_DIGIT5      = 0x6;
const uint8 MAX7219_REG_DIGIT6      = 0x7;
const uint8 MAX7219_REG_DIGIT7      = 0x8;
const uint8 MAX7219_REG_DECODEMODE  = 0x9;
const uint8 MAX7219_REG_INTENSITY   = 0xA;
const uint8 MAX7219_REG_SCANLIMIT   = 0xB;
const uint8 MAX7219_REG_SHUTDOWN    = 0xC;
const uint8 MAX7219_REG_DISPLAYTEST = 0xF;

uint16 pos = 0;

void SendPacketNoCS(const uint8 reg, uint8 data)
{    
    MAX7219_SpiUartWriteTxData(reg & 0x0F);
    MAX7219_SpiUartWriteTxData(data & 0xFF);
}

void showLetterIndex(const uint8 c, uint8 index, uint8 total)
{
    uint8 col;
    uint8 disp;
    
    for (col = 0; col < 8; col++)
    {
        CS_Write(LOW);
        CyDelayUs(SPI_DELAY);
        
        for (disp = 0; disp < total; disp++)
        {
            if (disp == index)
            {
                SendPacketNoCS (col + 1, cp437_font [c] [col]);
            }
            else
            {
                SendPacketNoCS (MAX7219_REG_NOOP, LOW);
            }
        }
        
        CyDelayUs(SPI_DELAY);
        CS_Write(HIGH);
        CyDelayUs(SPI_DELAY);
    }
}

void showString (const char * s, uint8 total)
{
    char c = *s++;
    uint8 disp = 0;
    
    while (c)
    {
        showLetterIndex(c, disp, total); 
        c = *s++;
        disp++;
    }
}

void scrollString (const char * s, const uint32 time)
{
    uint8 col;
    uint16 length = strlen(s) * 8;
    uint16 depth = 0;
    
    while (length-=1)
    {
        for (col = 0; col < 8; col++)
        {
            SendPacketNoCS (col + 1, cp437_font [(uint8)s[depth/8]] [depth%8]);
            depth++;
        }
        depth-=7;
        CyDelayUs(time);
    }
}

void showColumn(uint8 module, uint8 column, uint8 value)
{
    uint8 disp = 0;
    
    CS_Write(LOW);
    CyDelayUs(SPI_DELAY);
    
    for (disp = 0; disp < MODULE_COUNT; disp++)
    {
        if (module == disp)
        {
            SendPacketNoCS (column + 1, value);
        }
        else
        {
            SendPacketNoCS (MAX7219_REG_NOOP, LOW);
        }
    }
    
    CyDelayUs(SPI_DELAY);
    CS_Write(HIGH);
    CyDelayUs(SPI_DELAY);
}

char buffer[180] = {0};
char dispbuffer[MODULE_COUNT * 8] = {0};
CY_ISR(Display_ISR)
{
    uint16 disp;
    uint8 module, column;
    
    for (disp = ((MODULE_COUNT * 8) - 1); disp >= 1; disp--)
    {
        dispbuffer[disp] = dispbuffer[disp-1];
    }
    dispbuffer[0] = (char) cp437_font[(uint8)buffer[pos/8]][pos%8];
    
    for (disp = 0; disp < (MODULE_COUNT * 8); disp++)
    {
        module = ((MODULE_COUNT - (disp/8) - 1));
        column = (7 - (disp%8));
        showColumn(module, column, dispbuffer[disp]);
    }
    
    if (pos < ((strlen(buffer) + MODULE_COUNT) * 8))
    {
        pos++;
    }
    else
    {
        pos = 0;
    }
    
    DisplayTimer_ClearInterrupt(DisplayTimer_INTR_MASK_TC);
}

void DisplayMessage(char *message, uint8 length)
{
    uint8 i;
    
    Timer_CLK_Stop();
    pos = 0;
    
    stpncpy(buffer, message, length);
    for (i = length; i<180; i++)
    {
        buffer[i] = 0;
    }
    
    Timer_CLK_Start();
}

void DisplayBrightness(uint8 level)
{
    uint8 i = 0;
    
    CS_Write(LOW);
    CyDelayUs(SPI_DELAY);
    for (i = 0; i < MODULE_COUNT; i++)
    {
        SendPacketNoCS (MAX7219_REG_INTENSITY, level % 16);  // character intensity: range: 0 to 15
    }
    CyDelayUs(SPI_DELAY);
    CS_Write(HIGH);
    CyDelayUs(SPI_DELAY);
}

void DispaySpeed(uint8 speed)
{
    Timer_CLK_Stop();
    Timer_CLK_SetDividerValue(255 - (speed % 201));  // character speed: range: 80 to 255
    Timer_CLK_Start();
}

void StandardDisplayInit(void)
{
    uint8 col;
    uint8 i = 0;
    
    CS_Write(HIGH);
    MAX7219_Start();
    DispaySpeed(200);         // 0 - Slowest, 200 - Fastest
    DisplayTimer_Start();
    Disp_INT_StartEx(Display_ISR);

    CS_Write(LOW);
    CyDelayUs(SPI_DELAY);
    for (i = 0; i < MODULE_COUNT; i++)
    {
        SendPacketNoCS (MAX7219_REG_SCANLIMIT, 7);   // show all 8 digits
    }
    CyDelayUs(SPI_DELAY);
    CS_Write(HIGH);
    CyDelayUs(SPI_DELAY);
    
    CS_Write(LOW);
    CyDelayUs(SPI_DELAY);
    for (i = 0; i < MODULE_COUNT; i++)
    {
        SendPacketNoCS (MAX7219_REG_DECODEMODE, 0);  // using an led matrix (not digits)
    }
    CyDelayUs(SPI_DELAY);
    CS_Write(HIGH);
    CyDelayUs(SPI_DELAY);
    
    CS_Write(LOW);
    CyDelayUs(SPI_DELAY);
    for (i = 0; i < MODULE_COUNT; i++)
    {
        SendPacketNoCS (MAX7219_REG_DISPLAYTEST, 0); // no display test
    }
    CyDelayUs(SPI_DELAY);
    CS_Write(HIGH);
    CyDelayUs(SPI_DELAY);
    
    // clear display
    for (col = 0; col < 8; col++)
    {
        CS_Write(LOW);
        CyDelayUs(SPI_DELAY);
        for (i = 0; i < MODULE_COUNT; i++)
        {
            SendPacketNoCS (col + 1, 0);
        }
        CyDelayUs(SPI_DELAY);
        CS_Write(HIGH);
        CyDelayUs(SPI_DELAY);
    }

    CS_Write(LOW);
    CyDelayUs(SPI_DELAY);
    for (i = 0; i < MODULE_COUNT; i++)
    {
        SendPacketNoCS (MAX7219_REG_INTENSITY, 0);  // character intensity: range: 0 to 15
    }
    CyDelayUs(SPI_DELAY);
    CS_Write(HIGH);
    CyDelayUs(SPI_DELAY);
    
    CS_Write(LOW);
    CyDelayUs(SPI_DELAY);
    for (i = 0; i < MODULE_COUNT; i++)
    {
        SendPacketNoCS (MAX7219_REG_SHUTDOWN, 1);   // not in shutdown mode (ie. start it up)
    }
    CyDelayUs(SPI_DELAY);
    CS_Write(HIGH);
    CyDelayUs(SPI_DELAY);
    
    CyDelay(100);
}

/* [] END OF FILE */
