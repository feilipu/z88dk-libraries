/******************************************************************************
  I2C_LCD.cpp - I2C_LCD library Version 1.11
  Copyright (C), 2015, Sparking Work Space. All right reserved.

 ******************************************************************************

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is build for I2C_LCD12864. Please do not use this library on
  any other devices, that could cause unpredictable damage to the unknow device.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <arch/yaz180.h>
#include <arch/yaz180/i2c.h>

#include "i2c_lcd.h"
 
/*const Font_Info_t fontInfo[LCD_NUM_OF_FONT] =
{
    Font_6x8, 6, 8,
    Font_6x12, 6, 12,
    Font_8x16_1, 8, 16,
    Font_8x16_2, 8, 16,
    Font_10x20, 10, 20,
    Font_12x24, 12, 24,
    Font_16x32, 16, 32,
};*/

/*********** The start of I2C_LCD basic driver APIs ***********/

uint8_t LCD_Port;   // Global for PCA9665 Device Port MSB

const uint8_t fontYsizeTab[LCD_NUM_OF_FONT] = {8, 12, 16, 16, 20, 24, 32};

/**************************************************************
      I2C init.
***************************************************************/

void LCD_Init(enum LCD_AttachPort port)
{
    LCD_Port = (uint8_t)port;

    i2c_reset(LCD_Port);

    if (port == I2C1_PORT)
        i2c_interrupt_attach(LCD_Port, &i2c1_isr);
    if (port == I2C2_PORT)
        i2c_interrupt_attach(LCD_Port, &i2c2_isr);

    i2c_initialise(LCD_Port);
    i2c_set_speed(LCD_Port, I2C_SPEED_FAST);
}

/**************************************************************
      Read one byte from device register.
***************************************************************/
uint8_t LCD_ReadByteFromReg(enum LCD_RegAddress regAddr)
{
    static uint8_t regAddress;
    static uint8_t readByte;

    regAddress = (uint8_t)regAddr;

    i2c_write( LCD_Port, LCD_ADDRESS, &regAddress, 1, I2C_STOP|I2C_MODE_BUFFER );
    i2c_read_set( LCD_Port, LCD_ADDRESS, &readByte, 1, I2C_STOP|I2C_MODE_BUFFER );
    i2c_read_get( LCD_Port, LCD_ADDRESS, 1);
    return readByte;
}

/**************************************************************
      Write one byte to device register.
***************************************************************/
void LCD_WriteByteToReg(enum LCD_RegAddress regAddr, const uint8_t byte)
{
    static uint8_t writeBuffer[2];

    writeBuffer[0] = (uint8_t)regAddr;
    writeBuffer[1] = byte;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

/**************************************************************
      Read multiple bytes from device register.
***************************************************************/
void LCD_ReadSeriesFromReg(enum LCD_RegAddress regAddr, uint8_t *buf, uint16_t length)
{
    static uint8_t regAddress;
    static uint8_t *bufIndex;

    regAddress = (uint8_t)regAddr;
    bufIndex = (uint8_t *)buf;

    if(length > I2C_RX_SENTENCE) return; // maximum Rx sentence length is 68 Bytes

    i2c_write( LCD_Port, LCD_ADDRESS, &regAddress, 1, I2C_STOP|I2C_MODE_BUFFER );
    i2c_read_set( LCD_Port, LCD_ADDRESS, bufIndex, (uint8_t)length, I2C_STOP|I2C_MODE_BUFFER );
    i2c_read_get( LCD_Port, LCD_ADDRESS, (uint8_t)length);
}

/**************************************************************
      Write multiple bytes to device register.
***************************************************************/
void LCD_WriteSeriesToReg(enum LCD_RegAddress regAddr, const uint8_t *buf, uint16_t length)
{
    static uint8_t writeBuffer[I2C_TX_SENTENCE+1];
    static uint8_t *bufIndex;

    writeBuffer[0] = (uint8_t)regAddr;
    bufIndex = (uint8_t *)buf;

    while( length/I2C_TX_SENTENCE && i2c_available(LCD_Port) )
    {
        memcpy(&writeBuffer[1], bufIndex, I2C_TX_SENTENCE); // do buffer copy only when bus is available
        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, I2C_TX_SENTENCE+1, I2C_RESTART|I2C_MODE_BUFFER );
        bufIndex+=I2C_TX_SENTENCE;
        length-=I2C_TX_SENTENCE;
    }

    if( length%I2C_TX_SENTENCE && i2c_available(LCD_Port) )
    {
        memcpy(&writeBuffer[1], bufIndex, length); // do buffer copy only when bus is available
        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, length+1, I2C_STOP|I2C_MODE_BUFFER );
    }
    else
    {
        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 1, I2C_STOP|I2C_MODE_BUFFER );
    }
}

