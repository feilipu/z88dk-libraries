#ifndef _GPU_COPRO_CMDS_H_
#define _GPU_COPRO_CMDS_H_

/*
File:   CoPro_Cmds.h
*/

#include "GPU.h"

#ifdef __cplusplus
extern "C" {
#endif

ft_void_t GPU_CoCmd_Text(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s);
ft_void_t GPU_CoCmd_Number(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t font, ft_uint16_t options, ft_int32_t n);
ft_void_t GPU_CoCmd_LoadIdentity(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Toggle(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t font, ft_uint16_t options, ft_uint16_t state, const ft_char8_t* s);
ft_void_t GPU_CoCmd_Gauge(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t major, ft_uint16_t minor, ft_uint16_t val, ft_uint16_t range);
ft_void_t GPU_CoCmd_RegRead(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t result); // fixme but there's no result returned..?
ft_void_t GPU_CoCmd_GetProps(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t w, ft_uint32_t h); // fixme not sure where the properties are returned..?
ft_void_t GPU_CoCmd_Memcpy(GPU_HAL_Context_t *host, ft_uint32_t dest, ft_uint32_t src, ft_uint32_t num);
ft_void_t GPU_CoCmd_Spinner(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_uint16_t style, ft_uint16_t scale);
ft_void_t GPU_CoCmd_BgColor(GPU_HAL_Context_t *host, ft_uint32_t c);
ft_void_t GPU_CoCmd_Swap(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Inflate(GPU_HAL_Context_t *host, ft_uint32_t ptr);
ft_void_t GPU_CoCmd_Translate(GPU_HAL_Context_t *host, ft_int32_t tx, ft_int32_t ty);
ft_void_t GPU_CoCmd_Stop(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Slider(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t range);
ft_void_t GPU_CoCmd_Interrupt(GPU_HAL_Context_t *host, ft_uint32_t ms);
ft_void_t GPU_CoCmd_FgColor(GPU_HAL_Context_t *host, ft_uint32_t c);
ft_void_t GPU_CoCmd_Rotate(GPU_HAL_Context_t *host, ft_int32_t a);
ft_void_t GPU_CoCmd_Button(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s);
ft_void_t GPU_CoCmd_MemWrite(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num);
ft_void_t GPU_CoCmd_Scrollbar(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t size, ft_uint16_t range);
ft_void_t GPU_CoCmd_GetMatrix(GPU_HAL_Context_t *host, ft_int32_t a, ft_int32_t b, ft_int32_t c, ft_int32_t d, ft_int32_t e, ft_int32_t f);
ft_void_t GPU_CoCmd_Sketch(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_uint16_t w, ft_uint16_t h, ft_uint32_t ptr, ft_uint16_t format);
ft_void_t GPU_CoCmd_MemSet(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t value, ft_uint32_t num);
ft_void_t GPU_CoCmd_Calibrate(GPU_HAL_Context_t *host, ft_uint32_t result);
ft_void_t GPU_CoCmd_SetFont(GPU_HAL_Context_t *host, ft_uint32_t font, ft_uint32_t ptr);
ft_void_t GPU_CoCmd_Bitmap_Transform(GPU_HAL_Context_t *host, ft_int32_t x0, ft_int32_t y0, ft_int32_t x1, ft_int32_t y1, ft_int32_t x2, ft_int32_t y2, ft_int32_t tx0, ft_int32_t ty0, ft_int32_t tx1, ft_int32_t ty1, ft_int32_t tx2, ft_int32_t ty2, ft_uint16_t result);
ft_void_t GPU_CoCmd_GradColor(GPU_HAL_Context_t *host, ft_uint32_t c);
ft_void_t GPU_CoCmd_Append(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num);
ft_void_t GPU_CoCmd_MemZero(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num);
ft_void_t GPU_CoCmd_Scale(GPU_HAL_Context_t *host, ft_int32_t sx, ft_int32_t sy);
ft_void_t GPU_CoCmd_Clock(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t h, ft_uint16_t m, ft_uint16_t s, ft_uint16_t ms);
ft_void_t GPU_CoCmd_Gradient(GPU_HAL_Context_t *host, ft_int16_t x0, ft_int16_t y0, ft_uint32_t rgb0, ft_int16_t x1, ft_int16_t y1, ft_uint32_t rgb1);
ft_void_t GPU_CoCmd_SetMatrix(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Track(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t tag);
ft_void_t GPU_CoCmd_GetPtr(GPU_HAL_Context_t *host, ft_uint32_t result);
ft_void_t GPU_CoCmd_Progress(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t range);
ft_void_t GPU_CoCmd_ColdStart(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Keys(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s);
ft_void_t GPU_CoCmd_Dial(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t val);
ft_void_t GPU_CoCmd_LoadImage(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t options);
ft_void_t GPU_CoCmd_Dlstart(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Snapshot(GPU_HAL_Context_t *host, ft_uint32_t ptr);
ft_void_t GPU_CoCmd_ScreenSaver(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Memcrc(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num, ft_uint32_t result);
ft_void_t GPU_CoCmd_Logo(GPU_HAL_Context_t *host);
ft_void_t GPU_CoCmd_Calibrate(GPU_HAL_Context_t *host, ft_uint32_t result);

#ifdef __cplusplus
}
#endif

#endif  /* !_GPU_COPRO_CMDS_H_ */
