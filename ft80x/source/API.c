/* Sample API for the FTDI FT800 EVE */

#include "Platform.h"

/* Global used for HAL management */
extern GPU_HAL_Context_t host;
extern GPU_HAL_Context_t * phost;

/* Index into the Display List Buffer */
static ft_uint32_t DLBuffer_Index;

ft_void_t API_Write_CoCmd(ft_const_uint32_t cmd)
{
    GPU_HAL_WrCmd32( phost, cmd);
}

ft_void_t API_Write_DLCmd(ft_const_uint32_t cmd)
{
    GPU_HAL_Wr32( phost, (RAM_DL + DLBuffer_Index), cmd);
    /* Increment the DL Buffer index */
    DLBuffer_Index += CMD_SIZE;
}

ft_void_t API_Reset_DLBuffer( ft_void_t )
{
    /* Reset the DL Buffer index, start writing at RAM_DL first byte */
    DLBuffer_Index = 0;
}

/* API to check the status of previous DLSWAP and perform DLSWAP of new DL */
/* Check for the status of previous DLSWAP and if still not done wait for few ms and check again */
ft_void_t API_GPU_DLSwap(const ft_uint8_t DL_Swap_Type)
{
    ft_uint8_t Swap_Type = DLSWAP_FRAME;
    ft_uint8_t Swap_Done = DLSWAP_FRAME;

    if(DL_Swap_Type == DLSWAP_LINE)
    {
        Swap_Type = DLSWAP_LINE;
    }

    /* Perform a new DL swap */
    GPU_HAL_Wr8(phost,REG_DLSWAP,Swap_Type);

    /* Wait till the swap is done */
    while(Swap_Done)
    {
        Swap_Done = GPU_HAL_Rd8(phost,REG_DLSWAP);

//      if(DLSWAP_DONE != Swap_Done)
//          vTaskDelay( 1 );    //sleep for 1 system tick.
    }
}

/* API to wait until the command buffer is empty, following CMD_SWAP */
ft_void_t API_WaitCmdfifo_empty(ft_void_t)
{
    GPU_HAL_WaitCmdfifo_empty(phost);
}

/* API to give fade out effect by changing the display PWM from 128 till 0 */
ft_void_t API_fadeout(ft_void_t)
{
    for (ft_uint8_t i = 128; i >= 0; i -= 3)
    {
        GPU_HAL_Wr8(phost,REG_PWM_DUTY,i);
//      vTaskDelay( 1 );//sleep for 1 tick
    }
    /* Finally ensure the PWM is 0% */
    GPU_HAL_Wr8(phost,REG_PWM_DUTY,0x00); // 0
}

/* API to perform display fade in effect by changing the display PWM from 0 till 100 and finally 128 */
ft_void_t API_fadein(ft_void_t)
{
    for (ft_uint8_t i = 0; i <=128 ; i += 3)
    {
        GPU_HAL_Wr8(phost,REG_PWM_DUTY,i);
//      vTaskDelay( 1 );//sleep for 1 tick
    }
    /* Finally ensure the PWM is 100% */
    GPU_HAL_Wr8(phost,REG_PWM_DUTY,0x80); // 128
}

