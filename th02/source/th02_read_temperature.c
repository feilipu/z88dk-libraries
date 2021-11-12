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
/***       Functions                                                      ***/
/****************************************************************************/

float th02_read_temperature(enum TH02_AttachPort device)
{
    const uint8_t writeBuffer[2] = {REG_CONFIG, CMD_MEASURE_TEMP};
    const uint8_t regStatus = REG_STATUS;
    const uint8_t regData = REG_DATA_H;

    uint8_t readBuffer[2] = {0};

    uint16_t value;
    float temperature;

    /* Start a new temperature conversion */
    i2c_write( (uint8_t)device, TH02_I2C_DEV_ID, writeBuffer, 2, I2C_STOP|I2C_MODE_BUFFER);

    /* Wait until conversion is done */
    do{
    i2c_write( (uint8_t)device, TH02_I2C_DEV_ID, (uint8_t *)&regStatus, 1, I2C_RESTART|I2C_MODE_BUFFER);
    i2c_read_set( (uint8_t)device, TH02_I2C_DEV_ID, readBuffer, 1, I2C_STOP|I2C_MODE_BUFFER);
    i2c_read_get( (uint8_t)device, TH02_I2C_DEV_ID, 1);
    }while(readBuffer[0]&STATUS_RDY_MASK != 0);

    /* Get the reading */
    i2c_write( (uint8_t)device, TH02_I2C_DEV_ID, (uint8_t *)&regData, 1, I2C_RESTART|I2C_MODE_BUFFER);
    i2c_read_set( (uint8_t)device, TH02_I2C_DEV_ID, readBuffer, 2, I2C_STOP|I2C_MODE_BUFFER);
    i2c_read_get( (uint8_t)device, TH02_I2C_DEV_ID, 2);

    value = (readBuffer[0]<<8)|(readBuffer[1]);
    value = value >> 2;

    /* Conversion Formula: Temperature(C) = (Value/32) - 50 */
    return temperature = (value/32.0)-50.0;
}

