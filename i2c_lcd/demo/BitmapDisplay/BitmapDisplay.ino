#include <Wire.h>
#include <I2C_LCD.h>

I2C_LCD LCD;
extern GUI_Bitmap_t bmTuzki;       //Declare bitmap data package.
uint8_t I2C_LCD_ADDRESS = 0x51;  //Device address configuration, the default value is 0x51.

void setup(void)
{
    Wire.begin();         //I2C controller initialization.
}

void loop(void)
{
    LCD.CleanAll(WHITE);    //Clean the screen with black or white.
    delay(1000);            //Delay for 1s.

    //Display the boot Logo, open the backlight, chose the bitmap dispaly mode.
    //To display the characters, you need to set the mode to WM_CharMode.
    LCD.WorkingModeConf(ON, ON, WM_BitmapMode);

    //Display "bmTuzki" on coordinate of (30, 0).
    //Bitmap data package file generate method please refer to the 5.3 section of the "I2C_LCD User Manual".
    LCD.DrawScreenAreaAt(&bmTuzki, 30, 0);
    delay(5000);            //Delay for 5s.
    
    while(1); //Wait for ever.
}
