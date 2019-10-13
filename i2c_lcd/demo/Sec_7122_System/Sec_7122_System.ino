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
    uint8_t buf[3] = {0xFA,0xAF,0xA5};  //Buffer.
    
    LCD.CleanAll(WHITE);    //Erase all.
    delay(1000);            //Delay for 1s.
    
    //Set the font, character address update mode, display mode.
    //FM_ANL_AAA: FM_AutoNewLine_AutoAddrAdd
    LCD.FontModeConf(Font_8x16_1, FM_ANL_AAA, BLACK_BAC);
    LCD.DispStringAt("Hello!", 50, 10);     //Display string.
    delay(3000);            //Delay for 3s.

    //Booting logo ON, backlight ON, RAM work mode.
    //If you want to display characters please switch to WM_CharMode.
    LCD.WorkingModeConf(ON, ON, WM_RamMode);

    //Starting from the specified location, continuous write multiple bytes of data to ram.
    //Prototype: void WriteSeriesDispRAM(*buf, length, x, y)
    LCD.WriteSeriesDispRAM(buf, 3, 20, 2);
    delay(3000);            //Delay for 3s.

    //Booting logo ON, backlight ON, character work mode.
    LCD.WorkingModeConf(ON, ON, WM_CharMode);
    LCD.DispStringAt("Director.", 50, 30);  //Display string.
    
    while(1); //Wait for ever.
}

