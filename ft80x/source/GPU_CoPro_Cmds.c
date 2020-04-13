/* Sample Co-Processor API for the FTDI FT80x EVE */

#include "GPU_API.h"
#include "X11_RGB.h"
#include "CoPro_Cmds.h"

/******** Private Co-Processor Command Functions **********/

ft_void_t GPU_CoCmd_SendCmd(GPU_HAL_Context_t *host, ft_const_uint32_t cmd);
ft_void_t GPU_CoCmd_SendStr(GPU_HAL_Context_t *host, ft_const_char8_t *s);
ft_void_t GPU_CoCmd_StartFunc(GPU_HAL_Context_t *host, ft_const_uint16_t count);
ft_void_t GPU_CoCmd_EndFunc(GPU_HAL_Context_t *host, ft_const_uint16_t count);

/******** Public Co-Processor Command Functions ***********/

ft_void_t GPU_CoCmd_Text(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1);
  GPU_CoCmd_SendCmd(host, CMD_TEXT);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|(x&0xffff)));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)options<<16)|(font&0xffff)));
  GPU_CoCmd_SendStr(host, s);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1));
}

ft_void_t GPU_CoCmd_Number(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t font, ft_uint16_t options, ft_int32_t n)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4);
  GPU_CoCmd_SendCmd(host, CMD_NUMBER);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)options<<16)|font));
  GPU_CoCmd_SendCmd(host, n);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4));
}

ft_void_t GPU_CoCmd_LoadIdentity(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_LOADIDENTITY);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*1));
}

ft_void_t GPU_CoCmd_Toggle(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t font, ft_uint16_t options, ft_uint16_t state, const ft_char8_t* s)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1);
  GPU_CoCmd_SendCmd(host, CMD_TOGGLE);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)font<<16)|w));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)state<<16)|options));
  GPU_CoCmd_SendStr(host, s);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1));
}

/* Error handling for val is not done, so better to always use range of 65535 in order that needle is drawn within display region */
ft_void_t GPU_CoCmd_Gauge(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t major, ft_uint16_t minor, ft_uint16_t val, ft_uint16_t range)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*5);
  GPU_CoCmd_SendCmd(host, CMD_GAUGE);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)options<<16)|r));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)minor<<16)|major));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)range<<16)|val));
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*5));
}

ft_void_t GPU_CoCmd_RegRead(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t result) // fixme there's no result returned...?
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_REGREAD);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, 0);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_GetProps(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t w, ft_uint32_t h) // fixme not sure where the properties are returned..?
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4);
  GPU_CoCmd_SendCmd(host, CMD_GETPROPS);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, w);
  GPU_CoCmd_SendCmd(host, h);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4));
}

ft_void_t GPU_CoCmd_Memcpy(GPU_HAL_Context_t *host, ft_uint32_t dest, ft_uint32_t src, ft_uint32_t num)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4);
  GPU_CoCmd_SendCmd(host, CMD_MEMCPY);
  GPU_CoCmd_SendCmd(host, dest);
  GPU_CoCmd_SendCmd(host, src);
  GPU_CoCmd_SendCmd(host, num);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4));
}

ft_void_t GPU_CoCmd_Spinner(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_uint16_t style, ft_uint16_t scale)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_SPINNER);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)scale<<16)|style));
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_BgColor(GPU_HAL_Context_t *host, ft_uint32_t c)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_BGCOLOR);
  GPU_CoCmd_SendCmd(host, c);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_Swap(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_SWAP);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*1));
}

ft_void_t GPU_CoCmd_Inflate(GPU_HAL_Context_t *host, ft_uint32_t ptr)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_INFLATE);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_Translate(GPU_HAL_Context_t *host, ft_int32_t tx, ft_int32_t ty)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_TRANSLATE);
  GPU_CoCmd_SendCmd(host, tx);
  GPU_CoCmd_SendCmd(host, ty);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_Stop(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_STOP);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*1));
}

ft_void_t GPU_CoCmd_Slider(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t range)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*5);
  GPU_CoCmd_SendCmd(host, CMD_SLIDER);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)h<<16)|w));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)val<<16)|options));
  GPU_CoCmd_SendCmd(host, range);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*5));
}

