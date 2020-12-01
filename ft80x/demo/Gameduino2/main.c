////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////    main.c
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* time interface include file */
#include <time.h>
#include <sys/time.h>

#include <arch/yaz180.h>
#include <arch/yaz180/i2c.h>

/* Scheduler include files. */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

/* Gameduino 2 include file. */
#include "GPU_API.h"
#include "GPU_SampleApp.h"

/*--------------Definitions-------------------*/

#define LINE_SIZE         80            // size of command line (on heap)

/*--- Globals used for GPU HAL management ---*/

extern GPU_HAL_Context_t        theScreen;
extern GPU_HAL_Context_t        *pScreen;

/*---------------Functions-------------------*/
static void TaskWriteLCD(void *pvParameters); // Write to LCD

static void TaskMonitor(void *pvParameters);  // Serial monitor for Gameduino2

static ft_void_t FT_Main_Menu_Clock(); // build the main menu and draw clock.
static ft_void_t FT_Set_Clock();       // set the system time and date.

static void get_line (uint8_t *buff, uint8_t len);

/*--------------Functions---------------------------*/

/* Main program loop */
int main(void) __attribute__((OS_main));

int main(void)
{

    xTaskCreate(
        TaskWriteLCD
        ,  (const portCHAR *)"WriteLCD"
        ,  512        // measured 73 free stack bytes
        ,  (void *)0
        ,  3
        ,  (void *)0 ); // */

    vTaskStartScheduler();
}


/*-----------------------------------------------------------*/
static void TaskWriteLCD(void *pvParameters) // Write to LCD
{
    (void) pvParameters;

    TickType_t xLastWakeTime __attribute__ ((unused));
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    ft_uint8_t readTag;

    GPU_API_Boot_Config();
    GPU_API_Touch_Config();

    while(1)
    {

        FT_Main_Menu_Clock();

        readTag = GPU_HAL_Rd8(pScreen, REG_TOUCH_TAG);

        if(readTag != 0x00 && readTag != 0xFF)
            {
            /* Play sound  */
            GPU_HAL_Wr8(pScreen, REG_VOL_SOUND,0xFF); //set the volume to maximum
            GPU_HAL_Wr16(pScreen, REG_SOUND, (0x48<< 8) | 0x41); // C5 MIDI note on xzylophone
            GPU_HAL_Wr8(pScreen, REG_PLAY, 1); // play the sound
            }

        switch (readTag) {

#ifdef GPU_APP_ENABLE_APIS_SET0
            case 10 : //sample apps
                /* Sample code for GPU primitives */
                GPU_APP_Screen("Set 0   START");
                GPU_APP_Fonts();
                GPU_APP_Text8x8();
                GPU_APP_TextVGA();
                GPU_APP_Points();
                GPU_APP_Lines();
                GPU_APP_Rectangles();
                GPU_APP_Bitmap();
                GPU_APP_LineStrips();
                GPU_APP_EdgeStrips();
                GPU_APP_Scissor();
                GPU_APP_Polygon();
                GPU_APP_Cube(); // */
                GPU_APP_Ball_Stencil();
//                GPU_APP_Bargraph(); // fixme fix the random range to stop overflow
                GPU_APP_FTDIString();
                GPU_APP_StreetMap();
                GPU_APP_AdditiveBlendText();
                GPU_APP_MacroUsage();
                GPU_APP_AdditiveBlendPoints();
                GPU_APP_Screen("Set 0   End!");
                break;
#endif /* #ifdef GPU_API_ENABLE_APIS_SET0 */

#ifdef GPU_APP_ENABLE_APIS_SET1
            case 11 : //sample apps

                GPU_APP_Screen("Set 1   START");
                GPU_APP_CoPro_Widget_Logo();
//                GPU_APP_CoPro_Widget_Calibrate();
                GPU_APP_CoPro_Widget_Clock();
                GPU_APP_CoPro_Widget_Gauge();
                GPU_APP_CoPro_Widget_Gradient();
                GPU_APP_CoPro_Widget_Keys();
                GPU_APP_CoPro_Widget_Keys_Interactive();
                GPU_APP_CoPro_Widget_Number();
                GPU_APP_CoPro_Widget_Text();
                GPU_APP_CoPro_Widget_Button();
                GPU_APP_CoPro_Widget_Progressbar();
                GPU_APP_CoPro_Widget_Scroll();
                GPU_APP_CoPro_Widget_Slider();
                GPU_APP_CoPro_Widget_Dial();
                GPU_APP_CoPro_Widget_Toggle();
                GPU_APP_CoPro_Widget_Spinner();
                GPU_APP_Screen("Set 1   END!");
                break;
#endif /* #ifdef GPU_API_ENABLE_APIS_SET1 */

#ifdef GPU_APP_ENABLE_APIS_SET2
            case 12 : //sample apps
                GPU_APP_Screen("Set 2   START");
                GPU_APP_CoPro_Inflate();
                GPU_APP_CoPro_Loadimage();
                GPU_APP_Screen("Set 2   END!");
                break;
#endif /* #ifdef GPU_API_ENABLE_APIS_SET2 */

#ifdef GPU_APP_ENABLE_APIS_SET3
            case 13 : //sample apps
                GPU_APP_Screen("Set 3   START");
                GPU_APP_CoPro_Setfont();
                GPU_APP_Screen("Set 3   END!");
                break;
#endif /* #ifdef GPU_API_ENABLE_APIS_SET3 */

#ifdef GPU_APP_ENABLE_APIS_SET4
            case 14 : //sample apps
                GPU_APP_Screen("Set 4   START");
                /* Sample code for coprocessor widgets */

                GPU_APP_CoPro_AppendCmds();
//                GPU_APP_CoPro_Snapshot(); // fixme not working properly
                GPU_APP_CoPro_Matrix();
                GPU_APP_CoPro_Screensaver();
//                GPU_APP_CoPro_Widget_Calibrate();
                GPU_APP_Touch();
                GPU_APP_CoPro_Track();
                GPU_APP_CoPro_Sketch();

                GPU_APP_Sound();

//                GPU_APP_PowerMode();

                GPU_APP_Screen("Set 4   END!");
                break;
#endif /* #ifdef GPU_API_ENABLE_APIS_SET4 */

            case 20 : // set the time / date
                FT_Set_Clock();

                break;

            default :
                break;
        }
        vTaskDelayUntil( &xLastWakeTime, 50 / portTICK_PERIOD_MS );
    }
}


