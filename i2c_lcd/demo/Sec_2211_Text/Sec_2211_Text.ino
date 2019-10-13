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

    //8*16 font size, auto new line, black character on white back ground.
    LCD.FontModeConf(Font_6x8, FM_ANL_AAA, BLACK_BAC);

    LCD.DispCharAt('A', 0, 0);    //Display character.

    LCD.DispStringAt("Sparking...", 0, 10);    //Display string.
    
    //Set the start coordinate.
    LCD.CharGotoXY(0,20);
    //Print string on I2C_LCD at the start coordinate.
    LCD.print("Hello, World!");

    while(1)
    {
        //Set the start coordinate.
        LCD.CharGotoXY(0,30);
        //Print the number of seconds since reset:
        LCD.print(millis()/1000,DEC);
        delay(1000); //Delay for 1s.
    }
}
