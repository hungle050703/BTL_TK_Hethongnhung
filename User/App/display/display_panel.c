#include "display_panel.h"
#include <stdio.h>
#include <string.h>
#include "ugui.h"

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

void Display_FirePanelStatus(const DisplayRenderer* r,
                             const char* status_text,
                             UG_COLOR status_color,
                             uint8_t fire,
                             uint8_t trouble,
                             uint8_t supervisor,
                             uint8_t disable)
{
    if (!r || !r->Clear || !status_text) return;

    char buf[32];
    
    /* 1. XÓA TOÀN BỘ MÀN HÌNH BAN ĐẦU (Chỉ chạy khi khởi động hoặc reset để tránh nhấp nháy) */
    /* Nếu màn hình vẫn bị nhấp nháy liên tục, hãy comment dòng UG_FillScreen bên dưới lại */
    // UG_FillScreen(C_BLACK);

    // =========================================================================
    // HÀNG 1: HEADER - CĂN GIỮA MÀN HÌNH (Font 12x16)
    // =========================================================================
    UG_FontSelect(FONT_12X16);
    UG_SetForecolor(C_WHITE);
    UG_SetBackcolor(C_BLACK);

    const char* header_text = "HE THONG BAO CHAY SMART";
    int16_t header_x = (SCREEN_WIDTH - (12 * strlen(header_text))) / 2;
    if (header_x < 0) header_x = 0;
    UG_PutString(header_x, 15, (char*)header_text);

    // =========================================================================
    // HÀNG 2: TRẠNG THÁI HỆ THỐNG - CHỮ LỚN CĂN GIỮA (Font 16x26)
    // =========================================================================
    UG_FontSelect(FONT_16X26);
    UG_SetBackcolor(C_BLACK);
    UG_FillFrame(0, 55, SCREEN_WIDTH - 1, 85, C_BLACK);

    int16_t status_x = (SCREEN_WIDTH - (16 * strlen(status_text))) / 2;
    if (status_x < 0) status_x = 0;
    UG_SetForecolor(status_color);
    UG_PutString(status_x, 55, (char*)status_text);

    // =========================================================================
    // HÀNG 3-5: THÔNG SỐ CẢM BIẾN - THU NHỎ FONT VÀ CỐ ĐỊNH NHÃN (Font 12x16)
    // =========================================================================
    UG_FontSelect(FONT_12X16);
    UG_SetForecolor(C_WHITE);

    /* --- DÒNG 3: NHIỆT ĐỘ (Y = 120) --- */
    UG_PutString(15, 120, (char*)"Nhiet do :");
    UG_FillFrame(135, 120, 235, 140, C_BLACK);
    snprintf(buf, sizeof(buf), "%.1f C", (float)trouble);
    UG_SetForecolor(C_YELLOW);
    UG_PutString(140, 120, buf);

    /* --- DÒNG 4: NỒNG ĐỘ KHÓI (Y = 160) --- */
    UG_SetForecolor(C_WHITE);
    UG_PutString(15, 160, (char*)"Khoi     :");
    UG_FillFrame(135, 160, 235, 180, C_BLACK);
    snprintf(buf, sizeof(buf), "%d %%", supervisor);
    UG_SetForecolor(C_CYAN);
    UG_PutString(140, 160, buf);

    /* --- DÒNG 5: TIA LỬA (Y = 200) --- */
    UG_SetForecolor(C_WHITE);
    UG_PutString(15, 200, (char*)"Tia lua  :");
    UG_FillFrame(135, 200, 235, 220, C_BLACK);
    if (fire) {
        UG_SetForecolor(C_RED);
        UG_PutString(140, 200, (char*)"Yes");
    } else {
        UG_SetForecolor(C_WHITE);
        UG_PutString(140, 200, (char*)"No");
    }
}
