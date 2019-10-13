#include <Wire.h>
#include <I2C_LCD.h>
I2C_LCD LCD;
uint8_t I2C_LCD_ADDRESS = 0x51; //Device address setting, default: 0x51
//uint8_t I2C_LCD_ADDRESS = 0x52; //Device address setting, default: 0x51

void setup(void)
{
    Wire.begin();           //I2C controler init.
}

void loop(void)
{
    LCD.CleanAll(WHITE);    //Erase all.
    delay(1000);            //Delay for 1s.

    //Set the font, character address update mode, display mode.
    //FM_ANL_AAA: FM_AutoNewLine_AutoAddrAdd
    LCD.FontModeConf(Font_10x20, FM_ANL_AAA, BLACK_BAC);
    LCD.DispStringAt("Sparking", 0, 10);    //Display string.
    delay(2000);            //Delay for 2s.

    //Modify the device address to 0x52, the default setting is 0x51.
    LCD.DeviceAddrEdit(0x52);
    
    //I want to display character, but because the device address is different from the program current address setting, so it can't work properly.
    LCD.DispStringAt("Hello!", 0, 30);
    
    //I2C_LCD_ADDRESS variable is the device address of the current program,
    //after the value of the I2C_LCD_ADDRESS variable is changed to 0x52 as below, the I2C_LCD can work normally.
    //I2C_LCD_ADDRESS = 0x52;
    //LCD.DispStringAt("Hello!", 0, 30);
    
    //If you have forgotten the I2C_LCD device address, you can recover to the factory settings: 0x51.
    //For details, please refer to the 8.3 section of the user manual.
    
    while(1); //Wait for ever.
}

