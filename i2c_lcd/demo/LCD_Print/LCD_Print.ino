#include <Wire.h>
#include <I2C_LCD.h>
I2C_LCD LCD;
uint8_t I2C_LCD_ADDRESS = 0x51; //Device address configuration, the default value is 0x51.

//For detials of the function useage, please refer to "I2C_LCD User Manual". 
//You can download the "I2C_LCD User Manual" from I2C_LCD WIKI page: http://www.seeedstudio.com/wiki/I2C_LCD

//The usage of LCD.print() is same to the Serial.print().
//For more informations about print, please visit: http://www.arduino.cc/en/Serial/Print

void setup(void)
{
    Wire.begin();         //I2C controller initialization.
}

void loop(void)
{
    LCD.CleanAll(WHITE);    //Clean the screen with black or white.
    delay(1000);            //Delay for 1s.
    
    //8*16 font size, auto new line, black character on white back ground.
    LCD.FontModeConf(Font_8x16_1, FM_ANL_AAA, BLACK_BAC); 

    //Set the start coordinate.
    LCD.CharGotoXY(0,16);

    //Print string on I2C_LCD.
    LCD.print("Temp: ");
    //Print float on I2C_LCD.
    LCD.print(25.7, 1);
    LCD.println("`C");   
    
    LCD.print("Seconds:");
    while(1)
    {
        //Set the start coordinate.
        LCD.CharGotoXY(68,32);
        //Print the number of seconds since reset:
        LCD.print(millis()/1000,DEC);
        delay(1000);  
    }
}

