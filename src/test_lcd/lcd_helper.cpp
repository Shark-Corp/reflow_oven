#include "lcd_helper.hpp"

TFT_eSPI lcd_handle = TFT_eSPI();

void lcd_init(void) {
    lcd_handle.init();

    lcd_handle.fillScreen(TFT_BLACK);
    lcd_handle.setTextColor(TFT_WHITE, TFT_BLACK, false);
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