ft_void_t GPU_CoCmd_TouchTransform(GPU_HAL_Context_t *host, ft_int32_t x0, ft_int32_t y0, ft_int32_t x1, ft_int32_t y1, ft_int32_t x2, ft_int32_t y2, ft_int32_t tx0, ft_int32_t ty0, ft_int32_t tx1, ft_int32_t ty1, ft_int32_t tx2, ft_int32_t ty2, ft_uint16_t result)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*6*2+CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_TOUCH_TRANSFORM);
  GPU_CoCmd_SendCmd(host, x0);
  GPU_CoCmd_SendCmd(host, y0);
  GPU_CoCmd_SendCmd(host, x1);
  GPU_CoCmd_SendCmd(host, y1);
  GPU_CoCmd_SendCmd(host, x2);
  GPU_CoCmd_SendCmd(host, y2);
  GPU_CoCmd_SendCmd(host, tx0);
  GPU_CoCmd_SendCmd(host, ty0);
  GPU_CoCmd_SendCmd(host, tx1);
  GPU_CoCmd_SendCmd(host, ty1);
  GPU_CoCmd_SendCmd(host, tx2);
  GPU_CoCmd_SendCmd(host, ty2);
  GPU_CoCmd_SendCmd(host, result);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*6*2+CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_Interrupt(GPU_HAL_Context_t *host, ft_uint32_t ms)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_INTERRUPT);
  GPU_CoCmd_SendCmd(host, ms);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_FgColor(GPU_HAL_Context_t *host, ft_uint32_t c)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_FGCOLOR);
  GPU_CoCmd_SendCmd(host, c);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_Rotate(GPU_HAL_Context_t *host, ft_int32_t a)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_ROTATE);
  GPU_CoCmd_SendCmd(host, a);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_Button(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1);
  GPU_CoCmd_SendCmd(host, CMD_BUTTON);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)h<<16)|w));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)options<<16)|font));
  GPU_CoCmd_SendStr(host, s);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1));
}

ft_void_t GPU_CoCmd_MemWrite(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_MEMWRITE);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, num);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_Scrollbar(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t size, ft_uint16_t range)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*5);
  GPU_CoCmd_SendCmd(host, CMD_SCROLLBAR);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)h<<16)|w));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)val<<16)|options));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)range<<16)|size));
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*5));
}

ft_void_t GPU_CoCmd_GetMatrix(GPU_HAL_Context_t *host, ft_int32_t a, ft_int32_t b, ft_int32_t c, ft_int32_t d, ft_int32_t e, ft_int32_t f)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*7);
  GPU_CoCmd_SendCmd(host, CMD_GETMATRIX);
  GPU_CoCmd_SendCmd(host, a);
  GPU_CoCmd_SendCmd(host, b);
  GPU_CoCmd_SendCmd(host, c);
  GPU_CoCmd_SendCmd(host, d);
  GPU_CoCmd_SendCmd(host, e);
  GPU_CoCmd_SendCmd(host, f);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*7));
}

ft_void_t GPU_CoCmd_Sketch(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_uint16_t w, ft_uint16_t h, ft_uint32_t ptr, ft_uint16_t format)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*5);
  GPU_CoCmd_SendCmd(host, CMD_SKETCH);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)h<<16)|w));
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, format);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*5));
}

ft_void_t GPU_CoCmd_MemSet(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t value, ft_uint32_t num)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4);
  GPU_CoCmd_SendCmd(host, CMD_MEMSET);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, value);
  GPU_CoCmd_SendCmd(host, num);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4));
}

ft_void_t GPU_CoCmd_GradColor(GPU_HAL_Context_t *host, ft_uint32_t c)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_GRADCOLOR);
  GPU_CoCmd_SendCmd(host, c);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_BitmapTransform(GPU_HAL_Context_t *host, ft_int32_t x0, ft_int32_t y0, ft_int32_t x1, ft_int32_t y1, ft_int32_t x2, ft_int32_t y2, ft_int32_t tx0, ft_int32_t ty0, ft_int32_t tx1, ft_int32_t ty1, ft_int32_t tx2, ft_int32_t ty2, ft_uint16_t result)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*6*2+CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_BITMAP_TRANSFORM);
  GPU_CoCmd_SendCmd(host, x0);
  GPU_CoCmd_SendCmd(host, y0);
  GPU_CoCmd_SendCmd(host, x1);
  GPU_CoCmd_SendCmd(host, y1);
  GPU_CoCmd_SendCmd(host, x2);
  GPU_CoCmd_SendCmd(host, y2);
  GPU_CoCmd_SendCmd(host, tx0);
  GPU_CoCmd_SendCmd(host, ty0);
  GPU_CoCmd_SendCmd(host, tx1);
  GPU_CoCmd_SendCmd(host, ty1);
  GPU_CoCmd_SendCmd(host, tx2);
  GPU_CoCmd_SendCmd(host, ty2);
  GPU_CoCmd_SendCmd(host, result);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*6*2+CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_Calibrate(GPU_HAL_Context_t *host, ft_uint32_t result)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_CALIBRATE);
  GPU_CoCmd_SendCmd(host, result);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
  GPU_HAL_WaitCmdfifo_empty(host);
}

ft_void_t GPU_CoCmd_SetFont(GPU_HAL_Context_t *host, ft_uint32_t font, ft_uint32_t ptr)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_SETFONT);
  GPU_CoCmd_SendCmd(host, font);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_Logo(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_LOGO);
  GPU_CoCmd_EndFunc(host,CMD_SIZE*1);
}

