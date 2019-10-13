#include <Wire.h>
#include <I2C_LCD.h>
I2C_LCD LCD;
uint8_t I2C_LCD_ADDRESS = 0x51; //Device address setting, default: 0x51

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
    LCD.FontModeConf(Font_6x8, FM_ANL_AAA, BLACK_BAC);
    LCD.DispStringAt("Sparking", 108, 10);   //Display string.
    delay(3000);            //Delay for 3s.
    
    //Set the font, character address update mode, display mode.
    //FM_MNL_AAA: FM_ManualNewLine_AutoAddrAdd
    LCD.FontModeConf(Font_6x8, FM_MNL_AAA, BLACK_BAC);
    LCD.DispStringAt("Sparking", 108, 40);   //Display string.
    
    while(1); //Wait for ever.
}

