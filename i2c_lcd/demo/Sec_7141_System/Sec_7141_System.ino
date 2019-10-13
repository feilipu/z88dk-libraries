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
    
    //Booting logo ON, backlight ON, character work mode.
    LCD.WorkingModeConf(ON, ON, WM_CharMode);

    //Set the backlight brightness to 20, and save the configuration to ram.
    LCD.BacklightConf(LOAD_TO_RAM, 20);
    
    while(1); //Wait for ever.
}

