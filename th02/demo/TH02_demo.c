/*
 * Demo name   : TH02_demo 
 * Usage       : DIGITAL I2C HUMIDITY AND TEMPERATURE SENSOR 
 * Author      : Phillip Steven @feilipu
 * Version     : V0.1
*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <arch/yaz180.h>
#include <arch/yaz180/i2c.h>

#include <lib/yaz180/th02.h>

// ZSDCC compile
// zcc +yaz180 -subtype=app -v -m -SO3 --list --math32_z180  -llib/yaz180/th02 --c-code-in-asm --max-allocs-per-node200000 @i2ctest.lst -o i2ctest -create-app

// SCCZ80 compile
// zcc +yaz180 -subtype=app -clib=new -v -m -SO3 --list --math32_z180 -llib/yaz180/th02 @i2ctest.lst -o i2ctest -create-app

#pragma printf = "%s %c %f %g"     // Enables %s %c %f %g only. Others: %i %X %lx %u

void main(void)
{
    float temperature;
    float humidity;

    i2c_reset(I2C2_PORT); // connected to the I2C Device 2
    i2c_interrupt_attach(I2C2_PORT, &i2c2_isr);

    i2c_initialise(I2C2_PORT);
    i2c_set_speed(I2C2_PORT, I2C_SPEED_STD);

    temperature = th02_read_temperature(I2C2_PORT);
    printf("Temperature: %gc\n",temperature);

    humidity = th02_read_humidity(I2C2_PORT);
    printf("Humidity: %g%%\n",humidity);
}