ft_void_t API_Boot_Config(ft_void_t)  // you must do this first TO OPEN the API and initialise the Gameduino2
{
    /* Global used for HAL management */
    GPU_HAL_Open(&host);
    phost = &host;

    /* Access address 0 to wake up the FT800 */
    GPU_HostCommand(phost, GPU_ACTIVE_M);
//  vTaskDelay( 32 / portTICK_PERIOD_MS ); // assuming waking from POWERDOWN or SLEEP. From STANDBY the delay is unnecessary.

    /* Set the clk to internal clock (default anyway) */
    GPU_HostCommand(phost, GPU_INTERNAL_OSC);

    /* Switch PLL output to 48MHz */
    GPU_HostCommand(phost, GPU_PLL_48M);

    /* set the SPI bus to full speed, in case we're using a Goldilocks where SCK/2 is too fast for FT800 startup */
    GPU_HAL_Fast(phost);

    /* Do a core reset for safety */
    GPU_HostCommand(phost, GPU_CORE_RESET);

    {
        ft_uint8_t chipid, i = 0;
        //Read Register ID to check if FT800 is ready.
        chipid = GPU_HAL_Rd8(phost, REG_ID);
        while((chipid != 0x7C) && (++i != 0) )
            chipid = GPU_HAL_Rd8(phost, REG_ID);
    }

    GPU_HAL_Wr8(phost,  REG_GPIO_DIR, 0x80 | GPU_HAL_Rd8(phost, REG_GPIO_DIR));
    GPU_HAL_Wr8(phost,  REG_GPIO,     0x80 | GPU_HAL_Rd8(phost, REG_GPIO));

    GPU_HAL_Wr8(phost,  REG_PCLK,     0);            //after this, the display is turned off

    /* Configuration of LCD display */
    GPU_HAL_Wr8(phost,  REG_PCLK_POL, DispPCLKPol);
    GPU_HAL_Wr8(phost,  REG_SWIZZLE,  DispSwizzle);
    GPU_HAL_Wr16(phost, REG_HSIZE,    DispWidth);
    GPU_HAL_Wr16(phost, REG_VSIZE,    DispHeight);
    GPU_HAL_Wr16(phost, REG_HCYCLE,   DispHCycle);
    GPU_HAL_Wr16(phost, REG_HOFFSET,  DispHOffset);
    GPU_HAL_Wr16(phost, REG_HSYNC0,   DispHSync0);
    GPU_HAL_Wr16(phost, REG_HSYNC1,   DispHSync1);
    GPU_HAL_Wr16(phost, REG_VCYCLE,   DispVCycle);
    GPU_HAL_Wr16(phost, REG_VOFFSET,  DispVOffset);
    GPU_HAL_Wr16(phost, REG_VSYNC0,   DispVSync0);
    GPU_HAL_Wr16(phost, REG_VSYNC1,   DispVSync1);

    GPU_HAL_Wr8(phost,  REG_CSPREAD,  TRUE);
    GPU_HAL_Wr8(phost,  REG_DITHER,   TRUE);
    GPU_HAL_Wr8(phost,  REG_ROTATE,   TRUE);        // xxx rotates the screen by 180 degrees


    GPU_HAL_Wr8(phost,  REG_PCLK,     DispPCLK);    //after this, the display is visible on the LCD

    GPU_HAL_Wr8(phost,  REG_GPIO_DIR, 0x83 | GPU_HAL_Rd8(phost,  REG_GPIO_DIR));
    GPU_HAL_Wr8(phost,  REG_GPIO,     0x83 | GPU_HAL_Rd8(phost,  REG_GPIO));

    /* Touch configuration - configure the resistance value to 1200 - this value is specific to customer requirement and derived by experiment */
    GPU_HAL_Wr16(phost, REG_TOUCH_RZTHRESH, 1200);

    /* Set up Touch and other Interrupts */
    GPU_HAL_Wr8(phost,  REG_INT_MASK, 0);    // Interrupt masks available: INT_CONVCOMPLETE, INT_CMDFLAG, INT_CMDEMPTY, INT_PLAYBACK, INT_SOUND, INT_TAG, INT_TOUCH, INT_SWAP
    GPU_HAL_Wr8(phost,  REG_INT_EN, 0x01);   // Global enable for interrupts attached to INT0 on 328p or 1284p

    /* Reset the DL Buffer index, to start writing at RAM_DL first byte */
    DLBuffer_Index = 0;

    /* start a new display list and clear the screen  */
    GPU_HAL_WrCmd32(phost, CMD_DLSTART);                        // initialise and start a display list
    GPU_HAL_WrCmd32(phost, CLEAR(1,1,1));
    GPU_HAL_WrCmd32(phost, CMD_SWAP);                         // Do a DL swap to render the DL
    GPU_HAL_WaitCmdfifo_empty(phost);
}

ft_void_t API_Touch_Config(ft_void_t)
{
    GPU_HAL_WrCmd32(phost, CMD_DLSTART );
    GPU_HAL_WrCmd32(phost, CLEAR_COLOR_X11(BLACK) );
    GPU_HAL_WrCmd32(phost, CLEAR(1,1,1) );
    GPU_HAL_WrCmd32(phost, COLOR_X11(GREEN) );
    GPU_CoCmd_Text(phost, DispWidth/2,DispHeight/2,27,OPT_CENTERX|OPT_CENTERY, PSTR("Please tap on each dot!"));
    GPU_HAL_WrCmd32(phost, CMD_CALIBRATE );
    GPU_HAL_WrCmd32(phost, DISPLAY() );
    GPU_HAL_WrCmd32(phost, CMD_SWAP );
    GPU_HAL_WaitCmdfifo_empty(phost);
}

/* Nothing beyond this */
