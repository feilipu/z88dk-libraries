#include <Wire.h>
#include <I2C_LCD.h>
I2C_LCD LCD;
uint8_t I2C_LCD_ADDRESS = 0x51; //Device address configuration, the default value is 0x51.

//For detials of the function useage, please refer to "I2C_LCD User Manual". 
//You can download the "I2C_LCD User Manual" from I2C_LCD WIKI page: http://www.seeedstudio.com/wiki/I2C_LCD

void setup(void)
{
    Wire.begin();           //I2C controller initialization.
}

void loop(void)
{
    LCD.CleanAll(WHITE);    //Clean the screen with black or white.
    delay(1000);            //Delay for 1s.
    
    //Display the boot Logo, open the backlight, chose the bitmap dispaly mode.
    LCD.WorkingModeConf(ON, ON, WM_CharMode);
    
    //Config the backlight brightness to 100(The value is between 0 to 127.), and won't save the config when I2C_LCD restart.
    LCD.BacklightConf(LOAD_TO_RAM, 100);
    delay(2000);            //Delay for 1s.
    
    //Config the backlight brightness to 20(The value is between 0 to 127.), and won't save the config when I2C_LCD restart.
    LCD.BacklightConf(LOAD_TO_RAM, 20);
    
    while(1); //Wait for ever. 
}

