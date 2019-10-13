#include <Wire.h>
#include <I2C_LCD.h>
I2C_LCD LCD;
uint8_t I2C_LCD_ADDRESS = 0x51; //Device address configuration, the default value is 0x51.

//For detials of the function useage, please refer to "I2C_LCD User Manual". 
//You can download the "I2C_LCD User Manual" from I2C_LCD WIKI page: http://www.seeedstudio.com/wiki/I2C_LCD

void setup(void)
{
    Wire.begin();         //I2C controller initialization.
}

void loop(void)
{
    LCD.CleanAll(WHITE);    //Clean the screen with black or white.
    delay(1000);            //Delay for 1s.

    //6*8 Font size, auto new line, black character on white back ground.
    LCD.FontModeConf(Font_6x8, FM_ANL_AAA, BLACK_BAC);
    LCD.DispStringAt("Sparking", 0, 10);    //Display "Sparking" on coordinate of (0, 10).
    delay(2000);            //Delay for 2s.

    //Modify the I2C_LCD device address to 0x52, the factory default is 0x51
    LCD.DeviceAddrEdit(0x52);

    //I want to display characters, but because the device addresse is different from the target address, so it can't work properly.
    LCD.DispStringAt("I can't display!", 0, 30);

    //Use I2C_LCD_ADDRESS to set the target device address.
    //Set the value 0x52 to I2C_LCD_ADDRESS, I2C_LCD can work normally.
    //If you forgot  the device address of I2C_LCD, you can recover factory setting by short circuit the REC pad and reboot.
    //For detials, please refer to "I2C_LCD User Manual". 
    I2C_LCD_ADDRESS = 0x52;
    
    //I changed the I2C_LCD_ADDRESS value to 0x52, so it can work properly.
    LCD.DispStringAt("Hello, my friend!", 0, 30);

    //Set the I2C_LCD device address back to the default(0x51).
    LCD.DeviceAddrEdit(0x51);
    
    while(1); //Wait for ever. 
}

