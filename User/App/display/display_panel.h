#ifndef DISPLAY_PANEL_H
#define DISPLAY_PANEL_H

#include <stdint.h>
#include "ugui.h" /* for UG_COLOR constants, no direct UG_ calls here */

#ifdef __cplusplus
extern "C" {
#endif

/* Renderer interface to enforce layering: panel does not include service header */
typedef struct {
	void (*Clear)(UG_COLOR bg);
	void (*DrawHeader)(const char* text);
	void (*DrawLine)(const char* label, uint8_t value, UG_COLOR color, uint16_t y);
	void (*DrawFooter)(const char* text, uint16_t x, uint16_t y);
	void (*DrawString)(const char* text, UG_COLOR color, uint16_t x, uint16_t y, uint8_t font_size);
} DisplayRenderer;

/* Render the Fire Alarm Panel status as specified in User/README*.md */
void Display_FirePanelStatus(const DisplayRenderer* r,
                             const char* status_text,
                             UG_COLOR status_color,
                             uint8_t fire,
                             uint8_t trouble,
                             uint8_t supervisor,
                             uint8_t disable);
}
#endif

#endif /* DISPLAY_PANEL_H */