/************ The end of I2C_LCD basic driver APIs ************/

#ifdef  SUPPORT_FULL_API_LIB
#if  SUPPORT_FULL_API_LIB == TRUE

void LCD_CharGotoXY(uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[3];

    writeBuffer[0] = (uint8_t)CharXPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_FontModeConf(enum LCD_FontSort font, enum LCD_FontMode mode, enum LCD_CharMode cMode)
{
    static uint8_t writeBuffer[2];

    writeBuffer[0] = (uint8_t)FontModeRegAddr;
    writeBuffer[1] = (uint8_t)cMode|(uint8_t)mode|(uint8_t)font;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_DispCharAt(char character, uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[3];
    
    writeBuffer[0] = (uint8_t)CharXPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );
    
    if( i2c_available(LCD_Port) ){
        writeBuffer[0] = (uint8_t)DisRAMAddr;
        writeBuffer[1] = (uint8_t)character;
    }

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_DispStringAt(char *buf, uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[3];

    size_t length = strlen(buf);

    writeBuffer[0] = (uint8_t)CharXPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );

    LCD_WriteSeriesToReg(DisRAMAddr, (uint8_t *)buf, (uint16_t)length);
}

void LCD_CursorConf(enum LCD_SwitchState swi, uint8_t freq)
{
    static uint8_t writeBuffer[2];

    writeBuffer[0] = (uint8_t)CursorConfigRegAddr;
    writeBuffer[1] = (uint8_t)(swi<<7)|freq;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_CursorGotoXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    static uint8_t writeBuffer[5];

    writeBuffer[0] = (uint8_t)CharXPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;
    writeBuffer[3] = width;
    writeBuffer[4] = height;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 5, I2C_STOP|I2C_MODE_BUFFER );
}

#ifdef  SUPPORT_2D_GRAPHIC_LIB
#if  SUPPORT_2D_GRAPHIC_LIB == TRUE

void LCD_DrawDotAt(uint8_t x, uint8_t y, enum LCD_ColorSort color)
{
    static uint8_t writeBuffer[3];

    if(x<LCD_X_SIZE_MAX && y<LCD_Y_SIZE_MAX)
    {
        writeBuffer[0] = (uint8_t)DrawDotXPosRegAddr;
        writeBuffer[1] = x;
        writeBuffer[2] = (uint8_t)(color<<7)|y;

        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );
    }
}

void LCD_DrawHLineAt(uint8_t startX, uint8_t endX, uint8_t y, enum LCD_ColorSort color)
{
    LCD_DrawLineAt(startX, endX, y, y, (uint8_t)color);
}

void LCD_DrawVLineAt(uint8_t startY, uint8_t endY, uint8_t x, enum LCD_ColorSort color)
{
    LCD_DrawLineAt(x, x, startY, endY, (uint8_t)color);
}

void LCD_DrawLineAt(uint8_t startX, uint8_t endX, uint8_t startY, uint8_t endY, enum LCD_ColorSort color)
{
    static uint8_t writeBuffer[5];

    if(endY < LCD_Y_SIZE_MAX)
    {
        writeBuffer[0] = (uint8_t)DrawLineStartXRegAddr;
        writeBuffer[1] = startX;
        writeBuffer[2] = endX;
        writeBuffer[3] = startY;
        writeBuffer[4] = (uint8_t)(color<<7)|endY;

        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 5, I2C_STOP|I2C_MODE_BUFFER );
    }
}

void LCD_DrawRectangleAt(uint8_t x, uint8_t y, uint8_t width, uint8_t height, enum LCD_DrawMode mode)
{
    static uint8_t writeBuffer[6];

    writeBuffer[0] = (uint8_t)DrawRectangleXPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;
    writeBuffer[3] = width;
    writeBuffer[4] = height;
    writeBuffer[5] = (uint8_t)mode;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 6, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_DrawCircleAt(uint8_t x, uint8_t y, uint8_t r, enum LCD_DrawMode mode)
{
    static uint8_t writeBuffer[5];

    if(x<LCD_X_SIZE_MAX && y<LCD_Y_SIZE_MAX && r<LCD_Y_SIZE_MAX)
    {
        writeBuffer[0] = (uint8_t)DrawCircleXPosRegAddr;
        writeBuffer[1] = x;
        writeBuffer[2] = y;
        writeBuffer[3] = r;
        writeBuffer[4] = (uint8_t)mode;

        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 5, I2C_STOP|I2C_MODE_BUFFER );
    }
}

