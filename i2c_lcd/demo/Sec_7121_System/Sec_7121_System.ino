#include <Wire.h>
#include <I2C_LCD.h>

I2C_LCD LCD;
extern GUI_Bitmap_t bmCamera;     //Declare bitmap data packet.
uint8_t I2C_LCD_ADDRESS = 0x51;   //Device address setting, default: 0x51

void setup(void)
{
    Wire.begin();           //I2C controler init.
}

void loop(void)
{
    LCD.CleanAll(WHITE);    //Erase all.
    delay(1000);            //Delay for 1s.

    //Set the font, character address update mode, display mode.
    LCD.FontModeConf(Font_8x16_1, FM_ANL_AAA, BLACK_BAC);
    LCD.DispStringAt("Hello!", 50, 10);     //Display string.
    delay(3000);            //Delay for 3s.

    //Booting logo ON, backlight ON, bitmap work mode.
    //If you want to display characters please switch to WM_CharMode.
    LCD.WorkingModeConf(ON, ON, WM_BitmapMode);
    
    //Display bitmap at the specified location.
    //For details about bitmap display, please refer to the 5.3 section of user manual.
    LCD.DrawScreenAreaAt(&bmCamera, 0, 0);
    delay(3000);            //Delay for 3s.

    //Booting logo ON, backlight ON, character work mode.
    LCD.WorkingModeConf(ON, ON, WM_CharMode);
    LCD.DispStringAt("Director.", 50, 30);
    
    while(1); //Wait for ever.
}

