#ifndef _GPU_API_H_
#define _GPU_API_H_

#include "GPU_HAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GPU API structure definitions */
typedef struct GPU_API_Bitmap_header
{
    ft_uint8_t Format;
    ft_int16_t Width;
    ft_int16_t Height;
    ft_int16_t Stride;
    ft_int32_t ArrayOffset;
}GPU_API_Bitmap_header_t;

ft_void_t   GPU_API_Boot_Config(ft_void_t);             // you must do this first.

ft_void_t   GPU_API_Touch_Config(ft_void_t);            // you must do this before using touch.

ft_void_t   GPU_API_Write_CoCmd(ft_const_uint32_t cmd); // write a co-processor command

ft_void_t   GPU_API_Write_DLCmd(ft_const_uint32_t cmd); // write a Download List (DL) command

ft_void_t   GPU_API_Reset_DLBuffer(ft_void_t);          // reset (0) the Download List (DL) command offset

/* API to check the status of previous DLSWAP and perform DLSWAP of new DL */
/* Check for the status of previous DLSWAP and if still not done wait for few ms and check again */
ft_void_t   GPU_API_GPU_DLSwap(ft_const_uint8_t DL_Swap_Type);

/* API to wait until the command buffer is empty, following CMD_SWAP */
ft_void_t   GPU_API_WaitCmdfifo_empty(ft_void_t);

/********** utilities ********************/

ft_void_t   GPU_API_fadeout(ft_void_t);
ft_void_t   GPU_API_fadein(ft_void_t);

#ifdef __cplusplus
}
#endif

#endif /* !_GPU_API_H_ */

/* Nothing beyond this */
