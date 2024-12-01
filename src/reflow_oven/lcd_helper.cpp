#include "lcd_helper.hpp"

TFT_eSPI lcd_handle = TFT_eSPI();

void lcd_init(void) {
    lcd_handle.init();

    lcd_handle.fillScreen(TFT_BLACK);
    lcd_handle.setTextColor(TFT_WHITE, TFT_BLACK, false);
}

void lcd_clear(void) {
    lcd_handle.fillScreen(TFT_BLACK);
}


int16_t lcd_get_text_width(String text) {
    return lcd_handle.textWidth(text);
}


void lcd_rect(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t colour, bool fill) {
    if(fill) {
        lcd_handle.fillRect(x, y, width, height, colour);
    } else {
        lcd_handle.drawRect(x, y, width, height, colour);
    }
}

void lcd_text(uint32_t x, uint32_t y, String text, int32_t fg_color, uint8_t size, int32_t bg_color) {
    if(bg_color == -1) {
        lcd_handle.setTextColor(fg_color);
    }  else {
        lcd_handle.setTextColor(fg_color, bg_color, true);
    }

    lcd_handle.setTextSize(size);

    lcd_handle.drawString(text, x, y);
}

void lcd_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t colour) {
    lcd_handle.drawLine(x1, y1, x2, y2, colour);
}

void lcd_dashed_h_line(uint32_t x, uint32_t y, uint32_t width, uint8_t dash_size, uint8_t gap_size, uint32_t colour) {
    uint8_t nb_of_dash = width/(dash_size+gap_size);
    if(width % (dash_size+gap_size) > 0) {
        nb_of_dash++;
    }
    
    for(uint8_t i = 0; i < nb_of_dash; i++) {
        lcd_handle.drawLine(x+i*(dash_size+gap_size), y, x+dash_size+i*(dash_size+gap_size), y, colour);
    }
}



// // custom part, blc pour l'instant
// int lcd_custom_init(void) {
//     pinMode(LCD_RESET_PIN, OUTPUT);
//     pinMode(LCD_SPI_CS_PIN, OUTPUT);
//     pinMode(LCD_DC_PIN, OUTPUT);

//     digitalWrite(LCD_RESET_PIN, HIGH);
//     digitalWrite(LCD_SPI_CS_PIN, HIGH);
//     digitalWrite(LCD_DC_PIN, HIGH);

//     lcd_send_command(0x0000); // send nop to have spi known state
//     digitalWrite(LCD_RESET_PIN, LOW); // hardware reset
//     delay(30);
//     digitalWrite(LCD_RESET_PIN, HIGH);
//     delay(150); 

//     lcd_send_command(0x0011); // sleep out
//     delay(255);

//     lcd_send_command(0x0020); // invert mode of

//     lcd_send_command(0x003A); // Colour mode
//     lcd_send_data(0x05); // 16 bit mode
//     delay(10);

//     lcd_send_command(0x0036); // MADCTL
//     lcd_send_data(0xC0); 

//     lcd_send_command(0x0013); // normal mode
//     delay(10);
//     lcd_send_command(0x0029); // enable screen
//     delay(100);

//     lcd_clear();


//     return 0; // TODO: return 1 if error on begin ?
// }


// // BASE FUNCTIONS
// void lcd_send_command(uint16_t command) {
//     digitalWrite(LCD_SPI_CS_PIN, LOW); // select lcd spi
//     digitalWrite(LCD_DC_PIN, LOW); // enable command mode
//     SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0)); // lock spi lib

//     SPI.transfer16(command); // send command

//     SPI.endTransaction(); // unlock spi lib
//     digitalWrite(LCD_DC_PIN, HIGH); // disable command mode
//     digitalWrite(LCD_SPI_CS_PIN, HIGH); // unselect lcd spi

//     return;
// }

// void lcd_send_data(uint8_t data) {
//     digitalWrite(LCD_DC_PIN, HIGH); // ensure command mode is disabled
//     digitalWrite(LCD_SPI_CS_PIN, LOW); // select lcd spi
//     SPI.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0)); // lock spi lib

//     SPI.transfer(data); // send command

//     SPI.endTransaction(); // unlock spi lib
//     digitalWrite(LCD_SPI_CS_PIN, HIGH); // unselect lcd spi

//     return;
// }

// // DRAW FUNCTIONS
// void lcd_clear(void) {
//     lcd_draw_rect(0, 0, 128, 160, 0x0000);
// }

// void lcd_draw_rect(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height, uint16_t colour) {
//     lcd_send_command(0x002a); // set column
//     lcd_send_data(0x00);
//     lcd_send_data(start_x); // c start
//     lcd_send_data(0x00);
//     lcd_send_data(start_x+width-1); // c end

//     lcd_send_command(0x002b); // set row
//     lcd_send_data(0x00);
//     lcd_send_data(start_y); // r start
//     lcd_send_data(0x00);
//     lcd_send_data(start_y+height-1); // r end

//     lcd_send_command(0x002c); // LCD ram write
//     for(uint32_t pc = 0; pc < width*height; pc++) {
//         lcd_send_data( (colour & 0xFF00) >> 8 );
//         lcd_send_data(colour & 0xFF);
//     }
// }