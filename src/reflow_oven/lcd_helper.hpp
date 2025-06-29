#ifndef _LCD_HELPER_H_
#define _LCD_HELPER_H_

#include "util.hpp"
#include <Arduino.h>

#include <SPI.h>

// WARNING: Think about modifying libraries "User_Setup.h"; See "working_User_Setup.h" for an example or copy/paste
#include <TFT_eSPI.h> 

#define LCD_RESET_PIN   4
#define LCD_DC_PIN      16
#define LCD_SPI_CS_PIN  17

#define TXT_H   8 // text height (in px)

#define TFT_DARKBROWN 0x5140


extern TFT_eSPI lcd_handle;


void lcd_init(void);
void lcd_clear(void);

int16_t lcd_get_text_width(String text);
void lcd_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t colour, bool fill = true);
void lcd_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t colour);
void lcd_dashed_h_line(uint32_t x, uint32_t y, uint32_t width, uint8_t dash_size, uint8_t gap_size, uint32_t colour);
void lcd_text(uint32_t x, uint32_t y, String text, int32_t fg_color, uint8_t size = 1, int32_t bg_color = -1);





// int lcd_custom_init(void);

// void lcd_send_command(uint16_t command);
// // void lcd_send_data(uint16_t data);
// void lcd_send_data(uint8_t data);

// void lcd_clear(void);
// void lcd_draw_rect(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint16_t colour);


#endif