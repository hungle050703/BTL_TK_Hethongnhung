________________________________________
 1. Đã có “driver + API cơ bản” chứ không phải raw HAL
File lcd.h đã bao gồm sẵn:
•	Cấp cấu hình phần cứng (chọn chip, size, rotation…)
•	Enum command của ST7789
•	Các hàm API cấp thấp và trung gian
không chỉ là low-level, mà đã chen cả chức năng “vẽ, text, hình ảnh”.

________________________________________
2. Nhóm hàm chính đang tồn tại

***Nhóm Khởi tạo & cấu hình:
void LCD_init(void);
void LCD_SetRotation(uint8_t m);
void LCD_TearEffect(uint8_t tear);

***Nhóm Drawing cơ bản:
void LCD_DrawPixel(int16_t x, int16_t y, uint16_t color);
void LCD_DrawPixelFB(int16_t x, int16_t y, uint16_t color);
int8_t LCD_Fill(uint16_t xSta, uint16_t ySta, uint16_t xEnd, uint16_t yEnd, uint16_t color);
int8_t LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_InvertColors(uint8_t invert);

***Nhóm Text:
void LCD_PutChar(uint16_t x, uint16_t y, char ch, UG_FONT* font, uint16_t color, uint16_t bgcolor);
void LCD_PutStr(uint16_t x, uint16_t y, char *str, UG_FONT* font, uint16_t color, uint16_t bgcolor);

***Nhóm Image:
void LCD_DrawImage(uint16_t x, uint16_t y, UG_BMP* bmp);

Test:
void LCD_Test(void);
________________________________________


3. Liên kết với uGUI đã có:
#include "ugui.h" và hàm dùng UG_FONT, UG_BMP → chứng tỏ LCD đã được thiết kế để ghép với uGUI.
________________________________________