/*-----------------------------------------------------------*/
/* static functions */
/*-----------------------------------------------------------*/

ft_void_t FT_Main_Menu_Clock()
{
    ft_int16_t xOffset,yOffset,bWidth,bHeight,bDisty;

    time_t currentTime;
    tm calendar;

    bWidth = 120;                        // Draw buttons 120x28 resolution
    bHeight = 28;
    bDisty = bHeight + (bHeight>>1);    // separate buttons by half height
    xOffset = bWidth>>1;                // half a button width from the edge
    yOffset = (FT_DispHeight/2 - 3*bDisty);

    FT_GPU_CoCmd_Dlstart(pScreen);
    GPU_API_Write_CoCmd(CLEAR_COLOR_X11(BLACK));
    GPU_API_Write_CoCmd(CLEAR(1,1,1));
    GPU_API_Write_CoCmd(COLOR_X11(YELLOW));

    GPU_API_Write_CoCmd(TAG_MASK(FT_TRUE));

    /* 3d effect with gradient colour */
    FT_GPU_CoCmd_FgColor(pScreen, X11(DARKGREEN));
    FT_GPU_CoCmd_GradColor(pScreen, X11(TOMATO));

    GPU_API_Write_CoCmd(TAG(10));
    FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D,"Set 0");

    yOffset += bDisty;
    GPU_API_Write_CoCmd(TAG(11));
    FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D,"Set 1");

    yOffset += bDisty;
    GPU_API_Write_CoCmd(TAG(12));
    FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D,"Set 2");

    yOffset += bDisty;
    GPU_API_Write_CoCmd(TAG(13));
    FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D,"Set 3");

    yOffset += bDisty;
    GPU_API_Write_CoCmd(TAG(14));
    FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D,"Set 4");

    yOffset += bDisty;
    FT_GPU_CoCmd_Text(pScreen, xOffset - 30, yOffset + 10, 28, 0, "Gameduino 2 Demos");

    time(&currentTime);
    localtime_r(&currentTime, &calendar);
    GPU_API_Write_CoCmd(TAG(20));
    FT_GPU_CoCmd_Clock(pScreen, FT_DispWidth - (FT_DispHeight/2),  FT_DispHeight/2, FT_DispHeight/2, OPT_NOBACK, calendar.tm_hour, calendar.tm_min, calendar.tm_sec, 0);

    GPU_API_Write_CoCmd(DISPLAY());
    FT_GPU_CoCmd_Swap(pScreen);

    /* Wait till coprocessor completes the operation */
    GPU_HAL_WaitCmdfifo_empty(pScreen);

}

