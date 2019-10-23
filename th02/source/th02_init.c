/*
 * th02_init.c
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

void th02_init(enum TH02_AttachPort device)
{
    i2c_reset((uint8_t)device);

    if (device == I2C1_PORT)
        i2c_interrupt_attach(I2C1_PORT, &i2c1_isr);
    if (device == I2C2_PORT)
        i2c_interrupt_attach(I2C2_PORT, &i2c2_isr);

    i2c_initialise((uint8_t)device);
    i2c_set_speed((uint8_t)device, I2C_SPEED_STD);
}

