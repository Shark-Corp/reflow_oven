#ifndef _LCD_HELPER_H_
#define _LCD_HELPER_H_

#include "general.hpp"
#include <Arduino.h>

#include <SPI.h>

// WARNING: Think about modifying libraries "User_Setup.h"; See "working_User_Setup.h" for an example or copy/paste
#include <TFT_eSPI.h> 

#define LCD_RESET_PIN   4
#define LCD_SPI_CS_PIN  17
#define LCD_DC_PIN      16


void lcd_init(void);


// int lcd_custom_init(void);

// void lcd_send_command(uint16_t command);
// // void lcd_send_data(uint16_t data);
// void lcd_send_data(uint8_t data);

// void lcd_clear(void);
// void lcd_draw_rect(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint16_t colour);


#endif