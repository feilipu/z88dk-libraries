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

/**************************************************************
        Position Cursor.
***************************************************************/

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

