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

    //Draw a rectangle, and filled with black; 
    //Prototype: void DrawRectangleAt(x, y, width, height, mode);
    LCD.DrawRectangleAt(10, 15, 107, 33, BLACK_FILL);
    delay(2000);            //Delay for 2s.
    
    //Draw a circle, and filled with black; 
    //Prototype: void DrawCircleAt(x, y, r, mode);
    LCD.DrawCircleAt(63, 31, 30, BLACK_FILL);
    
    while(1); //Wait for ever.
}

