#include <Wire.h>
#include <I2C_LCD.h>
I2C_LCD LCD;
uint8_t I2C_LCD_ADDRESS = 0x51; //Device address setting, default: 0x51

void setup(void)
{
    Wire.begin();           //I2C controler init.
    Serial.begin(115200);   //Init serial port, and set the band rate as 115200.

void loop(void)
{
    uint8_t buf = 0;        //Buffer.
    
    LCD.CleanAll(WHITE);    //Erase all.
    delay(1000);            //Delay for 1s.

    //Booting logo ON, backlight ON, RAM work mode.
    //If you want to display characters please switch to WM_CharMode.
    LCD.WorkingModeConf(ON, ON, WM_RamMode);

    //Write one byte ram to the specified location. 
    //Prototype: void WriteByteDispRAM(buf, x, y);
    LCD.WriteByteDispRAM(0xAE, 0, 1);
    delay(3000);            //Delay for 3s.

    //Read one byte ram from the specified location. 
    //Prototype:uint8_t ReadByteDispRAM(x, y)
    buf = LCD.ReadByteDispRAM(0, 1);

    //Print the data that just read from RAM to serial monitor.
    //Print in hex format.
    Serial.print("buf = 0x");
    Serial.println(buf, HEX);
    
    while(1); //Wait for ever.
}

