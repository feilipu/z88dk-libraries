/*!
 * \file GPU_HAL.h
 *
 * \author FTDI
 * \date 2013.04.24
 *
 * Copyright 2013 Future Technology Devices International Limited
 *
 * Project: FT800 or EVE compatible silicon
 * File Description:
 *    This file defines the generic APIs of host access layer for the FT800 or EVE compatible silicon.
 *    Application shall access FT800 or EVE resources over these APIs,regardless of I2C or SPI protocol.
 *    I2C and SPI is selected by compiler switch "I2C_MODE"  and "SPI_MODE". In addition, there are
 *    some helper functions defined for FT800 co-processor engine as well as host commands.
 */
#ifndef _GPU_HAL_H_
#define _GPU_HAL_H_

#include "GPU.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GPU_I2C_MODE = 0,
    GPU_SPI_MODE,

    GPU_MODE_COUNT,
    GPU_MODE_UNKNOWN = GPU_MODE_COUNT
}GPU_HAL_MODE_E;

typedef enum {
    GPU_HAL_OPENED,
    GPU_HAL_READING,
    GPU_HAL_WRITING,
    GPU_HAL_CLOSED,

    GPU_HAL_STATUS_COUNT,
    GPU_HAL_STATUS_ERROR = GPU_HAL_STATUS_COUNT
}GPU_HAL_STATUS_E;

typedef enum
{
    LCD_PORT2 = I2C2_PORT,
    LCD_PORT1 = I2C1_PORT
}GPU_HAL_PORT_T;

/*APIs for Host Commands*/
typedef enum {
    GPU_INTERNAL_OSC = 0x48,         //default
    GPU_EXTERNAL_OSC = 0x44
}GPU_PLL_SOURCE_T;

typedef enum {
    GPU_PLL_48M = 0x62,              //default
    GPU_PLL_36M = 0x61,
    GPU_PLL_24M = 0x64
}GPU_PLL_FREQ_T;

typedef enum {
    GPU_ACTIVE_M =       0x00,
    GPU_STANDBY_M =      0x41,        //default
    GPU_SLEEP_M =        0x42,
    GPU_POWERDOWN_M =    0x50
}GPU_POWER_MODE_T;

#define GPU_CORE_RESET  (0x68)

typedef struct {
    ft_uint8_t reserved;
}GPU_App_Context_t;

typedef enum {
    GPU_READ = 0,
    GPU_WRITE
}GPU_TRANSFERDIR_T;

typedef struct {
    ft_uint16_t length; //IN and OUT
    ft_uint32_t address;
    ft_uint8_t  *buffer;
}GPU_App_Transfer_t;

typedef struct {
    GPU_HAL_PORT_T      port;
    GPU_HAL_STATUS_E    status;
    ft_uint16_t         ft_cmd_fifo_wp;     // co-processor fifo write pointer
    GPU_App_Context_t   app_header;         // prototype for application use
}GPU_HAL_Context_t;

/*******************************************************************************/
/*******************************************************************************/

/*The basic APIs Level 1*/
ft_bool_t   GPU_HAL_Open(GPU_HAL_Context_t *host); // API to initialise the I2C interface and enable the Interrupt
ft_void_t   GPU_HAL_Fast(GPU_HAL_Context_t *host); // Plaid Mode
ft_void_t   GPU_HAL_Close(GPU_HAL_Context_t *host);
//ft_void_t   GPU_HAL_Powercycle(GPU_HAL_Context_t *host,ft_bool_t up);

/*******************************************************************************/
/*******************************************************************************/

ft_void_t   GPU_HostCommand(GPU_HAL_Context_t *host, ft_const_uint8_t cmd);
ft_void_t   GPU_ClockSelect(GPU_HAL_Context_t *host, const GPU_PLL_SOURCE_T pllsource);
ft_void_t   GPU_PLL_FreqSelect(GPU_HAL_Context_t *host, const GPU_PLL_FREQ_T freq);
ft_void_t   GPU_PowerModeSwitch(GPU_HAL_Context_t *host, const GPU_POWER_MODE_T pwrmode);
ft_void_t   GPU_CoreReset(GPU_HAL_Context_t *host);

/*******************************************************************************/
/*******************************************************************************/

// APIs Read
ft_uint8_t  GPU_HAL_Rd8 (GPU_HAL_Context_t *host, ft_const_uint32_t addr);
ft_uint16_t GPU_HAL_Rd16(GPU_HAL_Context_t *host, ft_const_uint32_t addr);
ft_uint32_t GPU_HAL_Rd32(GPU_HAL_Context_t *host, ft_const_uint32_t addr);

// APIs Write
ft_void_t   GPU_HAL_Wr8 (GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint8_t v);
ft_void_t   GPU_HAL_Wr16(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint16_t v);
ft_void_t   GPU_HAL_Wr32(GPU_HAL_Context_t *host, ft_const_uint32_t addr, ft_const_uint32_t v);

ft_void_t   GPU_HAL_RdMem(GPU_HAL_Context_t *host, ft_uint32_t addr, ft_uint8_t *buffer, ft_const_uint16_t length);
ft_void_t   GPU_HAL_WrMem(GPU_HAL_Context_t *host, ft_uint32_t addr, ft_const_uint8_t *buffer, ft_const_uint16_t length);

/*******************************************************************************/
/*******************************************************************************/

/*Preferred public APIs for co-processor Fifo read/write and space management */

ft_void_t   GPU_HAL_CheckCmdBuffer(GPU_HAL_Context_t *host,ft_const_uint16_t count);
ft_void_t   GPU_HAL_Updatecmdfifo(GPU_HAL_Context_t *host, ft_const_uint16_t count);
ft_void_t   GPU_HAL_WaitCmdfifo_empty(GPU_HAL_Context_t *host);
ft_void_t   GPU_HAL_ResetCmdFifo(GPU_HAL_Context_t *host);
ft_void_t   GPU_HAL_WaitLogo_Finish(GPU_HAL_Context_t *host);

ft_void_t   GPU_HAL_WrCmd32(GPU_HAL_Context_t *host, ft_const_uint32_t cmd);
ft_void_t   GPU_HAL_WrCmdBuf(GPU_HAL_Context_t *host, ft_const_uint8_t *buffer, ft_const_uint16_t count);
// ft_uint8_t  GPU_HAL_TransferString(GPU_HAL_Context_t *host, ft_const_char8_t *string);

ft_int32_t  GPU_HAL_Dec2ASCII(ft_char8_t *pSrc,ft_int32_t value);

/*******************************************************************************/
/*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* !_GPU_HAL_H_ */
