#include "temp_sensor.hpp"

SPIClass *spi_interface;

void thermo_init_pins(void) {
    pinMode(MAX6675_CS_PIN, OUTPUT);
    digitalWrite(MAX6675_CS_PIN, HIGH);
}

void thermo_init_spi(SPIClass *lcd_interface) {
    spi_interface = lcd_interface;
}

void thermo_print_temp(void) {
    Serial.print("Temperature: ");
    Serial.print(thermo_read_temp_c());
    Serial.println(" C"); // shows degree symbol
}

uint16_t spi_temperature = 0;
float celsius_temp = 0.0;

float thermo_read_temp_c(void) {
    spi_interface->beginTransaction(SPISettings(THERMO_SPI_FREQUENCY, MSBFIRST, SPI_MODE0));

    digitalWrite(MAX6675_CS_PIN, LOW);
    spi_temperature = spi_interface->transfer16(0x00);	
    digitalWrite(MAX6675_CS_PIN, HIGH);

    spi_interface->endTransaction();

    celsius_temp = (float) (spi_temperature >> 3) * 0.25;
    return celsius_temp;
}