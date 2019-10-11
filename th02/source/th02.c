/*
 * th02.c
 * Driver for DIGITAL I2C HUMIDITY AND TEMPERATURE SENSOR
 *  
 * Copyright (c) 2014 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Oliver Wang
 * Create Time: April 2014
 * Change Log : October 2019 - @feilipu - C ified, and converted for z88dk
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <inttypes.h>
#include <stdlib.h>

#include <arch.h>

#if __YAZ180
#include <arch/yaz180/i2c.h> /* Declarations of YAZ180 I2C functions */
#endif

#include "th02.h"

/****************************************************************************/
/***       Local Functions                                                ***/
/****************************************************************************/

static uint8_t is_available(void);
static void th02_write_cmd(uint8_t u8Cmd);
static void th02_write_reg(uint8_t u8Reg,uint8_t u8Data);
static uint8_t th02_read_reg(uint8_t u8Reg);
static uint16_t th02_read_data(void);

/****************************************************************************/
/***       Functions                                                      ***/
/****************************************************************************/

float th02_read_temperature(void)
{
    uint16_t value;
    float temperature;

    /* Start a new temperature conversion */

    th02_write_reg(REG_CONFIG, CMD_MEASURE_TEMP);

    /* Wait until conversion is done */
    while(!is_available());

    value = th02_read_data();

    value = value >> 2;
    /*
      Formula:
      Temperature(C) = (Value/32) - 50
    */

    return temperature = (value/32.0)-50.0;
}

float th02_read_humidity(void)
{
    uint16_t value;
    float humidity;

 /* Start a new humidity conversion */
    th02_write_reg(REG_CONFIG, CMD_MEASURE_HUMI);

    /* Wait until conversion is done */
    while(!is_available());

    value = th02_read_data();

    value = value >> 4;
 
    /*
      Formula:
      Humidity(%) = (Value/16) - 24
    */

    return humidity = (value/16.0)-24.0;
}

/****************************************************************************/
/***       Local Functions                                                ***/
/****************************************************************************/
uint8_t is_available(void)
{
    uint8_t status =  th02_read_reg(REG_STATUS);
    if(status & STATUS_RDY_MASK)
    {
        return 0;    //ready
    }
    else
    {
        return 1;    //not ready yet
    }
}

void th02_write_cmd(uint8_t u8Cmd)
{
    i2c_write( I2C2_PORT, TH02_I2C_DEV_ID, &u8Cmd, 1, I2C_STOP|I2C_MODE_BUFFER);
}

void th02_write_reg(uint8_t u8Reg,uint8_t u8Data)
{ 
    uint8_t writeBuffer[2];

    writeBuffer[0] = u8Reg;
    writeBuffer[1] = u8Data;

    i2c_write( I2C2_PORT, TH02_I2C_DEV_ID, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER);
}

uint8_t th02_read_reg(uint8_t u8Reg)
{
    uint8_t u8_read_reg;

    i2c_write( I2C2_PORT, TH02_I2C_DEV_ID, &u8Reg, 1, I2C_RESTART|I2C_MODE_BUFFER);
    i2c_read_set( I2C2_PORT, TH02_I2C_DEV_ID, &u8_read_reg, 1, I2C_STOP|I2C_MODE_BUFFER);
    i2c_read_get( I2C2_PORT, TH02_I2C_DEV_ID, 1);

    return u8_read_reg;
} 

uint16_t th02_read_data(void)
{
    uint8_t readBuffer[2];
    uint16_t returnWord;

    readBuffer[0] = REG_DATA_H;

    i2c_write( I2C2_PORT, TH02_I2C_DEV_ID, readBuffer, 1, I2C_RESTART|I2C_MODE_BUFFER);
    i2c_read_set( I2C2_PORT, TH02_I2C_DEV_ID, readBuffer, 2, I2C_STOP|I2C_MODE_BUFFER);
    i2c_read_get( I2C2_PORT, TH02_I2C_DEV_ID, 1);

    return returnWord = (readBuffer[0]<<8)|(readBuffer[1]);
}
