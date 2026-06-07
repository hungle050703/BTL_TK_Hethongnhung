#include "display_panel.h"
#include <stdio.h>
#include <string.h>
#include "ugui.h"

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

void Display_FirePanelStatus(const DisplayRenderer* r,
                             uint8_t fire, uint8_t trouble,
                             uint8_t supervisor, uint8_t disable)
{
    if (!r || !r->Clear) return;

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

    char* header_text = "HE THONG BAO CHAY SMART";
    /* Tính toán căn giữa: (240 - (Chiều rộng 1 ký tự * Số ký tự)) / 2 */
    int16_t header_x = (SCREEN_WIDTH - (12 * strlen(header_text))) / 2;
    UG_PutString(header_x, 15, header_text);

    // =========================================================================
    // HÀNG 2: TRẠNG THÁI HỆ THỐNG - CHỮ LỚN CĂN GIỮA (Font 16x26)
    // =========================================================================
    UG_FontSelect(FONT_16X26);
    UG_SetBackcolor(C_BLACK);
    
    UG_COLOR status_color;
    char* status_text;
    
    /* Logic phát hiện lỗi cảm biến / Phân tầng cảnh báo */
    if (trouble < 10 && supervisor > 100) {
        status_text = "MAT CAM BIEN";
        status_color = C_LIGHT_GRAY;
    } else {
        int danger_count = 0;
        if (fire) danger_count++;
        if (trouble > 50) danger_count++;      // Ngưỡng nhiệt độ nguy hiểm
        if (supervisor > 50) danger_count++;   // Ngưỡng khói nguy hiểm
        
        if (danger_count >= 3) {
            status_text = "HOA HOAN!";
            status_color = C_RED;
        } else if (danger_count >= 2) {
            status_text = "DE PHONG!";
            status_color = C_YELLOW;
        } else {
            status_text = "AN TOAN!";
            status_color = C_WHITE;
        }
    }
    
    /* Xóa nền khu vực Trạng thái cũ để đổi chữ không bị đè */
    UG_FillFrame(0, 55, SCREEN_WIDTH - 1, 85, C_BLACK);

    /* Tính toán căn giữa cho hàng trạng thái chữ lớn */
    int16_t status_x = (SCREEN_WIDTH - (16 * strlen(status_text))) / 2;
    UG_SetForecolor(status_color);
    UG_PutString(status_x, 55, status_text);

    // =========================================================================
    // HÀNG 3-5: THÔNG SỐ CẢM BIẾN - THU NHỎ FONT VÀ CỐ ĐỊNH NHÃN (Font 12x16)
    // =========================================================================
    /* Đổi sang FONT_12X16 để giao diện thoáng, tinh tế và không lo tràn khung */
    UG_FontSelect(FONT_12X16);
    UG_SetForecolor(C_WHITE);
    
    /* --- DÒNG 3: NHIỆT ĐỘ (Y = 120) --- */
    UG_PutString(15, 120, (char*)"Nhiet do :");
    
    // Xóa nền vùng giá trị cũ (X từ 140 đến 235) trước khi ghi số mới nhằm chống Text Ghosting
    UG_FillFrame(135, 120, 235, 140, C_BLACK);
    
    // In giá trị nhảy số ép lề phải
    snprintf(buf, sizeof(buf), "%.1f C", (float)trouble);
    UG_SetForecolor(C_YELLOW);
    UG_PutString(140, 120, buf);

    /* --- DÒNG 4: NỒNG ĐỘ KHÓI (Y = 160) --- */
    UG_SetForecolor(C_WHITE);
    UG_PutString(15, 160, (char*)"Khoi     :");
    
    // Xóa nền vùng giá trị khói cũ
    UG_FillFrame(135, 160, 235, 180, C_BLACK);
    
    snprintf(buf, sizeof(buf), "%d %%", supervisor);
    UG_SetForecolor(C_CYAN);
    UG_PutString(140, 160, buf);

    /* --- DÒNG 5: TIA LỬA (Y = 200) --- */
    UG_SetForecolor(C_WHITE);
    UG_PutString(15, 200, (char*)"Tia lua  :");
    
    // Xóa nền vùng trạng thái tia lửa cũ
    UG_FillFrame(135, 200, 235, 220, C_BLACK);
    
    if (fire) {
        UG_SetForecolor(C_RED);
        UG_PutString(140, 200, (char*)"Yes");
    } else {
        UG_SetForecolor(C_WHITE);
        UG_PutString(140, 200, (char*)"No");
    }
}
