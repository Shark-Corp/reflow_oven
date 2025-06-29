#ifndef _TEMP_SENS_H_
#define _TEMP_SENS_H_

#include "util.hpp"
#include <Arduino.h>
#include <SPI.h>

#define MAX6675_CS_PIN  14

#define THERMO_SPI_FREQUENCY  1000000



void thermo_init_pins(void);
void thermo_init_spi(SPIClass *lcd_interface);

void thermo_print_temp(void);
float thermo_read_temp_c(void);



#endif