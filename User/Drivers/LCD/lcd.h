#ifndef __LCD_ST7789_H__
#define __LCD_ST7789_H__

#include "ugui.h"
#include "ugui_fonts.h"
#include "main.h"

/* ===============================
 * Hardware Configuration
 * =============================== */

/* Use SPI1 as LCD interface */
#define LCD_HANDLE            hspi1

/* Pin connections — match CubeMX labels */
#define LCD_DC                LCD_DC
#define LCD_RST               LCD_RST
#define LCD_CS                LCD_CS
//#define LCD_BL              LCD_BL  /* Enable if backlight control needed */

/* Enable DMA for SPI transfers (recommended) */
//#define USE_DMA

/* Select LCD Controller */
#define USE_ST7789
//#define USE_ST7735

/* Screen rotation and model */
#define LCD_ROTATION          3
//#define LCD_240X280
#define LCD_240X320

/* ===============================
 * LCD Geometry
 * =============================== */
#if defined USE_ST7789
  #if defined LCD_240X280
    #if (LCD_ROTATION == 0) || (LCD_ROTATION == 2)
      #define LCD_WIDTH   240
      #define LCD_HEIGHT  280
      #define LCD_X_SHIFT 0
      #define LCD_Y_SHIFT 0
    #elif (LCD_ROTATION == 1) || (LCD_ROTATION == 3)
      #define LCD_WIDTH   280
      #define LCD_HEIGHT  240
    #endif
	#define LCD_X_SHIFT 20
	#define LCD_Y_SHIFT 0
  #elif defined LCD_240X320
    #if (LCD_ROTATION == 0) || (LCD_ROTATION == 2)
      #define LCD_WIDTH   240
      #define LCD_HEIGHT  320
    #elif (LCD_ROTATION == 1) || (LCD_ROTATION == 3)
      #define LCD_WIDTH   320
      #define LCD_HEIGHT  240
    #endif
    #define LCD_X_SHIFT 0
    #define LCD_Y_SHIFT 0
  #endif

  #if LCD_ROTATION == 0
    #define LCD_ROTATION_CMD (CMD_MADCTL_MX | CMD_MADCTL_MY | CMD_MADCTL_RGB)
  #elif LCD_ROTATION == 1
    #define LCD_ROTATION_CMD (CMD_MADCTL_MY | CMD_MADCTL_MV | CMD_MADCTL_RGB)
  #elif LCD_ROTATION == 2
    #define LCD_ROTATION_CMD (CMD_MADCTL_RGB)
  #elif LCD_ROTATION == 3
    #define LCD_ROTATION_CMD (CMD_MADCTL_MX | CMD_MADCTL_MV | CMD_MADCTL_RGB)
  #endif
#endif

/* ===============================
 * LCD Commands
 * =============================== */
typedef enum {
  CMD_MADCTL_MY  = 0x80,
  CMD_MADCTL_MX  = 0x40,
  CMD_MADCTL_MV  = 0x20,
  CMD_MADCTL_ML  = 0x10,
  CMD_MADCTL_RGB = 0x00,
  CMD_MADCTL_BGR = 0x08,
  CMD_MADCTL_MH  = 0x04,
  CMD_NOP        = 0x00,
  CMD_SWRESET    = 0x01,
  CMD_RDDID      = 0x04,
  CMD_RDDST      = 0x09,
  CMD_SLPIN      = 0x10,
  CMD_SLPOUT     = 0x11,
  CMD_PTLON      = 0x12,
  CMD_NORON      = 0x13,
  CMD_INVOFF     = 0x20,
  CMD_INVON      = 0x21,
  CMD_GAMSET     = 0x26,
  CMD_DISPOFF    = 0x28,
  CMD_DISPON     = 0x29,
  CMD_CASET      = 0x2A,
  CMD_RASET      = 0x2B,
  CMD_RAMWR      = 0x2C,
  CMD_RAMRD      = 0x2E,
  CMD_MADCTL     = 0x36,
  CMD_COLMOD     = 0x3A,
  CMD_GCTRL      = 0xB7,
  CMD_PORCTRL    = 0xB2,
  CMD_VCOMS      = 0xBB,
  CMD_LCMCTRL    = 0xC0,
  CMD_VDVVRHEN   = 0xC2,
  CMD_VRHS       = 0xC3,
  CMD_VDVS       = 0xC4,
  CMD_VMCTR1     = 0xC5,
  CMD_FRCTRL2    = 0xC6,
  CMD_PWCTRL1    = 0xD0,
  CMD_GMCTRP1    = 0xE0,
  CMD_GMCTRN1    = 0xE1,
  CMD_COLOR_MODE_16bit = 0x55,
  CMD_COLOR_MODE_18bit = 0x66,
} lcd_cmds;

/* ===============================
 * Utility macros
 * =============================== */
#define color565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
#define ABS(x) ((x) > 0 ? (x) : -(x))

#define LCD_CON(a,b)  a##b
#define LCD_PIN(pin, out) \
    ( LCD_CON(pin,_GPIO_Port->BSRR) = (out ? LCD_CON(pin,_Pin) : LCD_CON(pin,_Pin)<<16) )

/* ===============================
 * External handle
 * =============================== */
extern SPI_HandleTypeDef LCD_HANDLE;

/* ===============================
 * API Prototypes
 * =============================== */
void LCD_init(void);
void LCD_SetRotation(uint8_t m);
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color);
void LCD_DrawPixelFB(int16_t x, int16_t y, uint16_t color);
int8_t LCD_Fill(uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color);
int8_t LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawImage(uint16_t x, uint16_t y, UG_BMP* bmp);
void LCD_InvertColors(uint8_t invert);
void LCD_PutChar(uint16_t x, uint16_t y, char ch, UG_FONT* font, uint16_t color, uint16_t bgcolor);
void LCD_PutStr(uint16_t x, uint16_t y,  char *str, UG_FONT* font, uint16_t color, uint16_t bgcolor);
void LCD_TearEffect(uint8_t tear);
/* Demo function removed */

/* Fonts are declared in ugui_fonts.h as arrays, pass them directly (e.g., FONT_16X26) */

#endif /* __LCD_ST7789_H__ */
