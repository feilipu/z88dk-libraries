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
    
    //Config the contrast to 21(This value is between 0 to 63.), and won't save the config when restart. Default value is 21.
    LCD.ContrastConf(LOAD_TO_RAM, 21);
    delay(2000);            //Delay for 2s.
    
    //10*20 font size��auto new line��black character on white back ground.
    LCD.FontModeConf(Font_10x20, FM_ANL_AAA, BLACK_BAC);
    LCD.DispStringAt("Sparking", 0, 20);    //Display "Sparking" on coordinate of (0, 20).
    delay(2000);            //Delay for 2s.
    
    //Config the contrast to 12(This value is between 0 to 63.), and won't save the config when restart.
    LCD.ContrastConf(LOAD_TO_RAM, 12);
    
    while(1); //Wait for ever. 
}

