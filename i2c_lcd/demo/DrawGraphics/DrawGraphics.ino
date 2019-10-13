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

    //Draw a black horizontal line.
    //Prototype: void DrawHLineAt(startX, endX, y, color)
    LCD.DrawHLineAt(0, 127, 20, BLACK);
    delay(2000);            //Delay for 2s.
    
    //Draw a black vertical line.
    //Prototype: void DrawVLineAt(startY, endY, x, color)
    LCD.DrawVLineAt(0, 63, 20, BLACK);
    delay(2000);            //Delay for 2s.

    //Draw a black dot.
    //Prototype: void DrawDotAt(x, y, color)
    LCD.DrawDotAt(63, 50, BLACK);
    delay(2000);            //Delay for 2s.

    //Draw a black line between (0,0) to (127, 63).
    //Prototype: void DrawLineAt(startX, endX, startY, endY, color)
    LCD.DrawLineAt(0, 127, 0, 63, BLACK);
    delay(2000);            //Delay for 2s.

    //Draw a black filled circle.
    //Prototype: void DrawCircleAt(x, y, r, mode)
    LCD.DrawCircleAt(63, 31, 30, BLACK_FILL);
    delay(2000);            //Delay for 2s.

    //Draw a black filled rectangle.
    //Prototype: void DrawRectangleAt(x, y, width, height, mode)
    LCD.DrawRectangleAt(10, 15, 107, 33, BLACK_FILL);


    while(1); //Wait for ever. 
}
