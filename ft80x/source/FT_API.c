/* Sample API for the FTDI FT800 EVE */

#include "FT_Platform.h"

/* Global used for HAL management */
extern FT_GPU_HAL_Context_t host;
extern FT_GPU_HAL_Context_t * phost;

/* Index into the Display List Buffer */
static ft_uint32_t FT_DLBuffer_Index;

ft_void_t FT_API_Write_CoCmd(ft_const_uint32_t cmd)
{
    FT_GPU_HAL_WrCmd32( phost, cmd);
}

ft_void_t FT_API_Write_DLCmd(ft_const_uint32_t cmd)
{
    FT_GPU_HAL_Wr32( phost, (RAM_DL + FT_DLBuffer_Index), cmd);
    /* Increment the DL Buffer index */
    FT_DLBuffer_Index += FT_CMD_SIZE;
}

ft_void_t FT_API_Reset_DLBuffer( ft_void_t )
{
    /* Reset the DL Buffer index, start writing at RAM_DL first byte */
    FT_DLBuffer_Index = 0;
}

/* API to check the status of previous DLSWAP and perform DLSWAP of new DL */
/* Check for the status of previous DLSWAP and if still not done wait for few ms and check again */
ft_void_t FT_API_GPU_DLSwap(const ft_uint8_t DL_Swap_Type)
{
    ft_uint8_t Swap_Type = DLSWAP_FRAME;
    ft_uint8_t Swap_Done = DLSWAP_FRAME;

    if(DL_Swap_Type == DLSWAP_LINE)
    {
        Swap_Type = DLSWAP_LINE;
    }

    /* Perform a new DL swap */
    FT_GPU_HAL_Wr8(phost,REG_DLSWAP,Swap_Type);

    /* Wait till the swap is done */
    while(Swap_Done)
    {
        Swap_Done = FT_GPU_HAL_Rd8(phost,REG_DLSWAP);

//      if(DLSWAP_DONE != Swap_Done)
//          vTaskDelay( 1 );    //sleep for 1 system tick.
    }
}

/* API to wait until the command buffer is empty, following CMD_SWAP */
ft_void_t FT_API_WaitCmdfifo_empty(ft_void_t)
{
    FT_GPU_HAL_WaitCmdfifo_empty(phost);
}

/* API to give fade out effect by changing the display PWM from 128 till 0 */
ft_void_t FT_API_fadeout(ft_void_t)
{
    for (ft_uint8_t i = 128; i >= 0; i -= 3)
    {
        FT_GPU_HAL_Wr8(phost,REG_PWM_DUTY,i);
//      vTaskDelay( 1 );//sleep for 1 tick
    }
    /* Finally ensure the PWM is 0% */
    FT_GPU_HAL_Wr8(phost,REG_PWM_DUTY,0x00); // 0
}

/* API to perform display fade in effect by changing the display PWM from 0 till 100 and finally 128 */
ft_void_t FT_API_fadein(ft_void_t)
{
    for (ft_uint8_t i = 0; i <=128 ; i += 3)
    {
        FT_GPU_HAL_Wr8(phost,REG_PWM_DUTY,i);
//      vTaskDelay( 1 );//sleep for 1 tick
    }
    /* Finally ensure the PWM is 100% */
    FT_GPU_HAL_Wr8(phost,REG_PWM_DUTY,0x80); // 128
}

