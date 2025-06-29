#ifndef _UTIL_H_
#define _UTIL_H_

#include <Arduino.h>
#include <EEPROM.h>

#define DEBUG_LED_PIN 2

#define SPI_CLK_PIN 18
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 23

#define EEPROM_ADDR_TEMP_POINT_1    10 // 2 bytes
#define EEPROM_ADDR_TEMP_POINT_2    12 // 2 bytes
#define EEPROM_ADDR_TEMP_SLOPE_A    20 // 1 byte
#define EEPROM_ADDR_TEMP_SLOPE_B    21 // 1 byte
#define EEPROM_ADDR_TEMP_SLOPE_C    22 // 1 byte


/*==========================================================
=============== Targeted Soldering Curve ===================
============================================================

T1. . . . . .| . . . . . . . . . ______
             |                  /!    !\
             |              S1 / !    ! \
T0 . . . . . |. . . __________/  !    !  \  S2?
             |     /!        !   !    !   \
             | S0 / !        !   !    !    \
tA. . . . . .| . /  !        !   !    !     \
             +--+---+--------+---+----+------+----
                |dA |   D0   |dB | D1 |  dC  |

tA is room temperature (can't be set/changed)
dA, dB and dC are dependant to respective T and S
T0 and T1 are targeted temperatures
S0 and S1 are slopes to reach targeted temperatures
S2 is not necesserally to be used, maybe can just use ambient cooling ?
D0 and D1 are duration of the flat parts
===========================================================*/

// TODO: update Slopes and temperatures structure according to the graph above
// TODO: add structure for delays
// TODO: update whats displayed on screen

// TEMPERATURES
typedef struct temperature_points_t {
    int16_t tp1; // ~150°C
    int16_t tp2; // ~245°C -> melting temp  25°C
} temperature_points_t;

typedef struct temperature_slopes_t {
    int16_t tsA; // 3°C/s
    int16_t tsB; // 2°C / s
    int16_t tsC; // -4°C / s
} temperature_slopes_t;

// TODO: add flat duration (between A & B, and B & C)


void copy_temp_points(temperature_points_t source, temperature_points_t *target);
void copy_temp_slopes(temperature_slopes_t source, temperature_slopes_t *target);

void temp_load_points(temperature_points_t *temp_points);
void temp_load_slopes(temperature_slopes_t *temp_slopes);

void temp_save_points(temperature_points_t temp_points);
void temp_save_slopes(temperature_slopes_t temp_slopes);


// NON-BLOCKING DELAY
typedef struct nb_delay_t {
    uint32_t duration_ms;
    uint32_t last_check;
} nb_delay_t;

void nb_delay_init(nb_delay_t *delay, uint32_t duration_ms);
bool nb_delay_check(nb_delay_t *delay);


#endif