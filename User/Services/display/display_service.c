#include <stdio.h>
#include "display_service.h"

void Display_Clear(UG_COLOR bg)
{
    UG_FillScreen(bg);
}

void Display_DrawHeader(const char* text)
{
    /* Header: reduced ~1/3 -> use FONT_12X16, white on black at (10,10) */
    UG_FontSelect(FONT_12X16);
    UG_SetForecolor(C_WHITE);
    UG_PutString(DISPLAY_HEADER_X, DISPLAY_HEADER_Y, (char*)text);
}

void Display_DrawLine(const char* label, uint8_t value, UG_COLOR color, uint16_t y)
{
    /* Body line: reduced ~1/3 -> use FONT_16X26; label at (10,y), value near right edge, arrow at far right */
    char buf[5];
    snprintf(buf, sizeof(buf), "%u", (unsigned)value);

    UG_FontSelect(FONT_16X26);
    UG_SetForecolor(color);

    UG_PutString(DISPLAY_LABEL_X, y, (char*)label);
    UG_PutString(DISPLAY_VALUE_X, y, buf);
    UG_PutString(DISPLAY_ARROW_X, y, ">"); /* ASCII fallback; change to UTF-8 '▶' if your font supports it */
}

void Display_DrawFooter(const char* text, uint16_t x, uint16_t y)
{
    /* Footer kept for future use; not called in current panel */
    UG_FontSelect(FONT_8X12);
    UG_SetForecolor(C_CYAN);
    UG_PutString(x, y, (char*)text);
}

void Display_DrawString(const char* text, UG_COLOR color, uint16_t x, uint16_t y, uint8_t font_size)
{
    /* Generic string drawing helper: supports multiple font sizes */
    if (font_size == 1) {
        UG_FontSelect(FONT_8X12);
    } else if (font_size == 2) {
        UG_FontSelect(FONT_12X16);
    } else {
        UG_FontSelect(FONT_16X26);
    }
    
    UG_SetForecolor(color);
    UG_PutString(x, y, (char*)text);
}

