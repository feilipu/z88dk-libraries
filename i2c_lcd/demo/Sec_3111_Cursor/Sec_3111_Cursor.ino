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
    LCD.FontModeConf(Font_8x16_1, FM_ANL_AAA, BLACK_BAC);
    LCD.DispStringAt("12:00:00", 0, 30);    //Display clcok string.
    delay(2000);            //Delay for 2s.

    //To move the cursor point to ones place of second.
    //Calculate the X position of the cursor in accordance with the font width: x=8X7=56.
    //Calculate the starting position of the cursor based on the starting position of the character: y=30.
    //Calculate the width of the cursor in accordance with the font width: width=8.
    //Calculate the height of the cursor in accordance with the font height: height=16
    //API Prototype: void CursorGotoXY(x, y, width, height);
    LCD.CursorGotoXY(56, 30, 8, 16);
    LCD.CursorConf(ON, 6);  //Open the cursor, and set the flicker cycle to 6-level.
    delay(5000);            //Delay for 5s.

    //To move the cursor point to decade of second.
    LCD.CursorGotoXY(48, 30, 8, 16);

    delay(5000);            //Delay for 5s.
    LCD.CursorConf(OFF, 6); //Turn off the cursor.
    
    while(1); //Wait for ever.
}

