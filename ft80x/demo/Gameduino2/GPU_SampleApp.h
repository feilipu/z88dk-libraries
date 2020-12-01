#ifndef _GPU_APP_H_
#define _GPU_APP_H_

/* Definitions used to determine which API are available */
#define GPU_APP_ENABLE_APIS_SET0    /* Sample code for GPU primitives */
//#define GPU_APP_ENABLE_APIS_SET1    /* Sample code for co-processor widgets */
//#define GPU_APP_ENABLE_APIS_SET2    /* Sample code for image handling */
//#define GPU_APP_ENABLE_APIS_SET3    /* Sample code for font handling */
//#define GPU_APP_ENABLE_APIS_SET4    /* Sample code for more co-processor widgets */

#define GPU_APP_Lenaface40_SIZE (2769)
#define GPU_APP_Mandrill256_SIZE (14368)
#define GPU_APP_Roboto_BoldCondensed_12_SIZE (19348)

/* Sample APIs for graphics primitives */
ft_void_t    GPU_APP_Points();
ft_void_t    GPU_APP_Lines();
ft_void_t    GPU_APP_Rectangles();
ft_void_t    GPU_APP_Bitmap();
ft_void_t    GPU_APP_Fonts();
ft_void_t    GPU_APP_Text8x8();
ft_void_t    GPU_APP_TextVGA();
ft_void_t    GPU_APP_Bargraph();
ft_void_t    GPU_APP_LineStrips();
ft_void_t    GPU_APP_EdgeStrips();
ft_void_t    GPU_APP_Scissor();
ft_void_t    GPU_APP_Polygon();
ft_void_t    GPU_APP_Cube();
ft_void_t    GPU_APP_Ball_Stencil();
ft_void_t    GPU_APP_FTDIString();
ft_void_t    GPU_APP_StreetMap();
ft_void_t    GPU_APP_AdditiveBlendText();
ft_void_t    GPU_APP_MacroUsage();
ft_void_t    GPU_APP_AdditiveBlendPoints();

/* Sample APIs for widgets */
ft_void_t    GPU_APP_CoPro_Widget_Logo();
ft_void_t    GPU_APP_CoPro_Widget_Calibrate();
ft_void_t    GPU_APP_CoPro_Widget_Button();
ft_void_t    GPU_APP_CoPro_Widget_Clock();
ft_void_t    GPU_APP_CoPro_Widget_Gauge();
ft_void_t    GPU_APP_CoPro_Widget_Gradient();
ft_void_t    GPU_APP_CoPro_Widget_Keys();
ft_void_t    GPU_APP_CoPro_Widget_Progressbar();
ft_void_t    GPU_APP_CoPro_Widget_Scroll();
ft_void_t    GPU_APP_CoPro_Widget_Slider();
ft_void_t    GPU_APP_CoPro_Widget_Dial();
ft_void_t    GPU_APP_CoPro_Widget_Text();
ft_void_t    GPU_APP_CoPro_Widget_Toggle();
ft_void_t    GPU_APP_CoPro_Widget_Number();
ft_void_t    GPU_APP_CoPro_Widget_Spinner();
ft_void_t    GPU_APP_CoPro_Widget_Keys_Interactive();

ft_void_t    GPU_APP_CoPro_AppendCmds();
ft_void_t    GPU_APP_CoPro_Inflate();
ft_void_t    GPU_APP_CoPro_Loadimage();
ft_void_t    GPU_APP_CoPro_Logo();
ft_void_t    GPU_APP_CoPro_Matrix();
ft_void_t    GPU_APP_CoPro_Screensaver();
ft_void_t    GPU_APP_CoPro_Setfont();
ft_void_t    GPU_APP_CoPro_Sketch();
ft_void_t    GPU_APP_CoPro_Snapshot();
ft_void_t     GPU_APP_CoPro_Track();

/* Sample API for demonstration purposes only */
ft_void_t    FT_Info();
ft_void_t    FT_Home_Setup();
ft_void_t    GPU_APP_Screen_P(ft_const_char8_t *str);
ft_void_t    GPU_APP_Sound();
ft_void_t    GPU_APP_Touch();
ft_void_t    GPU_APP_PowerMode();

#endif /* _GPU_APP_H_ */

/* Nothing beyond this */