ft_void_t FT_Set_Clock()
{
    TickType_t xLastWakeTime __attribute__ ((unused));
    /* The xLastWakeTime variable needs to be initialised with the current tick
    count.  Note that this is the only time we access this variable.  From this
    point on xLastWakeTime is managed automatically by the vTaskDelayUntil()
    API function. */
    xLastWakeTime = xTaskGetTickCount();

    ft_int16_t xOffset,yOffset,bWidth,bHeight,bDistx,bDisty;

    ft_int8_t tz;
    time_t currentTime;
    tm calendar;

    ft_char8_t timeString[40];

    ft_uint8_t readTag;

    bWidth = 70;    // Draw buttons 70x28 resolution
    bHeight = 28;
    bDistx = bWidth + (bWidth>>1); // separate buttons by half width
    bDisty = bHeight + (bHeight>>1); // separate buttons by half height

    eeprom_busy_wait();
    tz = eeprom_read_byte( (ft_uint8_t *)&eeSavedTZ ); // get the current Time Zone

    time(&currentTime); // get the current system time stamp
    localtime_r(&currentTime, &calendar); // break it into the calendar structure

    while(1)
    {

        FT_GPU_CoCmd_Dlstart(pScreen);
        GPU_API_Write_CoCmd(CLEAR_COLOR_X11(BLACK));
        GPU_API_Write_CoCmd(CLEAR(1,1,1));

        GPU_API_Write_CoCmd(TAG_MASK(FT_TRUE));        // turn on the TAG_MASK

        /* 3d effect with gradient colour */
        FT_GPU_CoCmd_FgColor(pScreen,0xFF0000);
        FT_GPU_CoCmd_GradColor(pScreen,0xB90007);

        GPU_API_Write_CoCmd(COLOR_X11(YELLOW));        // write in YELLOW

        xOffset = (bWidth>>1); // half a button width from the edge
        yOffset = (FT_DispHeight/2 - 3*bDisty);        // centre 6 buttons

        GPU_API_Write_CoCmd(TAG(10));                // set the first button to have TAG 10
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "-Yr");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(20));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "-Mth");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(30));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "-Day");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(40));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "-Hr");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(50));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "-Min");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(60));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "-Sec");

        GPU_API_Write_CoCmd(COLOR_X11(GREEN));        // write in GREEN

        xOffset += bDistx;
        yOffset = (FT_DispHeight/2 - 3*bDisty);

        GPU_API_Write_CoCmd(TAG(11));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "+Yr");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(21));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "+Mth");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(31));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "+Day");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(41));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "+Hr");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(51));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "+Min");

        yOffset += bDisty;
        GPU_API_Write_CoCmd(TAG(61));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, "+Sec");

        xOffset += bDistx;
        yOffset = (FT_DispHeight/2 - 3*bDisty);

        timeString[0] = '\0';
        strcat(timeString, "TZ ");            // fill the temporary string with the TZ label we're setting on the Button
        GPU_HAL_Dec2ASCII(timeString, tz);

        GPU_API_Write_CoCmd(TAG(70));
        FT_GPU_CoCmd_Button(pScreen,xOffset,yOffset,bWidth,bHeight,28,OPT_3D, timeString ); // can't use PROGMEM here, computed string

        GPU_API_Write_CoCmd(COLOR_X11(RED));            // write in RED
        GPU_API_Write_CoCmd(TAG(80));
        FT_GPU_CoCmd_Clock(pScreen, FT_DispWidth - (FT_DispHeight/2),  FT_DispHeight/2, FT_DispHeight/2 - 20, OPT_NOBACK, calendar.tm_hour, calendar.tm_min, calendar.tm_sec, 0);

        asctime_r(&calendar, timeString);
        FT_GPU_CoCmd_Text(pScreen, FT_DispWidth - (FT_DispHeight/2), FT_DispHeight - 20, 28, OPT_CENTER, timeString);

        GPU_API_Write_CoCmd(COLOR_X11(GREEN));        // write in GREEN

        xOffset = FT_DispWidth - bDistx;
        yOffset = FT_DispHeight/2 - 3*bDisty;

        GPU_API_Write_CoCmd(TAG(81));
        FT_GPU_CoCmd_Button(pScreen, xOffset, yOffset, bWidth, bHeight, 28, OPT_3D, "SET");

        GPU_API_Write_CoCmd(DISPLAY());
        FT_GPU_CoCmd_Swap(pScreen);

        vTaskDelayUntil( &xLastWakeTime, 100 / portTICK_PERIOD_MS ); // slow down touches, or we can't humanly keep up

        /* Wait till coprocessor completes the operation */
        GPU_HAL_WaitCmdfifo_empty(pScreen);

        readTag = GPU_HAL_Rd8(pScreen, REG_TOUCH_TAG);

        if (readTag != 0x00 && readTag != 0xFF) // if we get a touch on one of our TAGS, play a sound
            {
            /* Play sound  */
            GPU_HAL_Wr8(pScreen, REG_VOL_SOUND,0xFF); //set the volume to maximum
            GPU_HAL_Wr16(pScreen, REG_SOUND, (0x48<< 8) | 0x41); // C5 MIDI note on xzylophone
            GPU_HAL_Wr8(pScreen, REG_PLAY, 1); // play the sound
            }

        switch (readTag){

            case 10:
                if(calendar.tm_year == 100) {calendar.tm_year = 335;}
                else calendar.tm_year -= 1;
                break;
            case 20:
                if(calendar.tm_mon == 0) {calendar.tm_mon = 11; calendar.tm_year -= 1;}
                else calendar.tm_mon -= 1;
                break;
            case 30:
                if(calendar.tm_mday <= 1) {calendar.tm_mday = 31; calendar.tm_mon -= 1;}
                else calendar.tm_mday -= 1;
                break;
            case 40:
                if(calendar.tm_hour == 0) {calendar.tm_hour = 23; calendar.tm_mday -= 1;}
                else calendar.tm_hour -= 1;
                break;
            case 50:
                if(calendar.tm_min == 0) {calendar.tm_min = 59; calendar.tm_hour -= 1;}
                else calendar.tm_min -= 1;
                break;
            case 60:
                if(calendar.tm_sec == 0) {calendar.tm_sec = 59; calendar.tm_min -= 1;}
                else calendar.tm_sec -= 1;
                break;

            case 11:
                if(calendar.tm_year >= 335) {calendar.tm_year = 100;}
                else calendar.tm_year += 1;
                break;
            case 21:
                if(calendar.tm_mon >= 11) {calendar.tm_mon = 0; calendar.tm_year += 1;}
                else calendar.tm_mon += 1;
                break;
            case 31:
                if(calendar.tm_mday >= 31) {calendar.tm_mday = 1; calendar.tm_mon += 1;}
                else calendar.tm_mday += 1;
                break;
            case 41:
                if(calendar.tm_hour >= 23) {calendar.tm_hour = 0; calendar.tm_mday += 1;}
                else calendar.tm_hour += 1;
                break;
            case 51:
                if(calendar.tm_min >= 59) {calendar.tm_min = 0; calendar.tm_hour += 1;}
                else calendar.tm_min += 1;
                break;
            case 61:
                if(calendar.tm_sec >= 59) {calendar.tm_sec = 0; calendar.tm_min += 1;}
                else calendar.tm_sec += 1;
                break;

            case 70 :
                if(tz >= 12) {tz = -12;}
                else tz += 1;
                break;

            case 80 :
                return;
                break;

            case 81 :
                set_zone( tz * (int32_t)ONE_HOUR );     // first set the Time Zone, so the time being set represents the local time
                set_system_time( mktime( &calendar ) ); // then set the system time here based on what we've entered into the calendar structure
                return;
                break;

            default:
                break;
        }

        mktime(&calendar); // normalise the broken down time, to make sure we stay inside real dates
    }
}

/*-----------------------------------------------------------*/

