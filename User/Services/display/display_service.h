#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include <stdint.h>
#include "ugui.h"
#include "ugui_fonts.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public drawing helpers (Service layer) */
void Display_Clear(UG_COLOR bg);
void Display_DrawHeader(const char* text);
void Display_DrawLine(const char* label, uint8_t value, UG_COLOR color, uint16_t y);
void Display_DrawFooter(const char* text, uint16_t x, uint16_t y);
void Display_DrawString(const char* text, UG_COLOR color, uint16_t x, uint16_t y, uint8_t font_size);

/* Layout constants to match README.md strictly */
#ifndef DISPLAY_HEADER_X
#define DISPLAY_HEADER_X   10
#endif
#ifndef DISPLAY_HEADER_Y
#define DISPLAY_HEADER_Y   10
#endif
#ifndef DISPLAY_LABEL_X
#define DISPLAY_LABEL_X    10
#endif
#ifndef DISPLAY_VALUE_X
#define DISPLAY_VALUE_X    192   /* moved near right edge for FONT_16X26 (fits up to 3 digits) */
#endif
#ifndef DISPLAY_ARROW_X
#define DISPLAY_ARROW_X    224   /* far right within 240px width for FONT_16X26 */
#endif

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_SERVICE_H */
