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