ft_void_t FT_API_Boot_Config(ft_void_t)  // you must do this first TO OPEN the API and initialise the Gameduino2
{
    /* Global used for HAL management */
    FT_GPU_HAL_Open(&host);
    phost = &host;

    /* Access address 0 to wake up the FT800 */
    FT_GPU_HostCommand(phost, FT_GPU_ACTIVE_M);
//  vTaskDelay( 32 / portTICK_PERIOD_MS ); // assuming waking from POWERDOWN or SLEEP. From STANDBY the delay is unnecessary.

    /* Set the clk to internal clock (default anyway) */
    FT_GPU_HostCommand(phost, FT_GPU_INTERNAL_OSC);

    /* Switch PLL output to 48MHz */
    FT_GPU_HostCommand(phost, FT_GPU_PLL_48M);

    /* set the SPI bus to full speed, in case we're using a Goldilocks where SCK/2 is too fast for FT800 startup */
    FT_GPU_HAL_Fast(phost);

    /* Do a core reset for safety */
    FT_GPU_HostCommand(phost, FT_GPU_CORE_RESET);

    {
        ft_uint8_t chipid, i = 0;
        //Read Register ID to check if FT800 is ready.
        chipid = FT_GPU_HAL_Rd8(phost, REG_ID);
        while((chipid != 0x7C) && (++i != 0) )
            chipid = FT_GPU_HAL_Rd8(phost, REG_ID);
    }

    FT_GPU_HAL_Wr8(phost,  REG_GPIO_DIR, 0x80 | FT_GPU_HAL_Rd8(phost, REG_GPIO_DIR));
    FT_GPU_HAL_Wr8(phost,  REG_GPIO,     0x80 | FT_GPU_HAL_Rd8(phost, REG_GPIO));

    FT_GPU_HAL_Wr8(phost,  REG_PCLK,     0);            //after this, the display is turned off

    /* Configuration of LCD display */
    FT_GPU_HAL_Wr8(phost,  REG_PCLK_POL, FT_DispPCLKPol);
    FT_GPU_HAL_Wr8(phost,  REG_SWIZZLE,  FT_DispSwizzle);
    FT_GPU_HAL_Wr16(phost, REG_HSIZE,    FT_DispWidth);
    FT_GPU_HAL_Wr16(phost, REG_VSIZE,    FT_DispHeight);
    FT_GPU_HAL_Wr16(phost, REG_HCYCLE,   FT_DispHCycle);
    FT_GPU_HAL_Wr16(phost, REG_HOFFSET,  FT_DispHOffset);
    FT_GPU_HAL_Wr16(phost, REG_HSYNC0,   FT_DispHSync0);
    FT_GPU_HAL_Wr16(phost, REG_HSYNC1,   FT_DispHSync1);
    FT_GPU_HAL_Wr16(phost, REG_VCYCLE,   FT_DispVCycle);
    FT_GPU_HAL_Wr16(phost, REG_VOFFSET,  FT_DispVOffset);
    FT_GPU_HAL_Wr16(phost, REG_VSYNC0,   FT_DispVSync0);
    FT_GPU_HAL_Wr16(phost, REG_VSYNC1,   FT_DispVSync1);

    FT_GPU_HAL_Wr8(phost,  REG_CSPREAD,  FT_TRUE);
    FT_GPU_HAL_Wr8(phost,  REG_DITHER,   FT_TRUE);
    FT_GPU_HAL_Wr8(phost,  REG_ROTATE,   FT_TRUE);        // xxx rotates the screen by 180 degrees


    FT_GPU_HAL_Wr8(phost,  REG_PCLK,     FT_DispPCLK);    //after this, the display is visible on the LCD

    FT_GPU_HAL_Wr8(phost,  REG_GPIO_DIR, 0x83 | FT_GPU_HAL_Rd8(phost,  REG_GPIO_DIR));
    FT_GPU_HAL_Wr8(phost,  REG_GPIO,     0x83 | FT_GPU_HAL_Rd8(phost,  REG_GPIO));

    /* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
    FT_GPU_HAL_Wr16(phost, REG_TOUCH_RZTHRESH, 1200);

    /* Set up Touch and other Interrupts */
    FT_GPU_HAL_Wr8(phost,  REG_INT_MASK, 0);    // Interrupt masks available: INT_CONVCOMPLETE, INT_CMDFLAG, INT_CMDEMPTY, INT_PLAYBACK, INT_SOUND, INT_TAG, INT_TOUCH, INT_SWAP
    FT_GPU_HAL_Wr8(phost,  REG_INT_EN, 0x01);   // Global enable for interrupts attached to INT0 on 328p or 1284p

    /* Reset the DL Buffer index, to start writing at RAM_DL first byte */
    FT_DLBuffer_Index = 0;

    /* start a new display list and clear the screen  */
    FT_GPU_HAL_WrCmd32(phost, CMD_DLSTART);                        // initialise and start a display list
    FT_GPU_HAL_WrCmd32(phost, CLEAR(1,1,1));
    FT_GPU_HAL_WrCmd32(phost, CMD_SWAP);                         // Do a DL swap to render the DL
    FT_GPU_HAL_WaitCmdfifo_empty(phost);
}

ft_void_t FT_API_Touch_Config(ft_void_t)
{
    FT_GPU_HAL_WrCmd32(phost, CMD_DLSTART );
    FT_GPU_HAL_WrCmd32(phost, CLEAR_COLOR_X11(BLACK) );
    FT_GPU_HAL_WrCmd32(phost, CLEAR(1,1,1) );
    FT_GPU_HAL_WrCmd32(phost, COLOR_X11(GREEN) );
    FT_GPU_CoCmd_Text_P(phost, FT_DispWidth/2,FT_DispHeight/2,27,OPT_CENTERX|OPT_CENTERY, PSTR("Please tap on each dot!"));
    FT_GPU_HAL_WrCmd32(phost, CMD_CALIBRATE );
    FT_GPU_HAL_WrCmd32(phost, DISPLAY() );
    FT_GPU_HAL_WrCmd32(phost, CMD_SWAP );
    FT_GPU_HAL_WaitCmdfifo_empty(phost);
}

/* Nothing beyond this */