ft_void_t GPU_CoCmd_Append(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_APPEND);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, num);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_MemZero(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_MEMZERO);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, num);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_Scale(GPU_HAL_Context_t *host, ft_int32_t sx, ft_int32_t sy)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_SCALE);
  GPU_CoCmd_SendCmd(host, sx);
  GPU_CoCmd_SendCmd(host, sy);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_Clock(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t h, ft_uint16_t m, ft_uint16_t s, ft_uint16_t ms)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*5);
  GPU_CoCmd_SendCmd(host, CMD_CLOCK);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)options<<16)|r));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)m<<16)|h));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)ms<<16)|s));
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*5));
}

ft_void_t GPU_CoCmd_Gradient(GPU_HAL_Context_t *host, ft_int16_t x0, ft_int16_t y0, ft_uint32_t rgb0, ft_int16_t x1, ft_int16_t y1, ft_uint32_t rgb1)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*5);
  GPU_CoCmd_SendCmd(host, CMD_GRADIENT);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y0<<16)|x0));
  GPU_CoCmd_SendCmd(host, rgb0);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y1<<16)|x1));
  GPU_CoCmd_SendCmd(host, rgb1);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*5));
}

ft_void_t GPU_CoCmd_SetMatrix(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_SETMATRIX);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*1));
}

ft_void_t GPU_CoCmd_Track(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t tag)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4);
  GPU_CoCmd_SendCmd(host, CMD_TRACK);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)h<<16)|w));
  GPU_CoCmd_SendCmd(host, tag);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4));
}

ft_void_t GPU_CoCmd_GetPtr(GPU_HAL_Context_t *host,ft_uint32_t result)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_GETPTR);
  GPU_CoCmd_SendCmd(host, result);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_Progress(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_uint16_t options, ft_uint16_t val, ft_uint16_t range)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*5);
  GPU_CoCmd_SendCmd(host, CMD_PROGRESS);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)h<<16)|w));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)val<<16)|options));
  GPU_CoCmd_SendCmd(host, range);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*5));
}

ft_void_t GPU_CoCmd_ColdStart(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_COLDSTART);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*1));
}

ft_void_t GPU_CoCmd_Keys(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t w, ft_int16_t h, ft_int16_t font, ft_uint16_t options, const ft_char8_t* s)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1);
  GPU_CoCmd_SendCmd(host, CMD_KEYS);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)h<<16)|w));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)options<<16)|font));
  GPU_CoCmd_SendStr(host, s);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4 + (ft_uint16_t)strlen((ft_const_char8_t *)s) + 1));
}

ft_void_t GPU_CoCmd_Dial(GPU_HAL_Context_t *host, ft_int16_t x, ft_int16_t y, ft_int16_t r, ft_uint16_t options, ft_uint16_t val)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4);
  GPU_CoCmd_SendCmd(host, CMD_DIAL);
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)y<<16)|x));
  GPU_CoCmd_SendCmd(host, (((ft_uint32_t)options<<16)|r));
  GPU_CoCmd_SendCmd(host, val);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4));
}

ft_void_t GPU_CoCmd_LoadImage(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t options)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*3);
  GPU_CoCmd_SendCmd(host, CMD_LOADIMAGE);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, options);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*3));
}

ft_void_t GPU_CoCmd_Dlstart(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_DLSTART);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*1));
}

ft_void_t GPU_CoCmd_Snapshot(GPU_HAL_Context_t *host, ft_uint32_t ptr)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*2);
  GPU_CoCmd_SendCmd(host, CMD_SNAPSHOT);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*2));
}

ft_void_t GPU_CoCmd_ScreenSaver(GPU_HAL_Context_t *host)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*1);
  GPU_CoCmd_SendCmd(host, CMD_SCREENSAVER);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*1));
}

ft_void_t GPU_CoCmd_MemCrc(GPU_HAL_Context_t *host, ft_uint32_t ptr, ft_uint32_t num, ft_uint32_t result)
{
  GPU_CoCmd_StartFunc(host,CMD_SIZE*4);
  GPU_CoCmd_SendCmd(host, CMD_MEMCRC);
  GPU_CoCmd_SendCmd(host, ptr);
  GPU_CoCmd_SendCmd(host, num);
  GPU_CoCmd_SendCmd(host, result);
  GPU_CoCmd_EndFunc(host,(CMD_SIZE*4));
}

/******** Private Co-Processor Command Functions **********/

ft_void_t GPU_CoCmd_SendCmd(GPU_HAL_Context_t *host, ft_const_uint32_t cmd)
{
    GPU_HAL_WrCmd32(host, cmd);
}

ft_void_t GPU_CoCmd_SendStr(GPU_HAL_Context_t *host, ft_const_char8_t *s)
{
    GPU_HAL_TransferString(host, s);
}

ft_void_t GPU_CoCmd_StartFunc(GPU_HAL_Context_t *host, ft_const_uint16_t count)
{
  GPU_HAL_CheckCmdBuffer(host, count);
  GPU_HAL_StartCmdTransfer(host, GPU_WRITE );
}

ft_void_t GPU_CoCmd_EndFunc(GPU_HAL_Context_t *host, ft_const_uint16_t count)
{
  GPU_HAL_EndCmdTransfer(host);
  GPU_HAL_Updatecmdfifo(host, count);
}

/* Nothing beyond this */