void LCD_DrawScreenAreaAt(GUI_Bitmap_t *bitmap, uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[5];
    static uint8_t *buf;

    uint16_t byteMax;
    
    buf = (uint8_t *)bitmap->pData;

    if(y<LCD_X_SIZE_MAX && x<LCD_Y_SIZE_MAX)
    {
        writeBuffer[0] = (uint8_t)DrawBitmapXPosRegAddr;
        writeBuffer[1] = x;
        writeBuffer[2] = y;
        writeBuffer[3] = bitmap->XSize;

        byteMax = bitmap->BytesPerLine * bitmap->XSize;

        writeBuffer[4] = bitmap->YSize;

        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 5, I2C_STOP|I2C_MODE_BUFFER );

        LCD_WriteSeriesToReg(DisRAMAddr, buf, byteMax);
    }
}

void LCD_DrawFullScreen(const uint8_t *buf)
{
    static uint8_t writeBuffer[3];
    
    writeBuffer[0] = (uint8_t)WriteRAM_XPosRegAddr;
    writeBuffer[1] = 0;
    writeBuffer[2] = 0;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );

    for(uint16_t i=0; i<LCD_X_SIZE_MAX*LCD_Y_SIZE_MAX/8; ++i) {
        if( i2c_available(LCD_Port) ){
            writeBuffer[0] = (uint8_t)DisRAMAddr;
            writeBuffer[1] = buf[i];
        }
        i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
    }
}

#endif
#endif

uint8_t LCD_ReadByteDispRAM(uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[3];

    writeBuffer[0] = (uint8_t)ReadRAM_XPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );

    return LCD_ReadByteFromReg(DisRAMAddr);
}

void LCD_ReadSeriesDispRAM(uint8_t *buf, uint16_t length, uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[3];

    writeBuffer[0] = (uint8_t)ReadRAM_XPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );

    LCD_ReadSeriesFromReg(DisRAMAddr, buf, length);
}

void LCD_WriteByteDispRAM(uint8_t byte, uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[3];
    
    writeBuffer[0] = (uint8_t)WriteRAM_XPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );

    if( i2c_available(LCD_Port) ) {
        writeBuffer[0] = (uint8_t)DisRAMAddr;
        writeBuffer[1] = byte;
    }

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_WriteSeriesDispRAM(uint8_t *buf, uint16_t length, uint8_t x, uint8_t y)
{
    static uint8_t writeBuffer[3];

    writeBuffer[0] = (uint8_t)WriteRAM_XPosRegAddr;
    writeBuffer[1] = x;
    writeBuffer[2] = y;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );

    LCD_WriteSeriesToReg(DisRAMAddr, buf, length);
}

void LCD_DisplayConf(enum LCD_DisplayMode mode)
{
    static uint8_t writeBuffer[2];

    writeBuffer[0] = (uint8_t)DisplayConfigRegAddr;
    writeBuffer[1] = (uint8_t)mode;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_WorkingModeConf(enum LCD_SwitchState logoSwi, enum LCD_SwitchState backLightSwi, enum LCD_WorkingMode mode)
{
    static uint8_t writeBuffer[2];

    writeBuffer[0] = (uint8_t)WorkingModeRegAddr;
    writeBuffer[1] = 0x50|(uint8_t)(logoSwi<<3)|(uint8_t)(backLightSwi<<2)|(uint8_t)mode;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_BacklightConf(enum LCD_SettingMode mode, uint8_t byte)
{
    static uint8_t writeBuffer[2];

    writeBuffer[0] = (uint8_t)BackLightConfigRegAddr;
    writeBuffer[1] = (uint8_t)mode|(byte&0x7f);

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_ContrastConf(enum LCD_SettingMode mode, uint8_t byte)
{
    static uint8_t writeBuffer[2];

    writeBuffer[0] = (uint8_t)ContrastConfigRegAddr;
    writeBuffer[1] = (uint8_t)mode|(byte&0x3f);

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_DeviceAddrEdit(uint8_t newAddr)
{
    static uint8_t writeBuffer[3];

    writeBuffer[0] = (uint8_t)DeviceAddressRegAddr;
    writeBuffer[1] = 0x80;
    writeBuffer[2] = newAddr;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 3, I2C_STOP|I2C_MODE_BUFFER );
}

void LCD_CleanAll(enum LCD_ColorSort color)
{
    static uint8_t writeBuffer[2];
    uint8_t buf;

    buf = LCD_ReadByteFromReg(DisplayConfigRegAddr);

    writeBuffer[0] = (uint8_t)DisplayConfigRegAddr;
    writeBuffer[1] = buf;

    if(color == WHITE)
        writeBuffer[1] = (buf|0x40)&0xdf;
    if(color == BLACK)
        writeBuffer[1] = buf|0x60;

    i2c_write( LCD_Port, LCD_ADDRESS, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER );
}

#endif
#endif
