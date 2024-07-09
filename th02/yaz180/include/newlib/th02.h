/*
 * tho2.h
 * Driver for DIGITAL I2C HUMIDITY AND TEMPERATURE SENSOR
 *  
 * Copyright (c) 2014 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : Oliver Wang
 * Create Time: April 2014
 * Change Log :
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

include(__link__.m4)
undefine(`m4_SCCZ80_NOLIB')

#ifndef _TH02_H
#define _TH02_H

#include <inttypes.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Definitions                                                   ***/
/****************************************************************************/

/************************Readme first**************************
For the YAZ180 the LCD can be attached to either I2C Port 1 or Port 2.
These commence at addresses 0xA000 or 0x8000 respectively.
***************************************************************/

#define I2C_PORT2           0x80
#define I2C_PORT1           0xA0

#define TH02_I2C_DEV_ID     0x40

#define REG_STATUS          0x00 
#define REG_DATA_H          0x01
#define REG_DATA_L          0x02
#define REG_CONFIG          0x03
#define REG_ID              0x11

#define STATUS_RDY_MASK     0x01    //poll RDY,0 indicate the conversion is done

#define CMD_MEASURE_HUMI    0x01    //perform a humidity measurement
#define CMD_MEASURE_TEMP    0x11    //perform a temperature measurement

enum TH02_AttachPort
{
    TH02_PORT2 = I2C_PORT2,
    TH02_PORT1 = I2C_PORT1
};

/****************************************************************************/
/***        Function Definition                                           ***/
/****************************************************************************/

//  void th02_init(enum TH02_AttachPort device);
__OPROTO(,,void,,th02_init,enum TH02_AttachPort device)

//  float th02_read_temperature(enum TH02_AttachPort device);
__OPROTO(,,float,,th02_read_temperature,enum TH02_AttachPort device)

//  float th02_read_humidity(enum TH02_AttachPort device);
__OPROTO(,,float,,th02_read_humidity,enum TH02_AttachPort device)

#ifdef __cplusplus
}
#endif

#endif  /* _THO2_H  */
