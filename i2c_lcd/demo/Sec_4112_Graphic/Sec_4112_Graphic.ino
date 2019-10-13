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
    LCD.CleanAll(BLACK);    //Erase all.
    delay(1000);            //Delay for 1s.
    
    //Draw a white horizontal line.
    //Prototype: void DrawHLineAt(startX, endX, y, color)
    LCD.DrawHLineAt(0, 127, 20, WHITE);
    delay(2000);            //Delay for 2s.

    //Draw a white vertical line.
    //Prototype: void DrawVLineAt(startY, endY, x, color)
    LCD.DrawVLineAt(0, 63, 20, WHITE);
    delay(2000);            //Delay for 2s. 

    //Draw a white point.
    //Prototype: void DrawDotAt(x, y, color)
    LCD.DrawDotAt(63, 50, WHITE);
    delay(2000);            //Delay for 2s.

    //Draw any white line.
    //Prototype: void DrawLineAt(startX, endX, startY, endY, color)
    LCD.DrawLineAt(0, 127, 0, 63, WHITE);
    
    while(1); //Wait for ever.
}

