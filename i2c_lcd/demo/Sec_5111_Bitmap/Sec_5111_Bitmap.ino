#include <Wire.h>
#include <I2C_LCD.h>

I2C_LCD LCD;
extern GUI_Bitmap_t bmTuzki;       //Declare bitmap data packet.
extern GUI_Bitmap_t bmSPLogo;      //Declare bitmap data packet.
uint8_t I2C_LCD_ADDRESS = 0x51;  //Device address setting, default: 0x51

void setup(void)
{
    Wire.begin();           //I2C controler init.
}

void loop(void)
{
    LCD.CleanAll(WHITE);    //Erase all.
    delay(1000);            //Delay for 1s.

    //Booting logo ON, backlight ON, bitmap work mode.
    //If you want to display characters please switch to WM_CharMode.
    LCD.WorkingModeConf(ON, ON, WM_BitmapMode);
    
    //Display bitmap at the specified location.
    //For details about bitmap display, please refer to the 5.3 section of user manual.
    LCD.DrawScreenAreaAt(&bmTuzki, 30, 0);
    delay(5000);            //Delay for 5s.

    //Display bitmap at the specified location.
    LCD.DrawScreenAreaAt(&bmSPLogo, 0, 0);
    
    while(1); //Wait for ever.
}
