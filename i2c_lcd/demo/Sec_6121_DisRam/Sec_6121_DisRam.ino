#include <Wire.h>
#include <I2C_LCD.h>
I2C_LCD LCD;
uint8_t I2C_LCD_ADDRESS = 0x51; //Device address setting, default: 0x51

void setup(void)
{
    Wire.begin();           //I2C controler init.
    Serial.begin(115200);   //Init serial port, and set the band rate as 115200.
}

void loop(void)
{
    uint8_t buf1[3] = {0xFA,0xAF,0xA5}; //Write buffer.
    uint8_t buf2[3] = {0};              //Read buffer.
    uint8_t adder;
    
    LCD.CleanAll(WHITE);    //Erase all.
    delay(1000);            //Delay for 1s.

    //Print the data stored in buf1 array to serial monitor.
    Serial.print("buf1[3] =");          
    for(adder=0; adder<3; adder++)
    {
        //Print in hex format.
        Serial.print(" 0x");
        Serial.print(buf1[adder], HEX); 
    }
    Serial.println("");     //Newline.

    //Booting logo ON, backlight ON, RAM work mode.
    //If you want to display characters please switch to WM_CharMode.
    LCD.WorkingModeConf(ON, ON, WM_RamMode);

    //Starting from the specified location, continuous write multiple bytes of data to ram.
    //Prototype: void WriteSeriesDispRAM(*buf, length, x, y)
    LCD.WriteSeriesDispRAM(buf1, 3, 1, 1);
    delay(3000);            //Delay for 3s.
    
    //Starting from the specified location, continuous read multiple bytes of ram.
    //Prototype: void ReadSeriesDispRAM(*buf, length, x, y)
    LCD.ReadSeriesDispRAM(buf2, 3, 1, 1);
    
    //Print the data stored in buf2 array to serial monitor.
    Serial.print("buf2[3] =");
    for(adder=0; adder<3; adder++)
    {
        //Print in hex format.
        Serial.print(" 0x");
        Serial.print(buf2[adder], HEX); 
    }
    Serial.println("");     //Newline.
    
    while(1); //Wait for ever.
}

